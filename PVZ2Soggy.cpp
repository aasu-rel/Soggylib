#include <cstdint>
#include <cstring>
#include <string>
#include <atomic>
#include <thread>
#include <jni.h>
#include <unistd.h>
#include "logging.h"
#include "memUtils.h"
#include "offsets.h"

static std::atomic<uintptr_t> g_base{0};
static inline uintptr_t getBase() {
    uintptr_t b = g_base.load(std::memory_order_acquire);
    if (!b) { b = getLibraryAddress("libPVZ2.so"); g_base.store(b, std::memory_order_release); }
    return b;
}

static bool g_highView = true, g_loaded = false;

struct GStr { uint64_t flag, size, heap; };
static void makeKey(GStr& s, const char* k) {
    char* b = (char*)operator new(0x20);
    s.flag = 0x21; s.size = strlen(k); s.heap = (uintptr_t)b; strcpy(b, k);
}
static void freeKey(GStr& s) { if (s.flag & 1) operator delete((void*)s.heap); }

typedef long (*SaveBool)(uintptr_t, uintptr_t, char);
typedef long (*ReadBool)(uintptr_t, uintptr_t, uintptr_t);

static bool getHighView() {
    if (!g_loaded) {
        uintptr_t base = getBase();
        uintptr_t app = base ? *(uintptr_t*)(base + OFF_G_DisplayInfo) : 0;
        if (app) {
            GStr k; makeKey(k, "UseHighViewAngle");
            long hit = ((ReadBool)(base + OFF_PersistReadBool))(app, (uintptr_t)&k, app + CONFIG_USE_HIGH_VIEW_ANGLE);
            freeKey(k);
            g_highView = hit ? (*(uint8_t*)(app + CONFIG_USE_HIGH_VIEW_ANGLE) != 0) : true;
            g_loaded = true;
            LOGI("UseHighViewAngle=%d", (int)g_highView);
        }
    }
    return g_highView;
}
static void setHighView(bool v) {
    uintptr_t base = getBase(); if (!base) return;
    uintptr_t app = *(uintptr_t*)(base + OFF_G_DisplayInfo);
    if (!app || getHighView() == v) return;
    *(uint8_t*)(app + CONFIG_USE_HIGH_VIEW_ANGLE) = v ? 1 : 0;
    g_highView = v;
    uintptr_t mgr = *(uintptr_t*)(base + OFF_PersistManager);
    if (mgr) {
        GStr k; makeKey(k, "UseHighViewAngle");
        ((SaveBool)(base + OFF_PersistSave))(mgr, (uintptr_t)&k, v ? 1 : 0);
        freeKey(k);
    }
}

typedef long      (*BoardLayout_t)(uintptr_t);
typedef long      (*BoardZoom_t)(uintptr_t);
typedef uintptr_t (*CreateTab_t)(uintptr_t, uint32_t, uintptr_t, uintptr_t, uintptr_t);
typedef long      (*AddWidget_t)(uintptr_t, uintptr_t, uint8_t, float);
typedef int       (*Dispatch_t)(uintptr_t, uint32_t, char);
typedef uintptr_t (*CreateCB_t)(uintptr_t, uint32_t, uintptr_t, char, int);
typedef void      (*StrCreate_t)(uintptr_t, const wchar_t*, uint32_t);
typedef long      (*AddLabel_t)(uintptr_t, uint32_t, uintptr_t, uintptr_t, uintptr_t, uintptr_t);
typedef long      (*DrawPaths_t)(uintptr_t, uintptr_t);

static BoardLayout_t oBoardLayout; static BoardZoom_t oBoardZoom;
static CreateTab_t   oCreateTab;   static AddWidget_t oAddWidget;
static Dispatch_t    oDispatch;    static CreateCB_t  oCreateCB;
static AddLabel_t    oAddLabel;    static DrawPaths_t oDrawPaths;

static bool      g_injected = false;
static uintptr_t g_hiddenCB = 0;
static constexpr uint32_t kId = 30;

static long hkBoardLayout(uintptr_t b) {
    long r = oBoardLayout(b);
    if (getHighView()) {
        int a = *(int*)(b + BOARD_283), c = *(int*)(b + BOARD_286);
        *(int*)(b + BOARD_270) = -a;
        *(int*)(b + BOARD_284) = -a;
        *(int*)(b + BOARD_285) = (a + c) / 2;
    }
    return r;
}
static long hkBoardZoom(uintptr_t b) {
    long r = oBoardZoom(b);
    if (getHighView()) *(float*)(b + BOARD_280) = 1.0f;
    return r;
}

static bool skip(uint32_t id) {
    switch (id) { case 7: case 8: case 9: case 10: case 12: case 15: case 18: return true; }
    return false;
}
static uintptr_t hkCreateTab(uintptr_t page, uint32_t id, uintptr_t t, uintptr_t iN, uintptr_t iS) {
    if (id == 3) { g_injected = false; g_hiddenCB = 0; }
    if (!g_injected && id == 6 && oCreateCB && oAddWidget) {
        g_injected = true;
        alignas(16) uint8_t tb[24] = {};
        ((StrCreate_t)(getBase() + OFF_SettingsStringCreate))((uintptr_t)tb, L"Full lawn (wide view)", 21);
        uintptr_t cb = oCreateCB(page, kId, (uintptr_t)tb, getHighView() ? 1 : 0, 0);
        if (cb) { uintptr_t c = *(uintptr_t*)(page + SETTINGS_PAGE_CONTAINER); if (c) oAddWidget(c, cb, 0, 0.0f); }
    }

    // uncomment to hide ea's stuff
    // if (skip(id)) return 0;
    return oCreateTab(page, id, t, iN, iS);
}
static uintptr_t hkCreateCB(uintptr_t p, uint32_t id, uintptr_t l, char i, int x) {
    if (id == 48) i = 0;
    uintptr_t cb = oCreateCB(p, id, l, i, x);
    if (id == 48 && cb) g_hiddenCB = cb;
    return cb;
}
static long hkAddWidget(uintptr_t c, uintptr_t w, uint8_t cen, float s) {
    return (!w || w == g_hiddenCB) ? 0 : oAddWidget(c, w, cen, s);
}
static int hkDispatch(uintptr_t p, uint32_t id, char c) {
    if (id == kId) { setHighView(c != 0); *(uint8_t*)(p + SETTINGS_PAGE_DIRTY) = 1; return 0; }
    if (id == 4) return 0;
    return oDispatch ? oDispatch(p, id, c) : 0;
}

static inline const char* strAt(uintptr_t e, uintptr_t o) {
    uint64_t f = *(uint64_t*)(e + o);
    return !f ? "" : (f & 1) ? *(const char**)(e + o + 0x10) : (const char*)(e + o + 1);
}
static inline bool hides(uintptr_t e, uintptr_t o) {
    const char* h = strAt(e, 0xD0); if (!h[0]) return false;
    const char* p = strAt(e, 0x80); if (!p[0] || strcmp(h, p) != 0) return false;
    return strcmp(strAt(o, 0x20), p) == 0;
}
static long hkDrawPaths(uintptr_t m, uintptr_t r) {
    uintptr_t b = *(uintptr_t*)(m + 0x368), e = *(uintptr_t*)(m + 0x370);
    for (uintptr_t g = b; g < e; g += 0x20) {
        uintptr_t s = *(uintptr_t*)(g + 0x18);
        uintptr_t p = *(uintptr_t*)(g + 0x00), pe = *(uintptr_t*)(g + 0x08), w = p;
        for (; p < pe; p += 0x20) {
            uintptr_t d = *(uintptr_t*)(p + 0x10);
            if (hides(s, d) || hides(d, s)) continue;
            if (w != p) memcpy((void*)w, (void*)p, 0x20);
            w += 0x20;
        }
        *(uintptr_t*)(g + 0x08) = w;
    }
    return oDrawPaths(m, r);
}

static std::string g_verLabel;

static std::string extract(const std::string& s, const char* tag) {
    size_t a = s.find(tag); if (a == std::string::npos) return "";
    a += strlen(tag);
    size_t e = s.find_first_of(" |\n\t", a);
    return s.substr(a, e == std::string::npos ? std::string::npos : e - a);
}
static long hkAddLabel(uintptr_t a, uint32_t id, uintptr_t p, uintptr_t r, uintptr_t l, uintptr_t v) {
    if (id == 70 && v) {
        auto* s = (std::string*)v;
        g_verLabel = "a:" + extract(*s, "a:") + " | d:" + extract(*s, "d:") + " | Build date: " __DATE__ " " __TIME__;
        v = (uintptr_t)&g_verLabel;
    }
    return oAddLabel ? oAddLabel(a, id, p, r, l, v) : 0;
}
static void patchBuildDate() {
    uintptr_t base = getBase(); if (!base) return;
    uintptr_t app = *(uintptr_t*)(base + OFF_G_DisplayInfo); if (!app) return;
    *(std::string*)(app + 2312) = __DATE__ " " __TIME__;
}

static void ApplyHooks() {
    uintptr_t base = 0;
    while ((base = getLibraryAddress("libPVZ2.so")) == 0) usleep(100000);
    g_base.store(base, std::memory_order_release);
    LOGI("base = %p", (void*)base);

    getHighView();
    patchBuildDate();

    PVZ2HookFunction(OFF_SettingsAddWidget, (void*)hkAddWidget,   (void**)&oAddWidget);
    PVZ2HookFunction(OFF_SettingsCreate,    (void*)hkCreateTab,   (void**)&oCreateTab);
    PVZ2HookFunction(OFF_SettingsDispatch,  (void*)hkDispatch,    (void**)&oDispatch);
    PVZ2HookFunction(OFF_CheckboxCreate,    (void*)hkCreateCB,    (void**)&oCreateCB);
    PVZ2HookFunction(OFF_BoardZoom2,        (void*)hkBoardZoom,   (void**)&oBoardZoom);
    PVZ2HookFunction(OFF_BoardLayout,       (void*)hkBoardLayout, (void**)&oBoardLayout);
    PVZ2HookFunction(OFF_DrawPaths,         (void*)hkDrawPaths,   (void**)&oDrawPaths);
    PVZ2HookFunction(OFF_AddSettingLabel,   (void*)hkAddLabel,    (void**)&oAddLabel);

    LOGI("Soggylib hookde");
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    std::thread(ApplyHooks).detach();
    return JNI_VERSION_1_6;
}