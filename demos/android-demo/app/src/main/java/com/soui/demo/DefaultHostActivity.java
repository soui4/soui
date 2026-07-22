package com.soui.demo;

import android.content.res.Configuration;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.soui.android.SouiScreen;

/**
 * DefaultHostActivity - 使用默认 SHostWnd，仅靠 SOUI XML 布局渲染。
 *
 * <p>新架构（screenId 单一数据源 + C++ 直接 Create(NULL) 创建主窗口）启动流程：
 * <ol>
 *   <li>Activity 创建 {@link SouiScreen}（HWND_DESKTOP 桌面容器，不做绘制）；</li>
 *   <li>给 SouiScreen 通过 {@link SouiScreen#setScreenId(long)} /
 *       {@link SouiScreen#setSouiLayout(String)} 写入启动参数；</li>
 *   <li>把 SouiScreen MATCH_PARENT 挂到 Activity →
 *       {@link SouiScreen} 会在 View 测量完毕后自动
 *       post 一次 {@link SouiScreen#startSoui(long, String, int, int)}，
 *       省去 Activity 手动 post + startSoui 的样板代码；</li>
 *   <li>screenId / layout / width / height 只在 C++
 *       {@code AndroidPlatformAPI.m_screenContexts[SCREEN_ID]} 里存一份，
 *       作为 screenId 的**唯一真源**。</li>
 * </ol>
 *
 * <p>这种方式完全对应 Windows 端"SOUI 创建 SHostWnd 自身管理生命周期"的语义：
 * C++ 创建窗口代码与 Windows 端一字不差（new SHostWnd → Create(NULL) →
 * ShowWindow）；Activity / Java 层只是提供容器 + 一次性启动参数，不做二次缓存。
 */
public class DefaultHostActivity extends AppCompatActivity {

    /** 稳定的 screenId（每个 Activity 一个，务必全局唯一）。与
     *  {@code R.integer.screen_id_default_host} 保持一致，便于 XML/代码两端同步。 */
    public static final long SCREEN_ID = 0x0000_D001L;

    /**
     * SOUI 布局资源 ID，格式 {@code "type:name"} —— 与 {@code assets/uires/uires.idx} 中
     * LAYOUT 段的 name 对应：
     * <pre>
     *   <LAYOUT>
     *     <file name="dlg_main" path="xml/dlg_main.xml"/>
     *   </LAYOUT>
     * </pre>
     * 与 {@code R.string.layout_demo} 保持一致，便于 XML/代码两端复用。
     */
    public static final String LAYOUT_DEMO = "layout:dlg_main";

    private SouiScreen mScreen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // 1. 构造 SouiScreen（纯绝对定位布局基类，不做绘制，HWND_DESKTOP 容器）
        mScreen = new SouiScreen(this, SCREEN_ID, LAYOUT_DEMO);

        // 3. MATCH_PARENT 铺满 Activity（等价 Windows 端"窗口铺满桌面"的意思）。
        //    setContentView 走完 → SouiScreen.onAttachedToWindow 被调用
        //    → 内部自动 startSoui(SCREEN_ID, LAYOUT_DEMO, width, height)。
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
        Log.i("host","onConfigurationChanged,orientation="+newConfig.orientation);
        if (mScreen != null) {
            mScreen.requestLayout();
            mScreen.post(()->{
                View root = (View)mScreen.getParent();
                int w = root.getWidth();
                int h = root.getHeight();
                Log.i("host","onConfigurationChanged, root.w="+w);
                mScreen.syncScreenSize(w, h);
            });
        }
    }
}
