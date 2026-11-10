package com.soui.fundholdings;

import android.app.Application;
import android.os.StrictMode;
import android.util.Log;

import com.soui.android.SouiPlatformBridge;

/**
 * FundHoldingsApplication - 基金持仓助手的应用入口。
 *
 * <p>初始化 SOUI on Android 平台桥，并加载 libfundholdings-android.so。
 * 业务布局与桌面版共用同一套控件 name，仅资源目录换成 mobile/uires。
 */
public class FundHoldingsApplication extends Application {

    private static final String TAG = "FundHoldingsApplication";

    static {
        System.loadLibrary("fundholdings-android");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        // 初始化 SOUI Android 平台桥
        SouiPlatformBridge.getInstance().init(this, getAssets(), getFilesDir().getAbsolutePath());

        if (BuildConfig.DEBUG) {
            StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
                    .detectDiskReads()
                    .detectDiskWrites()
                    .detectNetwork()
                    .penaltyLog()
                    .build());
        }
        Log.i(TAG, "onCreate: SOUI initialized for fundholdings.");
    }

    @Override
    public void onTerminate() {
        SouiPlatformBridge.getInstance().destroy();
        super.onTerminate();
    }
}
