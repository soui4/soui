# 一套 C++ 代码，桌面与移动端通吃：SOUI for Android 实战移植中国象棋

> 我最近把一个原本运行在 Windows、Linux、macOS 桌面端的中国象棋联网游戏，使用 SOUI for Android 移植到了 Android 平台。整个移植过程几乎没有改动业务代码，C++ 主窗口类、游戏逻辑、UI 布局 XML 全部原样复用。本文结合这次实战，聊聊 SOUI 是如何做到"原生开发无缝集成"的，以及它为什么是目前唯一能达到这种水平的 C++ 跨端 UI 框架。
>
> 先交代一下背景：SOUI 此前已经完成了 **Windows、Linux、macOS** 三大桌面端的支持，移动端 Android 也已落地，iOS 后续补齐。也就是说，它不是"只在 Windows 上跑得动"的框架，而是一开始就奔着全平台统一去的。这次移植中国象棋，正好把"桌面端 → 移动端"这条链路走通了一遍。

{{视频：中国象棋 Android 实机演示}}

## 一、为什么说"无缝集成"？先看代码

很多跨平台框架都在讲"一次编写，多处运行"，但真正落地时往往要妥协：要么性能拉胯，要么在原生平台上嵌一个格格不入的"网页壳"，要么得用框架自己的 DSL 重写一遍 UI。

SOUI for Android 走的是另一条路——**把 C++ 的窗口系统直接嫁接到 Android 的 View 体系里**。

Java 侧的 `GameActivity` 长这样，和普通 Android 代码没有任何区别：

```java
public class GameActivity extends AppCompatActivity {
    private SouiScreen mScreen;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // SouiScreen 就是一个普通的 Android View
        mScreen = new SouiScreen(this, SCREEN_ID, LAYOUT_MAIN);
        FrameLayout decor = new FrameLayout(this);
        decor.addView(mScreen, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT));
        setContentView(decor);
    }
}
```

`SouiScreen` 是一个标准的 Android `View`，可以放进任何 `ViewGroup`，可以和原生控件混排，可以响应屏幕旋转、`requestLayout`、生命周期——和写原生 Android 应用一模一样。

C++ 侧通过实现一个 `Soui4AndroidEntry` 抽象接口接入平台层，业务代码完全感知不到自己跑在 Android 上：

```cpp
class CnchessAndroidApp : public Soui4AndroidEntry {
    SApplication* m_souiApp = nullptr;
    std::map<long, SAutoRefPtr<SHostWnd>> m_screenHostMap;
public:
    BOOL InitApp(AAssetManager* assetMgr, LPCSTR pszAssetDir) override;
    HWND ScreenStartup(long screenId, LPCSTR pszLayout) override;
    void ScreenShutdown(long screenId) override;
    void UninitApp() override;
};
```

资源通过 Android 的 `AAssetManager` 直接加载，不需要先把 assets 拷贝出来再解压——这是真正意义上的"原生集成"，而不是套一层壳。

## 二、桌面版 vs Android 版：代码复用度有多高？

这是最让我惊讶的部分。下面是同一个 `CMainDlg` 类，左边是 Windows 桌面版，右边是 Android 版：

```cpp
// 两边完全一致
class CMainDlg : public SHostWnd
               , public SDpiHandler<CMainDlg>
               , public WebSocketClient::IListener
{
    CChessGame*        m_pGame;          // 游戏核心逻辑
    LobbyHandler*      m_pLobbyHandler;  // 大厅处理
    WebSocketClient    m_webSocketClient;// 网络通信
    SAutoRefPtr<SGameTheme> m_pTheme;    // 主题
    BOOL               m_bMute;

    BEGIN_MSG_MAP_EX(CMainDlg)
        CHAIN_MSG_MAP(SDpiHandler<CMainDlg>)
        MSG_WM_INITDIALOG(OnInitDialog)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_SIZE(OnSize)
        CHAIN_MSG_MAP_MEMBER(*m_pGame)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP()
};
```

**头文件逐行一致**。消息映射、事件映射、DPI 处理、WebSocket 网络通信、游戏逻辑、主题管理——全部原样复用。UI 布局 XML、皮肤资源、SVG 图标也是同一套。

也就是说：从 Windows 桌面端到 Android 移动端，**核心代码改动量接近于零**，只需要写一个薄薄的 Java 入口和一个 C++ 平台适配入口。这种级别的复用，目前我没有在其它任何 C++ UI 框架上看到过。

## 三、C++ 跨端：高性能 + 真正的统一

很多人对"跨平台 UI"的印象还停留在 React Native、Flutter、Qt 这类方案。它们各有短板：

- **RN/Flutter**：用自己的渲染管线，和原生平台始终有隔阂，内存占用高；
- **Qt**：跨端能力强，但授权模式、体积、双UI线程，与原生 View 体系的融合都不够顺滑；
- **WebView 系**：性能天花板太低。

SOUI 选择 C++ 作为开发语言，带来的好处是实打实的：

1. **全平台统一**：SOUI 此前已经完成了 **Windows、Linux、macOS** 三大桌面端的覆盖，移动端 Android 也已落地，iOS 后续补齐后，**桌面端 + 移动端可以共用同一份 C++ 业务代码和 UI 资源**；
2. **原生性能**：Skia 渲染、C++ 直接操作内存，没有 JS Bridge 的开销，象棋这种需要动画+音效+网络同步的场景跑得非常顺；
3. **与平台 View 体系无缝融合**：Android 上是 `SouiScreen` 这个 View，可以和 `RecyclerView`、`Fragment` 自由组合，不存在"框架绑架整个 App"的问题。

这套方案特别适合**已经有桌面端产品、希望快速拓展移动端**的团队——无论你的桌面端跑在 Windows、Linux 还是 macOS，C++ 业务逻辑、算法、网络协议层都可以直接搬过来。

### 移动端体验适配：手指滑动与惯性滚动

跨端移植最容易翻车的不是代码能不能编译过，而是**交互体验能不能跟上平台习惯**。桌面端用鼠标滚轮 + 拖动滚动条，移动端用手指滑动 + 惯性 fling——如果框架只在桌面端打磨过，搬到手机上就会变成"能用但难用"。

SOUI 在这点上做了原生级别的适配。所有可滚动控件的基类 `SPanel`（定义在 `SOUI/include/core/SPanel.h`）从设计之初就内置了 **drag-to-scroll 和 fling animation 支持**：

```cpp
/**
 * @class     SPanel
 * @brief     Panel with Scrollbar Support
 *
 * Description: Implements a panel with support for scrollbars in the non-client area.
 *              Includes drag-to-scroll and fling animation support.
 */
class SOUI_EXP SPanel
    : public TWindowProxy<IPanel>
    , protected IScrollBarHost
    , public IAnimatorUpdateListener
    , public SAnimatorListener {
    ...
};
```

注意它同时继承了 `IAnimatorUpdateListener` 和 `SAnimatorListener`——这意味着滚动惯性动画是和 SOUI 自己的 `SValueAnimator` 动画系统深度整合的，而不是临时拼接的补丁。

由于 `SListView`、`SMCListView`、`STreeView`、`SScrollView` 等所有可滚动控件都继承自 `SPanel`，**它们在 Android 上自动获得手指滑动 + 惯性滚动 + 滚动条隐显的能力**，不需要业务层写一行适配代码。象棋 Demo 里的"大厅玩家列表""对局列表"在手机上滑起来非常顺滑，和原生 `RecyclerView` 的手感一致。

## 四、C++ 容易内存泄漏？SOUI 的解法：类 COM + 智能指针

公平地说，C++ 灵活操作内存的代价就是容易出内存泄漏、野指针、双重释放。这是很多人对 C++ 望而却步的原因。

SOUI 在框架层面给出了一个非常优雅的解决方案——**类 COM 的引用计数 + 智能指针**。

### 1. 所有对象自带引用计数

SOUI 的所有核心对象都继承自 `IObjRef` 接口（定义在 `utilities/include/interface/obj-ref-i.h`），它只声明了三个方法：

```cpp
DECLARE_INTERFACE(IObjRef)
{
    STDMETHOD_(long, AddRef)(THIS) PURE;          // 增加引用计数
    STDMETHOD_(long, Release)(THIS) PURE;         // 减少引用计数
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;  // 计数归零时的回调
};
```

具体的引用计数实现由模板类 `TObjRefImpl<T>` 提供（定义在 `utilities/include/helper/obj-ref-impl.hpp`），SOUI 对象通过继承它获得引用计数能力：

```cpp
template<class T>
class TObjRefImpl : public T {
public:
    TObjRefImpl() : m_cRef(1) {}
    STDMETHOD_(long, AddRef)(THIS) override {
        return InterlockedIncrement(&m_cRef);              // 原子加
    }
    STDMETHOD_(long, Release)(THIS) override {
        long lRet = InterlockedDecrement(&m_cRef);         // 原子减
        if (lRet == 0) {
            OnFinalRelease();                              // 计数为 0 触发回收
        }
        return lRet;
    }
    STDMETHOD_(void, OnFinalRelease)(THIS) override {
        delete this;                                       // 默认行为：自杀
    }
protected:
    LONG m_cRef;
};
```

`InterlockedIncrement`/`InterlockedDecrement` 保证多线程安全，`lRet == 0` 时通过 `OnFinalRelease` 自动 `delete this`——**对象的生命周期由引用计数管理，不需要手动 delete**。

### 2. SAutoRefPtr 智能指针

光有引用计数还不够，手动调 `AddRef`/`Release` 照样会忘。SOUI 提供了 `SAutoRefPtr<T>` 智能指针（类似 C++11 的 `std::shared_ptr`，但更轻量、专门为类 COM 对象设计）：

```cpp
template <class T>
class SAutoRefPtr {
public:
    SAutoRefPtr(T* lp, BOOL bAddRef = TRUE) {
        p = lp;
        if (p && bAddRef) p->AddRef();   // 构造时 AddRef
    }
    ~SAutoRefPtr() {
        if (p) p->Release();              // 析构时 Release
    }
    // ... 拷贝、赋值、operator-> 等
};
```

在前面的 `CMainDlg` 里就能看到它的用法：

```cpp
SAutoRefPtr<SGameTheme> m_pTheme;  // 自动管理主题对象生命周期
```

出作用域自动 `Release`，引用计数归零自动释放——**写 C++ 像 Java/Python 一样省心，但又保留了 C++ 的性能**。

### 3. RAII 防止人为错误

这种设计本质上是 RAII（Resource Acquisition Is Initialization）。相比裸指针，它杜绝了几类典型错误：

- 忘记 `delete` → 内存泄漏
- `delete` 两次 → 双重释放崩溃
- `delete` 后还有引用 → 野指针
- 多线程并发释放 → 竞态条件

在 `cnchess_native.cpp` 里也能看到这个模式的体现：窗口对象 `SHostWnd` 通过 `SAutoRefPtr<SHostWnd>` 存进 map，`ScreenShutdown` 时 `DestroyWindow` 后引用计数自动归零，不需要手动管理。

### 4. 不依赖 C++ RTTI 的类型安全：sobj_cast

引用计数解决了"什么时候释放"的问题，但 C++ 还有一类隐患来自**类型转换**——`dynamic_cast` 依赖编译器开启 RTTI（运行时类型信息），开销大、跨编译器/跨模块不可靠，而且在 SOUI 这种"接口 + 实现"分离的架构里，很多对象只暴露抽象接口 `IObject*`，`dynamic_cast` 根本派不上用场。

SOUI 的方案是**自带一套轻量级类 RTTI 机制**，完全不依赖 C++ 的 RTTI。每个 SOUI 对象在声明时通过 `DEF_SOBJECT` 宏登记自己的类名，并沿继承链构建一个类名字符串链表：

```cpp
// 摘自 SOUI/include/interface/sobject-i.h
#define DEF_SOBJECT(baseCls, clsName) DEF_SOBJECT_EX(baseCls, clsName, NULL)

// DEF_SOBJECT_EX 会生成：
//   static LPCWSTR GetClassName();
//   virtual BOOL IsClass(LPCWSTR lpszName) const;  // 沿基类链向上比较
//   virtual int GetClassNameList(...) const;        // 返回完整继承链
```

类型转换则交给 `sobj_cast<T>` 模板，它的实现非常简洁——**用类名字符串匹配代替 `dynamic_cast`**：

```cpp
// 摘自 SOUI/include/interface/sobject-i.h
template <class T>
T *sobj_cast(const IObject *pObj)
{
    if (!pObj)
        return NULL;
    if (pObj->IsClass(T::GetClassName()))  // 沿继承链查找类名
        return (T *)pObj;
    else
        return NULL;
}
```

`IsClass` 会递归向上比对类名（见 `DEF_SOBJECT_EX` 中的实现），所以 `sobj_cast` 支持向上/向下转型，且**转换失败返回 `NULL` 而不是抛异常**，调用方可以安全地判空。

这种设计带来的好处是实打实的：

- **跨模块安全**：SOUI 的对象工厂、事件系统、XML 属性派发都基于 `IObject*` 接口传递，跨 DLL/SO 也能正常 `sobj_cast`，不受编译器 RTTI 一致性影响；
- **零额外开销**：不开启 RTTI，二进制更小、虚表更轻；
- **配合引用计数保证数据安全**：跨层传递时，调用方拿到的是 `IObject*` 接口 + 引用计数保护，需要访问具体类型时再 `sobj_cast`，**类型不匹配就是空指针，绝不会访问到错误内存**。

象棋 Demo 里到处能看到这个模式的影子，比如事件处理中把通用 `IObject*` 事件参数安全转换成具体的 `EventSwndSize*`、`EventCmd*`：

```cpp
EventSwndSize *e2 = sobj_cast<EventSwndSize>(e);
EventCmd *e2 = sobj_cast<EventCmd>(evt);
```

引用计数管"生命周期"，`sobj_cast` 管"类型安全"——两者合起来，构成了 SOUI 在 C++ 下的数据安全基石。

## 五、小结

这次把中国象棋从 Windows 桌面端移植到 Android，让我对 SOUI 有了非常直观的认识：

| 维度 | SOUI 的表现 |
|------|------------|
| 与原生集成 | `SouiScreen` 就是 Android View，可混排、可生命周期管理 |
| 代码复用 | 桌面/移动端 `CMainDlg`、游戏逻辑、UI XML **逐行一致** |
| 移动端体验 | `SPanel` 内置 drag-to-scroll + fling，所有滚动控件自动支持手指滑动 |
| 性能 | C++ + Skia，无 JS Bridge 开销 |
| 内存安全 | 类 COM 引用计数（`IObjRef`+`TObjRefImpl`）+ `SAutoRefPtr` 智能指针，告别手动 delete |
| 类型安全 | `sobj_cast` 自带类 RTTI，不依赖 C++ RTTI，跨模块转换失败返回 NULL 而非访问错误内存 |
| 跨端愿景 | Windows/Linux/macOS 桌面三端已就绪，Android 已落地，iOS 后续补齐，全平台统一 |

如果你正在做桌面+移动端的产品，又不想被 Flutter/React Native 的生态绑架，也不想忍受 Qt 的体积和授权，**SOUI 目前是唯一能把"C++ 高性能"和"原生无缝集成"同时做到这个水平的方案**。

象棋 Demo 的视频已经在上面展示，源代码位于 SOUI 仓库的 `demos/cnchess-android`，桌面版在 `games/cnchess`，感兴趣的可以直接对比两份代码感受复用度。SOUI 框架本体在 `SOUI/` 目录，引用计数接口与实现分别在 `utilities/include/interface/obj-ref-i.h` 和 `utilities/include/helper/obj-ref-impl.hpp`，智能指针 `SAutoRefPtr` 也在同一文件；类 RTTI 与 `sobj_cast` 定义在 `SOUI/include/interface/sobject-i.h`，对象实现模板 `SObjectImpl` 在 `SOUI/include/sobject/Sobject.hpp`；`SPanel` 的滚动与惯性动画支持见 `SOUI/include/core/SPanel.h`。

## 代码获取
### gitee: https://gitee.com/setoutsoft/soui4
### github: https://github.com/soui4/soui

欢迎在评论区交流。
