//
// Created by setou on 2026/8/25.
// SOUI for OHOS N-API 入口。
//
// 对应 Android 端 Soui4Android.cpp 的 JNI 导出函数。
// 差异：N-API 替代 JNI，napi_module_register 替代 JNI_OnLoad。
//
// 导出函数与 Android Soui4Android.cpp 一一对应：
//   Android: Java_com_soui_android_SouiBaseSurface_nativeRender
//   OHOS:    surf_render (nativeRender)
//   Android: Java_com_soui_android_SouiBaseSurface_nativeOnMotionEvent
//   OHOS:    surf_onMotionEvent (nativeOnMotionEvent)
//   Android: Java_com_soui_android_SouiBaseSurface_nativeOnKeyEvent
//   OHOS:    surf_onKeyEvent (nativeOnKeyEvent)
//

#include "OhosPlatformAPI.h"
#include "SouiSurfaceProxy.h"
#include <helper/slog.h>
#include <cstring>

#define kLogTag "Soui4OhosNAPI"

// 本文件手动弱引用 N-API 符号（不 include node_api.h），NAPI_AUTO_LENGTH 需自行定义
#ifndef NAPI_AUTO_LENGTH
#define NAPI_AUTO_LENGTH ((size_t)-1)  // 与官方 node_api.h 一致：字符串长度自动计算
#endif

extern "C" void RegisterOhosPlatformAPI();
extern "C" void UnregisterOhosPlatformAPI();

// N-API 基础类型与符号（弱引用）
extern "C" {
    typedef struct napi_env__ * napi_env;
    typedef struct napi_value__ * napi_value;
    typedef struct napi_callback_info__ *napi_callback_info;
    typedef napi_value (*napi_callback)(napi_env, napi_callback_info);
    typedef napi_value (*napi_addon_register_func)(napi_env, napi_value);
    typedef enum {
        napi_default = 0,
        napi_writable = 1 << 0,
        napi_enumerable = 1 << 1,
        napi_configurable = 1 << 2,
    } napi_property_attributes;
    // napi_property_descriptor 结构必须与官方 js_native_api_types.h 完全一致
    // （8 字段，字段顺序不能变，运行时按此解释；之前 3 字段布局导致 method/getter 错位崩溃）
    typedef struct {
        const char *utf8name;
        napi_value name;
        napi_callback method;
        napi_callback getter;
        napi_callback setter;
        napi_value value;
        napi_property_attributes attributes;
        void *data;
    } napi_property_descriptor;
    // napi_module 结构需与官方 node_api.h 完全一致（字段顺序/数量不能变，运行时按此解释）
    typedef struct {
        int nm_version;
        unsigned int nm_flags;
        const char* nm_filename;
        napi_addon_register_func nm_register_func;
        const char* nm_modname;
        void* nm_priv;
        void* reserved[4];
    } napi_module;
    typedef enum { napi_ok = 0 } napi_status;
    napi_status __attribute__((weak)) napi_define_properties(napi_env, napi_value, size_t, const napi_property_descriptor*);
    napi_status __attribute__((weak)) napi_get_cb_info(napi_env, napi_callback_info,
        size_t*, napi_value*, void**, void**);
    napi_status __attribute__((weak)) napi_get_value_int32(napi_env, napi_value, int32_t*);
    napi_status __attribute__((weak)) napi_get_value_int64(napi_env, napi_value, int64_t*);
    napi_status __attribute__((weak)) napi_get_value_double(napi_env, napi_value, double*);
    napi_status __attribute__((weak)) napi_get_value_bool(napi_env, napi_value, bool*);
    napi_status __attribute__((weak)) napi_get_value_string_utf8(napi_env, napi_value, char*, size_t, size_t*);
    napi_status __attribute__((weak)) napi_create_int32(napi_env, int32_t, napi_value*);
    napi_status __attribute__((weak)) napi_create_int64(napi_env, int64_t, napi_value*);
    napi_status __attribute__((weak)) napi_create_bool(napi_env, bool, napi_value*);
    napi_status __attribute__((weak)) napi_create_string_utf8(napi_env, const char*, size_t, napi_value*);
    napi_status __attribute__((weak)) napi_create_array(napi_env, napi_value*);
    napi_status __attribute__((weak)) napi_set_element(napi_env, napi_value, uint32_t, napi_value);
    napi_status __attribute__((weak)) napi_get_undefined(napi_env, napi_value*);
    napi_status __attribute__((weak)) napi_get_null(napi_env, napi_value*);
    void __attribute__((weak)) napi_module_register(napi_module*);
}

static inline napi_value NapiOK(napi_env env, int64_t v) {
    napi_value r; napi_create_int64(env, v, &r); return r;
}
static inline napi_value NapiOK(napi_env env, int32_t v) {
    napi_value r; napi_create_int32(env, v, &r); return r;
}
static inline napi_value NapiOK(napi_env env, bool v) {
    return NapiOK(env, v?1:0);
}
static inline napi_value NapiOK(napi_env env, const char* s) {
    napi_value r; napi_create_string_utf8(env, s ? s : "", NAPI_AUTO_LENGTH, &r); return r;
}
static inline napi_value NapiUndef(napi_env env) {
    napi_value r; napi_get_undefined(env, &r); return r;
}

static size_t NapiArgs(napi_env e, napi_callback_info ci, size_t n, napi_value *a) {
    size_t ac = n;
    napi_get_cb_info(e, ci, &ac, a, nullptr, nullptr);
    return ac;
}
static int64_t NapiI64(napi_env e, napi_value v) {
    int64_t r = 0; napi_get_value_int64(e, v, &r); return r;
}
static int32_t NapiI32(napi_env e, napi_value v) {
    int32_t r = 0; napi_get_value_int32(e, v, &r); return r;
}
static bool NapiBool(napi_env e, napi_value v) {
    bool r = false; napi_get_value_bool(e, v, &r); return r;
}
static double NapiF64(napi_env e, napi_value v) {
    double r = 0; napi_get_value_double(e, v, &r); return r;
}
static std::string NapiStr(napi_env e, napi_value v) {
    char buf[512]; size_t n = 0;
    napi_get_value_string_utf8(e, v, buf, sizeof(buf), &n);
    return std::string(buf, n);
}

// ============================================================
// ======== 平台 API 注册 ====================================
// ============================================================ 

static napi_value registerPlatformAPI(napi_env env, napi_callback_info ci) {
    napi_value a[2]; size_t ac = NapiArgs(env, ci, 2, a);
    if (ac != 2) return NapiOK(env, (int32_t)-1);
    OhosPlatformAPI::instance().init(env, a[0], a[1]);
    RegisterOhosPlatformAPI();
    return NapiOK(env, (int32_t)0);
}

static napi_value unregisterPlatformAPI(napi_env env, napi_callback_info ci) {
    (void)ci;
    UnregisterOhosPlatformAPI();
    OhosPlatformAPI::instance().deinit();
    return NapiUndef(env);
}

// ============================================================
// ======== 定时器 ============================================
// ============================================================ 

static napi_value onTimerExpired(napi_env env, napi_callback_info ci) {
    napi_value a[2]; NapiArgs(env, ci, 2, a);
    OhosPlatformAPI::instance().onTimerExpired(
        reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[0]))),
        static_cast<UINT_PTR>(NapiI64(env, a[1])));
    return NapiUndef(env);
}

// ============================================================
// ======== 自绘 Surface：创建/销毁/渲染/事件 =================
//   完全对应 Android SouiBaseSurface native 方法
// ============================================================ 

// nativeCreate(nativeWnd) → nativeId
static napi_value surf_nativeCreate(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    UINT_PTR id = OhosPlatformAPI::instance().nativeViewCreate(env, a[0]);
    return NapiOK(env, (int64_t)id);
}

// nativeDestroy(nativeId)
static napi_value surf_nativeDestroy(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    OhosPlatformAPI::instance().nativeViewDestroy((UINT_PTR)NapiI64(env, a[0]));
    return NapiUndef(env);
}

// nativeRender(nativeId, pixelBuffer, width, height) → [l, t, r, b]
// 对应 Android: Java_com_soui_android_SouiBaseSurface_nativeRender(nativeId, bitmap)
// 返回本次实际重绘的更新区域（物理像素，客户区坐标）；空矩形表示无需上屏。
static napi_value surf_render(napi_env env, napi_callback_info ci) {
    napi_value a[4]; NapiArgs(env, ci, 4, a);
    RECT rc = {0, 0, 0, 0};
    auto sp = OhosPlatformAPI::instance().nativeViewLookup((UINT_PTR)NapiI64(env, a[0]));
    if (sp) sp->render(env, a[1], (int)NapiI32(env, a[2]), (int)NapiI32(env, a[3]), &rc);
    napi_value arr = nullptr;
    napi_create_array(env, &arr);
    if (arr) {
        const LONG *vals = (const LONG *)&rc;
        for (uint32_t i = 0; i < 4; ++i) {
            napi_value v = nullptr;
            napi_create_int32(env, vals[i], &v);
            if (v) napi_set_element(env, arr, i, v);
        }
    }
    return arr ? arr : NapiUndef(env);
}

// nativeOnSizeChanged(nativeId, width, height)
static napi_value surf_onSizeChanged(napi_env env, napi_callback_info ci) {
    napi_value a[3]; NapiArgs(env, ci, 3, a);
    auto sp = OhosPlatformAPI::instance().nativeViewLookup((UINT_PTR)NapiI64(env, a[0]));
    if (sp) sp->onSizeChanged((int)NapiI32(env, a[1]), (int)NapiI32(env, a[2]));
    return NapiUndef(env);
}

// nativeOnMotionEvent(nativeId, action, x, y, pointerId, buttonState, vscroll, hscroll, metaState, timestamp)
// 对应 Android: Java_com_soui_android_SouiBaseSurface_nativeOnMotionEvent
static napi_value surf_onMotionEvent(napi_env env, napi_callback_info ci) {
    napi_value a[10]; NapiArgs(env, ci, 10, a);
    auto sp = OhosPlatformAPI::instance().nativeViewLookup((UINT_PTR)NapiI64(env, a[0]));
    if (sp) sp->onMotionEvent(
        (int)NapiI32(env, a[1]),
        (float)NapiF64(env, a[2]),
        (float)NapiF64(env, a[3]),
        (int)NapiI32(env, a[4]),
        (int)NapiI32(env, a[5]),
        (float)NapiF64(env, a[6]),
        (float)NapiF64(env, a[7]),
        (int)NapiI32(env, a[8]),
        (long)NapiI64(env, a[9]));
    return NapiUndef(env);
}

// nativeOnKeyEvent(nativeId, keyCode, action, metaState, repeatCount, scanCode, unicodeChar, flags, timestamp)
// 对应 Android: Java_com_soui_android_SouiBaseSurface_nativeOnKeyEventEx
static napi_value surf_onKeyEvent(napi_env env, napi_callback_info ci) {
    napi_value a[9]; NapiArgs(env, ci, 9, a);
    auto sp = OhosPlatformAPI::instance().nativeViewLookup((UINT_PTR)NapiI64(env, a[0]));
    bool handled = false;
    if (sp) handled = sp->onKeyEvent(
        (int)NapiI32(env, a[1]),    // keyCode
        (int)NapiI32(env, a[2]),    // action
        (int)NapiI32(env, a[3]),    // metaState
        (int)NapiI32(env, a[4]),    // repeatCount
        (int)NapiI32(env, a[5]),    // scanCode
        (int)NapiI32(env, a[6]),    // unicodeChar
        (long)NapiI64(env, a[7]),   // flags
        (long)NapiI64(env, a[8]));  // timestamp
    return NapiOK(env, handled);
}

// ============================================================
// ======== 其余导出函数（与之前一致） =========================
// ============================================================ 

static napi_value nativeViewFromHwnd(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    napi_value r = OhosPlatformAPI::instance().hwndAsNativeWindow((UINT_PTR)NapiI64(env, a[0]));
    return r ? r : NapiUndef(env);
}

static napi_value screenStartup(napi_env env, napi_callback_info ci) {
    napi_value a[3]; NapiArgs(env, ci, 3, a);
    long screenId = (long)NapiI64(env, a[0]);
    long screenHwnd = (long)NapiI64(env, a[1]);
    std::string layout = NapiStr(env, a[2]);
    // screenHwnd = ArkUI 主 Surface 的 mNativeId（napi_ref 指针值），
    // 必须传下去注册进 swinx，否则主 Surface render 时 GetDC(hWnd) 的
    // WndMgr::fromHwnd 查不到窗口对象（对照 Android nativeScreenStartup）。
    return NapiOK(env, (int64_t)(UINT_PTR)OhosPlatformAPI::instance().screenStartup(
        env, screenId, (UINT_PTR)screenHwnd, layout.c_str()));
}

static napi_value screenShutdown(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    long screenId = (long)NapiI64(env, a[0]);
    OhosPlatformAPI::instance().screenShutdown(env, screenId);
    return NapiUndef(env);
}

static napi_value nativeNotifyEditEvent(napi_env env, napi_callback_info ci) {
    napi_value a[4]; NapiArgs(env, ci, 4, a);
    HWND hEditWnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[0])));
    HWND hParent = ::GetParent(hEditWnd);
    if (hParent != 0)
        ::SendMessageA(hParent, WM_COMMAND,
                        (WPARAM)NapiI64(env, a[2]), (LPARAM)NapiI64(env, a[3]));
    return NapiUndef(env);
}

static napi_value readCxxStringSlot(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    std::string s = OhosPlatformAPI::instance().ReadString((int)NapiI32(env, a[0]));
    return NapiOK(env, s.c_str());
}

static napi_value writeCxxStringSlot(napi_env env, napi_callback_info ci) {
    napi_value a[2]; NapiArgs(env, ci, 2, a);
    OhosPlatformAPI::instance().WriteString((int)NapiI32(env, a[0]), NapiStr(env, a[1]));
    return NapiUndef(env);
}

static napi_value notifyFocusGained(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    OhosPlatformAPI::instance().notifyFocusGained(
        reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[0]))));
    return NapiUndef(env);
}

// setScreenSize(screenId, hMainWnd, width, height)
// width/height 为物理像素（ArkTS 调用方已 ×density 换算）
static napi_value setScreenSize(napi_env env, napi_callback_info ci) {
    napi_value a[4]; NapiArgs(env, ci, 4, a);
    HWND hMainWnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[1])));
    int width = (int)NapiI32(env, a[2]);
    int height = (int)NapiI32(env, a[3]);
    ::MoveWindow(hMainWnd, 0, 0, width, height, TRUE);
    return NapiUndef(env);
}

// moveWindowNative(hwnd, x, y, w, h)
// ArkTS → C++ 同步 swinx wndObj->rc（物理像素）。
// ArkTS moveWindow/nativeMove 只更新 ArkUI .position 属性，不通知 C++ swinx，
// 导致 swinx wndObj->rc 位置停留在旧值（如 SHostWnd Create 后的 (-50,-50)），
// 后续 ClientToScreen 用错误的 rc.left/top 计算 combobox 下拉屏幕坐标。
// 本函数通过 ::SetWindowPos 触发 WM_MOVE/WM_SIZE 同步 swinx rc。
// w/h ≤ 0 时用 SWP_NOSIZE 跳过尺寸更新（保持原尺寸）。
static napi_value moveWindowNative(napi_env env, napi_callback_info ci) {
    napi_value a[5]; NapiArgs(env, ci, 5, a);
    HWND hWnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[0])));
    int x = (int)NapiI32(env, a[1]);
    int y = (int)NapiI32(env, a[2]);
    int w = (int)NapiI32(env, a[3]);
    int h = (int)NapiI32(env, a[4]);
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    if (w <= 0 || h <= 0) flags |= SWP_NOSIZE;
    ::SetWindowPos(hWnd, 0, x, y, w, h, flags);
    return NapiUndef(env);
}

static napi_value initSouiApp(napi_env env, napi_callback_info ci) {
    napi_value a[2]; NapiArgs(env, ci, 2, a);
    std::string dir = NapiStr(env, a[1]);
    BOOL ok = OhosPlatformAPI::instance().initSouiApp(env, a[0], dir);
    return NapiOK(env, (int32_t)ok);
}

static napi_value uninitSouiApp(napi_env env, napi_callback_info ci) {
    (void)ci;
    OhosPlatformAPI::instance().uninitSouiApp(env);
    return NapiUndef(env);
}

static napi_value processPendingMessages(napi_env env, napi_callback_info ci) {
    (void)ci;
    OhosPlatformAPI::instance().executePendingTask();
    return NapiUndef(env);
}

static napi_value processIdle(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    BOOL r = OhosPlatformAPI::instance().processIdle((int)NapiI32(env, a[0]));
    return NapiOK(env, (int32_t)r);
}

static napi_value setKeyboardHeight(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    OhosPlatformAPI::instance().setKeyboardHeight((int)NapiI32(env, a[0]));
    return NapiUndef(env);
}

static napi_value allocStringSlot(napi_env env, napi_callback_info ci) {
    (void)ci;
    return NapiOK(env, (int32_t)OhosPlatformAPI::instance().stringSlotAlloc(""));
}

static napi_value freeStringSlot(napi_env env, napi_callback_info ci) {
    napi_value a[1]; NapiArgs(env, ci, 1, a);
    OhosPlatformAPI::instance().stringSlotFree((int)NapiI32(env, a[0]));
    return NapiUndef(env);
}

static napi_value sendImeString(napi_env env, napi_callback_info ci) {
    napi_value a[2]; NapiArgs(env, ci, 2, a);
    HWND hWnd = reinterpret_cast<HWND>(static_cast<UINT_PTR>(NapiI64(env, a[0])));
    // Get UTF-8 string length first
    size_t utf8Len = 0;
    napi_get_value_string_utf8(env, a[1], nullptr, 0, &utf8Len);
    if (utf8Len == 0 || !hWnd) {
        return NapiUndef(env);
    }
    // Allocate buffer and get UTF-8 bytes
    std::string utf8Str;
    utf8Str.resize(utf8Len);
    size_t copied = 0;
    napi_get_value_string_utf8(env, a[1], &utf8Str[0], utf8Len + 1, &copied);
    OhosPlatformAPI::instance().sendImeString(
        hWnd,
        utf8Str.c_str(),
        copied);
    return NapiUndef(env);
}

// ============================================================
// ======== N-API 模块注册 ===================================
// ============================================================ 

#define DECL(fn)  { #fn, nullptr, fn, nullptr, nullptr, nullptr, napi_default, nullptr }
static const napi_property_descriptor g_soui4ohos_exports[] = {
    DECL(registerPlatformAPI),
    DECL(unregisterPlatformAPI),
    DECL(onTimerExpired),
    DECL(surf_nativeCreate),
    DECL(surf_nativeDestroy),
    DECL(surf_render),
    DECL(surf_onSizeChanged),
    DECL(surf_onMotionEvent),
    DECL(surf_onKeyEvent),
    DECL(nativeViewFromHwnd),
    DECL(screenStartup),
    DECL(screenShutdown),
    DECL(nativeNotifyEditEvent),
    DECL(readCxxStringSlot),
    DECL(writeCxxStringSlot),
    DECL(notifyFocusGained),
    DECL(setScreenSize),
    DECL(moveWindowNative),
    DECL(initSouiApp),
    DECL(uninitSouiApp),
    DECL(processPendingMessages),
    DECL(processIdle),
    DECL(setKeyboardHeight),
    DECL(allocStringSlot),
    DECL(freeStringSlot),
    DECL(sendImeString),
};
#undef DECL

// 需要 extern "C" 让符号名就是函数名，否则 alias 无法引用
extern "C" napi_value __napi_soui4ohos_entry(napi_env env, napi_value exports) {
    napi_define_properties(env, exports,
        sizeof(g_soui4ohos_exports) / sizeof(g_soui4ohos_exports[0]),
        g_soui4ohos_exports);
    SLOGI() << "libsoui4ohos.so loaded: exports registered="
            << (int)(sizeof(g_soui4ohos_exports)/sizeof(g_soui4ohos_exports[0]));
    return exports;
}

static napi_module g_soui4ohos_module = {
    // nm_version=
      1,      // NAPI_MODULE_VERSION
    // nm_flags=
        0,
    // nm_filename=
     nullptr,
    // nm_register_func=
 __napi_soui4ohos_entry,
    // nm_modname=
      "soui4ohos",
    // nm_priv=
         nullptr,
    // reserved=
        {nullptr, nullptr, nullptr, nullptr},
};

extern "C" __attribute__((constructor)) void __soui4ohos_init__() {
    if (napi_module_register)
        napi_module_register(&g_soui4ohos_module);
}

extern "C" napi_value napi_register_soui4ohos_module(napi_env env, napi_value exports)
    __attribute__((alias("__napi_soui4ohos_entry")));
