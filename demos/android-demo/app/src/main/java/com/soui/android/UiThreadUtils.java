package com.soui.android;

import android.os.Handler;
import android.os.Looper;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Supplier;

/**
 * Android 主线程同步执行工具。C++ JNI 回调可能在任意线程触发，需要通过此工具
 * 将 View 操作同步切换到主线程执行，避免 CalledFromWrongThreadException。
 */
public final class UiThreadUtils {

    /** 同步等待超时时间（秒），避免主线程阻塞导致永久挂起。 */
    public static final long UI_SYNC_TIMEOUT_SEC = 10L;

    /** 主线程 Handler。 */
    private static final Handler MAIN_HANDLER = new Handler(Looper.getMainLooper());

    /** 工具类不可实例化。 */
    private UiThreadUtils() {
        throw new AssertionError("UiThreadUtils must not be instantiated");
    }

    /**
     * 在主线程同步执行代码块并返回结果。
     * <ul>
     *   <li>已在主线程：直接执行，避免死锁</li>
     *   <li>非主线程：投递到主线程，用 CountDownLatch 等待结果</li>
     * </ul>
     * @param block 要执行的代码块
     * @param <T> 返回值类型
     * @return 执行结果
     */
    public static <T> T runOnUiSync(final Supplier<T> block) {
        if (block == null) throw new NullPointerException("block must not be null");

        // ① 快速路径：已在主线程 → 直接执行（避免死锁）
        if (Looper.myLooper() == Looper.getMainLooper()) {
            return block.get();
        }

        // ② 其它线程：post 到 MAIN_HANDLER 同步等待
        final CountDownLatch              latch  = new CountDownLatch(1);
        final AtomicReference<T>          result = new AtomicReference<>();
        final AtomicReference<Throwable> thrown = new AtomicReference<>();

        MAIN_HANDLER.post(() -> {
            try {
                result.set(block.get());
            } catch (Throwable t) {
                thrown.set(t);
            } finally {
                latch.countDown();
            }
        });

        try {
            final boolean done = latch.await(UI_SYNC_TIMEOUT_SEC, TimeUnit.SECONDS);
            if (!done) {
                throw new RuntimeException("UiThreadUtils: UI thread blocked > " + UI_SYNC_TIMEOUT_SEC
                        + "s; potential deadlock or ANR in main looper");
            }
        } catch (InterruptedException ie) {
            Thread.currentThread().interrupt();
            throw new RuntimeException("UiThreadUtils: interrupted waiting for UI thread", ie);
        }

        final Throwable t = thrown.get();
        if (t != null) {
            if (t instanceof RuntimeException) throw (RuntimeException) t;
            if (t instanceof Error)            throw (Error) t;
            throw new RuntimeException("UiThreadUtils: UI operation threw", t);
        }
        return result.get();
    }

    /**
     * {@link Runnable} 版主线程同步执行（无返回值）。
     *
     * @param block 要在主线程执行的代码块；不能为 {@code null}
     */
    public static void runOnUiSyncVoid(final Runnable block) {
        if (block == null) throw new NullPointerException("block must not be null");
        runOnUiSync(() -> {
            block.run();
            return null;
        });
    }
}
