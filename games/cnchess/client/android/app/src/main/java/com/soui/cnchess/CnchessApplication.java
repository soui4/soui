package com.soui.cnchess;

import android.app.Application;
import android.content.res.AssetManager;
import android.os.StrictMode;
import android.util.Log;

import com.soui.android.SouiPlatformBridge;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * CnchessApplication - 中国象棋应用入口。
 *
 * <p>初始化 SOUI on Android 平台桥，加载 libcnchess-android.so。
 * 主题资源（def_theme）不再从 assets 复制到 filesDir，
 * 而是客户端启动后通过 WebSocket 从服务器下载到缓存目录。
 * 仅 avatar 目录在启动时复制到 filesDir。
 */
public class CnchessApplication extends Application {

    private static final String TAG = "CnchessApplication";

    static {
        System.loadLibrary("cnchess-android");
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
        Log.i(TAG, "onCreate: SOUI initialized for cnchess.");
    }


    @Override
    public void onTerminate() {
        SouiPlatformBridge.getInstance().destroy();
        super.onTerminate();
    }
}
