# 在SOUI中实现生动的时钟动画

## 前言

在棋牌游戏界面中，显示一个倒时候时钟是一个常规需求。本文将详细介绍如何在SOUI框架中使用非常简单的代码实现一个动画效果丰富的时钟效果，重点介绍如何使用StackView的内置动画来实现数字切换以及如何使用XML定义Animation来实现倒数闹钟振动效果。

## StackView简介

StackView是SOUI框架中一个非常有用的控件，它允许我们创建类似TabControl的效果，可以在同一区域显示不同的页面内容。在时钟数字显示中，我们可以利用StackView的每一页来显示一个数字（0-9），然后通过切换页面来实现数字的变化。

## 数字切换动画效果实现

### 1. 皮肤资源定义

首先，我们需要定义数字显示的皮肤资源：

```xml
<imglist name="skin_clock" src="file:image/闹钟.png"/>
<imglist name="skin_clock_digital" src="file:image/闹钟数字.png" states="10"/>
```

这个皮肤将包含0-9十个数字的图片资源，每个数字占据一个状态。

### 2. XML布局定义

接下来，我们需要在XML中定义用于显示数字的StackView控件：

```xml
<!-- 闹钟数字显示,每个数字占一个页面,通过切换页面来显示不同的数字,利用StackView的内置动画来实现数字切换 -->
<?xml version="1.0" encoding="utf-8"?>
<stack size="-1,-1" curSel="0" aniType="push" vertical="1" duration="200">
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="0" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="1" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="2" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="3" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="4" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="5" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="6" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="7" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="8" />
    <img size="-1,-1" skin="skin_clock_digital" iconIndex="9" />
</stack>

<!-- 时钟容器, clock_digit.xml是前面这个stack的xml文件,通过include标签引用, 再通过设定name属性，就可以访问不同的数字 -->
<img name="myclock" ownerLayout="anchor" skin="skin_clock" fitImage="1" layout="hbox" gravity="center">
    <window size="0,0" weight="1"/>
    <include src="file:xml/clock_digit.xml" name="digit1" pos="0,0,4" size="-1,-1" offset="-0.5,-0.5"/>
    <include src="file:xml/clock_digit.xml" name="digit2" pos="0,0,4" size="-1,-1" offset="-0.5,-0.5"/>
    <window size="0,0" weight="1"/>
</img>
```

在这个布局中，我们定义了两个StackView控件，分别用于显示十位和个位数字。关键的动画相关属性包括：

- `vertical`: 切换页面是否垂直方向动画
- `duration`: 动画持续时间（毫秒）
- `ani_type`: 动画类型, 可选值有：push, fade, move, none


### 3. C++代码实现

在C++代码中，我们可以这样更新时钟显示：

```cpp
void CMyClock::UpdateTime(int nSecond)
{
    nSecond %= 100;  // 最大显示99秒
    
    // 获取时钟控件
    SWindow *pClock = FindChildByName(L"myclock");
    SStackView * pDigitTens = pClock->FindChildByName2<SStackView>(L"digit1");
    SStackView * pDigitUnits = pClock->FindChildByName2<SStackView>(L"digit2");
    
    // 切换页面，带动画效果
    pDigitTens->SelectPage(nSecond/10);
    pDigitUnits->SelectPage(nSecond%10);
}
```

当调用[SelectPage]方法时，StackView会自动播放预设的切换动画。

## 闹钟振动效果实现

### 1. XML动画定义

为了实现倒数最后5秒的闹钟振动效果，我们需要在XML中定义一个振动动画：

```xml
<!-- 振动动画定义,这个动画定义保存为文件:clock_alarm_shake.xml, 可以在主题资源中定义 -->
<!-- 这个动画通过定义两个旋转动画，实现左右摆动效果 -->
<?xml version="1.0" encoding="utf-8"?>
<set repeatCount="-1" fillAfter="false">
    <rotate duration="50" startOffset="0" fromDegrees="0" toDegrees="-20" pivotX="50%" pivotY="50%" repeatCount="1" repeatMode="reverse"
        interpolator="linear"/>
    <rotate duration="50" startOffset="900" fromDegrees="0" toDegrees="20" pivotX="50%" pivotY="50%" repeatCount="1" repeatMode="reverse"
        interpolator="linear"/>
</set>
```

这个动画定义了一个左右摆动的振动效果，关键参数说明：

- `duration`: 单次动画持续时间
- `repeatCount`: 重复次数（-1表示无限重复）
- `repeatMode`: 重复模式（reverse表示往返）
- `interpolator`: 插值器，控制动画速度曲线
- `fromDegrees/toDegrees`: 旋转角度

### 2. 主题资源管理

在主题文件中注册动画资源：

```cpp
// 在主题初始化时
IAnimation* pShakeAni = GetAnimation(L"clock_alarm_shake");

// 当倒计时达到临界点时应用振动动画
if(nSecond <= 10){
    SWindow *pClock = FindChildByID(@ID_ALARM_CLOCK);
    
    if(nSecond == 10){
        // 开始振动动画
        pClock->SetAnimation(pShakeAni);
        // 播放倒计时音效
        PlayEffectSound(L"count_down");
    } else if(nSecond == 0){
        // 清除动画
        pClock->ClearAnimation();
        // 触发超时回调
        OnTimeoutCallback();
    }
}
```

### 3. 完整示例代码

结合前面的示例，完整的时钟更新函数如下：

```cpp
void CMyClock::UpdateClock(int nSecond)
{
    nSecond %= 100;  // 最大显示99秒
    
    SWindow *pClock = FindChildByID(@ID_ALARM_CLOCK);
    pClock->SetUserData(nSecond);
    
    SStackView * pDigitTens = pClock->FindChildByName2<SStackView>(L"digit1");
    SStackView * pDigitUnits = pClock->FindChildByName2<SStackView>(L"digit2");
    
    // 更新数字显示，带动画效果
    pDigitTens->SelectPage(nSecond/10);
    pDigitUnits->SelectPage(nSecond%10);
    
    // 特殊时间点的处理
    if(nSecond == 10){
        // 获取振动动画并应用
        IAnimation * pAni = m_pTheme->GetAnimation(L"clock_alarm_shake");
        pClock->SetAnimation(pAni);
    }else if(nSecond == 0){
        // 清除动画并执行超时操作
        pClock->ClearAnimation();
        OnTimeoutCallback();
    }
    
    // 继续定时器更新
    if(nSecond > 0){
        SetTimer(TIMERID_CLOCK, 1000);
    }
    
    // 最后10秒播放倒计时音效
    if(nSecond > 0 && nSecond <= 10)
    {
        PlayEffectSound(L"count_down");
    }    
}
```

## 总结

通过以上步骤，我们成功地在SOUI中实现了一个动画效果丰富的时钟：

1. 利用StackView控件及其内置动画属性实现数字切换效果
2. 通过XML定义复杂的振动动画效果
3. 在合适的时间节点触发相应的动画和音效

这种实现方式充分利用了SOUI框架的强大功能，既保持了代码的简洁性，又提供了丰富的视觉效果。通过合理的XML布局定义和C++逻辑控制，我们可以轻松实现各种复杂的动画效果，提升用户体验。
