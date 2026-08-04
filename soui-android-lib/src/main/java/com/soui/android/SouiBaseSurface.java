package com.soui.android;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.AttributeSet;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import androidx.annotation.Nullable;

/**
 * SOUI 渲染 Surface 的通用基类。实现 INativeWindow，负责输入事件分发和 native 生命周期管理。
 * <ul>
 *   <li>HWND = 本 View 的 INativeWindow GlobalRef 指针</li>
 *   <li>接收 Java 输入事件（触摸/键盘/鼠标/悬停/滚轮）并转发到 C++ 层</li>
 *   <li>渲染 C++ 生成的 bitmap 到 Android Canvas</li>
 * </ul>
 */
public class SouiBaseSurface extends View implements INativeWindow {

    protected long nativeId = 0;

    public SouiBaseSurface(Context context) {
        super(context);
        init();
    }

    public SouiBaseSurface(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public SouiBaseSurface(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    protected void init() {
        if (nativeId != 0) return;
        setFocusable(true);
        setFocusableInTouchMode(true);
        setClickable(true);
        setOnFocusChangeListener((v, hasFocus) -> {
            if (hasFocus && nativeId != 0L) {
                final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();
                if (bridge.getFocus() != nativeId) {
                    bridge.setFocus(nativeId);
                }
                SouiPlatformBridge.notifyFocusGained(nativeId);
            }
        });
        nativeId = SouiPlatformBridge.getInstance().createNative(this);
    }

    /**
     * 请求局部重绘（由 C++ 层通过 JNI 回调调用）。
     */
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

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        if (nativeId != 0) {
            nativeOnSizeChanged(nativeId, w, h);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (nativeId == 0) return super.onTouchEvent(event);
        return dispatchTouchEventToNative(event);
    }

    /**
     * 触摸事件分发。支持 SetCapture 重定向，按下时自动聚焦。
     */
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
                if (actionMasked == MotionEvent.ACTION_DOWN) {
                    if (!isFocused()) {
                        requestFocus(View.FOCUS_DOWN);
                    }
                    final SouiPlatformBridge bridge2 = SouiPlatformBridge.getInstance();
                    if (bridge2.getFocus() != nativeId) {
                        bridge2.setFocus(nativeId);
                    }
                }
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, actionMasked, pIdx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                nativeOnMotionEvent(nativeId, actionMasked,
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
                    if (captured != null) continue; // 已重定向，本指针跳过自己的 HWND
                    nativeOnMotionEvent(nativeId, MotionEvent.ACTION_MOVE,
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
                nativeOnMotionEvent(nativeId, actionMasked,
                        event.getX(pIdx), event.getY(pIdx), pId,
                        buttonState,
                        0.f, 0.f, metaState, eventTime);
                return true;
            }
            default:
                return super.onTouchEvent(event);
        }
    }

    /**
     * 处理鼠标按键、滚轮和悬停事件。支持 SetCapture 重定向。
     */
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
                // 单指针事件，pIdx 就是 actionIndex
                final int idx = Math.max(0, Math.min(pIdx, Math.max(0, event.getPointerCount()-1)));
                Boolean captured = bridge.tryDispatchCapturedMotion(
                        this, nativeId, event, action, idx,
                        buttonState, 0.f, 0.f, metaState, eventTime);
                if (captured != null) return true;
                nativeOnMotionEvent(nativeId, action,
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
                        nativeOnMotionEvent(nativeId, MotionEvent.ACTION_SCROLL,
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
                nativeOnMotionEvent(nativeId, action,
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

        boolean handled = nativeOnKeyEvent(nativeId, keyCode, cppAction, metaState,
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
    protected void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (nativeId != 0) {
            SouiPlatformBridge.getInstance().destroyNative(nativeId);
            nativeId = 0;
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
    // 通用 nativeSendMessage：SOUI 自绘 Surface 不处理原生控件消息，默认 return 0
    @Override public long   nativeSendMessage(int msg, long wp, long lp)           { return 0L; }

    @Override
    public View asView() {
        return this;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (outAttrs != null) {
            outAttrs.inputType = android.text.InputType.TYPE_CLASS_TEXT
                    | android.text.InputType.TYPE_TEXT_VARIATION_NORMAL;
            outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN;
            outAttrs.hintText = "";
            outAttrs.label = "";
        }
        return new SouiInputConnection(this, false);
    }

    private static class SouiInputConnection extends BaseInputConnection {
        private final SouiBaseSurface mSurface;

        SouiInputConnection(SouiBaseSurface targetView, boolean fullEditor) {
            super(targetView, fullEditor);
            mSurface = targetView;
        }

        @Override
        public boolean commitText(CharSequence text, int newCursorPosition) {
            if (text != null && text.length() > 0) {
                final SouiPlatformBridge bridge = SouiPlatformBridge.getInstance();
                long focusHwnd = bridge.getFocus();
                if (focusHwnd == 0) {
                    focusHwnd = mSurface.nativeId;
                }
                bridge.sendImeString(focusHwnd, text.toString());
            }
            return true;
        }

        @Override
        public boolean setComposingText(CharSequence text, int newCursorPosition) {
            return true;
        }

        @Override
        public boolean finishComposingText() {
            return true;
        }
    }

    /** 尺寸变化时通知 C++ 层。 */
    public native void nativeOnSizeChanged(long nativeId, int width, int height);

    /** 输入事件扩展入口（鼠标/触摸/悬停/滚轮统一走这里）。 */
    public native void nativeOnMotionEvent(long nativeId, int action,
                                           float x, float y, int pointerId,
                                           int buttonState,
                                           float vscroll, float hscroll,
                                           int metaState, long timestamp);

    /** 键盘事件扩展入口（携带 repeatCount/scanCode/unicodeChar）。 */
    public native boolean nativeOnKeyEvent(long nativeId, int keyCode, int action,
                                           int metaState, int repeatCount, int scanCode,
                                           int unicodeChar, long flags, long timestamp);

    /** 渲染 C++ 生成的 bitmap。 */
    public native void nativeRender(long nativeId, Bitmap bitmap);
}
