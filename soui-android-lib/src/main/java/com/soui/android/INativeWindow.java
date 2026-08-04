package com.soui.android;

import android.view.View;

/**
 * Java 层承接 C++ 平台层对 Native Window 状态操作的接口。
 * <p>设计契约：HWND = INativeWindow 的 JNI GlobalRef 指针，一一对应。
 */
public interface INativeWindow {

    /** 获取 HWND。 */
    long nativeGetHwnd();

    /** 销毁窗口。 */
    void nativeDestroy();

    /** 请求局部（或全部）重绘。全为 0 表示整体重绘。 */
    void nativeInvalidate(int left, int top, int right, int bottom);

    /**
     * 显示/隐藏窗口。
     * @param cmdShow 0 = GONE，其他值 = VISIBLE
     * @return 操作是否成功
     */
    boolean nativeShow(int cmdShow);

    /** 移动并改变大小。 */
    boolean nativeMove(int x, int y, int width, int height);

    /** 仅改尺寸。 */
    boolean nativeSetSize(int cx, int cy);

    /** 仅改位置。 */
    boolean nativeSetPosition(int x, int y);

    /** 当前是否可见。 */
    boolean nativeIsVisible();

    /** 设置 enabled 状态。 */
    boolean nativeEnable(boolean enabled);

    /** 当前是否 enabled。 */
    boolean nativeIsEnabled();

    /** 获取窗口相关信息（对应 Win32 GetWindow）。 */
    long nativeGetWindow(long hWnd, int code);

    /** 返回对应的 Android View。 */
    View asView();

    /**
     * 通用消息转发。字符串参数通过字符串槽交换，纯数值消息按 Win32 语义传递。
     * @param msg 消息号（WM_* / EM_*）
     * @param wParam WPARAM
     * @param lParam LPARAM
     * @return LRESULT
     */
    long nativeSendMessage(int msg, long wParam, long lParam);
}
