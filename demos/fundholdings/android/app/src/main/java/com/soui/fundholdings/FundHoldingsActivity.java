package com.soui.fundholdings;

import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.soui.android.SouiScreen;

/**
 * FundHoldingsActivity - 基金持仓助手主界面。
 *
 * <p>使用 SouiScreen 承载 SOUI 的 CMainDlg 窗口。
 * C++ 侧 ScreenStartup 会创建 CMainDlg 并加载 layout:dlg_main。
 */
public class FundHoldingsActivity extends AppCompatActivity {

    public static final long SCREEN_ID = 0x0000_F101L;

    /** SOUI 布局资源名，对应 uires.idx 中的 LAYOUT dlg_main */
    public static final String LAYOUT_MAIN = "layout:dlg_main";

    private SouiScreen mScreen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

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
        Log.i("fundholdings", "onConfigurationChanged, orientation=" + newConfig.orientation);
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
