#ifndef SOUI_VIEW_NATIVE_H
#define SOUI_VIEW_NATIVE_H

#include <jni.h>
#include <android/bitmap.h>
#include <string>
#include <memory>
#include <mutex>
#include <souistd.h>

/**
 * SOUI Surface 的 C++ 代理类。负责处理输入事件转换（触摸/键盘/鼠标）和渲染。
 */
class SouiSurfaceProxy {
public:
    SouiSurfaceProxy(JNIEnv* env, jobject nativeWnd);
    ~SouiSurfaceProxy();

    void onSizeChanged(int width, int height);

    // 新版扩展 API：统一鼠标/悬停/滚轮/触摸事件
    void onMotionEvent(int action, float x, float y, int pointerId,
                       int buttonState, float vscroll, float hscroll,
                       int metaState, long timestamp);

    // 新版扩展键盘 API：携带 repeatCount/scanCode/unicodeChar/flags
    // @return true SOUI 已消费；false 未处理，交给系统
    bool onKeyEvent(int keyCode, int action, int metaState,
                    int repeatCount, int scanCode, int unicodeChar,
                    long flags, long timestamp);

    void render(JNIEnv* env, jobject bitmap);

    jlong getNativeId() const { return reinterpret_cast<jlong>(m_javaRef); }
    jobject getLocallRef();
private:
    int convertKeyCode(int keyCode);
    WPARAM buildMouseWParam(int metaState, int extraMkBits = 0);
    static WPARAM buttonStateToMkBits(int buttonState);
    UINT pickKeyMessage(UINT baseDown, int metaState, int vk);
    LPARAM buildKeyLParam(int action, int metaState, int repeatCount, int scanCode, bool extended, bool wasDown);
    static int buttonIndexFromButtonState(int buttonState);
    static bool buttonMessages(int btnIdx, UINT& outDown, UINT& outUp, UINT& outDbl, WPARAM& outMk);
    void dispatchMouseButton(int action, int btnIdx, int ix, int iy, WPARAM mkExtra, long timestamp);

    jobject m_javaRef;
    // 鼠标状态
    long m_lastDownTime[5];       // 按钮上次 DOWN 时间戳（ms）
    int  m_lastDownX[5];          // 上次 DOWN X
    int  m_lastDownY[5];          // 上次 DOWN Y
    bool m_hoverTracked;          // 是否已调用 TrackMouseEvent(TME_HOVER)
    int  m_buttonsDown;           // 当前按下的按钮位图：bit0=LEFT,1=RIGHT,2=MID,3=X1,4=X2
    int  m_pointerIdPrimary;      // 主指针（多指时主触点）
};

#endif // SOUI_VIEW_NATIVE_H
