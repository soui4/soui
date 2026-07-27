package com.soui.cnchess;

import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.soui.android.SouiScreen;

/**
 * GameActivity - 中国象棋主游戏界面。
 *
 * <p>使用 SouiScreen 承载 SOUI 的 CMainDlg 窗口。
 * C++ 侧 ScreenStartup 会创建 CMainDlg 并加载游戏布局。
 */
public class GameActivity extends AppCompatActivity {

    public static final long SCREEN_ID = 0x0000_C001L;

    /** SOUI 布局资源 ID，对应 uires.idx 中的 XML_MAINWND */
    public static final String LAYOUT_MAIN = "layout:XML_MAINWND";

    private SouiScreen mScreen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 构造 SouiScreen，传入 screenId 和布局
        mScreen = new SouiScreen(this, SCREEN_ID, LAYOUT_MAIN);

        FrameLayout decor = new FrameLayout(this);
        decor.addView(mScreen, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        mScreen.setOnEmptyListener(new SouiScreen.OnEmptyListener() {
            @Override
            public void onEmpty(SouiScreen screen) {
                finish();
            }
        });
        setContentView(decor);
    }

    @Override
    protected void onDestroy() {
        mScreen = null;
        super.onDestroy();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.i("cnchess", "onConfigurationChanged, orientation=" + newConfig.orientation);
        if (mScreen != null) {
            mScreen.requestLayout();
            mScreen.post(() -> {
                View root = (View) mScreen.getParent();
                int w = root.getWidth();
                int h = root.getHeight();
                mScreen.syncScreenSize(w, h);
            });
        }
    }
}
