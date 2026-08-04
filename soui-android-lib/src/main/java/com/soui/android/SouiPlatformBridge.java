package com.soui.android;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;
import android.os.MessageQueue;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.content.ClipboardManager;
import android.content.ClipData;
import android.media.MediaPlayer;

import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.function.Function;

/**
 * C++ 平台 API 与 Android View 系统之间的全局 JNI 桥接类。
 *
 * 作为 SOUI 窗口系统的平台抽象层，设计为单例，在应用启动时初始化一次。
 */
public class SouiPlatformBridge {
    static {
        System.loadLibrary("soui4android");
    }

    private static final String TAG = "SouiPlatformBridge";
    private static SouiPlatformBridge sInstance;

    private Context mContext;
    private MediaPlayer mCurrentMediaPlayer;
    private final HashMap<String, MediaPlayer> mActivePlayers = new HashMap<>();

    /** Windows WS_* 风格位常量（值与 Win32 完全一致） */
    public static final int WS_OVERLAPPED    = 0x00000000;
    public static final int WS_POPUP         = 0x80000000;
    public static final int WS_CHILD         = 0x40000000;
    public static final int WS_MINIMIZE      = 0x20000000;
    public static final int WS_VISIBLE       = 0x10000000;
    public static final int WS_DISABLED      = 0x08000000;
    public static final int WS_CLIPSIBLINGS  = 0x04000000;
    public static final int WS_CLIPCHILDREN  = 0x02000000;
    public static final int WS_BORDER        = 0x00800000;
    public static final int WS_CAPTION       = 0x00C00000; // BORDER | DLGFRAME
    public static final int WS_SYSMENU       = 0x00080000;
    public static final int WS_THICKFRAME    = 0x00040000;

    /** HWND_DESKTOP 等价：父容器为 SouiScreen */
    public static final long HWND_DESKTOP = 0L;
    /** HWND_MESSAGE 等价：创建不可见的消息占位窗口，不挂到视觉树 */
    public static final long HWND_MESSAGE = -3L;

    /**
     * HWND -> View 映射。在新架构下，C++ 层直接把 HWND 当作 INativeWindow GlobalRef 使用，
     * 本 map 仅用于 Java 层业务注册的自定义 View 反查和 findViewByHandle 的 fallback。
     */
    private final HashMap<Long, View> mViewMap;

    /** 消息窗口集合：存储 HWND_MESSAGE 创建的窗口，不挂到视觉树，仅用于消息处理（如定时器）。 */
    private final HashMap<Long, INativeWindow> mMessageOnlyWindows = new HashMap<>();

    /**
     * 窗口类名 → View 模板工厂注册表。C++ 层通过 className 查表创建 View，
     * 未命中时 fallback 到 SouiSurface（普通渲染 Surface）。
     */
    private final HashMap<String, Function<String, View>> mViewFactoryRegistry = new HashMap<>();

    /** 定时器管理：嵌套 Map (hWnd -> (timerId -> TimerRunnable))，语义与 Win32 SetTimer 一致。 */
    private final Handler mBridgeHandler = new Handler(Looper.getMainLooper());
    private final HashMap<Long, HashMap<Long, TimerRunnable>> mBridgeTimerMap = new HashMap<>();
    /** 下一个自动分配的 timerId（从 1 开始）。 */
    private long mNextTimerId = 1;

    /** 当前被捕获的 HWND。0 表示没有捕获（语义与 Win32 SetCapture 一致）。 */
    private long mCaptureHwnd = 0L;

    /** 当前持有输入焦点的 HWND。0 表示没有焦点（语义与 Win32 SetFocus 一致）。 */
    private volatile long mFocusHwnd = 0L;

    private int mIdleCount = 0;
    /**
     * Win32 语义：SetCapture(hWnd)。将所有输入事件重定向到指定 HWND。
     * @param hwnd 目标窗口 HWND，传 0 等价于 releaseCapture()
     * @return 之前被捕获窗口的 HWND，无捕获返回 0
     */
    @SuppressWarnings("unused")
    public synchronized long setCapture(long hwnd) {
        final long old = mCaptureHwnd;
        mCaptureHwnd = hwnd;
        return old;
    }

    /**
     * Win32 语义：ReleaseCapture()。解除输入捕获。
     * @return true 有捕获并已释放；false 本来就没有捕获
     */
    @SuppressWarnings("unused")
    public synchronized boolean releaseCapture() {
        if (mCaptureHwnd == 0L) return false;
        mCaptureHwnd = 0L;
        return true;
    }

    /** @return 当前被捕获的 HWND；无捕获返回 0。线程安全读取。 */
    public synchronized long getCapture() {
        return mCaptureHwnd;
    }

    /**
     * Win32 语义：SetFocus(hWnd)。将输入焦点切到指定 HWND，并同步 Android View 焦点和 IME 状态。
     * @param hwnd 目标窗口 HWND，或 0 清空焦点
     * @return 之前持有焦点的 HWND，无焦点返回 0
     */
    @SuppressWarnings("unused")
    public synchronized long setFocus(final long hwnd) {
        final long old = mFocusHwnd;
        if (old == hwnd) return old;
        mFocusHwnd = hwnd;

        // --- 同步 Android View 焦点 + IME（切到主线程执行） ---
        final Runnable apply = () -> applyFocusAndIme(old, hwnd);
        if (Looper.myLooper() == Looper.getMainLooper()) {
            apply.run();
        } else {
            mBridgeHandler.post(apply);
        }
        return old;
    }

    /**
     * 把 View 焦点 + IME 状态同步到当前 mFocusHwnd。必须在主线程调用。
     */
    private void applyFocusAndIme(long oldHwnd, long newHwnd) {
        final InputMethodManager imm = (InputMethodManager)
                (mContext == null ? null : mContext.getSystemService(Context.INPUT_METHOD_SERVICE));

        // 1) 旧焦点 View：清除焦点 + 先尝试收 IME（失败也不致命）
        if (oldHwnd != 0L) {
            final View oldView = findViewByHandle(oldHwnd);
            if (oldView != null) {
                if (imm != null) {
                    imm.hideSoftInputFromWindow(oldView.getWindowToken(), 0);
                }
                oldView.clearFocus();
            }
        }

        // 2) 新焦点 View：requestFocus + 根据是否可编辑决定弹/收 IME
        if (newHwnd == 0L) {
            // hwnd==0：全局无焦点，确保 IME 收起
            if (imm != null && oldHwnd != 0L) {
                imm.toggleSoftInput(InputMethodManager.HIDE_IMPLICIT_ONLY, 0);
            }
            return;
        }
        final View newView = findViewByHandle(newHwnd);
        if (newView == null) return;

        newView.requestFocus(View.FOCUS_DOWN);
        if (imm == null) return;

        // IME 策略：
        //   - EditText 子类（NativeEditView → className="edit"）一定弹 IME
        //   - 其它 SouiBaseSurface：SOUI 自己实现了 SEdit 等控件，用离屏渲染绘制文本，
        //     但仍依赖 Android IME 把按键输入转成字符 → 也弹 IME，让字符通过 KeyEvent /
        //     commitText 走 nativeOnKeyEventEx。若上层业务不需要，可单独覆盖 setInputType。
        final boolean editable = (newView instanceof EditText);
        if (editable) {
            imm.showSoftInput(newView, InputMethodManager.SHOW_IMPLICIT);
        } else {
            imm.hideSoftInputFromWindow(newView.getWindowToken(), 0);
        }
    }

    /**
     * Win32 语义：HWND GetFocus(VOID)。
     *
     * @return 当前持有输入焦点的 HWND；无焦点返回 0。线程安全读取。
     */
    @SuppressWarnings("unused") // 被 C++ AndroidPlatformAPI::getFocus 经由 JNI 调用
    public long getFocus() {
        return mFocusHwnd;
    }

    /**
     * 捕获事件重定向：如果存在 SetCapture 捕获窗口，将事件坐标转换到目标窗口客户区后投递。
     * @return Boolean.TRUE 表示事件已重定向；null 表示无需重定向
     */
    Boolean tryDispatchCapturedMotion(View sourceView, long sourceNativeId,
                                      MotionEvent event, int actionMasked, int pIdx,
                                      int buttonState,
                                      float vscroll, float hscroll,
                                      int metaState, long eventTime) {
        final long captureHwnd = getCapture();
        if (captureHwnd == 0L || captureHwnd == sourceNativeId) return null;

        View captureView = findViewByHandle(captureHwnd);
        if (!(captureView instanceof SouiBaseSurface)) {
            // 目标 HWND 已销毁或不属于 SouiBaseSurface（异常），兜底释放捕获。
            releaseCapture();
            return null;
        }
        // --- 坐标转换：event.getX/Y(pIdx)（相对 sourceView）→ 目标窗口客户区 ---
        //   1. sourceView 屏幕坐标 + event 相对 source 的坐标 = 屏幕像素点
        int[] sourceLoc = new int[2];
        sourceView.getLocationOnScreen(sourceLoc);
        final float screenX = sourceLoc[0] + event.getX(pIdx);
        final float screenY = sourceLoc[1] + event.getY(pIdx);
        //   2. 屏幕像素点 - captureView 屏幕左上角 = captureView 客户区坐标
        int[] captureLoc = new int[2];
        captureView.getLocationOnScreen(captureLoc);
        final float localX = screenX - captureLoc[0];
        final float localY = screenY - captureLoc[1];
        final int pId = event.getPointerId(pIdx);

        // --- 投递到目标 HWND（完全对齐原分发的 nativeOnMotionEventEx 调用签名）---
        ((SouiBaseSurface) captureView).nativeOnMotionEvent(
                captureHwnd, actionMasked, localX, localY, pId,
                buttonState, vscroll, hscroll, metaState, eventTime);
        return Boolean.TRUE;
    }

    /** 定时器任务，与 Win32 SetTimer 语义一致，默认循环触发。 */
    private class TimerRunnable implements Runnable {
        final long mTimerId;
        final long mHWnd;
        final long mInterval;
        volatile boolean mActive;

        TimerRunnable(long timerId, long hWnd, long interval) {
            mTimerId = timerId;
            mHWnd = hWnd;
            mInterval = interval;
            mActive = true;
        }

        void cancel() { mActive = false; }

        @Override
        public void run() {
            if (!mActive) return;
            synchronized (mBridgeTimerMap) {
                HashMap<Long, TimerRunnable> perHwnd = mBridgeTimerMap.get(mHWnd);
                if (perHwnd == null || perHwnd.get(mTimerId) != this || !mActive) return;
            }
            nativeOnTimerExpired(mHWnd, mTimerId);
            if (mActive) {
                mBridgeHandler.postDelayed(this, mInterval);
            }
        }
    }

    private SouiPlatformBridge() {
        mViewMap = new HashMap<>();
        registerViewFactory("Edit", NativeEditView::new);
    }

    /**
     * 给 C++ 层 createWindow 注册一个自定义 View 模板工厂。
     *
     * @param className   窗口类名（区分大小写），例如 "button"、"richedit"
     * @param factory     工厂，参数是 title（窗口标题/初始文本），返回实现 INativeWindow 的 View
     */
    @SuppressWarnings("unused")
    public void registerViewFactory(String className, Function<String, View> factory) {
        if (className == null || factory == null) return;
        className = className.toLowerCase();
        mViewFactoryRegistry.put(className, factory);
    }

    /**
     * 取消注册一个 View 模板工厂。
     */
    @SuppressWarnings("unused")
    public void unregisterViewFactory(String className) {
        if (className == null) return;
        className = className.toLowerCase();
        mViewFactoryRegistry.remove(className);
    }

    public Context getContext(){
        return mContext;
    }
    /** 获取单例实例。使用前必须先调用 init()。 */
    public static SouiPlatformBridge getInstance() {
        if (sInstance == null) {
            synchronized (SouiPlatformBridge.class) {
                if (sInstance == null) {
                    sInstance = new SouiPlatformBridge();
                }
            }
        }
        return sInstance;
    }

    /** 初始化平台桥，传入应用上下文。应在应用启动时调用一次。 */
    public void init(Context context,AssetManager am, String appFilesDir) {
        mContext = context.getApplicationContext();
        nativeRegisterPlatformAPI(mContext);
        nativeInitSouiApp(am,appFilesDir);
    }

    /** 销毁平台桥。应在应用退出时调用。 */
    public void destroy() {
        nativeUninitSouiApp();
        nativeUnregisterPlatformAPI();
        synchronized (mBridgeTimerMap) {
            for (HashMap<Long, TimerRunnable> perHwnd : mBridgeTimerMap.values()) {
                for (TimerRunnable runnable : perHwnd.values()) {
                    runnable.cancel();
                    mBridgeHandler.removeCallbacks(runnable);
                }
            }
            mBridgeTimerMap.clear();
        }
        mViewMap.clear();
        synchronized (this) { mCaptureHwnd = 0L; mFocusHwnd = 0L; }
        mContext = null;
        synchronized (SouiPlatformBridge.class) { sInstance = null; }
    }

    /**
     * @deprecated 新架构下不需要。容器不再对应 HWND，子窗口通过 getParent() 自动查找。
     */
    @Deprecated
    public void registerViewGroup(long hwnd, SouiAbsLayout viewGroup) {
        mViewMap.put(hwnd, viewGroup);
    }

    /** @deprecated 同 registerViewGroup。 */
    @Deprecated
    public void unregisterViewGroup(long hwnd) {
        mViewMap.remove(hwnd);
    }

    /**
     * SOUI 启动入口。一次调用完成 screen 注册、布局加载和主窗口创建。
     * @param screenId    屏幕 ID（非 0）
     * @param screenHwnd  屏幕 Surface 的 HWND
     * @param layout      SOUI 布局资源 ID（如 "layout:dlg_main"）
     * @return 主窗口 HWND
     */
    public long screenStartup(long screenId, long screenHwnd, String layout) {
        if (screenId == 0L) {
            Log.w(TAG, "screenStartup: screenId==0 ignored");
            return 0;
        }
        Looper.myQueue().addIdleHandler(new MessageQueue.IdleHandler() {
            @Override
            public boolean queueIdle() {
                boolean ret = nativeProcessIdle(mIdleCount++);
                if(!ret){
                    mIdleCount = 0;
                }
                return ret;
            }
        });
        return nativeScreenStartup(screenId, screenHwnd, layout);
    }

    /**
     * 关闭 SOUI（与 screenStartup 配对）。
     * @param screenId 屏幕 ID
     */
    public void screenShutdown(long screenId) {
        if (screenId == 0L) return;
        try {
            nativeScreenShutdown(screenId);
        } catch (Throwable t) {
            Log.w(TAG, "screenShutdown failed", t);
        }
    }

    /**
     * 调度一次消息处理任务。由 C++ 层 postMessage 调用，通过 Handler 延迟到主线程执行，
     * 在任务执行时清空待处理消息队列并处理空闲任务。
     */
    @SuppressWarnings("unused")
    public void scheduleMessageProcessing() {
        mBridgeHandler.post(() -> {
            nativeProcessPendingMessages();
        });
    }

    /**
     * 原生方法：处理所有待处理的消息。清空 C++ 层消息队列并分发消息。
     */
    private native void nativeProcessPendingMessages();

    /**
     * 原生方法：处理空闲时间任务，调用 SOUI 的 OnIdle 处理器。
     */
    private native boolean nativeProcessIdle(int idleCount);

    /**
     * 按 HWND 注册 View 到 fallback map。新架构下一般不需要调用。
     * @param hwnd  窗口句柄
     * @param view  Android View
     */
    public void registerView(long hwnd, View view) {
        mViewMap.put(hwnd, view);
    }

    /** 取消注册之前通过 registerView 注册的 View。 */
    public void unregisterView(long hwnd) {
        mViewMap.remove(hwnd);
    }

    /**
     * 请求重绘指定 HWND 对应的 View（对应 Win32 InvalidateRect）。
     * @param windowHandle 窗口句柄
     * @param left/top/right/bottom 重绘区域，全为 0 表示完整重绘
     */
    public void requestInvalidate(long windowHandle, int left, int top, int right, int bottom) {
        View view = findViewByHandle(windowHandle);
        if (view == null) return;
        if (left == 0 && top == 0 && right == 0 && bottom == 0) {
            view.invalidate();
        } else {
            view.invalidate(left, top, right, bottom);
        }
    }

    /**
     * 按 className 在 View 模板注册表中查找并创建 INativeWindow 实例。
     * @param className 窗口类名（如 "edit"），null/空串走默认 SouiSurface
     * @param title     窗口标题/初始文本
     * @return 新建的 INativeWindow View，从不返回 null
     */
    public View createViewByClassName(String className, String title) {
        Function<String, View> factory = null;
        if (className != null && !className.isEmpty()) {
            className = className.toLowerCase();
            factory = mViewFactoryRegistry.get(className);
        }
        View view = null;
        if (factory != null) {
            view = factory.apply(title);
        }
        if (view == null) {
            view = new SouiSurface(mContext);
        }
        return view;
    }

    /**
     * 创建窗口（C++ 层 JNI 调用入口）。语义与 Win32 CreateWindow 一致。
     * @param parentSurface 父 Surface View
     * @param hWndParent    父窗口 HWND（HWND_DESKTOP 表示顶层窗口）
     * @param lpClassName   窗口类名
     * @param lpWindowName  窗口标题
     * @param dwStyle       WS_* 风格位
     * @param dwExStyle     WS_EX_* 扩展风格位
     * @param X/Y           初始位置
     * @param nWidth/nHeight 初始尺寸（≤0 为 WRAP_CONTENT）
     * @param hMenu         控件 ID（WS_CHILD 时有效）
     * @return Surface HWND（GlobalRef），失败返回 0
     */
    @SuppressWarnings("unused")
    public long createWindow(View parentSurface, long hWndParent,
                             String lpClassName,
                             String lpWindowName,
                             int dwStyle, int dwExStyle,
                             int X, int Y, int nWidth, int nHeight,
                             long hMenu, long hInstance, long lpParam) {
        final boolean isChild = (dwStyle & WS_CHILD) != 0;
        if(hWndParent == HWND_MESSAGE){
            return createMessageOnlyWindow(lpClassName, lpWindowName, hMenu);
        }else {
            ViewGroup parent = (ViewGroup) parentSurface.getParent();
            return finishCreateWindow(parent,
                    hWndParent,
                    lpClassName, lpWindowName,
                    dwStyle, dwExStyle,
                    X, Y, nWidth, nHeight,
                    hMenu, hInstance, lpParam);
        }
    }

    /**
     * 实际窗口创建逻辑。创建 SouiWindow、设置初始属性、添加到父容器。
     * @param parent       父容器
     * @param hWndParent   父窗口 HWND（owner 语义）
     * @return 新窗口 HWND，失败返回 0
     */
    private long finishCreateWindow(ViewGroup parent,
                                    long hWndParent,
                                    String lpClassName,
                                    String lpWindowName,
                                    int dwStyle, int dwExStyle,
                                    int X, int Y, int nWidth, int nHeight,
                                    long hMenu, long hInstance, long lpParam) {
        try {
            // 1) 初始可见/禁用：根据 dwStyle 判定
            boolean vis = ((dwStyle & WS_VISIBLE) != 0);
            boolean dis = ((dwStyle & WS_DISABLED) != 0);

            // 2) className / title 不做推断，按用户原生参数下传
            String className = lpClassName;
            String title = lpWindowName;

            // 3) 新建 SouiWindow（容器 + Surface index=0）
            SouiWindow group = new SouiWindow(mContext);
            group.newSurface(className, title,
                    X, Y, nWidth, nHeight,
                    vis, dis);
            View surface = group.getSurfaceView();

            // 防御：surface 必须非 null 且 implements INativeWindow（SouiWindow 构造已强制）
            if (surface == null || !(surface instanceof INativeWindow)) {
                Log.e(TAG, "finishCreateWindow: SouiWindow.getSurfaceView()=" + surface
                        + " not INativeWindow, cls=" + lpClassName);
                return 0L;
            }

            // 3b) **HMENU=子窗口控件 ID**：按 hMenu 保存为 View.id（hMenu==0 → NO_ID）。
            //     容器(SouiWindow) 和 Surface 同步写 id，便于两种查找。
            int id = (hMenu == 0) ? View.NO_ID : (int)(hMenu & 0xFFFFFFFFL);
            group.setId(id);
            surface.setId(id);

            // 4) 把 SouiWindow 加到父容器
            if (parent instanceof SouiAbsLayout) {
                SouiAbsLayout.LayoutParams lp = new SouiAbsLayout.LayoutParams(
                        X, Y,
                        nWidth > 0 ? nWidth : SouiAbsLayout.LayoutParams.WRAP_CONTENT,
                        nHeight > 0 ? nHeight : SouiAbsLayout.LayoutParams.WRAP_CONTENT);
                parent.addView(group, lp);
            } else {
                // 非 SouiAbsLayout 的一般父容器，用标准 MATCH/WRAP + TranslationX/Y 定位
                ViewGroup.LayoutParams lp = new ViewGroup.LayoutParams(
                        nWidth > 0 ? nWidth : ViewGroup.LayoutParams.WRAP_CONTENT,
                        nHeight > 0 ? nHeight : ViewGroup.LayoutParams.WRAP_CONTENT);
                group.setTranslationX(X);
                group.setTranslationY(Y);
                parent.addView(group, lp);
            }

            long hwnd = ((INativeWindow) surface).nativeGetHwnd();;
            // 6) 回写 mViewMap（<HWND, Surface> 仅作 findViewByHandle fallback）
            mViewMap.put(hwnd, surface);

            return hwnd;
        } catch (Throwable t) {
            Log.e(TAG, "finishCreateWindow: unexpected exception cls=" + lpClassName
                    + " parent=" + (parent == null ? "null" : parent.getClass().getSimpleName()), t);
            return 0;
        }
    }

    /**
     * 创建消息占位窗口（HWND_MESSAGE）。不挂视觉树、0x0、不可见，仅用于消息处理。
     */
    private long createMessageOnlyWindow(String lpClassName, String lpWindowName, long hMenu) {
        // 轻量：直接 new SouiBaseSurface（不渲染、不 addView），用它自身的
        // NativeWindowDelegate 实现 INativeWindow。初始尺寸按 0x0 设置且 GONE。
        final View surface = new SouiBaseSurface(mContext) {
            // 消息-only 窗口：严格 0x0；Layout/measuring 期间也保持为 0。
            @Override
            protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
                setMeasuredDimension(0, 0);
            }
        };
        // 尺寸 0x0 + 不可见（GONE，避免任何 layout/绘制开销）。
        surface.measure(View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.EXACTLY),
                View.MeasureSpec.makeMeasureSpec(0, View.MeasureSpec.EXACTLY));
        surface.layout(0, 0, 0, 0);
        surface.setVisibility(View.GONE);

        // 控件 ID：hMenu==0 → NO_ID。表面与 ViewGroup 同步写 id（常规窗口语义一致）。
        int id = (hMenu == 0) ? View.NO_ID : (int)(hMenu & 0xFFFFFFFFL);
        surface.setId(id);

        // 注册：INativeWindow.surface 需要作为 HWND。
        if (!(surface instanceof INativeWindow)) {
            Log.e(TAG, "createMessageOnlyWindow: surface not INativeWindow: " + surface);
            return 0L;
        }
        long hwnd = ((INativeWindow) surface).nativeGetHwnd();
        if (hwnd == 0L) {
            Log.e(TAG, "createMessageOnlyWindow: nativeGetHwnd returned 0");
            return 0L;
        }
        // 作为 findViewByHandle fallback 记录
        mViewMap.put(hwnd, surface);
        // 存储到消息窗口集合，确保强引用且不挂视觉树
        mMessageOnlyWindows.put(hwnd, (INativeWindow) surface);
        return hwnd;
    }

    /**
     * 从一个 View 出发，沿 ViewParent 链向上查找最近的 SouiAbsWindow 容器。
     * 如果起点自身就是 SouiAbsWindow，则返回它自己。找不到返回 null。
     */
    private static SouiAbsWindow findAncestorSouiAbsWindow(View from) {
        View cur = from;
        while (cur != null) {
            if (cur instanceof SouiAbsWindow) return (SouiAbsWindow) cur;
            ViewParent vp = cur.getParent();
            cur = (vp instanceof View) ? (View) vp : null;
        }
        return null;
    }

    /**
     * 销毁 SOUI 窗口对应的 Android View。移除其所在容器（SouiWindow）。
     * @param windowHandle 窗口 HWND（=Surface GlobalRef）
     * @return true 成功销毁；false 未找到
     */
    public boolean destroyView(long windowHandle) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;

        // 消息窗口：没有parent，不挂视觉树，直接销毁并清理集合
        INativeWindow msgWindow = mMessageOnlyWindows.remove(windowHandle);
        if (msgWindow != null) {
            msgWindow.nativeDestroy();
            mViewMap.remove(windowHandle);
            return true;
        }

        // 正常成对出现：Surface 在 SouiAbsWindow/SouiWindow 容器内 → 移除整个容器
        ViewParent vp = surface.getParent();
        if (vp instanceof SouiAbsWindow) {
            View container = (View) vp;
            ViewParent grandParent = container.getParent();
            if (grandParent instanceof ViewGroup) {
                ((ViewGroup) grandParent).removeView(container);
            } else {
                // 没有 grandParent：至少把 Surface 从容器里拆下来（防御）
                ((SouiAbsWindow) vp).removeView(surface);
            }
            mViewMap.remove(windowHandle);
            return true;
        }
        // Fallback（异常状态，应尽早修复）：Surface 自己挂在某个 ViewGroup，直接 remove
        if (vp instanceof ViewGroup) {
            ((ViewGroup) vp).removeView(surface);
        }
        mViewMap.remove(windowHandle);
        return true;
    }

    /**
     * 显示/隐藏窗口（对应 Win32 ShowWindow）。操作提升到容器层级。
     * @param windowHandle 窗口 HWND
     * @param cmdShow 0 为隐藏，非 0 为显示
     * @return true 操作成功
     */
    public boolean showView(long windowHandle, int cmdShow) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        final int visibility = (cmdShow == 0) ? View.GONE : View.VISIBLE;
        ViewParent vp = surface.getParent();
        if (vp instanceof View) {
            ((View) vp).setVisibility(visibility);
        } else {
            surface.setVisibility(visibility);
        }
        return true;
    }

    /**
     * 获取窗口客户区矩形（对应 Win32 GetClientRect）。
     * @param windowHandle 窗口 HWND
     * @return 客户区矩形 (0,0,width,height)，未找到返回 null
     */
    @Nullable
    public Rect getClientRect(long windowHandle) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return null;
        ViewParent vp = surface.getParent();
        View sizeRef = (vp instanceof View) ? (View) vp : surface;
        return new Rect(0, 0, sizeRef.getWidth(), sizeRef.getHeight());
    }

    /**
     * 获取窗口屏幕坐标矩形（对应 Win32 GetWindowRect）。
     * @param windowHandle 窗口 HWND
     * @return 屏幕坐标矩形，未找到返回 null
     */
    @Nullable
    public Rect getWindowRect(long windowHandle) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return null;
        ViewParent vp = surface.getParent();
        View posRef = (vp instanceof View) ? (View) vp : surface;
        int[] location = new int[2];
        posRef.getLocationOnScreen(location);
        return new Rect(
                location[0], location[1],
                location[0] + posRef.getWidth(),
                location[1] + posRef.getHeight()
        );
    }

    /**
     * 检查窗口是否可见（对应 Win32 IsWindowVisible）。
     * @param windowHandle 窗口 HWND
     * @return true 可见；false 不可见或未找到
     */
    public boolean isWindowVisible(long windowHandle) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        View visRef = (vp instanceof View) ? (View) vp : surface;
        return visRef.getVisibility() == View.VISIBLE;
    }

    /**
     * 检查窗口是否启用（对应 Win32 IsWindowEnabled）。
     * @param windowHandle 窗口 HWND
     * @return true 已启用；false 已禁用或未找到
     */
    public boolean isWindowEnabled(long windowHandle) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        View enaRef = (vp instanceof View) ? (View) vp : surface;
        return enaRef.isEnabled();
    }

    /**
     * 移动并调整窗口大小（对应 Win32 MoveWindow）。
     * @param windowHandle 窗口 HWND
     * @param x/y 新位置
     * @param width/height 新尺寸
     * @return true 操作成功
     */
    public boolean moveWindow(long windowHandle, int x, int y, int width, int height) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        final View target = (vp instanceof View) ? (View) vp : surface;

        ViewParent parent = target.getParent();
        if (parent instanceof SouiAbsLayout) {
            ((SouiAbsLayout) parent).updateChildFrame(target, x, y, width, height);
            return true;
        } else if (parent instanceof ViewGroup) {
            ViewGroup.LayoutParams lp = target.getLayoutParams();
            if (lp == null) {
                lp = new ViewGroup.LayoutParams(width, height);
            } else {
                lp.width = width; lp.height = height;
            }
            target.setLayoutParams(lp);
            target.setTranslationX(x);
            target.setTranslationY(y);
            parent.requestLayout();
            return true;
        }
        return false;
    }

    /**
     * 设置窗口位置和大小（对应 Win32 SetWindowPos）。hWndInsertAfter 和 flags 在 Android 上未使用。
     */
    public boolean setWindowPos(long windowHandle, long hWndInsertAfter, int x, int y, int cx, int cy, int flags) {
        return moveWindow(windowHandle, x, y, cx, cy);
    }

    /**
     * 设置窗口大小。
     * @param windowHandle 窗口 HWND
     * @param cx/cy 新尺寸
     * @return true 操作成功
     */
    public boolean setWindowSize(long windowHandle, int cx, int cy) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        final View target = (vp instanceof View) ? (View) vp : surface;

        ViewParent parent = target.getParent();
        if (parent instanceof SouiAbsLayout) {
            ((SouiAbsLayout) parent).setChildSize(target, cx, cy);
            return true;
        }
        ViewGroup.LayoutParams lp = target.getLayoutParams();
        if (lp == null) return false;
        lp.width = cx; lp.height = cy;
        target.setLayoutParams(lp);
        if (parent != null) parent.requestLayout();
        return true;
    }

    /**
     * 设置窗口位置（仅 x, y）。
     * @param windowHandle 窗口 HWND
     * @param x/y 新位置
     * @return true 操作成功
     */
    public boolean setWindowPosEx(long windowHandle, int x, int y) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        final View target = (vp instanceof View) ? (View) vp : surface;

        ViewParent parent = target.getParent();
        if (parent instanceof SouiAbsLayout) {
            ((SouiAbsLayout) parent).setChildPosition(target, x, y);
            return true;
        } else if (parent != null) {
            target.setTranslationX(x);
            target.setTranslationY(y);
            return true;
        }
        return false;
    }

    /**
     * 启用/禁用窗口（对应 Win32 EnableWindow）。容器和 Surface 同步设置。
     * @param windowHandle 窗口 HWND
     * @param enabled true 启用；false 禁用
     * @return true 操作成功
     */
    public boolean enableWindow(long windowHandle, boolean enabled) {
        View surface = findViewByHandle(windowHandle);
        if (surface == null) return false;
        ViewParent vp = surface.getParent();
        if (vp instanceof View) {
            ((View) vp).setEnabled(enabled);
        }
        surface.setEnabled(enabled);
        return true;
    }

    /**
     * 按 HWND 查找 View。新架构下 HWND 就是 Surface 的 GlobalRef，直接返回。
     * @param windowHandle 窗口 HWND
     * @return Surface View，未找到返回 null
     */
    private View findViewByHandle(long windowHandle) {
        if (windowHandle == 0) return null;
        View v = nativeViewFromHwnd(windowHandle);
        if (v == null) v = mViewMap.get(windowHandle);
        if (v == null) return null;
        // 正路：Surface implements INativeWindow → 直接返回
        if (v instanceof INativeWindow) return v;
        // Legacy 兜底（SouiAbsWindow/SouiWindow/SouiScreen 容器）
        if (v instanceof SouiWindow) {
            View s = ((SouiWindow) v).getSurfaceView();
            if (s != null) return s;
            return v;
        }
        if (v instanceof SouiAbsWindow) {
            View s = ((SouiAbsWindow) v).getSurfaceView();
            if (s != null) return s;
        }
        return v;
    }

    /**
     * 按 HWND 查找对应的 SouiAbsWindow 容器。
     * @param windowHandle 窗口 HWND
     * @return SouiAbsWindow，未找到返回 null
     */
    @Nullable
    private SouiAbsWindow findViewGroup(long windowHandle) {
        View view = findViewByHandle(windowHandle);
        if (view == null) return null;
        if (view instanceof SouiAbsWindow) return (SouiAbsWindow) view;
        // Surface case: check parent first (fast path), then general ancestor walk
        ViewParent vp = view.getParent();
        if (vp instanceof SouiAbsWindow) return (SouiAbsWindow) vp;
        return findAncestorSouiAbsWindow(view);
    }

    /**
     * Get the View associated with a handle.
     * 
     * @param handle The HWND
     * @return The View object or null if not found
     */
    public View getView(long handle) {
        return findViewByHandle(handle);
    }

    /**
     * 创建定时器（对应 Win32 SetTimer）。语义与 Win32 一致：
     * <ul>
     *   <li>(hWnd, timerId) 二元组唯一标识定时器</li>
     *   <li>hWnd != 0 且 nIDEvent != 0：使用指定 id，同 id 替换旧定时器</li>
     *   <li>hWnd != 0 且 nIDEvent == 0：自动分配该 hWnd 下未使用的 id</li>
     *   <li>hWnd == 0：忽略 nIDEvent，自动分配全局唯一 id</li>
     * </ul>
     * @param hWnd 窗口句柄，0 表示全局定时器
     * @param nIDEvent 定时器标识
     * @param uElapse 超时时间（毫秒）
     * @return 定时器标识（成功非 0，失败 0）
     */
    public long setTimer(long hWnd, long nIDEvent, int uElapse) {
        if (uElapse <= 0) uElapse = 1;
        final long interval = uElapse;
        final long timerId;

        synchronized (mBridgeTimerMap) {
            // ① hWnd==0：非窗口定时器。Win32 语义：忽略用户传的 nIDEvent，
            //    一律自动分配一个全局未使用的 ID。
            if (hWnd == 0L) {
                timerId = allocateGlobalTimerIdLocked();
            }
            // ② 窗口定时器：用户指定了 id → 直接用（同 hWnd 下同名 id 替换旧者）
            else if (nIDEvent != 0L) {
                timerId = nIDEvent;
                // 若该 (hWnd, timerId) 已有旧定时器：先取消并移除
                HashMap<Long, TimerRunnable> perHwnd = mBridgeTimerMap.get(hWnd);
                if (perHwnd != null) {
                    TimerRunnable old = perHwnd.remove(timerId);
                    if (old != null) {
                        old.cancel();
                        mBridgeHandler.removeCallbacks(old);
                        if (perHwnd.isEmpty()) mBridgeTimerMap.remove(hWnd);
                    }
                }
            }
            // ③ 窗口定时器：用户传 nIDEvent==0 → 系统分配一个该 hWnd 下未使用的 id
            else {
                timerId = allocateHwndTimerIdLocked(hWnd);
            }

            // 登记到嵌套 Map
            HashMap<Long, TimerRunnable> perHwnd =
                    mBridgeTimerMap.computeIfAbsent(hWnd, k -> new HashMap<>());
            TimerRunnable runnable = new TimerRunnable(timerId, hWnd, interval);
            perHwnd.put(timerId, runnable);
            mBridgeHandler.postDelayed(runnable, interval);
        }

        return timerId;
    }

    /** 在 mBridgeTimerMap 全局范围 (跨所有 hWnd) 分配一个仍未使用的 timerId。
     *  用于 hWnd==0 的非窗口定时器（Win32 规定：这种情况下 nIDEvent 被忽略，系统分配新 ID）。*/
    private long allocateGlobalTimerIdLocked() {
        // 先快速从自增器取一个，大多数情况 O(1) 命中
        long candidate = mNextTimerId;
        if (candidate == 0L) candidate = 1L; // 安全钳位，永不返回 0
        // 扫描全 map，确保没被任何 hWnd 占过；一般 mNextTimerId 单调递增，1 次就过
        long start = candidate;
        while (true) {
            final long id = candidate;
            boolean used = false;
            for (HashMap<Long, TimerRunnable> perHwnd : mBridgeTimerMap.values()) {
                if (perHwnd.containsKey(id)) { used = true; break; }
            }
            if (!used) {
                // 保留下一步起点（加 1，跳过 0）
                mNextTimerId = (id == Long.MAX_VALUE) ? 1L : (id + 1L);
                return id;
            }
            candidate = (candidate == Long.MAX_VALUE) ? 1L : (candidate + 1L);
            if (candidate == start) {
                // 极端情况：ID 空间耗尽（理论上不可能）。抛错兜底，避免死循环。
                throw new IllegalStateException("Timer ID space exhausted (global)");
            }
        }
    }

    /** 在指定 hWnd 下分配一个尚未使用的 timerId（用于窗口定时器 + nIDEvent==0 场景）。
     *  与全局分配的区别：只要求"该 hWnd 下不重复"，其它 hWnd 已占用的同名 id 不冲突。*/
    private long allocateHwndTimerIdLocked(long hWnd) {
        HashMap<Long, TimerRunnable> perHwnd = mBridgeTimerMap.get(hWnd);
        long candidate = mNextTimerId;
        if (candidate == 0L) candidate = 1L;
        long start = candidate;
        while (true) {
            final long id = candidate;
            boolean used = (perHwnd != null) && perHwnd.containsKey(id);
            if (!used) {
                mNextTimerId = (id == Long.MAX_VALUE) ? 1L : (id + 1L);
                return id;
            }
            candidate = (candidate == Long.MAX_VALUE) ? 1L : (candidate + 1L);
            if (candidate == start) {
                throw new IllegalStateException("Timer ID space exhausted (for hWnd=" + hWnd + ")");
            }
        }
    }

    /**
     * 销毁定时器（对应 Win32 KillTimer）。(hWnd, uIDEvent) 二元组都匹配才会销毁。
     * @param hWnd 窗口句柄（0 表示全局定时器）
     * @param uIDEvent 定时器标识
     * @return true 成功销毁；false 未找到
     */
    public boolean killTimer(long hWnd, long uIDEvent) {
        if (uIDEvent == 0L) return false;
        synchronized (mBridgeTimerMap) {
            HashMap<Long, TimerRunnable> perHwnd = mBridgeTimerMap.get(hWnd);
            if (perHwnd == null) return false;
            TimerRunnable runnable = perHwnd.remove(uIDEvent);
            if (runnable == null) return false;
            runnable.cancel();
            mBridgeHandler.removeCallbacks(runnable);
            if (perHwnd.isEmpty()) mBridgeTimerMap.remove(hWnd);
            return true;
        }
    }

    /**
     * 销毁指定窗口关联的所有定时器。传 0 销毁所有全局定时器。
     * @param hWnd 窗口句柄
     */
    public void killWindowTimers(long hWnd) {
        synchronized (mBridgeTimerMap) {
            HashMap<Long, TimerRunnable> perHwnd = mBridgeTimerMap.remove(hWnd);
            if (perHwnd == null) return;
            for (TimerRunnable r : perHwnd.values()) {
                r.cancel();
                mBridgeHandler.removeCallbacks(r);
            }
            perHwnd.clear();
        }
    }

    public String[][] getInputDevices() {
        int[] deviceIds = InputDevice.getDeviceIds();
        ArrayList<String[]> deviceList = new ArrayList<>();
        for (int i = 0; i < deviceIds.length; i++) {
            InputDevice device = InputDevice.getDevice(deviceIds[i]);
            if (device == null) continue;
            String name = device.getName();
            if (name == null) name = "";
            String type;
            int sources = device.getSources();
            if ((sources & InputDevice.SOURCE_KEYBOARD) != 0 ||
                    (sources & InputDevice.SOURCE_DPAD) != 0) {
                type = "keyboard";
            } else if ((sources & InputDevice.SOURCE_MOUSE) != 0 ||
                    (sources & InputDevice.SOURCE_TOUCHPAD) != 0) {
                type = "mouse";
            } else {
                type = "hid";
            }
            deviceList.add(new String[]{name, type});
        }
        return deviceList.toArray(new String[0][]);
    }

    boolean showSoftKeyboard(View v,boolean bShow){
        InputMethodManager imm = (InputMethodManager) getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        if(imm == null)
            return false;
        if(bShow)
            return imm.showSoftInput(v, InputMethodManager.SHOW_IMPLICIT);
        else
            return imm.hideSoftInputFromWindow(v.getWindowToken(),0);
    }

    public  long createNative(INativeWindow wnd){
        return nativeCreate(wnd);
    }

    public  void destroyNative(long nativeId){
        // 清理捕获/焦点，避免残留指向已销毁的 HWND
        if (nativeId == getCapture()) releaseCapture();
        if (nativeId == getFocus()) setFocus(0L);
        killWindowTimers(nativeId);
        nativeDestroy(nativeId);
    }

    private native void nativeRegisterPlatformAPI(Context ctx);

    private native void nativeUnregisterPlatformAPI();

    /** 定时器触发时的 native 回调，投递 WM_TIMER 到 SOUI 消息循环。 */
    private native void nativeOnTimerExpired(long hWnd, long timerId);

    /** C++ SOUI 启动入口。一次调用完成 screen 注册、布局加载和主窗口创建。 */
    private native long nativeScreenStartup(long screenId, long screenHwnd, String layout);

    private native void nativeScreenShutdown(long screenId);

    /** Java 焦点变化同步到 C++，投递 WM_SETFOCUS/WM_KILLFOCUS。 */
    private static native void nativeNotifyFocusGained(long hwnd);

    /** 包内可见：SouiBaseSurface/NativeEditView 获焦时调用，同步到 C++。 */
    static void notifyFocusGained(long hwnd) {
        if (hwnd == 0L) return;
        nativeNotifyFocusGained(hwnd);
    }
    /** 将 HWND（=INativeWindow GlobalRef 指针）还原为 View。仅供内部使用。 */
    private static native View nativeViewFromHwnd(long hwnd);


    /** 从 C++ 字符串槽读取入参（如 WM_SETTEXT 的标题）。 */
    public static native String nativeReadCxxStringSlot(int slotId);
    /** 把字符串出参（如 WM_GETTEXT 的结果）写回 C++ 返回槽。 */
    public static native void nativeWriteCxxStringSlot(int slotId, String str);
    /** 分配一个字符串槽，返回槽 ID（>0）。 */
    public static native int nativeAllocStringSlot();
    /** 释放字符串槽。 */
    public static native void nativeFreeStringSlot(int slotId);

    public static native void nativeSetScreenSize(long screen_id, long hMainWnd, int width, int height);
    public static native void nativeSetKeyboardHeight(long keyboardHeight);
    private static native boolean nativeInitSouiApp(AssetManager am, String appFilesDir);
    private static native void nativeUninitSouiApp();

    private static native long nativeCreate(INativeWindow wnd);
    private static native void nativeDestroy(long nativeId);

    // Clipboard API constants (matching Win32 CF_* values)
    private static final int CF_TEXT = 1;
    private static final int CF_UNICODETEXT = 13;

    private long mClipOwner = 0;
    /** Clipboard methods for C++ layer */
    @SuppressWarnings("unused")
    public boolean clipboardOpen(long hWndNewOwner) {
        mClipOwner = hWndNewOwner;
        return true;
    }

    @SuppressWarnings("unused")
    public boolean clipboardClose() {
        mClipOwner = 0;
        return true;
    }

    @SuppressWarnings("unused")
    public boolean clipboardEmpty() {
        if (mContext == null) return false;
        ClipboardManager cm = (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        if (cm == null) return false;
        cm.setPrimaryClip(ClipData.newPlainText("", ""));
        return true;
    }

    @SuppressWarnings("unused")
    public String clipboardGetData(int format) {
        if (mContext == null) return null;
        ClipboardManager cm = (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        if (cm == null || !cm.hasPrimaryClip()) return null;

        ClipData clipData = cm.getPrimaryClip();
        if (clipData == null || clipData.getItemCount() == 0) return null;

        ClipData.Item item = clipData.getItemAt(0);
        if (item == null) return null;

        CharSequence text = item.getText();
        if (text == null) return null;

        return text.toString();
    }

    @SuppressWarnings("unused")
    public boolean clipboardSetData(int format, String text) {
        if (mContext == null) return false;
        ClipboardManager cm = (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        if (cm == null) return false;

        if (text == null || text.isEmpty()) return false;

        cm.setPrimaryClip(ClipData.newPlainText("", text));
        return true;
    }

    @SuppressWarnings("unused")
    public boolean clipboardIsFormatAvailable(int format) {
        return clipboardHasFormat(format);
    }

    @SuppressWarnings("unused")
    public long clipboardGetOwner() {
        return mClipOwner;
    }

    @SuppressWarnings("unused")
    public boolean clipboardHasFormat(int format) {
        if (mContext == null) return false;
        ClipboardManager cm = (ClipboardManager) mContext.getSystemService(Context.CLIPBOARD_SERVICE);
        if (cm == null) return false;
        return cm.hasPrimaryClip();
    }

    public void sendImeString(long hwnd, String str) {
        int slotid = nativeAllocStringSlot();
        try {
            nativeWriteCxxStringSlot(slotid, str);
            nativeSendImeString(hwnd, slotid);
        } finally {
            nativeFreeStringSlot(slotid);
        }
    }

    private static native void nativeSendImeString(long hwnd, int slotid);

    /**
     * Play sound file via Android platform.
     * @param pszSound Sound file path
     * @param fdwSound Sound flags (SND_*)
     * @return true if sound started playing successfully
     */
    @SuppressWarnings("unused")
    public synchronized boolean playSound(String pszSound, int fdwSound) {
        if (mContext == null || pszSound == null || pszSound.isEmpty()) {
            return false;
        }

        // SND_PURGE: Stop all playing sounds
        if ((fdwSound & 0x00000040) != 0) { // SND_PURGE
            stopAllSounds();
            return true;
        }

        // Check if sound file exists
        java.io.File soundFile = new java.io.File(pszSound);
        if (!soundFile.exists()) {
            Log.w(TAG, "playSound: file not found: " + pszSound);
            return false;
        }

        try {
            // Stop previous sound if playing
            if (mCurrentMediaPlayer != null) {
                try {
                    mCurrentMediaPlayer.stop();
                    mCurrentMediaPlayer.release();
                } catch (Exception e) {
                    // Ignore
                }
                mCurrentMediaPlayer = null;
            }

            MediaPlayer mediaPlayer = new MediaPlayer();
            mediaPlayer.setDataSource(pszSound);

            final boolean loop = (fdwSound & 0x00000008) != 0; // SND_LOOP
            mediaPlayer.setLooping(loop);

            mediaPlayer.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
                @Override
                public void onCompletion(MediaPlayer mp) {
                    if (mp == mCurrentMediaPlayer) {
                        mCurrentMediaPlayer = null;
                    }
                    mp.release();
                }
            });

            mediaPlayer.setOnErrorListener(new MediaPlayer.OnErrorListener() {
                @Override
                public boolean onError(MediaPlayer mp, int what, int extra) {
                    Log.e(TAG, "playSound error: what=" + what + " extra=" + extra);
                    if (mp == mCurrentMediaPlayer) {
                        mCurrentMediaPlayer = null;
                    }
                    mp.release();
                    return true;
                }
            });

            mediaPlayer.prepare();
            mediaPlayer.start();
            mCurrentMediaPlayer = mediaPlayer;

            return true;
        } catch (Exception e) {
            Log.e(TAG, "playSound failed: " + pszSound, e);
            return false;
        }
    }

    private void stopAllSounds() {
        if (mCurrentMediaPlayer != null) {
            try {
                mCurrentMediaPlayer.stop();
                mCurrentMediaPlayer.release();
            } catch (Exception e) {
                // Ignore
            }
            mCurrentMediaPlayer = null;
        }
        for (MediaPlayer player : mActivePlayers.values()) {
            try {
                player.stop();
                player.release();
            } catch (Exception e) {
                // Ignore
            }
        }
        mActivePlayers.clear();
    }
}
