package com.soui.android;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.Nullable;

/**
 * SOUI 主渲染 Surface，对应 C++ 层 SHostWnd。在 onDraw 中渲染 C++ 生成的 bitmap。
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
        setFocusable(true);
        setFocusableInTouchMode(true);
        setHovered(true);
        setClickable(false);
        setLongClickable(false);
        setOnGenericMotionListener(null);
    }

    /** 请求完整重绘。 */
    public void requestFullInvalidate() {
        invalidate();
    }

    /** 请求局部重绘（由 C++ 层通过 JNI 回调调用）。 */
    public void requestInvalidate(int left, int top, int right, int bottom) {
        if (left == 0 && top == 0 && right == 0 && bottom == 0) {
            invalidate();
        } else {
            invalidate(left, top, right, bottom);
        }
    }

    public long nativeGetHwnd() {
        return nativeId;
    }

    /** 创建或复用离屏渲染 Bitmap，尺寸匹配时复用避免内存抖动。 */
    private void createOffscreenBitmap(int width, int height) {
        if (width <= 0 || height <= 0) return;
        if (offscreenBitmap != null
                && offscreenBitmap.getWidth() == width
                && offscreenBitmap.getHeight() == height
                && !offscreenBitmap.isRecycled()) {
            // 尺寸完全匹配：直接复用，避免内存抖动
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

            if (nativeId != 0) {
                nativeOnSizeChanged(nativeId, w, h);
            }
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (nativeId == 0) return super.onTouchEvent(event);
        return dispatchTouchEventToNative(event);
    }

    /** 触摸事件分发。支持 SetCapture 重定向。 */
    private boolean dispatchTouchEventToNative(MotionEvent event) {
        final int actionMasked = event.getActionMasked();
        final long eventTime = event.getEventTime();
        final int metaState = event.getMetaState();
        final int buttonState = event.getButtonState();
        final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();

        switch (actionMasked) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL: {
                final int pIdx = event.getActionIndex();
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, actionMasked, pIdx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                nativeOnMotionEventEx(nativeId, actionMasked,
                        event.getX(pIdx), event.getY(pIdx), event.getPointerId(pIdx),
                        buttonState,
                        /*vscroll=*/0.f, /*hscroll=*/0.f,
                        metaState, eventTime);
                return true;
            }
            case MotionEvent.ACTION_MOVE: {
                for (int i = 0, N = event.getPointerCount(); i < N; ++i) {
                    Boolean captured = bridge.tryDispatchCapturedMotion(
                            this, nativeId, event, MotionEvent.ACTION_MOVE, i,
                            buttonState, 0.f, 0.f, metaState, eventTime);
                    if (captured != null) continue;
                    nativeOnMotionEventEx(nativeId, MotionEvent.ACTION_MOVE,
                            event.getX(i), event.getY(i), event.getPointerId(i),
                            buttonState,
                            0.f, 0.f, metaState, eventTime);
                }
                return true;
            }
            case MotionEvent.ACTION_POINTER_DOWN:
            case MotionEvent.ACTION_POINTER_UP: {
                final int pIdx = event.getActionIndex();
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, actionMasked, pIdx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                final int pId = event.getPointerId(pIdx);
                nativeOnMotionEventEx(nativeId, actionMasked,
                        event.getX(pIdx), event.getY(pIdx), pId,
                        buttonState,
                        0.f, 0.f, metaState, eventTime);
                return true;
            }
            default:
                return super.onTouchEvent(event);
        }
    }

    /** 处理鼠标按键、滚轮和悬停事件。支持 SetCapture 重定向。 */
    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
        if (nativeId == 0) return super.onGenericMotionEvent(event);
        final int action = event.getActionMasked();
        final long eventTime = event.getEventTime();
        final int metaState = event.getMetaState();
        final int buttonState = event.getButtonState();
        final int pIdx = event.getActionIndex();
        final int pId;
        if (event.getPointerCount() > 0) {
            pId = event.getPointerId(Math.max(0, Math.min(pIdx, event.getPointerCount()-1)));
        } else {
            pId = 0;
        }
        final float x = event.getAxisValue(MotionEvent.AXIS_X);
        final float y = event.getAxisValue(MotionEvent.AXIS_Y);
        final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();

        switch (action) {
            case MotionEvent.ACTION_BUTTON_PRESS:
            case MotionEvent.ACTION_BUTTON_RELEASE: {
                final int idx = Math.max(0, Math.min(pIdx, Math.max(0, event.getPointerCount()-1)));
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, action, idx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                nativeOnMotionEventEx(nativeId, action,
                        x, y, pId, buttonState,
                        0.f, 0.f, metaState, eventTime);
                return true;
            }
            case MotionEvent.ACTION_SCROLL: {
                final float vScroll = event.getAxisValue(MotionEvent.AXIS_VSCROLL);
                final float hScroll = event.getAxisValue(MotionEvent.AXIS_HSCROLL);
                if (vScroll != 0.f || hScroll != 0.f) {
                    final int idx = Math.max(0, Math.min(pIdx, Math.max(0, event.getPointerCount()-1)));
                    Boolean captured = bridge.tryDispatchCapturedMotion(
                            this, nativeId, event, MotionEvent.ACTION_SCROLL, idx,
                            buttonState, vScroll, hScroll, metaState, eventTime);
                    if (captured == null) {
                        nativeOnMotionEventEx(nativeId, MotionEvent.ACTION_SCROLL,
                                x, y, pId, buttonState,
                                vScroll, hScroll, metaState, eventTime);
                    }
                }
                return true;
            }
            case MotionEvent.ACTION_HOVER_MOVE:
            case MotionEvent.ACTION_HOVER_ENTER:
            case MotionEvent.ACTION_HOVER_EXIT: {
                final int idx = Math.max(0, Math.min(pIdx, Math.max(0, event.getPointerCount()-1)));
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, action, idx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                nativeOnMotionEventEx(nativeId, action,
                        x, y, pId, buttonState,
                        0.f, 0.f, metaState, eventTime);
                return true;
            }
            default:
                return super.onGenericMotionEvent(event);
        }
    }

    /**
     * 键盘事件分发。将事件转换为 C++ 层格式（含 repeatCount/scanCode/unicodeChar）。
     * @return true SOUI 已消费；false 未处理，交给系统默认行为
     */
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        if (nativeId == 0) return super.dispatchKeyEvent(event);

        final int action = event.getAction(); // 0=ACTION_DOWN, 1=ACTION_UP, 2=ACTION_MULTIPLE
        final int keyCode = event.getKeyCode();
        final int metaState = event.getMetaState();
        final int repeatCount = Math.max(0, event.getRepeatCount());
        final int scanCode = event.getScanCode();
        final long eventTime = event.getEventTime();

        int unicodeChar = 0;
        if (action == KeyEvent.ACTION_DOWN || action == KeyEvent.ACTION_MULTIPLE) {
            try {
                unicodeChar = event.getUnicodeChar(metaState);
                if (unicodeChar < 0) unicodeChar = 0;
                if (unicodeChar > 0xFFFF) unicodeChar = 0;
            } catch (Throwable t) {
                unicodeChar = 0;
            }
        }

        int cppAction = action;
        int cppRepeat = 1;
        if (action == KeyEvent.ACTION_DOWN) {
            cppRepeat = repeatCount + 1;
            if (repeatCount > 0) cppAction = 2;
        } else if (action == KeyEvent.ACTION_MULTIPLE) {
            cppRepeat = repeatCount;
            if (cppRepeat < 1) cppRepeat = 1;
        }

        boolean handled = nativeOnKeyEventEx(nativeId, keyCode, cppAction, metaState,
                cppRepeat, scanCode, unicodeChar, /*flags=*/0L, eventTime);
        if (!handled) {
            try {
                return super.dispatchKeyEvent(event);
            } catch (Throwable t) {
                return false;
            }
        }
        return true;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (nativeId == 0) return super.onKeyDown(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (nativeId == 0) return super.onKeyUp(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event) {
        if (nativeId == 0) return super.onKeyLongPress(keyCode, event);
        return false;
    }

    @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event) {
        if (nativeId == 0) return super.onKeyMultiple(keyCode, repeatCount, event);
        return false;
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
                // 第一次创建位图时若 native 尚未收到尺寸事件，补齐一次
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
        if (offscreenBitmap != null) {
            offscreenBitmap.recycle();
            offscreenBitmap = null;
        }
    }

    private final NativeWindowDelegate mNativeDelegate = new NativeWindowDelegate(this);

    @Override public void nativeDestroy()                              { mNativeDelegate.nativeDestroy(); }
    @Override public void nativeInvalidate(int l, int t, int r, int b) { mNativeDelegate.nativeInvalidate(l,t,r,b); }
    @Override public boolean nativeShow(int cmdShow)                   { return mNativeDelegate.nativeShow(cmdShow); }
    @Override public boolean nativeMove(int x, int y, int w, int h)    { return mNativeDelegate.nativeMove(x,y,w,h); }
    @Override public boolean nativeSetSize(int cx, int cy)             { return mNativeDelegate.nativeSetSize(cx,cy); }
    @Override public boolean nativeSetPosition(int x, int y)           { return mNativeDelegate.nativeSetPosition(x,y); }
    @Override public boolean nativeIsVisible()                         { return mNativeDelegate.nativeIsVisible(); }
    @Override public boolean nativeEnable(boolean enabled)             { return mNativeDelegate.nativeEnable(enabled); }
    @Override public boolean nativeIsEnabled()                         { return mNativeDelegate.nativeIsEnabled(); }
    @Override public long nativeGetWindow(long hwnd,int code)          { return mNativeDelegate.nativeGetWindow(hwnd,code); }

    @Override
    public View asView() {
        return this;
    }
}
