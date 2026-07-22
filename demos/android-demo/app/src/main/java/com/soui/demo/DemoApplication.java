package com.soui.demo;

import android.app.Application;
import android.content.res.AssetManager;
import android.os.StrictMode;
import android.util.Log;

import com.soui.android.SouiPlatformBridge;

/**
 * DemoApplication - 应用入口。
 *
 * <p>SOUI on Android 的初始化顺序（使用 SResProviderAndroidAsset，免拷贝）：
 * <ol>
 *   <li>{@link SouiPlatformBridge#getInstance()} 触发 static 块加载 libsoui-android-demo.so；</li>
 *   <li>调用 {@link SouiPlatformBridge#init(android.content.Context)}；</li>
 *   <li>调用 {@link SouiPlatformBridge#nativeInitSouiApp(AssetManager, String)}，C++ 侧通过
 *       AAssetManager 直接从 APK assets 读取两套资源包
 *       （{@code soui_sys_res/uires.idx} + {@code uires/uires.idx}），
 *       <b>不再需要把 assets 拷贝到 filesDir</b>；</li>
 * </ol>
 */
public class DemoApplication extends Application {

    private static final String TAG = "DemoApplication";

    static {
        System.loadLibrary("soui-android-demo");
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // 初始化 SOUI Android 平台桥（nativeRegisterPlatformAPI 注册 AndroidPlatformAPI）
        SouiPlatformBridge.getInstance().init(this,getAssets(), getFilesDir().getAbsolutePath());
        // 在 Application 或 Activity 的 onCreate 中启用
        if (BuildConfig.DEBUG) {
            StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                    .detectDiskReads()
                    .detectDiskWrites()
                    .detectNetwork()
                    .penaltyLog() // 在 logcat 中打印违规信息，而非崩溃
                    .build());
        }
        Log.i(TAG, "onCreate: SOUI initialized (assets loaded directly via AAssetManager, no copy).");
    }

    @Override
    public void onTerminate() {
        SouiPlatformBridge.getInstance().destroy();
        super.onTerminate();
    }

}
