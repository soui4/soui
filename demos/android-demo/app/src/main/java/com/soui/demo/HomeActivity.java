package com.soui.demo;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

/**
 * HomeActivity - 演示入口。
 *
 * <p>提供两个按钮，分别演示 SouiScreen 的两种启动方式：
 * <ul>
 *   <li>演示 1：{@link DefaultHostActivity} —— 代码方式。
 *       Activity 手动 new SouiScreen + setScreenId/setSouiLayout，
 *       之后 onAttachedToWindow 自动 startSoui。</li>
 *   <li>演示 2：{@link XmlHostActivity} —— XML 声明方式。
 *       Activity 只需 setContentView(R.layout.activity_xml_host)，
 *       screenId / layout 两个属性写在 XML 的自定义属性里，
 *       SouiScreen 在构造阶段读取，同样走 onAttachedToWindow 自动 startSoui。</li>
 * </ul>
 */
public class HomeActivity extends AppCompatActivity {

    public void OnClickDefault(View v){
        startActivity(new Intent(HomeActivity.this, DefaultHostActivity.class));
    }

    public void OnClickFromXml(View v){
        startActivity(new Intent(HomeActivity.this, XmlHostActivity.class));
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
    }
}
