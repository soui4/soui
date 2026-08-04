package com.soui.android;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

/**
 * SOUI 窗口的容器基类。对应 1 个 C++ 层 SHostWnd/SNativeWnd。
 */
public class SouiAbsWindow extends SouiAbsLayout {

    public SouiAbsWindow(Context context) {
        super(context);
    }

    public SouiAbsWindow(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public SouiAbsWindow(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public SouiAbsWindow(Context context, @Nullable AttributeSet attrs, int defStyleAttr,
                         int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    /** @return 本容器对应的 Surface View（index=0）；没有则返回 null。 */
    @Nullable
    public View getSurfaceView() {
        return getPrimarySurface();
    }

    /** @return 索引 0 的主 Surface View，承载当前窗口的 INativeWindow 实现。 */
    @Nullable
    public View getPrimarySurface() {
        final int cnt = getChildCount();
        if (cnt == 0) return null;
        final View v = getChildAt(0);
        return isPrimarySurface(v) ? v : null;
    }

    /** @return 给定 child 是否符合“主 Surface”契约（index=0 且实现 INativeWindow）。 */
    protected boolean isPrimarySurface(View child) {
        return child != null && child instanceof INativeWindow;
    }

    /**
     * SouiAbsWindow 对 Surface 的 measure 规则：Surface 必须填满整个容器
     * （MATCH_PARENT × MATCH_PARENT），因为 Surface 是该容器对应 HWND 的客户区。
     */
    @Override
    protected void measureChild(View child, int parentW, int parentH, LayoutParams lp) {
        // 若该 child 是 Surface（index=0 且 implements INativeWindow） → 强制 MATCH_PARENT
        final int idx = indexOfChild(child);
        if (idx == 0 && isPrimarySurface(child)) {
            final int ws = MeasureSpec.makeMeasureSpec(
                    Math.max(parentW,0), MeasureSpec.EXACTLY);
            final int hs = MeasureSpec.makeMeasureSpec(
                    Math.max(parentH,0), MeasureSpec.EXACTLY);
            child.measure(ws, hs);
            return;
        }
        // 其它 child（子窗口容器，index≥1）：按自身 LayoutParams 走默认
        super.measureChild(child, parentW, parentH, lp);
    }
}
