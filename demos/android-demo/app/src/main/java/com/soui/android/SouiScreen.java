package com.soui.android;

import static android.view.WindowInsetsAnimation.Callback.DISPATCH_MODE_CONTINUE_ON_SUBTREE;

import android.app.Activity;
import android.content.Context;
import android.content.res.TypedArray;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsAnimation;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.soui.demo.R;

import java.util.List;

/**
 * SOUI 窗口系统的"桌面容器"，对应 Win32 的 GetDesktopWindow。
 *
 * <p>设计要点：不继承 SouiAbsWindow，而是与 SouiAbsWindow 同为 SouiAbsLayout 的子类。
 * 原因：SouiAbsWindow 设计为携带一个主 Surface 作为自己的 HWND，而 SouiScreen 不需要
 * 自己的 Surface——它只是一个容器，顶层窗口（popup/overlapped/dialog）作为子 View 附加其上。
 * 两者共享绝对定位引擎（SouiAbsLayout），但生命周期语义完全不同。
 */
public class SouiScreen extends SouiWindow {
    long mSouiMainHwnd = 0;
    /** 当 SouiScreen 仅剩自身 Surface 时的回调接口。 */
    public interface OnEmptyListener {
        void onEmpty(SouiScreen screen);
    }

    /** 监听 View 层级变化，当子 View 移除后检查是否仅剩自身 Surface。 */
    public class EmptyStateWatcher implements ViewGroup.OnHierarchyChangeListener {
        @Override
        public void onChildViewAdded(View parent, View child) {
        }

        @Override
        public void onChildViewRemoved(View parent, View child) {
            if (parent instanceof SouiScreen) {
                SouiScreen screen = (SouiScreen) parent;
                if (screen.getChildCount() == 1 && screen.onEmptyListener != null) {
                    screen.onEmptyListener.onEmpty(screen);
                }
            }
        }
    }



    OnEmptyListener onEmptyListener = null;
    /** 用户指定的启动 screenId（=startSoui 参数）。0 表示尚未指定。 */
    private long mScreenId = 0L;

    /** 用户指定的 SOUI 布局资源字符串（=startSoui 参数）。空串表示"未指定"。 */
    private String mSouiLayout = "";


    // -----------------------------------------------------------------------
    //  构造函数 —— 支持直接传入 screenId/layout，或从 XML 属性读取。
    // -----------------------------------------------------------------------

    public SouiScreen(Context context, long screenId, String layoutId) {
        super(context);
        mScreenId = screenId;
        mSouiLayout = layoutId;
        Init(context, null);
    }

    public SouiScreen(Context context) {
        super(context);
        Init(context, null);
    }

    public SouiScreen(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        Init(context, attrs);
    }

    public SouiScreen(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        Init(context, attrs);
    }

    public SouiScreen(Context context, @Nullable AttributeSet attrs, int defStyleAttr,
                      int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        Init(context, attrs);
    }

    public void setOnEmptyListener(OnEmptyListener onEmptyListener) {
        this.onEmptyListener = onEmptyListener;
    }

    /**
     * 构造阶段的属性初始化：
     * <ul>
     *   <li>从 XML 属性读取 screenId 和 souiLayout，写入成员变量。</li>
     *   <li>设置层级变化监听器 EmptyStateWatcher。</li>
     *   <li>创建自身的 Surface（类型为 SouiBaseSurface）。</li>
     * </ul>
     */
    private void Init(Context context, @Nullable AttributeSet attrs) {
        setOnHierarchyChangeListener(new EmptyStateWatcher());
        if (attrs != null) {
            TypedArray ta = null;
            try {
                ta = context.obtainStyledAttributes(attrs, R.styleable.SouiScreen);
                final int xmlId = ta.getInt(R.styleable.SouiScreen_screenId, 0);
                if (xmlId != 0) {
                    mScreenId = (long) xmlId;
                }
                String xmlLayout = ta.getString(R.styleable.SouiScreen_souiLayout);
                if (xmlLayout != null) {
                    mSouiLayout = xmlLayout;
                }
            } finally {
                if (ta != null) ta.recycle();
            }
        }
        newSurface("screen", "", 0, 0, 0, 0, true, false);
    }

    @Override
    protected View createSurface(Context context, @Nullable String className,
                              @Nullable String title) {
        return new SouiBaseSurface(context);
    }
    public Activity getActivity() {
        Context context = getContext();
        if (context instanceof Activity) {
            return (Activity) context;
        }
        return null;
    }

    @Override
    protected void onAttachedToWindow() {
        super.onAttachedToWindow();
        Activity activity = getActivity();
        if(activity != null) {
            // 设置WindowInsetsAnimation监听器
            View rootView = activity.getWindow().getDecorView().getRootView();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R){
                rootView.setWindowInsetsAnimationCallback(new WindowInsetsAnimation.Callback(DISPATCH_MODE_CONTINUE_ON_SUBTREE) {
                    @Override
                    @NonNull
                    public WindowInsets onProgress(@NonNull WindowInsets insets,
                                                   @NonNull List<WindowInsetsAnimation> runningAnimations) {
                        // 在动画过程中实时获取键盘高度
                        int imeHeight = insets.getInsets(WindowInsets.Type.ime()).bottom;
                        SouiPlatformBridge.nativeSetKeyboardHeight(imeHeight);
                        return insets;
                    }
                });
            }
        }
        if (mScreenId != 0L) {
            mSouiMainHwnd = SouiPlatformBridge.getInstance().screenStartup(mScreenId, getSurfaceHwnd(), mSouiLayout);
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        if (mScreenId != 0L) {
            SouiPlatformBridge.getInstance().screenShutdown(mScreenId);
        }
        Activity activity = getActivity();
        if(activity != null){
            View rootView = activity.getWindow().getDecorView().getRootView();
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                rootView.setWindowInsetsAnimationCallback(null);
            }
        }
        super.onDetachedFromWindow();
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        syncScreenSize(w, h);
    }

    public void syncScreenSize(int w, int h) {
        Log.i("souiscreen","syncScreenSize,w="+w+" h="+h);
        if (w <= 0 || h <= 0) return;
        final long surfaceHwnd = getSurfaceHwnd();
        if (surfaceHwnd == 0L) return;
        try {
            SouiPlatformBridge.getInstance().moveWindow(surfaceHwnd, 0, 0, w, h);
            if (mSouiMainHwnd != 0) {
                post(() -> SouiPlatformBridge.nativeSetScreenSize(mScreenId, mSouiMainHwnd, w, h));
            }
        } catch (Throwable t) {
            Log.w("SouiScreen", "syncScreenSize: moveWindow sync failed (" + w + "x" + h + ")", t);
        }
    }
}
