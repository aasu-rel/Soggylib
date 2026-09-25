#include <cstdint>
#include <cstring>
#include <atomic>
#include <thread>
#include <jni.h>
#include <unistd.h>

#include "logging.h"
#include "memUtils.h"
#include "offsets.h"

static std::atomic<uintptr_t> g_cachedBase{0};

static inline uintptr_t getBase() {
    uintptr_t b = g_cachedBase.load(std::memory_order_acquire);
    if (!b) {
        b = getLibraryAddress("libPVZ2.so");
        g_cachedBase.store(b, std::memory_order_release);
    }
    return b;
}

static std::atomic<bool> g_highView{true};
static std::atomic<bool> g_loaded{false};

struct GStr { uint64_t flag, size, heap; };   // libc++ std::string, 24B SSO

static void makeKey(GStr& s, const char* key) {
    char* buf = (char*)operator new(0x20);
    s.flag = 0x21;
    s.size = strlen(key);
    s.heap = (uintptr_t)buf;
    strcpy(buf, key);
}
static void freeKey(GStr& s) { if (s.flag & 1) operator delete((void*)s.heap); }

typedef long (*SaveBool)(uintptr_t, uintptr_t, char);
typedef long (*ReadBool)(uintptr_t, uintptr_t, uintptr_t);

static bool getHighView() {
    if (!g_loaded.load(std::memory_order_acquire)) {
        uintptr_t base = getBase();
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
    uintptr_t base = getBase();
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
    if (g_highView.load(std::memory_order_relaxed)) {
        int b283 = *(int*)(board + BOARD_283);
        int b286 = *(int*)(board + BOARD_286);
        *(int*)(board + BOARD_270) = -b283;
        *(int*)(board + BOARD_284) = -b283;
        *(int*)(board + BOARD_285) = (b283 + b286) / 2;
    }
    return ret;
}

typedef long (*BoardZoom2_t)(uintptr_t board);
static BoardZoom2_t oBoardZoom2 = nullptr;

static long hkBoardZoom2(uintptr_t board) {
    long ret = oBoardZoom2(board);

    if (g_highView.load(std::memory_order_relaxed)) {
        *(float*)(board + BOARD_280) = 1.0f;
    }
    return ret;
}

typedef uintptr_t (*CreateTab)(uintptr_t, uint32_t, uintptr_t, uintptr_t, uintptr_t);
typedef long      (*AddWidget)(uintptr_t, uintptr_t, uint8_t, float);
typedef int       (*Dispatch)(uintptr_t, uint32_t, char);
typedef uintptr_t (*CreateCB)(uintptr_t, uint32_t, uintptr_t, char, int);
typedef void      (*StrCreate)(uintptr_t, const wchar_t*, uint32_t);

static CreateTab oCreateTab = nullptr;
static AddWidget oAddWidget = nullptr;
static Dispatch  oDispatch  = nullptr;
static CreateCB  oCreateCB  = nullptr;

static bool      g_injected           = false;
static uintptr_t g_hiddenCB           = 0;
static uintptr_t g_lastSettingsPage   = 0;

static constexpr uint32_t kViewAngleId = 30;

static uintptr_t hkCreateTab(uintptr_t page, uint32_t id, uintptr_t title, uintptr_t iconN, uintptr_t iconS)
{
    // New page → reset injection state
    if (page != g_lastSettingsPage) {
        g_lastSettingsPage = page;
        g_hiddenCB         = 0;
        g_injected         = false;
    }

    if (!g_injected && id == 6 && oCreateCB && oAddWidget) {
        g_injected = true;

        uintptr_t base = getBase();
        if (base) {
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
    }
    return oCreateTab(page, id, title, iconN, iconS);
}

static uintptr_t hkCreateCB(uintptr_t page, uint32_t id, uintptr_t label, char init, int p5)
{
    if (id == 48) init = 0; // disable slib option
    uintptr_t cb = oCreateCB(page, id, label, init, p5);
    if (id == 48 && cb) g_hiddenCB = cb;
    return cb;
}

static long hkAddWidget(uintptr_t cont, uintptr_t w, uint8_t centered, float s)
{
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

static inline const char* strAt(uintptr_t evt, uintptr_t off) {
    uint64_t fl = *(uint64_t*)(evt + off);
    if (!fl) return "";
    return (fl & 1) ? *(const char**)(evt + off + 0x10)
                    : (const char*)(evt + off + 1);
}
static inline const char* evtName  (uintptr_t e) { return strAt(e, 0x20); }
static inline const char* evtParent(uintptr_t e) { return strAt(e, 0x80); }
static inline const char* evtHide  (uintptr_t e) { return strAt(e, 0xD0); }

typedef long (*DrawPaths_t)(uintptr_t worldMap, uintptr_t renderCtx);
static DrawPaths_t oDrawPaths = nullptr;

static inline bool endpoint_hides(uintptr_t e, uintptr_t other) {
    const char* hide = evtHide(e);
    if (!hide[0]) return false;
    const char* parent = evtParent(e);
    if (!parent[0] || strcmp(hide, parent) != 0) return false;
    return strcmp(evtName(other), parent) == 0;
}

static long hkDrawPaths(uintptr_t worldMap, uintptr_t renderCtx) {
    uintptr_t gBegin = *(uintptr_t*)(worldMap + 0x368);
    uintptr_t gEnd   = *(uintptr_t*)(worldMap + 0x370);
    for (uintptr_t g = gBegin; g < gEnd; g += 0x20) {
        uintptr_t srcEvt = *(uintptr_t*)(g + 0x18);
        uintptr_t pBegin = *(uintptr_t*)(g + 0x00);
        uintptr_t pEnd   = *(uintptr_t*)(g + 0x08);
        uintptr_t write  = pBegin;
        for (uintptr_t p = pBegin; p < pEnd; p += 0x20) {
            uintptr_t dstEvt = *(uintptr_t*)(p + 0x10);
            if (endpoint_hides(srcEvt, dstEvt) ||
                endpoint_hides(dstEvt, srcEvt)) continue;
            if (write != p) memcpy((void*)write, (void*)p, 0x20);
            write += 0x20;
        }
        *(uintptr_t*)(g + 0x08) = write;
    }
    return oDrawPaths(worldMap, renderCtx);
}

static void ApplyHooks() {
    uintptr_t base = 0;
    while ((base = getLibraryAddress("libPVZ2.so")) == 0) usleep(100000);
    g_cachedBase.store(base, std::memory_order_release);
    getHighView();
    PVZ2HookFunction(OFF_SettingsAddWidget, (void*)hkAddWidget,  (void**)&oAddWidget);
    PVZ2HookFunction(OFF_SettingsCreate, (void*)hkCreateTab,  (void**)&oCreateTab);
    PVZ2HookFunction(OFF_SettingsDispatch, (void*)hkDispatch,   (void**)&oDispatch);
    PVZ2HookFunction(OFF_CheckboxCreate, (void*)hkCreateCB,   (void**)&oCreateCB);
    PVZ2HookFunction(OFF_BoardZoom2, (void*)hkBoardZoom2, (void**)&oBoardZoom2);
    PVZ2HookFunction(OFF_BoardLayout, (void*)hkBoardLayout,(void**)&oBoardLayout);
    PVZ2HookFunction(OFF_DrawPaths, (void*)hkDrawPaths,  (void**)&oDrawPaths);
    LOGI("Soggylib hookde)");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    std::thread(ApplyHooks).detach();
    return JNI_VERSION_1_6;
}
