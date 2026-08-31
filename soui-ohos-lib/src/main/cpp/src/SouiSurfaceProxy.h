//
// SouiSurfaceProxy.h
//
// SOUI 自绘 Surface 的 C++ 代理类，对应 Android SouiSurfaceProxy。
// 负责处理输入事件转换（触摸/键盘/鼠标）和渲染（PixelMap/ImageData 像素缓冲区写入）。
//
// 架构：
//   ArkTS 侧 SouiSurface 组件（基于 Canvas 自绘）
//     ↕ N-API
//   C++ SouiSurfaceProxy（事件转换 + 渲染）
//     ↕ SendMessage / WM_PAINT
//   swinx 消息循环 / SOUI 窗口过程
//

#ifndef SOUI_OHOS_SURFACE_PROXY_H
#define SOUI_OHOS_SURFACE_PROXY_H

#include <windows.h>
#include <string>
#include <memory>
#include <mutex>
#include <cstring>

// N-API 前置声明
#ifdef __cplusplus
extern "C" {
#endif
typedef struct napi_env__ * napi_env;
typedef struct napi_value__ * napi_value;
typedef struct napi_ref__ * napi_ref;
#ifdef __cplusplus
}
#endif

class SouiSurfaceProxy {
public:
    SouiSurfaceProxy(napi_env env, napi_value nativeWnd);
    ~SouiSurfaceProxy();

    void clean(napi_env env);
    
    void onSizeChanged(int width, int height);

    // 触摸/鼠标事件统一入口，对应 Android onMotionEvent 
    void onMotionEvent(int action, float x, float y, int pointerId,
                       int buttonState, float vscroll, float hscroll,
                       int metaState, long timestamp);

    // 键盘事件，对应 Android onKeyEvent 
    bool onKeyEvent(int keyCode, int action, int metaState,
                    int repeatCount, int scanCode, int unicodeChar,
                    long flags, long timestamp);

    //
// 渲染 SOUI 内容到像素缓冲区。
// 对应 Android render(env, bitmap)：
//   1. 获取 typed array 的底层指针
//   2. 创建 Cairo image surface / HBITMAP 包装该缓冲区
//   3. GetDC → SelectObject → SendMessage(WM_PAINT) → 还原
//   4. BGRA→RGBA 通道交换（Cairo ARGB32 vs ArkUI RGBA）
// rcUpdate 输出本次实际重绘的更新区域（物理像素，窗口客户区坐标）。
//                 空矩形（l>=r 或 t>=b）表示本次没有需要上屏的内容。
    void render(napi_env env, napi_value pixelBuffer, int width, int height,
                RECT *rcUpdate);

    // 获取 nativeId（即 napi_ref 的指针值，作为 HWND 使用） 
    int64_t getNativeId() const { return reinterpret_cast<int64_t>(m_ref); }

    // 获取 ArkTS 侧对象引用（local ref） 
    napi_value getLocalRef(napi_env env);

private:
    int convertKeyCode(int keyCode);
    WPARAM buildMouseWParam(int metaState, int extraMkBits = 0);
    static WPARAM buttonStateToMkBits(int buttonState);
    UINT pickKeyMessage(UINT baseDown, int metaState, int vk);
    LPARAM buildKeyLParam(int action, int metaState, int repeatCount,
                          int scanCode, bool extended, bool wasDown);
    static int buttonIndexFromButtonState(int buttonState);
    static bool buttonMessages(int btnIdx, UINT& outDown, UINT& outUp,
                               UINT& outDbl, WPARAM& outMk);
    void dispatchMouseButton(int action, int btnIdx, int ix, int iy,
                             WPARAM mkExtra, long timestamp);

    napi_ref m_ref;
    // 鼠标状态
    long m_lastDownTime[5];
    int  m_lastDownX[5];
    int  m_lastDownY[5];
    bool m_hoverTracked;
    int  m_buttonsDown;
    int  m_pointerIdPrimary;
};

#endif // SOUI_OHOS_SURFACE_PROXY_H
