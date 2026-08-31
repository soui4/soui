package com.soui.android;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;

import androidx.annotation.Nullable;

/**
 * SOUI 主渲染 Surface，对应 C++ 层 SHostWnd。在 onDraw 中渲染 C++ 生成的 bitmap。
 * 事件处理、native 生命周期等均由父类 SouiBaseSurface 实现。
 */
public class SouiSurface extends SouiBaseSurface {

    private Paint paint;
    private Bitmap offscreenBitmap;
    private int viewWidth;
    private int viewHeight;

    public SouiSurface(Context context) {
        super(context);
        init();
    }

    public SouiSurface(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public SouiSurface(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    @Override
    protected void init() {
        super.init();
        paint = new Paint();
        paint.setAntiAlias(true);
        paint.setFilterBitmap(true);
        setHovered(true);
        setClickable(false);
        setLongClickable(false);
    }

    /** 创建或复用离屏渲染 Bitmap，尺寸匹配时复用避免内存抖动。 */
    private void createOffscreenBitmap(int width, int height) {
        if (width <= 0 || height <= 0) return;
        if (offscreenBitmap != null
                && offscreenBitmap.getWidth() == width
                && offscreenBitmap.getHeight() == height
                && !offscreenBitmap.isRecycled()) {
            return;
        }
        if (offscreenBitmap != null && !offscreenBitmap.isRecycled()) {
            offscreenBitmap.recycle();
        }
        offscreenBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        viewWidth = width;
        viewHeight = height;
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        if (w > 0 && h > 0) {
            createOffscreenBitmap(w, h);
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (nativeId == 0) return;

        // 懒初始化 + 异常防御：
        //   - 如果 View 已完成 layout (getWidth>0,getHeight>0) 但还没创建位图，立即创建
        //   - 若位图已被 recycle，重建
        if (offscreenBitmap == null || offscreenBitmap.isRecycled()) {
            final int w = getWidth();
            final int h = getHeight();
            if (w > 0 && h > 0) {
                createOffscreenBitmap(w, h);
                if (viewWidth != w || viewHeight != h) {
                    nativeOnSizeChanged(nativeId, w, h);
                }
            }
        }

        if (offscreenBitmap != null && !offscreenBitmap.isRecycled()) {
            nativeRender(nativeId, offscreenBitmap);
                canvas.drawBitmap(offscreenBitmap, 0, 0, paint);
            }
        }

    @Override
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (offscreenBitmap != null && !offscreenBitmap.isRecycled()) {
            offscreenBitmap.recycle();
        }
        offscreenBitmap = null;
    }
}
