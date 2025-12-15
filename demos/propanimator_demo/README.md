# SOUI布局动画演示

这个演示项目展示了SOUI新的Android风格布局动画API的使用方法。

## 功能特性

### 支持的动画类型
- **宽度动画** - 动画改变窗口宽度
- **高度动画** - 动画改变窗口高度  
- **大小动画** - 同时动画宽度和高度
- **权重动画** - 动画改变线性布局中的权重
- **多属性动画** - 同时动画多个属性，支持关键帧
- **偏移动画** - 动画改变窗口偏移量

### Android风格API设计
参考Android ObjectAnimator和PropertyValuesHolder的设计模式：

```cpp
// 简单属性动画
ILayoutObjectAnimator *animator = LayoutObjectAnimator::ofLayoutSize(
    pLayoutParam, LayoutProperty::WIDTH, values, 2);
animator->setDuration(1000)->start();

// 多属性动画
IPropertyValuesHolder *holders[] = { widthHolder, heightHolder };
ILayoutObjectAnimator *animator = LayoutObjectAnimator::ofPropertyValuesHolder(
    pLayoutParam, holders, 2);
animator->setDuration(1200)->start();
```

## 项目结构

```
layout_animation_demo/
├── CMakeLists.txt          # CMake构建配置
├── main.cpp                # 程序入口点
├── MainDlg.h              # 主对话框头文件
├── MainDlg.cpp            # 主对话框实现
├── stdafx.h               # 预编译头文件
├── resource.h             # 资源定义
├── layout_animation_demo.rc # 资源脚本
├── build_rc2.bat          # 资源构建脚本
├── res/                   # 资源文件目录
│   └── soui.ico          # 应用图标
└── uires/                 # UI资源目录
    ├── uires.idx         # 资源索引文件
    ├── uidef/            # UI定义
    ├── values/           # 值资源
    ├── xml/              # XML布局
    └── image/            # 图片资源
```

## 构建说明

### 前提条件
1. 已安装SOUI4开发环境
2. 设置了`SOUI4_INSTALL_64`环境变量（64位）或`SOUI4_INSTALL_32`（32位）
3. CMake 3.16或更高版本

### 构建步骤

#### 方法1: 使用CMake（推荐）
```bash
# 在soui4根目录下
mkdir build
cd build
cmake ..
cmake --build . --target layout_animation_demo
```

#### 方法2: 单独构建
```bash
cd demos/layout_animation_demo
mkdir build
cd build
cmake ..
cmake --build .
```

### 资源构建
如果需要手动构建UI资源：
```bash
cd demos/layout_animation_demo
build_rc2.bat
```

## 使用说明

### 界面说明
- **动画演示区域**: 显示一个线性布局容器，包含3个窗口（2个静态窗口和1个动画目标窗口）
- **控制按钮**: 点击不同按钮触发不同类型的动画
- **API示例**: 底部显示相关API的使用代码示例

### 动画效果
1. **宽度动画**: 目标窗口宽度从100dp变化到300dp
2. **高度动画**: 目标窗口高度从50dp变化到150dp
3. **大小动画**: 同时改变宽度和高度
4. **权重动画**: 改变线性布局中的权重值
5. **多属性动画**: 使用关键帧同时动画多个属性
6. **偏移动画**: 改变窗口的X偏移量
7. **重置**: 恢复所有属性到默认值

### 事件响应机制
项目使用SOUI标准的事件响应机制：

```cpp
// 在头文件中声明事件映射
EVENT_MAP_BEGIN()
    EVENT_NAME_COMMAND(L"btn_width_anim", OnWidthAnimation)
    EVENT_NAME_COMMAND(L"btn_height_anim", OnHeightAnimation)
    // ... 其他事件
EVENT_MAP_END2(SHostWnd)

// 在实现文件中定义事件处理函数
void CMainDlg::OnWidthAnimation()
{
    // 动画实现代码
}
```

## 技术要点

### 动画监听器
项目实现了自定义的动画监听器来处理动画完成后的布局更新：

```cpp
class CAnimationListener : public IAnimatorListener
{
public:
    void onAnimationEnd(IValueAnimator *animator) override 
    {
        // 动画结束后触发重新布局
        m_pMainDlg->m_pLinearContainer->RequestRelayout();
    }
};
```

### 属性类型支持
- `SLayoutSize` - 布局大小（支持dp、px等单位）
- `float` - 浮点数（权重、偏移等）
- `int` - 整数（跨度等）

### 关键帧动画
支持多个关键帧的复杂动画序列：

```cpp
SLayoutSize keyframes[] = {
    SLayoutSize(80.0f, SLayoutSize::dp),   // 起始
    SLayoutSize(160.0f, SLayoutSize::dp),  // 中间
    SLayoutSize(120.0f, SLayoutSize::dp)   // 结束
};
```

## 扩展说明

这个演示项目可以作为学习SOUI布局动画的起点，开发者可以：

1. 添加更多动画类型（网格布局、锚点布局等）
2. 实现自定义插值器
3. 添加动画序列和组合
4. 集成到实际项目中

## 相关文档

- [SOUI布局动画API使用指南](../AndroidStyleLayoutAnimation_README.md)
- [SOUI布局动画系统设计总结](../SOUI_AndroidStyle_LayoutAnimation_Summary.md)
