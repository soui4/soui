#include <jni.h>
#include <cstdlib>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <helper/slog.h>
#include "SouiSurfaceProxy.h"
#include "AndroidPlatformAPI.h"
#define kLogTag "soui4android"

extern "C" void RegisterAndroidPlatformAPI();
extern "C" void UnregisterAndroidPlatformAPI();



extern "C" {

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeRegisterPlatformAPI(JNIEnv *env, jobject thiz, jobject ctx) {
    SLOGI()<<"nativeRegisterPlatformAPI called";
    AndroidPlatformAPI::instance().init(env, thiz, ctx);
    RegisterAndroidPlatformAPI();
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeUnregisterPlatformAPI(JNIEnv *env, jobject thiz) {
    SLOGI()<<"nativeUnregisterPlatformAPI called";
    UnregisterAndroidPlatformAPI();
    AndroidPlatformAPI::instance().deinit();
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeOnTimerExpired(JNIEnv *env, jobject thiz, jlong hWnd, jlong timerId) {
    AndroidPlatformAPI::instance().onTimerExpired(
        reinterpret_cast<HWND>(static_cast<UINT_PTR>(hWnd)),
        static_cast<UINT_PTR>(timerId));
}

JNIEXPORT jlong JNICALL
Java_com_soui_android_SouiNativeHandle_NativeCreate(JNIEnv *env, jclass cls, jobject obj) {
    SouiSurfaceProxy* rawNative = nullptr;
    try {
        rawNative = new SouiSurfaceProxy(
            env, obj);
        jlong nativeId = rawNative->getNativeId();
        std::shared_ptr<SouiSurfaceProxy> nativeView(rawNative);
        rawNative = nullptr; // 所有权交给 shared_ptr

        if (!AndroidPlatformAPI::instance().nativeViewInsert(nativeId, nativeView)) {
            SLOGE()<<"SouiView_nativeCreate duplicate nativeId="<<(long long)nativeId<<", aborting";
            nativeView.reset(); // 立即释放，走析构
            return 0;
        }
        SLOGI()<<"SouiView_nativeCreate succeeded, nativeId="<<reinterpret_cast<void*>(nativeId)<<" (HWND==GlobalRef)";
        return nativeId;
    } catch (const std::exception& e) {
        SLOGE()<<"SouiView_nativeCreate exception: "<<e.what();
        delete rawNative;
        rawNative = nullptr;
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiNativeHandle_NativeDestroy(JNIEnv *env, jclass cls, jlong nativeId) {
    SLOGI()<<"SouiView_nativeDestroy called, nativeId="<<(long long)nativeId;

    // 先从 map 移除（此时 map 不再持有引用），再让局部 shared_ptr 出作用域时析构
    // 这样即使 onSizeChanged/onTouchEvent 等正在其它线程持有 lookup 返回的 shared_ptr，
    // 它们仍然能安全地完成本次调用，不会 UAF。
    std::shared_ptr<SouiSurfaceProxy> nativeView = AndroidPlatformAPI::instance().nativeViewLookup(nativeId);
    if (!nativeView) {
        SLOGW()<<"SouiView_nativeDestroy called with invalid/already-destroyed nativeId="<<(long long)nativeId;
        return;
    }
    AndroidPlatformAPI::instance().nativeViewErase(nativeId);
    // nativeView 在这里仍持有 1 个引用计数，等函数退出会自动析构（包括 m_hostWnd.reset()）
    SLOGI()<<"SouiView_nativeDestroy completed, nativeId="<<(long long)nativeId;
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiBaseSurface_nativeOnSizeChanged(JNIEnv *env, jobject thiz, jlong nativeId, jint width, jint height) {
    SLOGD()<<"SouiView_nativeOnSizeChanged called, nativeId="<<(long long)nativeId<<", width="<<width<<", height="<<height;

    auto nativeView = AndroidPlatformAPI::instance().nativeViewLookup(nativeId);
    if (nativeView) {
        nativeView->onSizeChanged(width, height);
    } else {
        SLOGW()<<"SouiView_nativeOnSizeChanged: unknown nativeId="<<(long long)nativeId;
    }
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiBaseSurface_nativeOnMotionEventEx(JNIEnv *env, jobject thiz, jlong nativeId,
                                                jint action,
                                                jfloat x, jfloat y, jint pointerId,
                                                jint buttonState,
                                                jfloat vscroll, jfloat hscroll,
                                                jint metaState, jlong timestamp) {
    (void)env; (void)thiz;
    auto nativeView = AndroidPlatformAPI::instance().nativeViewLookup(nativeId);
    if (nativeView) {
        nativeView->onMotionEventEx(
            (int)action, (float)x, (float)y, (int)pointerId,
            (int)buttonState, (float)vscroll, (float)hscroll,
            (int)metaState, (long)timestamp);
    } else {
        SLOGW()<<"SouiView_nativeOnMotionEventEx: unknown nativeId="<<(long long)nativeId<<" action="<<(int)action;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_soui_android_SouiBaseSurface_nativeOnKeyEventEx(JNIEnv *env, jobject thiz, jlong nativeId,
                                             jint keyCode, jint action,
                                             jint metaState, jint repeatCount, jint scanCode,
                                             jint unicodeChar, jlong flags, jlong timestamp) {
    (void)env; (void)thiz;
    auto nativeView = AndroidPlatformAPI::instance().nativeViewLookup(nativeId);
    if (nativeView) {
        bool handled = nativeView->onKeyEventEx(
            (int)keyCode, (int)action, (int)metaState,
            (int)repeatCount, (int)scanCode, (int)unicodeChar,
            (long)flags, (long)timestamp);
        SLOGI()<<"SouiView_nativeOnKeyEventEx, keyCode="<<keyCode<<",handled="<<(int)handled;
        return handled ? JNI_TRUE : JNI_FALSE;
    } else {
        SLOGW()<<"SouiView_nativeOnKeyEventEx: unknown nativeId="<<(long long)nativeId<<" keyCode="<<(int)keyCode<<" action="<<(int)action;
        return JNI_FALSE;
    }
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiBaseSurface_nativeRender(JNIEnv *env, jobject thiz, jlong nativeId, jobject bitmap) {
    auto nativeView = AndroidPlatformAPI::instance().nativeViewLookup(nativeId);
    if (nativeView) {
        nativeView->render(env, bitmap);
    } else {
        SLOGW()<<"SouiView_nativeRender: unknown nativeId="<<(long long)nativeId;
    }
}


JNIEXPORT jobject JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeViewFromHwnd(JNIEnv *env, jclass clazz, jlong hwnd) {
    (void)env;
    (void)clazz;
    if (hwnd == 0) return nullptr;
    jobject view = AndroidPlatformAPI::instance().hwndAsView((UINT_PTR)hwnd);
    if (view == nullptr) {
        SLOGW()<<"nativeViewFromHwnd: hwnd="<<(long long)hwnd<<" INativeWindow.asView() returned null or raised exception";
    }
    return view;
}


JNIEXPORT jlong JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeScreenStartup(JNIEnv *env, jobject thiz,
                                                             jlong screenId,
                                                             jlong screenHwnd,
                                                             jstring jLayout) {
    (void)thiz;
    if (screenId == 0L) {
        SLOGW()<<"nativeSouiStartup: ignored (screenId="<<(long long)screenId<<")";
        return 0;
    }
    SLOGI()<<"nativeSouiStartup, screenId="<<screenId<<" screen hwnd="<<screenHwnd;
    RECT rc = {0,0,100,100};
    BOOL ret = RegisterVirtualHWND(screenHwnd,0,WS_POPUP,0,&rc,screenId);
    if(!ret)
        return 0;
    AndroidPlatformAPI::instance().setScreenHwnd(screenId,screenHwnd);

    const char* layout = nullptr;
    if (jLayout) {
        layout = env->GetStringUTFChars(jLayout, nullptr);
    }
    // 一个入口 = 注册 + 启动 原子完成。
    HWND hWnd = AndroidPlatformAPI::instance().screenStartup(env, screenId, layout);

    if (layout) {
        env->ReleaseStringUTFChars(jLayout, layout);
    }
    return hWnd;
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeScreenShutdown(JNIEnv *env, jobject thiz,
                                                              jlong screenId) {
    (void)thiz;
    if (screenId == 0L) {
        SLOGW()<<"nativeSouiShutdown: screenId==0 ignored";
        return;
    }
    HWND hwndScreen = AndroidPlatformAPI::instance().getScreenHwnd(screenId);
    SLOGI()<<"nativeSouiShutdown, screenId="<<screenId<<" screen hwnd="<<hwndScreen;
    // 一个入口 = 业务销毁 + unregister（DeleteGlobalRef / 擦 bookkeeping / 擦 Context）原子完成。
    AndroidPlatformAPI::instance().screenShutdown(env, screenId);
    AndroidPlatformAPI::instance().eraseScreen(screenId);
    UnregisterVirtualHWND(hwndScreen);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soui_android_NativeEditView_nativeNotifyEditEvent(
    JNIEnv *env, jclass clazz,
    jlong hEdit, jint code, jlong wparam, jlong lparam)
{
    (void)env; (void)clazz; (void)code;
    HWND hEditWnd  = reinterpret_cast<HWND>(static_cast<UINT_PTR>(hEdit));
    HWND hParent   = ::GetParent(hEditWnd);
    if (hParent != 0) {
        ::SendMessageA(hParent, WM_COMMAND,
                       (WPARAM)wparam, (LPARAM)lparam);
    }
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeReadCxxStringSlot(JNIEnv *env, jclass clazz, jint slotId) {
    (void)clazz;
    const std::string s = AndroidPlatformAPI::instance().ReadString((int)slotId);
    if (s.empty()) return env->NewStringUTF("");
    return env->NewStringUTF(s.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeWriteCxxStringSlot(JNIEnv *env, jclass clazz,
                                                          jint slotId, jstring jStr) {
    (void)clazz;
    if (jStr == nullptr) {
        AndroidPlatformAPI::instance().WriteString((int)slotId, "");
        return;
    }
    const char *utf = env->GetStringUTFChars(jStr, nullptr);
    if (utf == nullptr) {
        AndroidPlatformAPI::instance().WriteString((int)slotId, "");
        return;
    }
    AndroidPlatformAPI::instance().WriteString((int)slotId, std::string(utf));
    env->ReleaseStringUTFChars(jStr, utf);
}


JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeNotifyFocusGained(JNIEnv *env, jclass clazz,
                                                         jlong hwnd) {
    (void)env; (void)clazz;
    if (hwnd == 0L) return;
    HWND h = reinterpret_cast<HWND>(static_cast<UINT_PTR>(hwnd));
    // 走 SWinx 全局 SetFocus：内部既更新 m_hFocus、投递 WM_SETFOCUS/WM_KILLFOCUS，
    // 又会再调用 g_platformAPI.window.setFocus → bridge.setFocus。
    // 此时 Java 端 old==new（我们就是从 Java View.focus=true 来的），
    // bridge.setFocus 快速返回，不会死循环。
    ::SetFocus(h);
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeSetScreenSize(JNIEnv *env, jclass clazz, jlong screen_id, jlong hMainWnd,
                                                     jint width, jint height) {
    SLOGW()<<"Java_com_soui_android_SouiPlatformBridge_nativeSetScreenSize, hMainWnd="<<(int)hMainWnd<<",wid="<<width<<",hei="<<height;
    ::MoveWindow(hMainWnd,0,0,width,height,TRUE);
}


extern "C" JNIEXPORT jboolean JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeInitSouiApp(
        JNIEnv* env, jclass clazz,
        jobject assetManagerJ,      // android.content.res.AssetManager
        jstring appFilesDirJ)
{
    // 一个入口 = 注册 + 启动 原子完成。
    return AndroidPlatformAPI::instance().initSouiApp(env, assetManagerJ, appFilesDirJ);
}

extern "C" JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeUninitSouiApp(
        JNIEnv* env,jclass clazz)
{
    AndroidPlatformAPI::instance().uninitSouiApp(env);
}

extern "C" JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeProcessPendingMessages(JNIEnv *env, jobject thiz) {
    AndroidPlatformAPI::instance().executePendingTask();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeProcessIdle(JNIEnv *env, jobject thiz,jint idleCount) {
    return AndroidPlatformAPI::instance().processIdle(idleCount);
}

JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeSetKeyboardHeight(JNIEnv *env, jclass clazz,
                                                         jlong keyboard_height) {
    SLOGW()<<"nativeSetKeyboardHeight, height="<<(int)keyboard_height;
    HWND hFocus = GetFocus();
    HWND hRoot = GetAncestor(hFocus,GA_ROOT);
    if(hRoot){
        SendMessage(hRoot,WM_KEYBOARD_HEIGHT,keyboard_height,0);
    }
}

extern "C" JNIEXPORT jint JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeAllocStringSlot(JNIEnv *env, jclass clazz) {
    return AndroidPlatformAPI::instance().stringSlotAlloc("");
}

extern "C" JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeFreeStringSlot(JNIEnv *env, jclass clazz, jint slotId) {
    AndroidPlatformAPI::instance().stringSlotFree(slotId);
}

extern "C" JNIEXPORT void JNICALL
Java_com_soui_android_SouiPlatformBridge_nativeSendImeString(JNIEnv *env, jclass clazz, jlong hwnd, jint slotid) {
    AndroidPlatformAPI::instance().sendImeString((UINT_PTR) hwnd, slotid);
}

}//extern "C"



