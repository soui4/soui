# SOUI5：全网首款全平台原生兼容的 C++ GUI 框架——一套代码终结多端多团队维护噩梦

> 你的 App 是否还在 iOS、Android 各养一个团队、各维护一套代码？逻辑对不齐、bug 翻倍、功能上线永远不同步？本文介绍全网第一款覆盖 Win/Linux/macOS/Android/iOS 五大平台、且与原生开发完美兼容的 C++ GUI 框架——**SOUI5**。

---

## 一、移动端开发的困局与跨平台困境

### 移动端双团队维护的隐形代价

截至目前，绝大多数商用 App 仍然采用 iOS、Android 各自维护一套原生代码的模式。这看似"各自原生、各平台体验最优"，实则代价沉重：

- **人力翻倍**：同一个功能需要 Swift/Objective-C 团队和 Kotlin/Java 团队各写一遍，人力成本 2x
- **逻辑难对齐**：两个团队、两套语言、两套架构，业务逻辑几乎不可能 100% 一致，边界条件处理差异导致平台特有 bug
- **bug 翻倍**：同一个 bug 可能在 iOS 修了、Android 漏了；或者两端表现不一致，排查需跨团队联调
- **功能上线不同步**：一个平台先发、另一个滞后，运营和用户都在等
- **长期腐化**：两套代码各自的演进路径不同，重构难度倍增，技术债持续累积

这还只是移动端。如果你的产品还要覆盖 Windows、macOS 桌面端，那就是三套、四套代码——维护成本呈指数级增长。

### 跨平台方案的困境

为解决多端维护问题，业界已探索多年，但现有方案各有硬伤：

| 需求 | 可选方案 | 痛点 |
|------|---------|------|
| 全平台覆盖 | Qt | 商用授权费用高、体积大、移动端双线程交互体验差，基本无产品使用 Qt 做移动端 |
| Linux 原生 | GTK | C 语言风格、Windows/macOS 支持弱、不支持移动端 |
| 跨端统一 UI | Flutter | 非"原生"开发、Dart 语言门槛、自绘不使用原生 View、桌面端仍在追赶 |
| Web 技术栈 | Electron | 内存占用高（一个应用一个 Chromium）、性能弱、不支持移动端 |
| React Native | RN | 仅移动端、不支持桌面、Bridge 通信有性能损耗 |
| 纯原生各平台 SDK | iOS+Android+Win+Mac | 四套代码四套团队，维护成本灾难 |

**核心矛盾**：要么牺牲性能/体积/原生体验换跨平台，要么牺牲开发效率换原生体验。**有没有可能两者都要——既全平台一套代码，又与原生开发完美兼容？**

SOUI5 给出的答案是：**用 C++ 和 Windows API 编程模型，编译出全平台原生应用，与各平台原生系统完美兼容。**

---

## 二、SOUI5 是什么

SOUI5 是一个以 C++ 编写的跨平台原生 GUI 框架，当前已完整支持 **Windows、Linux、macOS、Android、iOS** 五大平台，是全网第一款覆盖全平台且与原生开发完美兼容的方案。

它的核心理念可以概括为一句话：

> **用 Windows 的方式开发，在所有平台上原生运行，与各平台原生系统完美兼容。**

"完美兼容"意味着什么？

- **Windows 上**：直接使用 Win32 API + GDI/Direct2D，和原生 Windows 开发完全一致
- **Android 上**：SOUI 的 SouiView 继承自 Android 原生 View，参与原生 View 系统，软键盘、输入法、剪贴板等系统功能直接可用
- **iOS 上**：SOUI 的 SUIView 实现 UITextInput 协议，完美支持 IME，与原生 UIKit 无缝交互
- **Linux/macOS 上**：使用原生窗口系统（X11/Cocoa）+ Cairo/CGContext 渲染

这不是套一层 WebView 的"伪原生"，也不是跑在虚拟机上的"跨平台"。SOUI5 全部使用 C++ 编译为各平台的**原生机器码**，直接调用平台原生图形 API，没有中间层转译开销。一套代码，五个平台，原生运行，原生交互。

### 架构全景

```
┌─────────────────────────────────────────────┐
│              你的业务代码（C++）               │
│         XML 布局 + C++ 逻辑，全平台共享         │
├─────────────────────────────────────────────┤
│                SOUI 框架层                    │
│   控件库 / 插件化渲染 / 资源管理 / 脚本扩展     │
├─────────────────────────────────────────────┤
│              swinx 兼容层                    │
│   Win32 API 在非 Windows 平台的完整实现        │
│   (CreateWindow / GetMessage / GDI / ... )   │
├──────────┬──────────┬──────────┬───────────┤
│ Windows  │  Linux   │  macOS   │ Android/iOS│
│ 原生 Win32│   X11    │  Cocoa  │ JNI/Obj-C  │
│ GDI/D2D  │ Cairo+GDI│CGContext│  Skia     │
│ ~200K    │ swinx 3-4M│swinx 3-4M│          │
└──────────┴──────────┴──────────┴───────────┘
```

要理解 SOUI5 的价值，我们需要从三个层面来看：**框架工程设计**决定代码可维护性，**控件生态**决定开发效率，**跨平台技术**决定覆盖能力。下面依次展开。

---

## 三、框架工程设计

SOUI5 的优秀不仅体现在跨平台能力上，更在于其扎实的框架工程设计。这些设计决定了代码的可维护性和开发体验。

### 1. WTL 风格消息映射表 + 事件映射表：代码集中，维护友好

这是 SOUI5 在**代码可维护性**上最关键的设计决策。SOUI 沿用了 WTL 的消息映射表模式，通过宏将消息/事件 ID 与处理函数集中映射，让相关代码**聚合在一处**：

```cpp
// 消息映射表（WTL 风格）——所有窗口消息处理集中在此
class CMyWindow : public SWindow {
    SOUI_MSG_MAP_BEGIN()
        MSG_WM_PAINT_EX(OnPaint)           // WM_PAINT -> OnPaint
        MSG_WM_ERASEBKGND_EX(OnEraseBkgnd)  // WM_ERASEBKGND -> OnEraseBkgnd
        MSG_WM_LBUTTONDOWN(OnLButtonDown)  // WM_LBUTTONDOWN -> OnLButtonDown
        MSG_WM_TIMER_EX(OnTimer)            // WM_TIMER -> OnTimer
    SOUI_MSG_MAP_END()
};

// 事件映射表——所有 UI 事件处理集中在此
class CMyCtrl : public SWindow {
    EVENT_MAP_BEGIN()
        EVENT_HANDLER(EventButtonClick::EventID, OnButtonClick)
        EVENT_HANDLER(EventItemSelected::EventID, OnItemSelected)
        EVENT_NAME_HANDLER(L"evt_login", OnLoginEvent)
    EVENT_MAP_END()
};
```

**映射表模式的核心价值**：

- **代码集中**：同一个类的所有消息/事件处理函数声明在一张表里，一览无余。要找"这个控件处理了哪些消息"，看一眼映射表即可
- **可维护性强**：新增/删除处理逻辑只需增删一行映射宏，处理函数声明和实现分离，结构清晰
- **继承链转发**：映射表支持 `__baseCls` 自动转发——当前类未处理的消息会自动交给基类处理，事件映射表还支持 `CHAIN_EVENT_MAP` 链式转发到成员对象

**对比 Qt 的 sigslot 模式**：Qt 的信号槽连接代码散落在各处（`connect(sender, signal, receiver, slot)`），查找"哪些对象订阅了这个信号"需要全局搜索。映射表则把处理逻辑集中到类定义内部，符合"高内聚"原则，对于大型项目的长期维护显著更友好。

### 2. 双模式共存：映射表 + sigslot 灵活兼顾

SOUI5 并非只有映射表，它**同时支持类 Qt 的 sigslot 事件订阅模式**，两种模式可以在同一个应用中共存：

```cpp
// 模式一：映射表（集中、可维护）——适合控件内部处理
EVENT_MAP_BEGIN()
    EVENT_HANDLER(EventButtonClick::EventID, OnButtonClick)
EVENT_MAP_END()

// 模式二：sigslot 订阅（灵活、解耦）——适合跨对象动态订阅
pButton->subscribeEvent(EventButtonClick::EventID, 
    Subscriber(&MyHandler::OnButtonClicked, this));

// 也支持自由函数订阅
pButton->subscribeEvent([](EventButtonClick *pEvt) -> BOOL {
    // 处理逻辑
    return TRUE;
});
```

**两全其美**：
- **控件内部处理**用映射表：集中、有序、易维护
- **跨对象动态订阅**用 sigslot：灵活、解耦、适合运行时动态绑定

这种"映射表为主、sigslot 为辅"的设计，既保证了代码的可维护性，又提供了足够的灵活性，比单一模式更务实。

### 3. 类 COM 架构：引用计数 + 接口抽象

SOUI5 借鉴了 COM 的核心思想，在 C++ 层面实现了完整的组件对象模型。

**引用计数**：所有 SOUI 对象的基接口是 `IObjRef`，提供 `AddRef / Release / OnFinalRelease` 三个方法，是 SOUI 对象生命周期管理的基石。`IObject`（SOUI 对象基类）、`IWindow`（窗口基类）、`ISkinObj`（皮肤基类）等核心接口全部继承自 `IObjRef`：

```cpp
// IObjRef —— 所有 SOUI 对象的根接口
DECLARE_INTERFACE(IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;            // 增加引用计数
    STDMETHOD_(long, Release)(THIS) PURE;           // 减少引用计数，为0时自销毁
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;   // 释放前回调
};

// IObject 继承 IObjRef，增加类信息、属性、XML 序列化等能力
DECLARE_INTERFACE_(IObject, IObjRef) { ... };

// IWindow 继承 IObject，增加窗口消息、绘制、布局等能力
DECLARE_INTERFACE_(IWindow, IObject) { ... };
```

配合 `SAutoRefPtr<T>` 智能指针，实现 RAII 式自动引用管理，杜绝内存泄漏和悬挂指针：

```cpp
SAutoRefPtr<IRenderTarget> pRT = ...;  // 构造时 AddRef，析构时自动 Release
```

### 4. 类型安全的对象系统

SOUI5 实现了完善的类 RTTI（运行时类型识别）机制，在不依赖 C++ RTTI 的前提下提供了类型安全保证：

- **`DEF_SOBJECT` 宏**：每个 SOUI 对象类通过宏注册类名和基类信息，编译期生成 `GetClassName()` / `IsClass()` 等类型元信息
- **`sobj_cast<T>`**：类型安全的向下转换，转换失败返回 `nullptr`，比 `dynamic_cast` 更轻量：

```cpp
IObject *pObj = objFactory->CreateObject(SObjectInfo(L"button", Window));
SButton *pBtn = sobj_cast<SButton>(pObj);  // 类型安全，失败返回 NULL
```

- **`SObjectType` 枚举**：对象分类（Window/Skin/Layout/Animation/Interpolator 等），工厂创建时按类别查找，避免跨类型误用

---

## 四、控件生态与扩展体系

好的设计理念需要丰富的功能来支撑。SOUI5 提供了覆盖客户端开发所需的完整控件库和可靠的扩展机制，全部基于 XML 驱动、皮肤系统驱动。

### 1. 完整的控件体系

| 类别 | 控件 |
|------|------|
| **基础控件** | SWindow（基类）、SStatic、SButton、SImageWnd、SProgress、SImageButton、SCheckBox、SRadioBox |
| **编辑控件** | SEdit、SRichEdit（富文本）、SComboEdit、SComboBox、SComboView |
| **列表控件** | SListCtrl、SListBox、SListView（虚表）、SMCListView（多列虚表）、STileView（瓦片虚表）、STreeView（树形虚表）、STreeCtrl |
| **容器控件** | SPanel、STabCtrl/STabPage、SSplitPane/SSplitWnd、SStackView/SStackPage、SScrollView、SFrame |
| **功能控件** | SScrollBar、SHeaderCtrl、SSliderBar、SHotKeyCtrl、SSpinButtonCtrl、SCalendar、SDateTimePicker |
| **皮肤系统** | SSkinImgList、SSkinImgFrame、SSkinImgCenter、SSkinButton、SSkinGradation 等 |
| **动画系统** | SAnimation、SAnimationSet、SAlphaAnimation、SScaleAnimation、SRotateAnimation、STranslateAnimation |
| **插值器** | SLinearInterpolator、SAccelerateInterpolator、SBounceInterpolator 等 |
| **值动画** | SValueAnimator、IntAnimator、FloatAnimator、ColorAnimator |

所有控件支持 XML 属性配置、CSS 风格样式表、脚本扩展（Lua/JavaScript），形成完整的 DirectUI 生态。

### 2. 高性能虚表控件：大数据量场景的利器

在上述控件库中，有一套专为大数据量场景设计的虚表控件族，**只渲染可视区域内的 item**，无论数据量多大都保持流畅：

| 控件 | 类名 | XML 标签 | 适用场景 |
|------|------|---------|---------|
| **单列列表** | `SListView` | `listview` | 单列垂直滚动列表（聊天记录、文件列表） |
| **多列列表** | `SMCListView` | `mclistview` | 带表头的多列列表（表格数据、详情列表） |
| **瓦片网格** | `STileView` | `tileview` | 等宽高瓦片网格（图片墙、图标网格） |
| **树形列表** | `STreeView` | `treeview` | 支持展开/折叠的树形结构（目录树、组织架构） |

**虚表机制原理**：

```
数据源（ILvAdapter）          可视区域             渲染池（回收复用）
┌──────────────┐          ┌─────────────┐      ┌──────────────────┐
│ Item 0       │          │             │      │ 复用的 Item View  │
│ Item 1       │   ┌───▶  │ Item 5      │ ◀─┐  │ (3-5 个，循环复用) │
│ Item 2       │   │      │ Item 6      │   │  │                  │
│ Item 3       │   │      │ Item 7      │   └──│ 绑定数据后显示    │
│ Item 4       │   │      │ Item 8      │      │                  │
│ Item 5  ─────┘   │      └─────────────┘      │ 滚出可视区后回收到池│
│ ...           │      滚动时动态计算           │                  │
│ Item 99999   │      可见 item 范围           └──────────────────┘
└──────────────┘
```

**核心设计**：
- **数据源接口**（`ILvAdapter` / `ITvAdapter`）：数据与视图分离，业务层只实现 `getItemCount` 和 `getView`，框架负责按需创建和复用
- **Item 定位器**（`ItemLocator`）：计算每个 item 的位置和尺寸，支持固定高度和可变高度两种模式
- **View 回收池**：滚动出可视区域的 item view 回收复用，避免反复创建销毁
- **DataSetObserver**：数据变化时通知控件局部刷新，而非全量重建

**性能表现**：10 万条数据列表滚动保持 60fps，内存占用仅取决于可视区可见 item 数（通常 5-10 个 view），与数据总量无关。

### 3. 可靠的扩展机制：窗口/皮肤/布局的统一注册

SOUI5 的窗口控件、皮肤、布局三大可扩展模块全部通过统一的 `SObjectFactoryMgr` 工厂管理，提供一致的注册和创建机制：

**注册方式一：编译期模板注册**（框架内置控件使用）

```cpp
// SApp.cpp 中框架启动时注册全部内置控件
objFactory->TplRegisterFactory<SButton>();        // 窗口控件
objFactory->TplRegisterFactory<SSkinImgList>();   // 皮肤
objFactory->TplRegisterFactory<SLinearLayout>();  // 布局
```

**注册方式二：运行时动态注册**（用户自定义扩展使用）

```cpp
// 注册自定义窗口控件——一行代码即可让 XML 识别新标签
app.RegisterWindowClass<MyCustomCtrl>();    // XML: <myctrl .../>
app.RegisterSkinClass<MyCustomSkin>();     // XML: <skin name="MySkin" .../>
app.RegisterLayoutClass<MyCustomLayout>();  // XML: layout="MyLayout"
```

**注册方式三：外部模块动态加载**（插件化部署）

SOUI 提供 `SComLoader` 类，支持从动态库（DLL/SO/dylib）运行时加载组件：

```cpp
SComLoader loader;
SAutoRefPtr<IUnknown> pObj;
loader.CreateInstance("myplugin", &pObj);  // 加载 myplugin.dll/.so/.dylib
// 适用于渲染插件、资源提供者插件等场景
```

### 4. 内置布局体系

| 布局 | XML 标签 | 说明 |
|------|---------|------|
| `SouiLayout` | 默认 | 绝对定位，子控件通过 x/y/width/height 定位 |
| `SLinearLayout` | `linearLayout` | 线性布局（垂直或水平） |
| `SHBox` | `hbox` | 水平线性布局（Android 风格） |
| `SVBox` | `vbox` | 垂直线性布局（Android 风格） |
| `SGridLayout` | `grid` | 网格布局 |
| `SFlowLayout` | `flow` | 流式布局（自动换行） |
| `SAnchorLayout` | `anchor` | 锚点布局（相对定位） |
| `SFrameLayout` | `frame` | 提供MFC类似的CMainFrame框架布局，更好的支持toolbar, statusbar, dockbar等 |

所有布局都支持自定义 LayoutParams（margin/padding/weight/gravity 等），并支持外部扩展自定义布局算法。

---

## 五、全平台原生技术

有了完善的框架设计和控件生态，接下来看看 SOUI5 如何把这些能力延伸到五大平台。跨平台能力的核心在于 swinx 兼容层，以及在此基础上构建的渲染、部署和适配技术。

### 1. swinx：Windows API 的全平台兼容层

这是 SOUI5 最硬核的技术。`swinx` 是一个类似 Wine 但专为客户端 GUI 设计的兼容层，它实现了作为客户端应用所需的**全部核心 Windows API**，并与 Windows API **安全兼容**。

什么意思？你在 Linux、macOS、Android、iOS 上写的是这样的代码：

```cpp
// 这段代码在 Windows、Linux、macOS、Android、iOS 上完全一致
HWND hWnd = CreateWindowEx(0, L"MyWindowClass", L"标题",
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600,
    NULL, NULL, hInstance, NULL);

ShowWindow(hWnd, SW_SHOW);

MSG msg;
while (GetMessage(&msg, NULL, 0, 0))
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

**无需 `#ifdef`，无需平台分支，直接编译运行。** swinx 在底层将这些调用映射到各平台的原生窗口系统：Linux 上创建 X11 窗口，macOS 上创建 NSWindow，Android 上创建 SouiView 挂载到 ViewGroup，iOS 上创建 UIView。

**与 Wine 的关键区别**：Wine 是一个完整的 Windows 运行时（包含注册表、PE 加载器等），体积庞大；swinx 只实现 GUI 客户端所必需的 API 子集，轻量且高效，是 SOUI 框架的专用基础设施。

**关于体积**：swinx 在不同平台上的实现方式不同，体积也有所差异。Linux 上 GDI 兼容依赖 Cairo 实现，Cairo 编译后约 3-4MB；Apple 平台（macOS/iOS）的 GDI 兼容直接使用原生 CGContext 实现，不依赖 Cairo，但 swinx 自身编译后同样约 3-4MB。Windows 上则无需 swinx，直接使用系统原生 API。

### 2. 全 C++ 开发：Android 上比"原生"更快

Android 平台依赖 Java 虚拟机，"原生开发"通常意味着用 Java/Kotlin 写 UI + JNI 调 C++ 做计算。SOUI5 的做法截然不同：

- **UI 渲染全部用 C++**：直接绘制，跳过 Android View 系统的 Java 开销
- **事件处理全部用 C++**：触摸事件从 Native 层直接进入 C++ 消息循环
- **唯一的 Java 代码**：一个精简的 `SouiPlatformBridge` 桥接类，仅负责 View 创建、软键盘、定时器等平台特性代理

这意味着：
- **没有 JNI 频繁调用的性能损耗**（JNI 只在窗口创建/销毁等低频操作时使用）
- **没有 Java GC 带来的 UI 卡顿**
- **渲染性能可以超越 Android 原生 View 系统**

实测在复杂列表滚动、动画密集场景下，SOUI5 的帧率表现优于同等功能的 Android 原生实现。

### 3. 插件化渲染：按需选择，体积可控

SOUI5 的渲染层是**插件化**的，开发者可以根据目标平台和场景灵活选择渲染后端：

| 渲染后端 | 适用场景 | 模块体积 |
|---------|---------|---------|
| **GDI** | Windows 原生渲染，零依赖 | ~200KB |
| **Direct2D** | Windows 硬件加速渲染 | ~200KB |
| **Skia** | 全平台统一渲染（Linux/macOS/Android/iOS） | 3-4MB |
| **Cairo**（swinx 内置） | Linux 上 GDI 兼容实现 | 3-4MB |

在 Windows 上，使用原生 GDI 或 Direct2D 渲染，渲染模块仅 200KB 左右，应用整体可以做到非常精简。在移动端和 Apple 平台上使用 Skia 获得一致的跨平台渲染效果。这种按需选择的设计让 SOUI5 在不同场景下都能兼顾性能和体积。

### 4. MinGW RC 编译：单文件绿色软件

这是桌面端开发者会爱不释手的特性。SOUI5 通过 MinGW 的 `windres` 工具，在**所有桌面平台**（包括 Linux 和 macOS）上都能编译 Windows `.rc` 资源文件。

效果是什么？你可以把所有 UI 资源（XML 布局、图片、字体、音效）嵌入到一个 `.rc` 文件中，编译链接后得到**单个可执行文件**：

```
你的应用.exe（Windows）/ 你的应用（Linux）/ 你的应用.app（macOS）
├── 程序代码
├── 所有 UI 资源（嵌入在 PE/ELF/Mach-O 节区中）
└── 无需任何外部依赖文件
```

- Windows 上：一个 `.exe` 文件即可运行，绿色免安装
- Linux 上：一个 ELF 可执行文件，资源嵌入在二进制中
- macOS 上：一个 `.app` Bundle，资源嵌入在 Mach-O 中

**对比 Electron 动辄 150-300MB 的体积**，SOUI5 在 Windows 上使用 GDI 渲染时，整个应用可以控制在 3-5MB 以内。

### 5. SVG 引擎级支持：完美的高分屏方案

高分屏（Retina、4K）适配是跨平台框架的痛点。传统方案要么为每个分辨率准备多套图片（笨重），要么依赖外部 SVG 库做光栅化（与框架割裂）。

SOUI5 的 SVG 支持与一般框架有本质区别——**不是简单地内置一个 SVG 解析器并光栅化渲染，而是在引擎层面将 SVG 完美适配到 SOUI 的皮肤框架中**：

- **皮肤框架原生集成**：SVG 不只是一个"可渲染的图片格式"，而是作为 SOUI 皮肤系统的一等公民。控件的状态切换（按钮 hover/press/disabled）、九宫格拉伸、动画帧等皮肤机制全部支持 SVG 格式
- **功能全面替代 PNG**：在 SOUI 应用中，SVG 可以完全替代 PNG——按钮背景、图标、皮肤动画全部使用 SVG，无需准备任何位图资源
- **小内存占用**：一个 SVG 矢量描述替代多套不同分辨率的 PNG，内存占用显著降低
- **高分屏完美适配**：矢量图在 1x/2x/3x 屏幕上自动完美缩放，无锯齿，无需为每个 DPI 准备独立资源
- **渲染高效**：SVG 解析后缓存为渲染路径，绘制效率接近位图

这不是"能不能渲染 SVG"的问题，而是"SVG 能否真正融入 UI 引擎"的问题。SOUI5 在这一点上做到了深度集成，让 SVG 从"图片"升级为"皮肤资源"。

---

## 六、实战案例：一个中国象棋，五平台共享代码

说再多不如看实例。如果用传统方案，一个中国象棋 App 要覆盖五个平台，需要 iOS、Android、Windows、macOS 四个团队、四套代码。用 SOUI5 呢？**一个项目，五平台原生运行，95% 代码完全共享。** 来看看仓库中自带的 cnchess 客户端。

### 代码共享策略

```
games/cnchess/client/
├── MainDlg.cpp          ← 全平台共享（主窗口、消息处理、游戏逻辑）
├── MainDlg.h
├── WebSocketClient.cpp  ← 全平台共享（网络通信）
├── ChessBoard.cpp       ← 全平台共享（棋盘渲染）
├── ChessPiece.cpp       ← 全平台共享（棋子渲染）
├── ChessGame.cpp        ← 全平台共享（游戏规则）
├── ThemeDownloadManager.cpp ← 全平台共享（主题热更新）
├── CnchessSkin.cpp      ← 全平台共享（自定义皮肤）
├── utils.cpp            ← 全平台共享
├── main.cc              ← 桌面端入口（Windows/Linux/macOS/iOS 共享）
├── android_entry.cc     ← Android 专用入口（启动器）
└── uires/               ← 桌面端 UI 资源
    └── android/app/src/main/assets/uires/  ← 移动端 UI 资源
```

**结论**：除 Android 需要一个约 100 行的启动器（`android_entry.cc`）外，**其余所有平台完全共享同一份 C++ 代码**。

### 入口文件的优雅处理

桌面平台（Windows/Linux/macOS）和 iOS 共享一个 `main.cc` 入口：

```cpp
// main.cc —— Windows/Linux/macOS/iOS 共享入口
int WINAPI _tWinMain(HINSTANCE hInstance, ...)
{
    SApplication app(hInstance);
    // 配置渲染后端、图片解码器
    SAppCfg cfg;
    cfg.SetRender(Render_Skia)
       .SetImgDecoder(ImgDecoder_Stb);

    // 创建主窗口（同一个 CMainDlg，全平台通用）
    CMainDlg dlgMain(theme);
    dlgMain.Create(GetActiveWindow());
    dlgMain.SendMessage(WM_INITDIALOG);
    dlgMain.ShowWindow(SW_SHOWNORMAL);

    return app.Run(dlgMain.m_hWnd);  // 标准消息循环
}

// iOS：由 swinx 的 ios_entry 桥接到 _tWinMain
#if defined(__IOS__)
int main(int argc, char **argv) {
    return swinx_ios_entry(argc, argv, _tWinMain);
}
// Linux/macOS：直接调用
#elif !defined(_WIN32)
int main(int argc, char **argv) {
    HINSTANCE hInst = GetModuleHandle(NULL);
    return _tWinMain(hInst, 0, NULL, SW_SHOWNORMAL);
}
#endif
```

Android 入口（`android_entry.cc`）同样创建 `CMainDlg`，只是通过 JNI 桥接到 Java 层的 `SouiScreen`：

```cpp
// android_entry.cc —— Android 专用启动器
void CnchessAndroidApp::ScreenStartup(...) {
    SApplication app(...);
    SAppCfg cfg;
    cfg.SetRender(Render_Skia).SetImgDecoder(ImgDecoder_Stb);
    cfg.DoConfig(&app);

    CMainDlg *pDlg = new CMainDlg(theme);  // 同一个 CMainDlg！
    pDlg->Create(NULL);
    pDlg->SendMessage(WM_INITDIALOG);
    pDlg->ShowWindow(SW_SHOWNORMAL);
}
```

### 平台差异的极简处理

极少数需要平台差异的地方，用编译宏优雅处理：

```cpp
// 主题缓存目录（MainDlg.cpp）
#if defined(__ANDROID__) || defined(__IOS__)
    SHGetSpecialFolderPath(0, szCachePath, CSIDL_PERSONAL, TRUE);  // 移动端沙盒目录
#else
    m_strThemeCacheDir = SApplication::GetAppDir();                // 桌面端应用目录
#endif

// 主题请求带上平台 ID（ThemeDownloadManager.cpp）
#if defined(__ANDROID__)
    req.dwOSId = OS_ID_ANDROID;
#elif defined(__IOS__)
    req.dwOSId = OS_ID_IOS;
#else
    req.dwOSId = OS_ID_WINDOWS;
#endif
// 服务器据此下发不同平台的主题资源包
```

这就是 SOUI5 跨平台开发的全部——**没有 `#ifdef` 地狱，没有平台分支维护**，95% 以上的代码完全共享。

---

## 七、横向对比：SOUI5 vs Qt vs GTK vs Flutter vs Electron

| 维度 | SOUI5 | Qt | GTK | Flutter | Electron |
|------|-------|-----|-----|---------|----------|
| **支持平台** | Win/Linux/macOS/Android/iOS | Win/Linux/macOS/Android/iOS | Linux为主,Win/mac弱,无移动端 | Win/Linux/macOS/Android/iOS/Web | Win/Linux/macOS |
| **开发语言** | C++ | C++/QML | C/C++ | Dart | JS/TS |
| **编程模型** | Win32 API 兼容 | Qt 自有 API | GObject | Widget 树 | Web DOM |
| **渲染方式** | 插件化：GDI/D2D/Skia 可选 | 软光栅/GL | Cairo/GL | Skia | Chromium |
| **移动端原生交互** | 深度集成，直接操作原生 View | 双 UI 线程，交互体验差 | 不支持移动端 | 自绘，不使用原生 View | 不支持移动端 |
| **Windows 体积** | ~3-5MB（GDI/D2D 渲染） | 30-80MB | 5-15MB | 5-15MB | 150-300MB |
| **内存占用** | 低 | 中 | 中低 | 中 | 高 |
| **API 学习成本** | 会 Win32 即可上手 | 需学 Qt 信号槽/元对象 | 需学 GObject | 需学 Dart+Widget | 会 Web 即可 |
| **单文件绿色** | 支持（RC嵌入） | 不支持 | 不支持 | 不支持 | 不支持 |
| **SVG 支持** | 引擎级，适配皮肤框架，全面替代PNG | 需 QSvg，光栅化渲染 | 需 librsvg | 内置光栅化 | 内置(CSS) |
| **授权协议** | 个人免费，商用收费（远低于Qt） | LGPL/商业（商用较贵） | LGPL | BSD | MIT |

### 关键差异解读

**vs Qt**：两者都支持全平台，但移动端的实现方式天差地别。Qt 在 Android 上采用了**双 UI 线程方案**——Qt 的 UI 线程和 Android 原生的 UI 线程分离运行，导致与原生组件交互时需要跨线程通信，体验非常差。SOUI5 则直接在原生 View 上绘制，UI 逻辑全在 C++ 层，与原生功能（软键盘、输入法、剪贴板等）的交互是同一级别，体验完全原生。

此外，Qt 商业授权费用较高，SOUI5 采用自定义协议——**个人用户免费，商用适当收费，费用远低于 Qt**。

在体积方面，SOUI5 在 Windows 上使用 GDI/D2D 渲染仅约 3-5MB，Qt 动辄 30-80MB。Qt 的生态和文档更成熟是其优势。

**vs Flutter**：Flutter 需要学 Dart，SOUI5 用 C++ 一把梭。Flutter 桌面端仍在发展期，SOUI5 桌面端已成熟。Flutter 的热重载是优势，但 SOUI5 的原生性能在移动端更优（尤其 Android 上绕过了 View 体系开销）。Flutter 的 SVG 支持仅限于光栅化渲染，而 SOUI5 在引擎层面深度集成皮肤框架。

**vs Electron**：不在一个量级。Electron 本质是打包 Chromium 的 Web 应用，内存动辄数百 MB；SOUI5 是编译型原生应用，内存占用极低。如果你的应用不需要 Web 技术栈，SOUI5 在各方面都优于 Electron。

**vs GTK**：GTK 是 Linux 桌面的好选择，但跨平台能力弱（Windows/macOS 支持差，不支持移动端）。SOUI5 在 Linux 上同样原生，且覆盖全平台。C++ 开发体验远优于 GTK 的 C 风格。

---

## 八、什么时候该选 SOUI5

**强烈推荐**：
- 有 Windows / Win32 开发背景，想低成本扩展到全平台
- 注重应用体积和性能（工具类、游戏类应用）
- 需要单文件绿色部署
- C++ 技术栈团队，不想引入额外语言
- 桌面+移动端同时开发，希望最大化代码复用

**可以考虑**：
- 新项目从零开始，评估全平台 C++ 方案
- 现有 Windows 应用想跨平台，不想重写

**可能不适合**：
- 团队只有 Web 前端背景（选 Electron/Tauri）
- 需要 Web 内容嵌入（WebView 场景）
- 依赖大量 Qt 生态库的现有项目

---

## 九、总结：一套代码，终结多端维护噩梦

回到开篇的问题：iOS、Android 各养一个团队、各维护一套代码，人力翻倍、bug 翻倍、功能上线不同步——这个困扰移动开发行业十多年的顽疾，终于有了根治方案。

SOUI5 的独特价值在于：**它是全网第一款覆盖全平台且与原生开发完美兼容的 C++ GUI 框架。** 它没有发明新的编程模型，而是把最成熟的 Windows 客户端编程模型（Win32 API + 消息循环）扩展到了所有平台，同时与各平台原生系统深度集成、完美兼容。

这意味着：

- **一套代码替代四套**：Windows/Linux/macOS/Android/iOS 共享 95%+ 的 C++ 代码，一个团队、一套逻辑，从根源上消灭平台间 bug 不一致问题
- **原生兼容而非模拟**：Android 上继承原生 View，iOS 上实现 UITextInput 协议，软键盘、输入法、剪贴板等系统功能与原生开发完全一致
- **学习成本最低**：会 Win32 API 就会全平台开发，无需学习 Dart、QML、GObject 等平台专属技术
- **性能损耗最小**：原生 C++ 编译，无虚拟机，无中间层，Android 上甚至比原生 Java/Kotlin View 系统更快
- **开发体验一致**：XML 布局 + C++ 逻辑 + 消息映射表，桌面和移动端完全统一流程

对于还在 iOS/Android 双端维护的团队来说，SOUI5 不仅仅是一个 GUI 框架——它是一次架构层面的降本增效：一个团队干两个团队的活，一套代码跑五个平台，bug 不再翻倍，功能不再滞后。

在跨平台 GUI 框架百花齐放的今天，SOUI5 提供了一个务实而强大的选择：不追求花哨的新概念，而是用扎实的工程实现，让 C++ 开发者真正实现"一次编写，全平台原生运行"。

如果你对 SOUI5 感兴趣，可以访问项目仓库查看完整的源码和中国象棋实战示例。

- **Gitee**：https://gitee.com/setoutsoft/soui4
- **GitHub**：https://github.com/soui4/soui

---

*SOUI5 —— 全网首款全平台原生兼容 C++ GUI 框架，一套代码，终结多端维护噩梦。*
