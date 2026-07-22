#include "AndroidPlatformAPI.h"
#include <android/configuration.h>
#include <algorithm>
#include <sstream>
#include <souistd.h>
#include <map>
#include <helper/slog.h>
#define kLogTag "AndroidPlatformAPI"

using namespace SNS;
extern JavaVM *g_javaVM;

static Soui4AndroidEntry * s_entry=nullptr;
void InitSoui4AndroidEntry(Soui4AndroidEntry * entry){
    s_entry = entry;
}

namespace {
    inline void ClearEx(JNIEnv *env) {
        if (env && env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
    }
}

AndroidPlatformAPI::AndroidPlatformAPI():m_theApp(NULL) {
    SLOGI()<<"AndroidPlatformAPI created";
}

AndroidPlatformAPI::~AndroidPlatformAPI() {
    SLOGI()<<"AndroidPlatformAPI destroyed";
}

AndroidPlatformAPI &AndroidPlatformAPI::instance() {
    static AndroidPlatformAPI inst;
    return inst;
}

JNIEnv *AndroidPlatformAPI::getJNIEnv() {
    JNIEnv *env = nullptr;
    if (!m_javaVM) return nullptr;
    jint ret = m_javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (ret == JNI_OK && env) return env;
    if (ret == JNI_EDETACHED) {
        ret = m_javaVM->AttachCurrentThread(&env, nullptr);
        if (ret == JNI_OK) return env;
    }
    SLOGE()<<"getJNIEnv failed: ret="<<(int)ret;
    return nullptr;
}

void AndroidPlatformAPI::init(JNIEnv *env, jobject bridge, jobject ctx) {
    if (!env) return;
    m_javaVM = nullptr;
    env->GetJavaVM(&m_javaVM);
    if (!m_javaVM) { SLOGE()<<"init: GetJavaVM failed"; return; }

    m_javaBridge = env->NewGlobalRef(bridge);
    m_javaCtx    = env->NewGlobalRef(ctx);

    // 1) SouiPlatformBridge 方法（createWindow/timer/capture）
    jclass clsBridge = env->FindClass("com/soui/android/SouiPlatformBridge");
    if (clsBridge) {
        m_createWindowMethod = env->GetMethodID(
            clsBridge, "createWindow",
            "(Landroid/view/View;JLjava/lang/String;Ljava/lang/String;IIIIIIJJJ)J");
        if (!m_createWindowMethod) {
            SLOGE()<<"init: failed to resolve SouiPlatformBridge.createWindow";
            ClearEx(env);
        }
        m_setTimerMethod = env->GetMethodID(clsBridge, "setTimer", "(JJI)J");
        m_killTimerMethod = env->GetMethodID(clsBridge, "killTimer", "(JJ)Z");
        m_killWindowTimersMethod = env->GetMethodID(clsBridge, "killWindowTimers", "(J)V");
        // Win32 SetCapture / ReleaseCapture —— Java 层统一维护捕获窗口与事件重定向
        m_setCaptureMethod = env->GetMethodID(clsBridge, "setCapture", "(J)J");
        m_releaseCaptureMethod = env->GetMethodID(clsBridge, "releaseCapture", "()Z");
        // Win32 SetFocus / GetFocus —— Java 层统一维护焦点 HWND + View 焦点 + IME
        m_setFocusMethod = env->GetMethodID(clsBridge, "setFocus", "(J)J");
        m_getFocusMethod = env->GetMethodID(clsBridge, "getFocus", "()J");
        m_postMessageMethod = env->GetMethodID(clsBridge, "scheduleMessageProcessing", "()V");
        m_getInputDevicesMethod = env->GetMethodID(clsBridge, "getInputDevices", "()[[Ljava/lang/String;");
        m_showSoftKeyboard = env->GetMethodID(clsBridge, "showSoftKeyboard", "(Landroid/view/View;Z)Z");
        // Clipboard methods
        m_clipboardOpenMethod = env->GetMethodID(clsBridge, "clipboardOpen", "(J)Z");
        m_clipboardCloseMethod = env->GetMethodID(clsBridge, "clipboardClose", "()Z");
        m_clipboardEmptyMethod = env->GetMethodID(clsBridge, "clipboardEmpty", "()Z");
        m_clipboardGetDataMethod = env->GetMethodID(clsBridge, "clipboardGetData", "(I)Ljava/lang/String;");
        m_clipboardSetDataMethod = env->GetMethodID(clsBridge, "clipboardSetData", "(ILjava/lang/String;)Z");
        m_clipboardIsFormatAvailableMethod = env->GetMethodID(clsBridge, "clipboardIsFormatAvailable", "(I)Z");
        m_clipboardGetOwnerMethod = env->GetMethodID(clsBridge, "clipboardGetOwner", "()J");
        m_clipboardHasFormatMethod = env->GetMethodID(clsBridge, "clipboardHasFormat", "(I)Z");
        env->DeleteLocalRef(clsBridge);
    }

    // 1.5) Context.getSystemService（用于获取系统服务，如剪贴板）
    {
        jclass clsContext = env->FindClass("android/content/Context");
        if (clsContext) {
            m_getSystemServiceMethod = env->GetMethodID(clsContext, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
            if (!m_getSystemServiceMethod) {
                SLOGE()<<"init: can't resolve Context.getSystemService(Ljava/lang/String;)Ljava/lang/Object;";
            }
            env->DeleteLocalRef(clsContext);
        } else {
            SLOGE()<<"init: can't find android/content/Context";
        }
    }

    // 2) INativeWindow 接口（HWND 就是实现了这个接口的 jobject GlobalRef）
    jclass clsNativeWnd = env->FindClass("com/soui/android/INativeWindow");
    if (!clsNativeWnd) { SLOGE()<<"init: can't find com/soui/INativeWindow"; ClearEx(env); }
    else {
        m_clsNativeWnd = (jclass) env->NewGlobalRef(clsNativeWnd);
        m_nwDestroy        = env->GetMethodID(clsNativeWnd, "nativeDestroy",       "()V");
        m_nwInvalidate     = env->GetMethodID(clsNativeWnd, "nativeInvalidate",    "(IIII)V");
        m_nwShow           = env->GetMethodID(clsNativeWnd, "nativeShow",          "(I)Z");
        m_nwMove           = env->GetMethodID(clsNativeWnd, "nativeMove",          "(IIII)Z");
        m_nwSetSize        = env->GetMethodID(clsNativeWnd, "nativeSetSize",       "(II)Z");
        m_nwSetPosition    = env->GetMethodID(clsNativeWnd, "nativeSetPosition",   "(II)Z");
        m_nwIsVisible      = env->GetMethodID(clsNativeWnd, "nativeIsVisible",     "()Z");
        m_nwEnable         = env->GetMethodID(clsNativeWnd, "nativeEnable",        "(Z)Z");
        m_nwIsEnabled      = env->GetMethodID(clsNativeWnd, "nativeIsEnabled",     "()Z");
        m_nwGetWindow      = env->GetMethodID(clsNativeWnd, "nativeGetWindow",     "(JI)J");
        m_nwAsView         = env->GetMethodID(clsNativeWnd, "asView",              "()Landroid/view/View;");
        // INativeWindow 通用消息转发（统一 slot 版）—— 必须在 m_clsNativeWnd 赋值之后再查
        //   原型（2026-07 slot 统一：去掉 nativeSendMessageStr / nativeSendMessageStrOut，
        //    所有消息走一个 nativeSendMessage(int msg, long wParam, long lParam)，
        //    字符串通过 StringSlot（C++ WriteString/ReadString + Java nativeRead/WriteCxxStringSlot）交换）。
        m_nativeSendMessageMethod = env->GetMethodID(clsNativeWnd, "nativeSendMessage", "(IJJ)J");
        // 防御性告警，生产环境可删
        if (!m_nativeSendMessageMethod) {
            SLOGE()<<"init: INativeWindow.nativeSendMessage methodId missing (slot-unified version)";
        }
        env->DeleteLocalRef(clsNativeWnd);
    }

    // 3) SouiWindow.getSurfaceView（souiStartup/souiShutdown 辅助取 Surface）
    {
        jclass clsNwg = env->FindClass("com/soui/android/SouiWindow");
        if (clsNwg) {
            m_nwgGetSurfaceViewMethod = env->GetMethodID(clsNwg, "getSurfaceView",
                                                         "()Landroid/view/View;");
            if (!m_nwgGetSurfaceViewMethod) {
                SLOGE()<<"init: can't resolve SouiWindow.getSurfaceView()Landroid/view/View;";
            }
            env->DeleteLocalRef(clsNwg);
        } else {
            SLOGE()<<"init: can't find com/soui/SouiWindow";
        }
    }
    ClearEx(env);
    SLOGI()<<"AndroidPlatformAPI init ok";
}

void AndroidPlatformAPI::deinit() {
    JNIEnv *env = getJNIEnv();
    if (env) {
        if (m_javaBridge)  { env->DeleteGlobalRef(m_javaBridge);  m_javaBridge  = nullptr; }
        if (m_javaCtx)     { env->DeleteGlobalRef(m_javaCtx);     m_javaCtx     = nullptr; }
        if (m_clsNativeWnd){ env->DeleteGlobalRef(m_clsNativeWnd);m_clsNativeWnd= nullptr; }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_screenContexts.clear();
        m_activeScreenStack.clear();
    }
    m_createWindowMethod = m_setTimerMethod = m_killTimerMethod = m_killWindowTimersMethod = nullptr;
    m_setCaptureMethod = m_releaseCaptureMethod = nullptr;
    m_setFocusMethod = m_getFocusMethod = nullptr;
    m_postMessageMethod = nullptr;
    m_nativeSendMessageMethod = nullptr;
    m_nwDestroy = m_nwInvalidate = m_nwShow = m_nwMove = m_nwSetSize = m_nwSetPosition = nullptr;
    m_nwIsVisible = m_nwEnable = m_nwIsEnabled = m_nwGetWindow = m_nwAsView = nullptr;
    m_nwgGetSurfaceViewMethod = nullptr;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_timerEntries.clear();
    }
    // 防御性兜底：清理仍残留的 SouiSurfaceProxy（正常路径 nativeDestroy 已 erase）
    {
        std::lock_guard<std::mutex> lock(m_nativeViewMutex);
        m_nativeViewMap.clear();
    }
    // StringSlot 清理（deinit 期间不再允许 SendMessage）
    {
        std::lock_guard<std::mutex> lock(m_slotMtx);
        m_slotStrings.clear();
    }
    SLOGI()<<"AndroidPlatformAPI deinit done";
}

UINT_PTR AndroidPlatformAPI::createWindow(UINT_PTR hParent,
                                          const char* pszClsName, const char* pszTitle,
                                          DWORD dwStyle, DWORD dwExStyle,
                                          int x, int y, int nWidth, int nHeight,
                                          UINT_PTR hMenu, UINT_PTR hInstance, LPVOID lpParam) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge) {
        SLOGE()<<"createWindow: JNI bridge not ready";
        return 0;
    }
    if (!m_createWindowMethod) {
        SLOGE()<<"createWindow: createWindow methodID missing (init failed?)";
        return 0;
    }
    const bool isMessageOnly = (HWND)hParent == HWND_MESSAGE;
    const bool isChild = (dwStyle & WS_CHILD) != 0;
    UINT_PTR parentHwnd = hParent;
    jobject parentView= nullptr;
    if(isMessageOnly){
        //set to invisible with size of (0,0)
        dwStyle = WS_POPUP;
        nWidth = nHeight = 0;
    }else{
        if (!isChild && parentHwnd == HWND_DESKTOP) {
            jlong screenId = getActiveScreenId();
            parentHwnd = getScreenHwnd(screenId);
        }
        parentView =  HwndLocalObj(parentHwnd);
        if(!parentView){
            SLOGE()<<"invalid parent";
        }
    }

    SLOGI()<<"createWindow +++,class name="<<pszClsName<<" title="<<pszTitle;

    jstring jClassName  = (pszClsName && *pszClsName)  ? env->NewStringUTF(pszClsName)  : nullptr;
    jstring jWindowName = (pszTitle  && *pszTitle)   ? env->NewStringUTF(pszTitle)   : nullptr;
    const jlong hwnd = env->CallLongMethod(
        m_javaBridge, m_createWindowMethod,
        parentView,
        (jlong)(INT_PTR)hParent,
        jClassName, jWindowName,
        (jint)dwStyle, (jint)dwExStyle,
        (jint)x, (jint)y, (jint)nWidth, (jint)nHeight,
        (jlong)hMenu, (jlong)hInstance,
        (jlong)reinterpret_cast<uintptr_t>(lpParam));

    if (jClassName)  env->DeleteLocalRef(jClassName);
    if (jWindowName) env->DeleteLocalRef(jWindowName);
    if (parentView)  env->DeleteLocalRef(parentView);
    ClearEx(env);

    if (hwnd == 0) {
        SLOGE()<<"createWindow: Java bridge returned 0, parent="<<reinterpret_cast<void*>(hParent)
               <<" cls="<<(pszClsName ? pszClsName : "(null)")<<" style=0x"<<std::hex<<(unsigned)dwStyle
               <<" id(menu)="<<reinterpret_cast<void*>(hMenu);
        return 0;
    }
    SLOGI()<<"createWindow +++, ret="<<hwnd<<" class name="<<pszClsName<<" title="<<pszTitle;
    return (UINT_PTR)hwnd;
}

BOOL AndroidPlatformAPI::destroyWindow(UINT_PTR hWnd) {
    JNIEnv *env = getJNIEnv();
    if (!env) return FALSE;
    jobject viewObj = HwndObj(hWnd);
    if (!viewObj) return FALSE;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        unregisterWindowTimersInternal(hWnd);
    }
    // 先让 Java 层 INativeWindow.nativeDestroy() 把 View 从父容器里移除
    if (m_nwDestroy) {
        env->CallVoidMethod(viewObj, m_nwDestroy);
        if (env->ExceptionCheck()) { ClearEx(env); }
    }
    SLOGI()<<"destroywindow ---, hwnd="<<hWnd;
    return TRUE;
}

jobject AndroidPlatformAPI::HwndObj(UINT_PTR hWnd) {
    if (hWnd == 0) return nullptr;
    if (!isValidHwnd(hWnd)) {
        return nullptr;
    }
    return reinterpret_cast<jobject>(hWnd);
}

jobject AndroidPlatformAPI::HwndLocalObj(UINT_PTR hWnd) {
    const jobject globalObj = HwndObj(hWnd);
    if(!globalObj)
        return nullptr;
    return getJNIEnv()->NewLocalRef(globalObj);
}


bool AndroidPlatformAPI::nativeViewInsert(jlong nativeId, std::shared_ptr<SouiSurfaceProxy> nativeView) {
    if (!nativeView || nativeId == 0) return false;
    std::lock_guard<std::mutex> lock(m_nativeViewMutex);
    return m_nativeViewMap.emplace(nativeId, std::move(nativeView)).second;
}

std::shared_ptr<SouiSurfaceProxy> AndroidPlatformAPI::nativeViewLookup(jlong nativeId) {
    if (nativeId == 0) return nullptr;
    std::lock_guard<std::mutex> lock(m_nativeViewMutex);
    auto it = m_nativeViewMap.find(nativeId);
    if (it == m_nativeViewMap.end()) return nullptr;
    return it->second; // shared_ptr 拷贝，延长生命周期到调用方局部作用域
}

bool AndroidPlatformAPI::nativeViewErase(jlong nativeId) {
    if (nativeId == 0) return false;
    std::lock_guard<std::mutex> lock(m_nativeViewMutex);
    return m_nativeViewMap.erase(nativeId) > 0;
}

bool AndroidPlatformAPI::isValidHwnd(UINT_PTR hWnd) const {
    if (hWnd == 0) return false;
    const jlong nativeId = static_cast<jlong>(hWnd);
    // 主 SouiSurface 的 HWND（= SouiSurfaceProxy.nativeId）
    {
        std::lock_guard<std::mutex> lock(m_nativeViewMutex);
        if (m_nativeViewMap.find(nativeId) != m_nativeViewMap.end()) {
            return true;
        }
    }
    return false;
}

jobject AndroidPlatformAPI::hwndAsView(UINT_PTR hWnd) {
    if (!hWnd) return nullptr;
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwAsView) return nullptr;
    jobject nwObj = HwndObj(hWnd);
    if (!nwObj) return nullptr;
    jobject view = env->CallObjectMethod(nwObj, m_nwAsView);
    if (env->ExceptionCheck()) {
        ClearEx(env);
        return nullptr;
    }
    return view;
}

HWND AndroidPlatformAPI::getWindow(HWND hWnd,int code){
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwGetWindow) return 0;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj)
        return 0;
    jlong res = env->CallLongMethod(wndObj, m_nwGetWindow,
                                    (jlong)(UINT_PTR)hWnd, (jint)code);
    if (env->ExceptionCheck()) { ClearEx(env); return 0; }
    return (HWND)(UINT_PTR)res;
}

BOOL AndroidPlatformAPI::showWindow(HWND hWnd, int nCmdShow) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwShow) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwShow, (jint) nCmdShow);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::enableWindow(HWND hWnd, BOOL bEnable) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwEnable) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwEnable, (jboolean) bEnable);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::isWindowVisible(HWND hWnd) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwIsVisible) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwIsVisible);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::isWindowEnabled(HWND hWnd) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwIsEnabled) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwIsEnabled);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::moveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwMove) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwMove,
                                         (jint) x, (jint) y, (jint) nWidth, (jint) nHeight);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    if (bRepaint && ok) {
        if (m_nwInvalidate) {
            wndObj = HwndObj((UINT_PTR)hWnd);
                if (!wndObj) return FALSE;
            env->CallVoidMethod(wndObj, m_nwInvalidate, 0, 0, 0, 0);
            ClearEx(env);
        }
    }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::setWindowSize(HWND hWnd, int cx, int cy) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwSetSize) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwSetSize,
                                         (jint) cx, (jint) cy);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::setWindowPosEx(HWND hWnd, int x, int y) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_nwSetPosition) return FALSE;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return FALSE;
    jboolean ok = env->CallBooleanMethod(wndObj, m_nwSetPosition,
                                         (jint) x, (jint) y);
    if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::setWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y,
                                      int cx, int cy, UINT uFlags) {
    bool hasSize = (uFlags & 0x0001 /*SWP_NOSIZE*/) == 0;
    bool hasPos  = (uFlags & 0x0002 /*SWP_NOMOVE*/) == 0;
    if (!hasSize && !hasPos) return TRUE;

    if (hasSize && hasPos) {
        JNIEnv *env = getJNIEnv();
        if (!env || !m_nwMove) return FALSE;
        jobject wndObj = HwndObj((UINT_PTR)hWnd);
        if (!wndObj) return FALSE;
        jboolean ok = env->CallBooleanMethod(wndObj, m_nwMove,
                                             (jint) X, (jint) Y, (jint) cx, (jint) cy);
        if (env->ExceptionCheck()) { ClearEx(env); return FALSE; }
        return ok ? TRUE : FALSE;
    }
    if (hasSize) return setWindowSize(hWnd, cx, cy);
    if (hasPos)  return setWindowPosEx(hWnd, X, Y);
    return TRUE;
}

bool AndroidPlatformAPI::invalidateRect(HWND hWnd, int left, int top, int right, int bottom) {
    JNIEnv *env = getJNIEnv();
    if (!env) return false;
    if (!m_nwInvalidate) return false;
    jobject wndObj = HwndObj((UINT_PTR)hWnd);
    if (!wndObj) return false;
    env->CallVoidMethod(wndObj, m_nwInvalidate,
                        (jint) left, (jint) top, (jint) right, (jint) bottom);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return true;
}

void AndroidPlatformAPI::unregisterWindowTimersInternal(UINT_PTR hWnd) {
    m_timerEntries.erase(hWnd);
}

UINT_PTR AndroidPlatformAPI::setTimer(HWND hWnd, UINT_PTR id, UINT uElapse, TIMERPROC proc) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_setTimerMethod) return 0;
    const UINT_PTR hWndKey = reinterpret_cast<UINT_PTR>(hWnd);
    const jlong jid = env->CallLongMethod(m_javaBridge, m_setTimerMethod,
                                          static_cast<jlong>(hWndKey),
                                          static_cast<jlong>(id),
                                          static_cast<jint>(uElapse));
    ClearEx(env);
    if (jid == 0) return 0;
    const UINT_PTR actualTimerId = static_cast<UINT_PTR>(jid);
    if (proc != nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        AndroidTimerEntry e{};
        e.timerId   = actualTimerId;
        e.proc      = proc;
        e.interval  = uElapse;
        e.repeating = true;
        m_timerEntries[hWndKey][actualTimerId] = e;
    }
    return actualTimerId;
}

BOOL AndroidPlatformAPI::killTimer(HWND hWnd, UINT_PTR id) {
    if (id == 0) return FALSE;
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_killTimerMethod) return FALSE;
    const UINT_PTR hWndKey = reinterpret_cast<UINT_PTR>(hWnd);
    jboolean ok = env->CallBooleanMethod(m_javaBridge, m_killTimerMethod,
                                         static_cast<jlong>(hWndKey),
                                         static_cast<jlong>(id));
    ClearEx(env);
    // 本地 bookkeeping 同样按 (hWnd, timerId) 二元擦除
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto outerIt = m_timerEntries.find(hWndKey);
        if (outerIt != m_timerEntries.end()) {
            outerIt->second.erase(id);
            if (outerIt->second.empty()) {
                m_timerEntries.erase(outerIt);
            }
        }
    }
    return ok ? TRUE : FALSE;
}

BOOL AndroidPlatformAPI::killWindowTimers(HWND hWnd) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_killWindowTimersMethod) return FALSE;
    const UINT_PTR hWndKey = reinterpret_cast<UINT_PTR>(hWnd);
    env->CallVoidMethod(m_javaBridge, m_killWindowTimersMethod,
                        static_cast<jlong>(hWndKey));
    ClearEx(env);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        unregisterWindowTimersInternal(hWndKey);
    }
    return TRUE;
}

void AndroidPlatformAPI::onTimerExpired(HWND hWnd, UINT_PTR timerId) {
    if (timerId == 0) return;
    TIMERPROC proc = nullptr;
    const UINT_PTR hWndKey = reinterpret_cast<UINT_PTR>(hWnd);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto outerIt = m_timerEntries.find(hWndKey);
        if (outerIt != m_timerEntries.end()) {
            auto innerIt = outerIt->second.find(timerId);
            if (innerIt != outerIt->second.end()) {
                proc = innerIt->second.proc;
            }
        }
    }
    if (proc != nullptr) {
        // Win32 TIMERPROC(HWND, UINT, UINT_PTR id, DWORD elapsed)
        proc(hWnd, WM_TIMER, timerId, 0);
    }else{
        if(!IsWindow(hWnd))
            return;
        ::SendMessage(hWnd,WM_TIMER,timerId,0);
    }
}

HWND AndroidPlatformAPI::getScreenHwnd(jlong screenId) const{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_screenContexts.find(screenId);
    if(it == m_screenContexts.end())
        return 0;
    return it->second;
}
int AndroidPlatformAPI::getScreenWidth(jlong screenId) const {
    if (screenId == 0) return 0;
    HWND hDesktop = getScreenHwnd(screenId);
    if(!hDesktop){
        return 1280;
    }
    RECT rc;
    ::GetWindowRect(hDesktop,&rc);
    return rc.right-rc.left;
}

int AndroidPlatformAPI::getScreenHeight(jlong screenId) const {
    if (screenId == 0) return 0;
    HWND hDesktop = getScreenHwnd(screenId);
    if(!hDesktop){
        return 1280;
    }
    RECT rc;
    ::GetWindowRect(hDesktop,&rc);
    return rc.bottom-rc.top;
}

int AndroidPlatformAPI::getScreenWidth() const {
    jlong screenId = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_activeScreenStack.empty()) {
            screenId = m_activeScreenStack.back();
        } else if (!m_screenContexts.empty()) {
            screenId = m_screenContexts.cbegin()->first;
        }
    }
    return (screenId == 0) ? 0 : getScreenWidth(screenId);
}

int AndroidPlatformAPI::getScreenHeight() const {
    jlong screenId = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_activeScreenStack.empty()) {
            screenId = m_activeScreenStack.back();
        } else if (!m_screenContexts.empty()) {
            screenId = m_screenContexts.cbegin()->first;
        }
    }
    return (screenId == 0) ? 0 : getScreenHeight(screenId);
}

HWND AndroidPlatformAPI::setCapture(UINT_PTR hwnd) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_setCaptureMethod) {
        return 0;
    }
    const jlong oldHwnd = env->CallLongMethod(
        m_javaBridge, m_setCaptureMethod, (jlong)hwnd);
    ClearEx(env);
    return reinterpret_cast<HWND>(static_cast<UINT_PTR>(oldHwnd));
}

BOOL AndroidPlatformAPI::releaseCapture() {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_releaseCaptureMethod) {
        return FALSE;
    }
    const jboolean ok = env->CallBooleanMethod(
        m_javaBridge, m_releaseCaptureMethod);
    ClearEx(env);
    return ok ? TRUE : FALSE;
}

HWND AndroidPlatformAPI::setFocus(UINT_PTR hwnd) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_setFocusMethod) {
        return 0;
    }
    const jlong oldHwnd = env->CallLongMethod(
        m_javaBridge, m_setFocusMethod, (jlong)hwnd);
    ClearEx(env);
    return reinterpret_cast<HWND>(static_cast<UINT_PTR>(oldHwnd));
}

HWND AndroidPlatformAPI::getFocus() const {
    // getFocus 仅读 volatile long，不需要 JNI 调用 —— 但若平台桥还没初始化
    // 或多线程并发场景下希望跟 Java 真源严格一致，就走 JNI。这里为了一致性走 JNI。
    auto *self = const_cast<AndroidPlatformAPI *>(this);
    JNIEnv *env = self->getJNIEnv();
    if (!env || !m_javaBridge || !m_getFocusMethod) {
        return 0;
    }
    const jlong hwnd = env->CallLongMethod(m_javaBridge, m_getFocusMethod);
    if (env->ExceptionCheck()) { env->ExceptionClear(); }
    return reinterpret_cast<HWND>(static_cast<UINT_PTR>(hwnd));
}

// SWinx TCHAR → UTF-8 char* 编码：与 demo MainDlg S_CW2A(text, CP_UTF8) Toast 写法一致。
static inline const char* toUtf8OrEmpty(const char* maybeNull) {
    return maybeNull ? maybeNull : "";
}


int AndroidPlatformAPI::stringSlotAlloc(const char * pszStr) {
    std::string initValue = pszStr ? pszStr : "";
    // ① 快速路径（原子自增）：绝大多数命中 [1, 65535]
    int id = m_slotNextId.fetch_add(1, std::memory_order_relaxed);
    if (id >= 1 && id <= 65535) {
        std::lock_guard<std::mutex> lk(m_slotMtx);
        m_slotStrings[id]= std::move(initValue);
        return id;
    }
    // ② wrap / 慢速路径：id 超界或异常（负数/0）→ 串行化重置 + 探测下一个有效 id
    std::lock_guard<std::mutex> lk(m_slotMtx);
    // 再读一次（其他线程可能已经帮我们 reset 过）
    int next = m_slotNextId.load(std::memory_order_relaxed);
    if (next < 1 || next > 65535) {
        next = 1;  // 重置起点：永远跳过 0
    }
    id = next;
    // 下一个分配 id：环形（id==65535 → 下一轮回到 1）
    const int after = (id >= 65535) ? 1 : (id + 1);
    m_slotNextId.store(after, std::memory_order_relaxed);
    m_slotStrings[id] = std::move(initValue);
    return id;
}

void AndroidPlatformAPI::WriteString(int id, std::string s) {
    if (id <= 0) return;  // 0 / 负保留
    std::lock_guard<std::mutex> lk(m_slotMtx);
    // insert_or_assign：槽不存在则创建（对齐头文件"put + create-if-absent"语义）；
    // 槽已存在则覆盖其值，避免 wrap 复用 id 时读到上一轮残留。
    m_slotStrings[id] = std::move(s);
}

std::string AndroidPlatformAPI::ReadString(int id) {
    if (id <= 0) return {};
    std::lock_guard<std::mutex> lk(m_slotMtx);
    auto it = m_slotStrings.find(id);
    if (it == m_slotStrings.end()) return {};
    return it->second;
}

void AndroidPlatformAPI::stringSlotFree(int id) {
    if (id <= 0) return;
    std::lock_guard<std::mutex> lk(m_slotMtx);
    m_slotStrings.erase(id);
}


LRESULT AndroidPlatformAPI::sendMessage(UINT_PTR hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (hwnd == 0) return 0;
    JNIEnv *env = getJNIEnv();
    if (!env) return 0;
    jobject wndObj = reinterpret_cast<jobject>(hwnd);
    if (!m_nativeSendMessageMethod) return 0;

    const jlong res = env->CallLongMethod(wndObj, m_nativeSendMessageMethod,
                                          (jint)msg, (jlong)wParam, (jlong)lParam);
    ClearEx(env);
    if (msg == EM_GETSEL) {
        const DWORD s = (DWORD)((jlong)res & 0xffffffffLL);
        const DWORD e = (DWORD)(((jlong)res >> 32) & 0xffffffffLL);
        if (wParam) *reinterpret_cast<DWORD*>(wParam) = s;
        if (lParam) *reinterpret_cast<DWORD*>(lParam) = e;
        return 1;
    }
    return (LRESULT)res;
}

void AndroidPlatformAPI::setScreenHwnd(jlong screenId, UINT_PTR hwnd) {
    if (screenId == 0) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_screenContexts.find(screenId);
    if (it == m_screenContexts.end()) {
        m_screenContexts.emplace(screenId, hwnd);
    } else {
        it->second = hwnd;
    }
}

void AndroidPlatformAPI::eraseScreen(jlong screenId){
    if (screenId == 0) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_screenContexts.erase(screenId);
}

// 激活栈：HWND_DESKTOP 路由的精确归属依据（支持多 Activity 多 SouiScreen）。
jlong AndroidPlatformAPI::pushActiveScreen(jlong screenId) {
    if (screenId == 0) return 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    m_activeScreenStack.push_back(screenId);
    SLOGD()<<"pushActiveScreen: id="<<(long long)screenId<<" (stackSize="<<m_activeScreenStack.size()<<")";
    return screenId;
}

jlong AndroidPlatformAPI::popActiveScreen(jlong screenId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (screenId != 0) {
        // 先从尾部（栈顶）向前查找：正常 pop 就是栈顶 = screenId，O(1)
        if (!m_activeScreenStack.empty() && m_activeScreenStack.back() == screenId) {
            m_activeScreenStack.pop_back();
        } else {
            // shutdown 提前退出或异常：按 id 从中间擦除
            auto it = std::find(m_activeScreenStack.begin(), m_activeScreenStack.end(), screenId);
            if (it != m_activeScreenStack.end()) m_activeScreenStack.erase(it);
        }
        SLOGD()<<"popActiveScreen: id="<<(long long)screenId<<" removed (stackSize="<<m_activeScreenStack.size()<<")";
    }
    return m_activeScreenStack.empty() ? 0 : m_activeScreenStack.back();
}

jlong AndroidPlatformAPI::getActiveScreenId() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeScreenStack.empty() ? 0 : m_activeScreenStack.back();
}

HWND AndroidPlatformAPI::souiStartup(JNIEnv* env, jlong screenId,const char* layout) {
    if (!env || screenId == 0)
        return 0;
    pushActiveScreen(screenId);
    SASSERT(s_entry);
    return s_entry->Startup(screenId, layout);
}

void AndroidPlatformAPI::souiShutdown(JNIEnv* env, jlong screenId) {
    if (!env || screenId == 0) return;
    SASSERT(s_entry);
    s_entry->Shutdown(screenId);
    popActiveScreen(screenId);
}

bool AndroidPlatformAPI::initSouiApp(JNIEnv *env, jobject assetManagerJ, jstring appFilesDirJ) {
    if (m_theApp) {
        SLOGW()<<"AndroidPlatformAPI::initSouiApp, SApplication already initialized, skip";
        return false;
    }
    // ---- 参数解析 ----
    AAssetManager* assetMgr = AAssetManager_fromJava(env, assetManagerJ);
    if (!assetMgr) {
        SLOGE()<<"nativeInitSouiApp: AAssetManager_fromJava returned nullptr. "
               "Java 层必须传入一个有效的 AssetManager 实例。";
        return false;
    }
    AConfiguration* config = AConfiguration_new();
    AConfiguration_fromAssetManager(config, assetMgr);
    m_density = AConfiguration_getDensity(config);
    AConfiguration_delete(config);

    const char* appFilesDir = env->GetStringUTFChars(appFilesDirJ, nullptr);
    SLOGI()<<"nativeInitSouiApp: AAssetManager*="<<assetMgr<<" filesDir="<<appFilesDir;
    SASSERT(s_entry);
    m_theApp = s_entry->InitApp(assetMgr,appFilesDir);
    env->ReleaseStringUTFChars(appFilesDirJ,appFilesDir);
    if(!m_theApp)
        return false;
    m_theApp->GetMsgLoop()->OnStart();
    return true;
}

void AndroidPlatformAPI::uninitSouiApp(JNIEnv *env) {
    SASSERT(s_entry);
    if(m_theApp){
        s_entry->UninitApp(m_theApp);
        m_theApp->GetMsgLoop()->OnStop();
        m_theApp=nullptr;
    }
}

void AndroidPlatformAPI::postMessage() {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_postMessageMethod) {
        return;
    }
    env->CallVoidMethod(m_javaBridge, m_postMessageMethod);
    ClearEx(env);
}

bool AndroidPlatformAPI::processIdle(int idleCount) {
    SApplication *pApp = SApplication::getSingletonPtr();
    if (pApp) {
        IMessageLoop *pMsgLoop = pApp->GetMsgLoop();
        if (pMsgLoop) {
            return pMsgLoop->OnIdle(idleCount);
        }
    }
    return false;
}

// Clipboard implementations (delegated to Java layer)
BOOL AndroidPlatformAPI::clipboardOpen(HWND hWndNewOwner) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardOpenMethod) {
        return FALSE;
    }
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardOpenMethod, (jlong)(INT_PTR)hWndNewOwner);
    ClearEx(env);
    return (BOOL)result;
}

BOOL AndroidPlatformAPI::clipboardClose() {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardCloseMethod) {
        return FALSE;
    }
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardCloseMethod);
    ClearEx(env);
    return (BOOL)result;
}

BOOL AndroidPlatformAPI::clipboardEmpty() {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardEmptyMethod) {
        return FALSE;
    }
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardEmptyMethod);
    ClearEx(env);
    return (BOOL)result;
}

HANDLE AndroidPlatformAPI::clipboardGetData(UINT uFormat) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardGetDataMethod) {
        return NULL;
    }
    jstring jText = (jstring)env->CallObjectMethod(m_javaBridge, m_clipboardGetDataMethod, (jint)uFormat);
    ClearEx(env);
    if (!jText) {
        return NULL;
    }
    
    const char *text = env->GetStringUTFChars(jText, nullptr);
    if (!text) {
        env->DeleteLocalRef(jText);
        return NULL;
    }
    
    HGLOBAL hMem;
    if (uFormat == CF_UNICODETEXT) {
        int wlen = (int)strlen(text) + 1;
        hMem = GlobalAlloc(GMEM_MOVEABLE, wlen * sizeof(wchar_t));
        if (hMem) {
            wchar_t *buf = (wchar_t *)GlobalLock(hMem);
            if (buf) {
                MultiByteToWideChar(CP_UTF8, 0, text, -1, buf, wlen);
                GlobalUnlock(hMem);
            }
        }
    } else {
        hMem = GlobalAlloc(GMEM_MOVEABLE, strlen(text) + 1);
        if (hMem) {
            char *buf = (char *)GlobalLock(hMem);
            if (buf) {
                strcpy(buf, text);
                GlobalUnlock(hMem);
            }
        }
    }
    
    env->ReleaseStringUTFChars(jText, text);
    env->DeleteLocalRef(jText);
    return hMem;
}

HANDLE AndroidPlatformAPI::clipboardSetData(UINT uFormat, HANDLE hMem) {
    if (!hMem) {
        return NULL;
    }
    
    std::string text;
    if (uFormat == CF_UNICODETEXT) {
        wchar_t *wbuf = (wchar_t *)GlobalLock(hMem);
        if (wbuf) {
            int len = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
            if (len > 0) {
                text.resize(len);
                WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, &text[0], len, nullptr, nullptr);
            }
            GlobalUnlock(hMem);
        }
    } else {
        char *buf = (char *)GlobalLock(hMem);
        if (buf) {
            text = buf;
            GlobalUnlock(hMem);
        }
    }
    
    if (text.empty()) {
        return NULL;
    }
    
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardSetDataMethod) {
        return NULL;
    }
    
    jstring jText = env->NewStringUTF(text.c_str());
    if (!jText) {
        return NULL;
    }
    
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardSetDataMethod, (jint)uFormat, jText);
    ClearEx(env);
    env->DeleteLocalRef(jText);
    return (BOOL)result ? hMem : NULL;
}

BOOL AndroidPlatformAPI::clipboardIsFormatAvailable(UINT format) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardIsFormatAvailableMethod) {
        return FALSE;
    }
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardIsFormatAvailableMethod, (jint)format);
    ClearEx(env);
    return (BOOL)result;
}

UINT AndroidPlatformAPI::clipboardRegisterFormat(LPCSTR pszName) {
    static std::map<std::string, UINT> s_formatMap;
    static UINT s_nextFormat = 0xC000;
    
    if (!pszName || !*pszName) {
        return 0;
    }
    
    auto it = s_formatMap.find(pszName);
    if (it != s_formatMap.end()) {
        return it->second;
    }
    
    UINT fmt = s_nextFormat++;
    s_formatMap[pszName] = fmt;
    return fmt;
}

HWND AndroidPlatformAPI::clipboardGetOwner() {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardGetOwnerMethod) {
        return 0;
    }
    jlong result = env->CallLongMethod(m_javaBridge, m_clipboardGetOwnerMethod);
    ClearEx(env);
    return (HWND)(INT_PTR)result;
}

BOOL AndroidPlatformAPI::clipboardHasFormat(UINT format) {
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_clipboardHasFormatMethod) {
        return FALSE;
    }
    jboolean result = env->CallBooleanMethod(m_javaBridge, m_clipboardHasFormatMethod, (jint)format);
    ClearEx(env);
    return (BOOL)result;
}

void AndroidPlatformAPI::sendImeString(UINT_PTR hwnd, int slotid) {
    if (hwnd == 0) return;
    if(slotid == 0) return;
    std::string str = ReadString(slotid);
    if(!str.empty()){
        SStringW strW = S_CA2W(SStringA(str.c_str(),str.length()),CP_UTF8);
        for(int i=0;i<strW.GetLength();i++){
            ::SendMessage((HWND)hwnd, WM_IME_CHAR, strW[i], 0);
        }
    }
}

int AndroidPlatformAPI::getDpi() {
    return m_density;
}

// SWINX 层平台注册：把 C++ 实现绑定到 SWINX 的 PlatformAPI 虚表。
// RegisterAndroidPlatformAPI/UnregisterAndroidPlatformAPI 的具体注册实现
// 由 soui-android-lib 提供；此处放 weak stub 保证本 demo 编译通过。
__attribute__((weak)) extern "C" void RegisterAndroidPlatformAPI() { SLOGI()<<"RegisterAndroidPlatformAPI (weak stub)"; }
__attribute__((weak)) extern "C" void UnregisterAndroidPlatformAPI() { SLOGI()<<"UnregisterAndroidPlatformAPI (weak stub)"; }

struct RawInputDeviceEntry {
    std::string device_path;
    DWORD device_type;
};

static std::map<int, RawInputDeviceEntry> s_rawInputDevices;
static std::recursive_mutex s_rawInputMutex;
static int s_nextDeviceId = 1;

UINT AndroidPlatformAPI::getRawInputDeviceList(PRAWINPUTDEVICELIST pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize)
{
    if (cbSize < sizeof(RAWINPUTDEVICELIST)) {
        return 0;
    }

    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_getInputDevicesMethod) {
        *puiNumDevices = 0;
        return 0;
    }

    jobjectArray deviceArray = (jobjectArray)env->CallObjectMethod(m_javaBridge, m_getInputDevicesMethod);
    if (!deviceArray) {
        ClearEx(env);
        *puiNumDevices = 0;
        return 0;
    }

    {
        std::lock_guard<std::recursive_mutex> lock(s_rawInputMutex);
        s_rawInputDevices.clear();
        s_nextDeviceId = 1;
    }

    jsize deviceCount = env->GetArrayLength(deviceArray);
    int max_devices = (pRawInputDeviceList != NULL) ? *puiNumDevices : 0;
    int filled = 0;

    for (jsize i = 0; i < deviceCount; i++) {
        jobjectArray infoArray = (jobjectArray)env->GetObjectArrayElement(deviceArray, i);
        if (!infoArray) continue;

        jsize infoLen = env->GetArrayLength(infoArray);
        if (infoLen < 2) {
            env->DeleteLocalRef(infoArray);
            continue;
        }

        jstring pathStr = (jstring)env->GetObjectArrayElement(infoArray, 0);
        jstring typeStr = (jstring)env->GetObjectArrayElement(infoArray, 1);

        const char* path = env->GetStringUTFChars(pathStr, nullptr);
        const char* typeStrC = env->GetStringUTFChars(typeStr, nullptr);

        DWORD deviceType = RIM_TYPEMOUSE;
        if (strcmp(typeStrC, "keyboard") == 0) {
            deviceType = RIM_TYPEKEYBOARD;
        } else if (strcmp(typeStrC, "hid") == 0) {
            deviceType = RIM_TYPEHID;
        }

        int deviceId;
        {
            std::lock_guard<std::recursive_mutex> lock(s_rawInputMutex);
            deviceId = s_nextDeviceId++;
            s_rawInputDevices[deviceId] = { path, deviceType };
        }

        if (pRawInputDeviceList != NULL && filled < max_devices) {
            pRawInputDeviceList[filled].hDevice = (HANDLE)(intptr_t)deviceId;
            pRawInputDeviceList[filled].dwType = deviceType;
            filled++;
        }

        env->ReleaseStringUTFChars(pathStr, path);
        env->ReleaseStringUTFChars(typeStr, typeStrC);
        env->DeleteLocalRef(pathStr);
        env->DeleteLocalRef(typeStr);
        env->DeleteLocalRef(infoArray);
    }

    env->DeleteLocalRef(deviceArray);
    ClearEx(env);

    *puiNumDevices = deviceCount;
    return (UINT)filled;
}

UINT AndroidPlatformAPI::getRawInputDeviceInfoA(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize)
{
    if (!pcbSize)
        return (UINT)-1;

    UINT requiredSize = 0;
    int deviceId = (int)(intptr_t)hDevice;
    std::string device_path;
    DWORD device_type = RIM_TYPEMOUSE;

    {
        std::lock_guard<std::recursive_mutex> lock(s_rawInputMutex);
        auto it = s_rawInputDevices.find(deviceId);
        if (it == s_rawInputDevices.end()) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (UINT)-1;
        }
        device_path = it->second.device_path;
        device_type = it->second.device_type;
    }

    switch (uiCommand)
    {
        case RIDI_DEVICENAME:
        {
            requiredSize = (UINT)device_path.length() + 1;
            if (pData && *pcbSize >= requiredSize)
                strcpy((char*)pData, device_path.c_str());
            *pcbSize = requiredSize;
            break;
        }
        case RIDI_DEVICEINFO:
        {
            requiredSize = sizeof(RID_DEVICE_INFO);
            if (pData && *pcbSize >= requiredSize)
            {
                RID_DEVICE_INFO* pInfo = (RID_DEVICE_INFO*)pData;
                pInfo->cbSize = sizeof(RID_DEVICE_INFO);
                pInfo->dwType = device_type;
                if (pInfo->dwType == RIM_TYPEMOUSE)
                {
                    pInfo->mouse.dwId = 0;
                    pInfo->mouse.dwNumberOfButtons = 3;
                    pInfo->mouse.dwSampleRate = 125;
                    pInfo->mouse.fHasHorizontalWheel = FALSE;
                }
                else if (pInfo->dwType == RIM_TYPEKEYBOARD)
                {
                    pInfo->keyboard.dwType = 1;
                    pInfo->keyboard.dwSubType = 0;
                    pInfo->keyboard.dwKeyboardMode = 0;
                    pInfo->keyboard.dwNumberOfFunctionKeys = 12;
                    pInfo->keyboard.dwNumberOfIndicators = 3;
                    pInfo->keyboard.dwNumberOfKeysTotal = 104;
                }
                else if (pInfo->dwType == RIM_TYPEHID)
                {
                    pInfo->hid.dwVendorId = 0;
                    pInfo->hid.dwProductId = 0;
                    pInfo->hid.dwVersionNumber = 0;
                    pInfo->hid.usUsagePage = 0;
                    pInfo->hid.usUsage = 0;
                }
            }
            *pcbSize = requiredSize;
            break;
        }
        case RIDI_PREPARSEDDATA:
        {
            requiredSize = 0;
            *pcbSize = requiredSize;
            break;
        }
        default:
            SetLastError(ERROR_INVALID_PARAMETER);
            return (UINT)-1;
    }

    return requiredSize;
}

UINT AndroidPlatformAPI::getRawInputDeviceInfoW(HRAWINPUT hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize)
{
    if (uiCommand != RIDI_DEVICENAME) {
        return getRawInputDeviceInfoA(hDevice, uiCommand, pData, pcbSize);
    }
    if (!pcbSize)
        return (UINT)-1;

    UINT requiredSize = 0;
    int deviceId = (int)(intptr_t)hDevice;
    std::string device_path;

    {
        std::lock_guard<std::recursive_mutex> lock(s_rawInputMutex);
        auto it = s_rawInputDevices.find(deviceId);
        if (it == s_rawInputDevices.end()) {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (UINT)-1;
        }
        device_path = it->second.device_path;
    }

    int wLen = MultiByteToWideChar(CP_UTF8, 0, device_path.c_str(), -1, NULL, 0);
    requiredSize = (UINT)(wLen * sizeof(wchar_t));
    if (pData && *pcbSize >= requiredSize)
        MultiByteToWideChar(CP_UTF8, 0, device_path.c_str(), -1, (wchar_t*)pData, wLen);
    *pcbSize = requiredSize;
    return requiredSize;
}

BOOL AndroidPlatformAPI::showSoftKeyboard(HWND hWnd,BOOL bShow){
    HWND hFocus = hWnd?hWnd:GetFocus();
    if(!hFocus)
        return FALSE;
    JNIEnv *env = getJNIEnv();
    if (!env || !m_javaBridge || !m_showSoftKeyboard) {
        return FALSE;
    }
    jobject  v = hwndAsView(hFocus);
    if(!v){
        SLOGW()<<"focus window invalid";
        return FALSE;
    }
    jboolean ret = env->CallBooleanMethod(m_javaBridge, m_showSoftKeyboard, v, (jboolean)bShow);
    return ret;
}

void AndroidPlatformAPI::executePendingTask() {
    MSG msg;
    while(GetMessage(&msg,0,0,0)){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    SApplication *pApp = SApplication::getSingletonPtr();
    if (pApp) {
        IMessageLoop *pMsgLoop = pApp->GetMsgLoop();
        if (pMsgLoop) {
            pMsgLoop->ExecutePendingTask();
        }
    }
}
