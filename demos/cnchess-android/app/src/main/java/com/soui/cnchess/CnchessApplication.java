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
 * 在 SOUI 初始化前，将 def_theme 和 avatar 从 assets 复制到 filesDir，
 * 以便 C++ 侧的 SGameTheme::Load 可以从文件系统读取主题资源。
 */
public class CnchessApplication extends Application {

    private static final String TAG = "CnchessApplication";

    static {
        System.loadLibrary("cnchess-android");
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // 先将 def_theme 和 avatar 从 assets 复制到 filesDir
        copyAssetDir("cnchess/def_theme", getFilesDir().getAbsolutePath() + "/cnchess/def_theme");
        copyAssetDir("cnchess/avatar", getFilesDir().getAbsolutePath() + "/cnchess/avatar");

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

    /**
     * 递归复制 assets 目录到目标路径
     */
    private void copyAssetDir(String assetPath, String destPath) {
        AssetManager assetManager = getAssets();
        try {
            String[] children = assetManager.list(assetPath);
            if (children == null || children.length == 0) {
                // 这是一个文件，直接复制
                copyAssetFile(assetPath, destPath);
                return;
            }
            // 这是一个目录，创建目标目录并递归复制
            File destDir = new File(destPath);
            if (!destDir.exists()) {
                destDir.mkdirs();
            }
            for (String child : children) {
                String childAssetPath = assetPath + "/" + child;
                String childDestPath = destPath + "/" + child;
                copyAssetDir(childAssetPath, childDestPath);
            }
        } catch (IOException e) {
            Log.e(TAG, "copyAssetDir failed: " + assetPath, e);
        }
    }

    /**
     * 复制单个 assets 文件到目标路径
     */
    private void copyAssetFile(String assetPath, String destPath) {
        try (InputStream in = getAssets().open(assetPath);
             OutputStream out = new FileOutputStream(destPath)) {
            byte[] buf = new byte[8192];
            int len;
            while ((len = in.read(buf)) > 0) {
                out.write(buf, 0, len);
            }
        } catch (IOException e) {
            Log.e(TAG, "copyAssetFile failed: " + assetPath, e);
        }
    }

    @Override
    public void onTerminate() {
        SouiPlatformBridge.getInstance().destroy();
        super.onTerminate();
    }
}
