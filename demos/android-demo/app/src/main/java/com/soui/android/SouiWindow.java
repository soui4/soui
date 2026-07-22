package com.soui.android;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.Nullable;

/** 单个 SOUI 窗口（对应一个 HWND）的容器 ViewGroup，含 1 个主 Surface + 子窗口组。 */
public class SouiWindow extends SouiAbsWindow {

    /** index=0 的主 Surface View，承载像素绘制 / 原生控件，implements INativeWindow。 */
    protected View mSurface;

    SouiWindow(Context context){
        super(context);
    }

    public SouiWindow(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public SouiWindow(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public SouiWindow(Context context, @Nullable AttributeSet attrs, int defStyleAttr,
                      int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    /** 创建 Surface。子类覆盖以自定义创建逻辑。 */
    protected View createSurface(Context context, @Nullable String className,
                              @Nullable String title){
        final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();
        return bridge.createViewByClassName(className, title);
    }

    /**
     * 初始化窗口：创建主 Surface，添加到 index=0，设置初始位置和可见性。
     * @param className 窗口类名（如 "edit"）
     * @param title 窗口标题/初始文本
     * @param x/y/w/h 初始位置和尺寸
     * @param visible 初始可见性
     * @param disabled 初始禁用状态
     */
    public void newSurface(@Nullable String className,
                              @Nullable String title,
                              int x, int y, int w, int h,
                              boolean visible, boolean disabled) {
        final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();
        mSurface = createSurface(bridge.getContext(), className, title);

        SouiAbsLayout.LayoutParams surfaceLp = new SouiAbsLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        surfaceLp.x = 0; surfaceLp.y = 0;
        addView(mSurface, /*index=*/0, surfaceLp);

        ViewGroup.LayoutParams selfLp = getLayoutParams();
        if (selfLp instanceof SouiAbsLayout.LayoutParams) {
            // 父容器是绝对布局 → 精确定位
            SouiAbsLayout.LayoutParams absLp = (SouiAbsLayout.LayoutParams) selfLp;
            if (w > 0 && h > 0) {
                absLp.x = x;
                absLp.y = y;
                absLp.width = w;
                absLp.height = h;
            } else {
                absLp.x = x;
                absLp.y = y;
            }
        } else if (selfLp == null) {
            // 尚未被 addView → 创建默认 LP（后续 generateLayoutParams 会转成父容器子类）
            if (w > 0 && h > 0) {
                setLayoutParams(new SouiAbsLayout.LayoutParams(x, y, w, h));
            } else {
                SouiAbsLayout.LayoutParams absLp = new SouiAbsLayout.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT);
                absLp.x = x;
                absLp.y = y;
                setLayoutParams(absLp);
            }
        }
        // selfLp != null 但不是 SouiAbsLayout.LayoutParams：父容器是标准 ViewGroup，
        // 大小由外部 MATCH_PARENT 控制，不做任何改写。

        setVisibility(visible ? VISIBLE : GONE);
        setEnabled(!disabled);
    }

    /** @return 本 Group 对应的 Surface View（implements INativeWindow，HWND 的真身）。 */
    @Override
    public View getSurfaceView() { return mSurface; }

    /** @return 本 Group 主 Surface 的 native HWND；未初始化返回 0。 */
    public long getSurfaceHwnd() {
        View surface = getSurfaceView();
        return (surface instanceof INativeWindow)
                ? ((INativeWindow) surface).nativeGetHwnd()
                : 0L;
    }
}