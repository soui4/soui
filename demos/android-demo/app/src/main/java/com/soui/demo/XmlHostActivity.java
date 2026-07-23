package com.soui.demo;

import android.os.Bundle;

import androidx.appcompat.app.AppCompatActivity;

/**
 * XmlHostActivity - 演示"纯 XML 声明 SouiScreen + 自定义属性即可启动 SOUI"。
 *
 * <p>关键差异对比 {@link DefaultHostActivity}：
 * <table border="1" cellpadding="6" cellspacing="0">
 *   <tr><th>DefaultHostActivity</th><th>XmlHostActivity (本类)</th></tr>
 *   <tr>
 *     <td>代码 new SouiScreen + setScreenId / setSouiLayout + setContentView，
 *         由 onAttachedToWindow 自动 startSoui。</td>
 *     <td>setContentView(R.layout.activity_xml_host) 仅此一行。screenId 和 layout
 *         全在 XML 里用 {@code app:screenId} / {@code app:soui_souiLayout}
 *         声明；构造阶段写入成员，onAttachedToWindow 自动启动。</td>
 *   </tr>
 * </table>
 *
 * <p>onDestroy 无需清理：SouiScreen.onDetachedFromWindow() 会自动调用
 * {@code bridge.screenShutdown(screenId)}，释放 C++ 侧 GlobalRef 与窗口。
 * View 树拆离即可触发，无需 Activity 手动介入。
 */
public class XmlHostActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // 一行代码：引入布局。布局里 <com.soui.SouiScreen> 已通过 app:screenId
        // 和 app:souiLayout 声明了启动参数，SOUI 会在 onAttachedToWindow 时
        // 自动启动。整个类无需再写任何与 screen / startSoui 相关的代码。
        setContentView(R.layout.activity_xml_host);
    }
}
