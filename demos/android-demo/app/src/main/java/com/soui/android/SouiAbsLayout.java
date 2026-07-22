package com.soui.android;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.Nullable;

/**
 * 绝对布局基类。为 SouiAbsWindow 和 SouiScreen 提供统一的绝对定位引擎。
 * <ul>
 *   <li>子 View 使用 LayoutParams 的 (x, y, width, height) 绝对定位</li>
 *   <li>子类可覆盖 measureChild 针对特定子项定制</li>
 * </ul>
 */
public abstract class SouiAbsLayout extends ViewGroup {

    public SouiAbsLayout(Context context) {
        super(context);
    }

    public SouiAbsLayout(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
    }

    public SouiAbsLayout(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    public SouiAbsLayout(Context context, @Nullable AttributeSet attrs, int defStyleAttr,
                         int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    /**
     * 子 View 布局参数。(x, y) 相对父容器左上角像素偏移，width/height 遵循 Android 语义。
     */
    public static class LayoutParams extends ViewGroup.MarginLayoutParams {
        public int x;
        public int y;

        public LayoutParams(int width, int height) {
            super(width, height);
            this.x = 0;
            this.y = 0;
        }

        public LayoutParams(int x, int y, int width, int height) {
            super(width, height);
            this.x = x;
            this.y = y;
        }

        /** Convert framework LayoutParams (preserves width/height + margins, places at origin). */
        public LayoutParams(ViewGroup.LayoutParams source) {
            super(source);
            this.x = 0;
            this.y = 0;
        }
    }

    @Override
    protected ViewGroup.LayoutParams generateDefaultLayoutParams() {
        return new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
    }

    @Override
    protected boolean checkLayoutParams(ViewGroup.LayoutParams p) {
        return p instanceof LayoutParams;
    }

    @Override
    protected ViewGroup.LayoutParams generateLayoutParams(ViewGroup.LayoutParams p) {
        if (p instanceof LayoutParams) {
            return new LayoutParams((LayoutParams) p);
        }
        return new LayoutParams(p);
    }

    public LayoutParams obtainChildLayoutParams(View child) {
        ViewGroup.LayoutParams raw = child.getLayoutParams();
        if (raw instanceof LayoutParams) {
            return (LayoutParams) raw;
        }
        LayoutParams lp = new LayoutParams(raw != null ? raw.width : LayoutParams.WRAP_CONTENT,
                raw != null ? raw.height : LayoutParams.WRAP_CONTENT);
        child.setLayoutParams(lp);
        return lp;
    }

    public void updateChildFrame(View child, int x, int y, int width, int height) {
        LayoutParams lp = obtainChildLayoutParams(child);
        lp.x = x;
        lp.y = y;
        lp.width = width;
        lp.height = height;
        child.setLayoutParams(lp);
        requestLayout();
    }

    public void setChildPosition(View child, int x, int y, int width, int height) {
        updateChildFrame(child, x, y, width, height);
    }

    public void setChildPosition(View child, int x, int y) {
        LayoutParams lp = obtainChildLayoutParams(child);
        lp.x = x;
        lp.y = y;
        child.setLayoutParams(lp);
        requestLayout();
    }

    public void setChildSize(View child, int width, int height) {
        LayoutParams lp = obtainChildLayoutParams(child);
        lp.width = width;
        lp.height = height;
        child.setLayoutParams(lp);
        requestLayout();
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        final int width = MeasureSpec.getSize(widthMeasureSpec);
        final int height = MeasureSpec.getSize(heightMeasureSpec);

        final int count = getChildCount();
        for (int i = 0; i < count; i++) {
            final View child = getChildAt(i);
            if (child.getVisibility() == GONE) continue;
            final LayoutParams lp = (LayoutParams) child.getLayoutParams();
            measureChild(child, width, height, lp);
        }

        setMeasuredDimension(width, height);
    }

    /**
     * 测量单个子 View。默认直接使用 {@code lp.width/lp.height}。
     * 子类覆盖可为特定子项定制（例如 SouiAbsWindow 强制主 Surface 填满父容器）。
     */
    protected void measureChild(View child, int parentW, int parentH, LayoutParams lp) {
        final int childWs;
        if (lp.width == LayoutParams.MATCH_PARENT) {
            childWs = MeasureSpec.makeMeasureSpec(Math.max(0, parentW), MeasureSpec.EXACTLY);
        } else if (lp.width == LayoutParams.WRAP_CONTENT) {
            childWs = MeasureSpec.makeMeasureSpec(Math.max(0, parentW), MeasureSpec.AT_MOST);
        } else {
            childWs = MeasureSpec.makeMeasureSpec(Math.max(0, lp.width), MeasureSpec.EXACTLY);
        }

        final int childHs;
        if (lp.height == LayoutParams.MATCH_PARENT) {
            childHs = MeasureSpec.makeMeasureSpec(Math.max(0, parentH), MeasureSpec.EXACTLY);
        } else if (lp.height == LayoutParams.WRAP_CONTENT) {
            childHs = MeasureSpec.makeMeasureSpec(Math.max(0, parentH), MeasureSpec.AT_MOST);
        } else {
            childHs = MeasureSpec.makeMeasureSpec(Math.max(0, lp.height), MeasureSpec.EXACTLY);
        }

        child.measure(childWs, childHs);
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        final int count = getChildCount();
        for (int i = 0; i < count; i++) {
            final View child = getChildAt(i);
            if (child.getVisibility() == GONE) continue;
            final LayoutParams lp = (LayoutParams) child.getLayoutParams();
            layoutChild(child, lp);
        }
    }

    /** 按 LayoutParams 定位单个子 View。默认放到 {@code (lp.x, lp.y)}，使用已测量尺寸。 */
    protected void layoutChild(View child, LayoutParams lp) {
        final int cl = lp.x;
        final int ct = lp.y;
        final int cr = cl + child.getMeasuredWidth();
        final int cb = ct + child.getMeasuredHeight();
        child.layout(cl, ct, cr, cb);
    }
}
