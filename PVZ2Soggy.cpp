#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <jni.h>
#include <unistd.h>

#include "logging.h"
#include "memUtils.h"
#include "offsets.h"

// get views angle
static std::atomic<bool> g_highView{true};
static std::atomic<bool> g_loaded{false};

struct GStr { uint64_t flag, size, heap; };   // libc++ std::string, 24B SSO

static void makeKey(GStr& s, const char* key) {
    char* buf = (char*)operator new(0x20);
    s.flag = 0x21;                              // cap 32 | heap flag
    s.size = strlen(key);
    s.heap = (uintptr_t)buf;
    strcpy(buf, key);
}
static void freeKey(GStr& s) { if (s.flag & 1) operator delete((void*)s.heap); }

typedef long (*SaveBool)(uintptr_t, uintptr_t, char);
typedef long (*ReadBool)(uintptr_t, uintptr_t, uintptr_t);

static bool getHighView() {
    if (!g_loaded.load(std::memory_order_acquire)) {
        uintptr_t base = getLibraryAddress("libPVZ2.so");
        uintptr_t app = base ? *(uintptr_t*)(base + OFF_G_DisplayInfo) : 0;
        if (app) {
            GStr k; makeKey(k, "UseHighViewAngle");
            long hit = ((ReadBool)(base + OFF_PersistReadBool))(
                           app, (uintptr_t)&k, app + CONFIG_USE_HIGH_VIEW_ANGLE);
            freeKey(k);
            bool v = hit ? (*(uint8_t*)(app + CONFIG_USE_HIGH_VIEW_ANGLE) != 0) : true;
            g_highView.store(v, std::memory_order_release);
            g_loaded.store(true, std::memory_order_release);
        }
    }
    return g_highView.load(std::memory_order_acquire);
}

static void setHighView(bool v) {
    uintptr_t base = getLibraryAddress("libPVZ2.so");
    if (!base) return;
    uintptr_t app = *(uintptr_t*)(base + OFF_G_DisplayInfo);
    if (!app || getHighView() == v) return;

    *(uint8_t*)(app + CONFIG_USE_HIGH_VIEW_ANGLE) = v ? 1 : 0;
    g_highView.store(v, std::memory_order_release);

    uintptr_t mgr = *(uintptr_t*)(base + OFF_PersistManager);
    if (mgr) {
        GStr k; makeKey(k, "UseHighViewAngle");
        ((SaveBool)(base + OFF_PersistSave))(mgr, (uintptr_t)&k, v ? 1 : 0);
        freeKey(k);
    }
}

typedef long (*BoardLayout_t)(uintptr_t board);
static BoardLayout_t oBoardLayout = nullptr;

static long hkBoardLayout(uintptr_t board) {
    long ret = oBoardLayout(board);
    if (getHighView()) {
        int b283 = *(int*)(board + BOARD_283);
        int b286 = *(int*)(board + BOARD_286);
        *(int*)(board + BOARD_270) = -b283;      // fix left edge
        *(int*)(board + BOARD_284) = -b283;
        *(int*)(board + BOARD_285) = (b283 + b286) / 2;
    }
    return ret;
}

typedef long (*BoardZoom2_t)(uintptr_t board); // board zoom
static BoardZoom2_t oBoardZoom2 = nullptr;

static long hkBoardZoom2(uintptr_t board) {
    long ret = oBoardZoom2(board);
    if (getHighView()) *(float*)(board + BOARD_280) = 1.0f;
    return ret;
}

typedef uintptr_t (*CreateTab)(uintptr_t, uint32_t, uintptr_t, uintptr_t, uintptr_t);
typedef long      (*AddWidget)(uintptr_t, uintptr_t, uint8_t, float);
typedef int       (*Dispatch)(uintptr_t, uint32_t, char);
typedef uintptr_t (*CreateCB)(uintptr_t, uint32_t, uintptr_t, char, int);
typedef void      (*StrCreate)(uintptr_t, const wchar_t*, uint32_t);

static CreateTab oCreateTab;
static AddWidget oAddWidget;
static Dispatch  oDispatch;
static CreateCB  oCreateCB;

static bool      g_injected;
static uintptr_t g_hiddenCB;

static constexpr uint32_t kViewAngleId = 30;

static bool isSkipped(uint32_t id) {
    switch (id) {
        case 7: case 8: case 9: case 10:
        case 12: case 15: case 18: // tab ids (i forgot which one is which)
            return true;
        default:
            return false;
    }
}

static uintptr_t hkCreateTab(uintptr_t page, uint32_t id, uintptr_t title,
                             uintptr_t iconN, uintptr_t iconS) {
    if (id == 3) { // change order in settings
        g_injected = false;
        g_hiddenCB = 0;
    }

    if (!g_injected && id == 6 && oCreateCB && oAddWidget) {
        g_injected = true;
        uintptr_t base = getLibraryAddress("libPVZ2.so");
        alignas(16) uint8_t tb[24] = {};
        ((StrCreate)(base + OFF_SettingsStringCreate))(
            (uintptr_t)tb, L"Full lawn (wide view)", 21);

        uintptr_t cb = oCreateCB(page, kViewAngleId, (uintptr_t)tb,
                                 getHighView() ? 1 : 0, 0);
        if (cb) {
            uintptr_t cont = *(uintptr_t*)(page + SETTINGS_PAGE_CONTAINER);
            if (cont) oAddWidget(cont, cb, 0, 0.0f);
        }
    }

    // delete comment if you want to remove EA legals, etc.
    // if (isSkipped(id)) return 0;
    return oCreateTab(page, id, title, iconN, iconS);
}

static uintptr_t hkCreateCB(uintptr_t page, uint32_t id, uintptr_t label, char init, int p5) {
    if (id == 48) init = 0; // disable slib full lawn since it's buggy
    uintptr_t cb = oCreateCB(page, id, label, init, p5);
    if (id == 48 && cb) g_hiddenCB = cb; // hide slib full lawn, same reason
    return cb;
}

static long hkAddWidget(uintptr_t cont, uintptr_t w, uint8_t centered, float s) {
    if (!w || w == g_hiddenCB) return 0;
    return oAddWidget(cont, w, centered, s);
}

static int hkDispatch(uintptr_t page, uint32_t id, char checked) {
    if (id == kViewAngleId) {
        setHighView(checked != 0);
        *(uint8_t*)(page + SETTINGS_PAGE_DIRTY) = 1;
        return 0;
    }
    if (id == 4) return 0;
    return oDispatch ? oDispatch(page, id, checked) : 0;
}

static void ApplyHooks() {
    uintptr_t base = 0;
    while ((base = getLibraryAddress("libPVZ2.so")) == 0) usleep(100000);
    getHighView();

    PVZ2HookFunction(OFF_SettingsAddWidget, (void*)hkAddWidget,  (void**)&oAddWidget);
    PVZ2HookFunction(OFF_SettingsCreate,    (void*)hkCreateTab,  (void**)&oCreateTab);
    PVZ2HookFunction(OFF_SettingsDispatch,  (void*)hkDispatch,   (void**)&oDispatch);
    PVZ2HookFunction(OFF_CheckboxCreate,    (void*)hkCreateCB,   (void**)&oCreateCB);
    PVZ2HookFunction(OFF_BoardLayout, (void*)hkBoardLayout, (void**)&oBoardLayout);
    PVZ2HookFunction(OFF_BoardZoom2,        (void*)hkBoardZoom2, (void**)&oBoardZoom2);
    LOGI("Soggylib hookde");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    std::thread(ApplyHooks).detach();
    return JNI_VERSION_1_6;
}
