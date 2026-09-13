# SOUI 无障碍（ACC / IAccessible）支持实现

> 本文介绍 SOUI 如何支持桌面无障碍（Accessibility，下称 ACC）：一是 SOUI 自己这一层的
> 对象模型与实现，核心是**用一套 accproxy 对象把 MSAA 语义隔离在控件系统之外**；
> 二是 SOUI 如何借助 swinx 兼容层，在 Windows / macOS / Linux 三个桌面平台上提供
> ACC 支持，包括在 gcc / clang 这类**不支持 `__uuidof` 关键词、甚至不提供 COM 运行时**
> 的编译器与系统上，如何实现一套可用的 COM。
>
> 姊妹篇：[`swinx/doc/swinx-无障碍桥实现流程-macOS与Linux.md`](../../swinx/doc/swinx-无障碍桥实现流程-macOS与Linux.md)
> 从"平台桥实现流程"角度讲 macOS/Linux 两个桥的内部流转；本文则从 SOUI 侧出发，
> 讲清"SOUI 支持 ACC 的整体结构与设计取舍"。两者互补。

---

## 目录

1. [总览与设计目标](#1-总览与设计目标)
2. [整体架构与端到端调用流](#2-整体架构与端到端调用流)
3. [accproxy：隔离 MSAA 的关键设计](#3-accproxy隔离-msaa-的关键设计)
4. [子元素模型：AccChild 与 AccSubItem](#4-子元素模型acchild-与-accsubitem)
5. [控件如何零 ACC 代码接入](#5-控件如何零-acc-代码接入)
6. [经 swinx 实现三平台 ACC](#6-经-swinx-实现三平台-acc)
7. [gcc / clang 上的 COM：无 `__uuidof`、无 COM 运行时](#7-gcc--clang-上的-com无-__uuidof无-com-运行时)
8. [构建开关、启用与验证](#8-构建开关启用与验证)
9. [已知限制与注意事项](#9-已知限制与注意事项)
10. [关键代码索引](#10-关键代码索引)

---

## 1. 总览与设计目标

SOUI 是一个**全自绘**的 DirectUI 框架：所有控件都是同一个宿主原生窗口（HWND）上的
`SWindow` 对象，没有各自的原生子窗口。这对无障碍是天然不利的——操作系统与屏幕阅读器
看到的只是"一个大空白窗口"。

SOUI 的 ACC 支持要解决三件事：

| 目标 | 做法 |
|---|---|
| **把自绘控件树暴露给系统 AT** | 实现 MSAA（`IAccessible`）对象树，让 SWindow 树在 AT 眼里变成可访问元素树 |
| **不让 ACC 侵入/污染控件系统** | 引入 `IAccProxy` 隔离层，控件系统只认识 SOUI 语义，全部 MSAA 语义收敛在唯一门面类里 |
| **三平台统一** | 依赖 swinx 提供的进程内 MSAA 兼容层，Windows 走原生 oleacc，macOS/Linux 走平台桥把 MSAA 翻译成 NSAccessibility / AT-SPI2 |

### 1.1 三条设计原则

**原则一：单一语义源（Single Source of Truth）。**
MSAA 的语义（child id 寻址、`CHILDID_SELF`、`VT_DISPATCH` vs `VT_I4`、`S_FALSE` 约定、
`IDispatch` 转发……）只在一个类里实现——`SAccessible`。平台桥、控件、业务代码都不
重复实现这些语义。

**原则二：控件系统不认识 MSAA。**
控件基类 `SWindow` 上只挂 3 个最小接口点（`GetAccessible` / `GetAccProxy` /
`accNotifyEvent`）。控件类本身**零 ACC 代码**——`SButton`、`STabCtrl`、`SListView`
的源码里找不到 `IAccessible`、`VARIANT`、`BSTR` 这些 MSAA 词汇。

**原则三：ACC 语义集中在工厂，而非散落在控件里。**
"哪个控件对应哪个角色的 proxy"由 `SApplication::CreateAccProxy()` 一处集中分派。
新增控件要支持 ACC，只需**新增一个 proxy 类 + 在工厂里加一行**，不必修改控件类。

### 1.2 设计动机：为什么不做"控件直接实现 IAccessible"

如果让控件类各自实现 `IAccessible`，会面临：

- `IAccessible` 继承 `IDispatch`，每个控件类都要实现 `Invoke`/`GetIDsOfNames`
  等 4 个纯 COM 方法——**这些是 MSAA 的历史包袱，与控件业务毫无关系**。
- `IAccessible` 的方法语义是三元组 `(VARIANT varChild, …)`，需要维护"child id →
  控件/子项"的双向映射。这份映射是 ACC 特有的，塞进控件里会与控件自身的数据结构
  （如列表的可见项窗口、Tab 的页签数组）纠缠。
- 不同平台对 `IAccessible` 的调用路径不同（Windows 原生 oleacc 跨进程 marshal、
  macOS/Linux 桥现查现用），控件层若直接暴露 IAccessible，就等于把平台差异也拉进了
  控件层。
- **ACC 是可选功能**（构建开关 `SOUI_ENABLE_ACC` 默认 OFF）。如果语义写在控件里，
  关掉 ACC 会留下一堆 `#ifdef` 与死代码；隔离到独立文件后，关掉 ACC 是
  **整批文件不参与编译**，控件系统分毫不变。

---

## 2. 整体架构与端到端调用流

### 2.1 四层架构

```
┌──────────────────────────────────────────────────────────────────────┐
│ 系统 AT（无障碍客户端）                                                │
│   Windows: Narrator / NVDA / Inspect.exe                             │
│   macOS:   VoiceOver                                                 │
│   Linux:   Orca / accerciser                                         │
└───────────────────────────┬──────────────────────────────────────────┘
                            │ 平台原生无障碍协议
                            │  Windows: MSAA/COM 跨进程
                            │  macOS:   NSAccessibility (ObjC)
                            │  Linux:   AT-SPI2 over D-Bus
┌───────────────────────────▼──────────────────────────────────────────┐
│ 平台桥（swinx）                                                       │
│   Windows: 原生 oleacc.dll（swinx 不介入）                            │
│   macOS:   swinx/src/platform/cocoa/SNsAccessibility.mm              │
│   Linux:   swinx/src/platform/linux/SAtSpi.cpp                       │
│                                                                      │
│  ── 三平台桥都收敛到同一组进程内 MSAA API ──                          │
│     swinx/src/oleacc.cpp:                                            │
│       LresultFromObject / ObjectFromLresult                          │
│       AccessibleObjectFromWindow / ...FromPoint / ...FromEvent       │
│       AccessibleChildren / NotifyWinEvent / SetWinEventHook          │
│       SwinxAccResolvePath（桥共享下钻内核）                            │
└───────────────────────────┬──────────────────────────────────────────┘
                            │ SendMessage(hwnd, WM_GETOBJECT, wParam, OBJID)
┌───────────────────────────▼──────────────────────────────────────────┐
│ SOUI MSAA 门面层                                                      │
│   SHostWnd::OnGetObject        ← WM_GETOBJECT 汇合点                 │
│        └─ SWindow::GetAccessible() → SAccessible                     │
│              class SAccessible : IAccHelper, SUnkImpl<IAccessible>   │
│              ── 唯一承载全部 MSAA 语义的类 ──                         │
└───────────────────────────┬──────────────────────────────────────────┘
                            │ 纯 SOUI 语义调用（IAccProxy）
┌───────────────────────────▼──────────────────────────────────────────┐
│ accproxy 隔离层                                                       │
│   IAccProxy（saccproxy-i.h，继承 IObjRef，**不继承 IAccessible**）    │
│        └─ SAccProxyWindow（默认）                                     │
│        └─ SAccProxyButton / SListView / STabCtrl / SHeaderCtrl ...   │
│           由 SApplication::CreateAccProxy() 集中分派                  │
└───────────────────────────┬──────────────────────────────────────────┘
                            │ SWindow / 具体控件（零 ACC 代码）
┌───────────────────────────▼──────────────────────────────────────────┐
│ SOUI 控件系统                                                         │
│   SWindow（基类，仅 3 个 ACC 接口点）                                  │
│   SButton / STabCtrl / SListView / SMCListView / STreeView ...       │
└──────────────────────────────────────────────────────────────────────┘
```

### 2.2 一次 WM_GETOBJECT 往返

MSAA 的对象获取是**拉取式（pull）**的：客户端主动向窗口发 `WM_GETOBJECT`，
服务端返回一个可跨进程传递的句柄，客户端再用 `ObjectFromLresult` 解析回接口指针。

```
[AT 客户端 / 平台桥]
    AccessibleObjectFromWindow(hwnd, OBJID_CLIENT, IID_IAccessible, &pAcc)
        │
        │  swinx/src/oleacc.cpp:306
        │  LRESULT lResult = SendMessageA(hwnd, WM_GETOBJECT,
        │                                  (WPARAM)-1, (LPARAM)(LONG)OBJID_CLIENT);
        ▼
[SOUI 宿主窗口]
    SHostWnd::OnGetObject(uMsg, wParam, lParam)          shostwnd.cpp:1775
        DWORD dwObjectId = lParam;                       // 允许 custom object ID
        if (dwObjectId == OBJID_CLIENT)
            pAcc = GetRoot()->GetAccessible();           // 控件树根
        else if ((pWnd = SWindowMgr::GetWindow(dwObjectId)) != NULL)
            pAcc = pWnd->GetAccessible();                // 注释：user swnd to identify a swindow.
        if (pAcc)
            return LresultFromObject(IID_IAccessible, wParam, pAcc);
        SetMsgHandled(FALSE);  return 0;                 // 未开 ACC 时走这条
        │
        │  LresultFromObject（swinx/src/oleacc.cpp:267）：
        │    1. pAcc == NULL           → 返回 (LRESULT)E_INVALIDARG
        │    2. pAcc->QueryInterface(riid) 失败 → 返回该 HRESULT
        │    3. 弱登记进句柄表 AccTable().Acquire(pAcc, wParam) → 返回句柄
        ▼
[AT 客户端]
    ObjectFromLresult(lResult, IID_IAccessible, (WPARAM)-1, &pAcc)
        AccTable().Lookup((LONG)lResult, wParam)   // 按 (句柄, wParam) 匹配
            不匹配 → E_FAIL
            匹配   → pUnk->QueryInterface(riid, ppv)
```

几个**必须复刻**的 Win32 行为（都是在真实 oleacc 上实测确认的，注释写在
`swinx/src/oleacc.cpp` 里）：

- **`wParam` 原样回传。** 客户端发 `(WPARAM)-1`，服务端把它原样交给
  `LresultFromObject`，两端按 `(句柄, wParam)` 匹配。这是防串扰的握手。
- **OBJID 的符号扩展。** 真实 oleacc 把 OBJID 当 32 位有符号数放进 `LPARAM`，
  `OBJID_CLIENT`（= -4）在 64 位上是 `0xFFFFFFFFFFFFFFFC`。所以 swinx 里写
  `(LPARAM)(LONG)dwObjectID`——不这么转，服务端 `(LONG)lParam == OBJID_CLIENT`
  在 LP64 上不成立，整个 MSAA 就断了。
- **失败时返回 HRESULT 而非 0。** 文档说"失败返回 0"，实测不是：`null → E_INVALIDARG`，
  QI 失败 → 返回该失败 HRESULT。swinx 按实测实现。
- **弱登记（不 AddRef）。** swinx 的句柄表只是"同一次同步往返的解析凭据"，
  **不持有 IAccessible 引用**。对象生命周期完全由 SOUI 掌握；控件销毁后陈旧句柄
  自然失效（`Lookup` 找不到或 QI 失败），不会访问悬垂指针。

### 2.3 属性查询流：从 IAccessible 到控件

拿到 `IAccessible*` 之后，客户端的每次属性查询都走同一条翻译链。以
`get_accName(CHILDID_SELF)` 为例：

```
客户端:  pAcc->get_accName(varChild = {VT_I4, CHILDID_SELF}, &bstrName)
   │
   │  SwndAccessible.cpp
   ▼
SAccessible::get_accName(VARIANT varChild, BSTR *pszName)
   ├─ int vi = VirtualChildIndex(varChild);
   │     vi > 0  → 目标是"自绘子项"  → 走 GetAccSubItemName(vi)
   │     vi == 0 → 目标是真实对象     → 走 GetAccTarget(varChild)->GetAccProxy()
   ├─ SWindow *pTarget = GetAccTarget(varChild);
   │     CHILDID_SELF → m_pWnd
   │     否则          → (SWindow*)m_pWnd->GetAccProxy()->GetAccChild(lVal)
   └─ pTarget->GetAccProxy()->get_accName(pszName)
          │
          │  惰性创建 IAccProxy（Swnd.cpp:4161）
          ▼
      SApplication::CreateAccProxy(pTarget)     // SApp.cpp:380 集中分派
          → new SAccProxyButton(pTarget)        // 按类名选 proxy
                │
                ▼
          从 SWindow 取窗口文本 / 类名 / 角色…… 组装成 BSTR 返回
```

这条链上，**MSAA → SOUI 的翻译只发生在 `SAccessible` 一处**。`IAccProxy` 这一侧
看到的已经是"窗口 + 语义化问题"（"你的名字是什么？""你的角色是什么？"），与
"这是第 3 个子对象请通过 IDispatch 转发"这类 MSAA 细节完全无关。

---

## 3. accproxy：隔离 MSAA 的关键设计

这是 SOUI ACC 支持最核心的设计。理解三个问题即可：**接口怎么切**、**门面怎么写**、
**工厂怎么分派**。

### 3.1 `IAccProxy`：一个"不认识 MSAA"的接口

`IAccProxy` 定义在 `SOUI/include/interface/saccproxy-i.h`。它的继承关系就已经说明了
设计意图：

```cpp
// SOUI/include/interface/saccproxy-i.h
#undef  INTERFACE
#define INTERFACE IAccProxy
DECLARE_INTERFACE_(IAccProxy, IObjRef)   // ← 注意：不是 IAccessible，也不是 IDispatch
{
    // 引用计数三件套（IObjRef 语义，显式复述一遍以保持接口定义自包含）
    STDMETHOD_(long, AddRef)(THIS) PURE;
    STDMETHOD_(long, Release)(THIS) PURE;
    STDMETHOD_(void, OnFinalRelease)(THIS) PURE;

    // MSAA 属性集（对象是 IWindow，不是 MSAA 对象）
    // 注意：**没有 VARIANT varChild 参数** —— child id 寻址已被剥离
    STDMETHOD(get_accName)(THIS_ BSTR *pszName) PURE;
    STDMETHOD(get_accValue)(THIS_ BSTR *pszValue) PURE;
    STDMETHOD(get_accDescription)(THIS_ BSTR *pszDescription) PURE;
    STDMETHOD(get_accRole)(THIS_ VARIANT *pvarRole) PURE;
    STDMETHOD(get_accState)(THIS_ VARIANT *pvarState) PURE;
    STDMETHOD(get_accHelp)(THIS_ BSTR *pszHelp) PURE;
    STDMETHOD(get_accHelpTopic)(THIS_ BSTR *pszHelpFile, long *pidTopic) PURE;
    STDMETHOD(get_accKeyboardShortcut)(THIS_ BSTR *pszKeyboardShortcut) PURE;
    STDMETHOD(get_accDefaultAction)(THIS_ BSTR *pszDefaultAction) PURE;
    STDMETHOD(accSelect)(THIS_ long flagsSelect) PURE;
    STDMETHOD(accDoDefaultAction)(THIS) PURE;
    STDMETHOD(put_accName)(THIS_ BSTR szName) PURE;
    STDMETHOD(put_accValue)(THIS_ BSTR szValue) PURE;

    // ── AccChild：真实子窗口（自身有独立 IAccessible），1-based ──
    STDMETHOD_(int, GetAccChildCount)(THIS) PURE;
    STDMETHOD_(IWindow *, GetAccChild)(THIS_ int iChild) PURE;
    STDMETHOD_(int, GetAccSelIndex)(THIS) PURE;

    // ── AccSubItem：自绘子项（不是 SWindow，无独立 IAccessible），
    //    在 MSAA 里以 CHILDID_SELF + N 寻址，1-based ──
    STDMETHOD_(int,  GetAccSubItemCount)(THIS) PURE;
    STDMETHOD_(BSTR, GetAccSubItemName)(THIS_ int iChild) PURE;
    STDMETHOD_(long, GetAccSubItemRole)(THIS_ int iChild) PURE;
    STDMETHOD_(DWORD,GetAccSubItemState)(THIS_ int iChild) PURE;
    STDMETHOD_(void, GetAccSubItemRect)(THIS_ int iChild, RECT *prc) PURE;
    STDMETHOD_(int,  GetAccSubItemSel)(THIS) PURE;
    STDMETHOD(SetAccSubItemSel)(THIS_ int iChild) PURE;
    STDMETHOD_(int,  HitTestAccSubItem)(THIS_ long x, long y) PURE;
};
```

**关键观察：**

1. **它不继承 `IAccessible`，也不继承 `IDispatch`。** 它的基类是本框架自己的
   `IObjRef`（`utilities/include/interface/obj-ref-i.h`：`AddRef` / `Release` /
   `OnFinalRelease`）。也就是说，实现一个 accproxy **完全不需要写 COM 的
   `QueryInterface`/`Invoke`/`GetIDsOfNames` 样板**——`TObjRefImpl<IAccProxy>`
   模板已经代劳。

2. **方法名沿用 `get_accXxx`，但方法签名与 MSAA 无关。**
   没有 `VARIANT varChild` 参数、没有 `CHILDID_SELF`、没有 `IDispatch**`。
   `GetAccChild(iChild)` 直接返回 **`IWindow*`**——一个 SOUI 自己的接口指针。
   换言之：**ACC 的数据源是 SOUI 对象模型（IWindow），而不是 MSAA 对象模型。**

3. **Child / SubItem 的命名区分。**
   `GetAccChild*` 与 `GetAccSubItem*` 分属两类不同的子元素，命名上严格区分、
   互不复用：
   - `GetAccChildCount` / `GetAccChild` / `GetAccSelIndex` → **真实子窗口**
     （可返回独立 `IAccessible`）
   - `GetAccSubItem*` / `SetAccSubItemSel` / `HitTestAccSubItem` → **自绘子项**
     （共享父对象的 IAccessible，以 child id 寻址）

4. **精简的接口面。** 只暴露实际需要的方法，不留冗余入口。

### 3.2 `SAccessible`：唯一承载 MSAA 语义的门面

`SOUI/include/core/SWndAccessible.h`：

```cpp
class SOUI_EXP SAccessible
    : public IAccHelper
    , public SUnkImpl<IAccessible>       // ← MSAA 的全部接口
{
  protected:
    SWindow *m_pWnd;   // 唯一的成员：一个 SWindow 指针

    // ── 私有翻译助手 ──
    BOOL      accValidateNavStart(VARIANT *pvar) const;
    SWindow  *GetAccTarget(VARIANT varChild) const;
    int       RealChildCount() const;      // = GetAccProxy()->GetAccChildCount()
    int       VirtualChildCount() const;   // = GetAccProxy()->GetAccSubItemCount()
    long      TotalChildCount() const;     // = Real + Virtual
    int       VirtualChildIndex(VARIANT) const;

  public:
    // IAccHelper：反向拿回 owner 窗口
    STDMETHOD_(void, SetOwner)(THIS_ IWindow *pOwner) OVERRIDE { m_pWnd = (SWindow*)pOwner; }
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST OVERRIDE { return m_pWnd; }

    // IAccessible 全套方法（声明略）＋ IDispatch 四方法
    ...

  public:
    IUNKNOWN_BEGIN(IAccessible)          // QueryInterface 支持表
    IUNKNOWN_ADD_IID(IAccHelper)
    IUNKNOWN_ADD_IID(IDispatch)
    IUNKNOWN_END()
};
```

**这个类的三个设计要点：**

**(a) 它是唯一能让"控件系统"与"MSAA"发生接触的地方。**

`m_pWnd` 是它唯一的成员——它不复制、不缓存任何控件状态。所有数据都在查询时
经 `m_pWnd->GetAccProxy()` 现取。因此它天生无状态一致性问题：控件数据变了，
下次查询自然拿到新值。

**(b) 对象身份（identity）与控件对象分离。**

每个 SWindow 惰性创建**一个** `SAccessible`（`m_pAcc`），并被平台桥以
`WM_GETOBJECT` 的 `(hwnd, swnd)` 锚定。这个"锚"不随控件重建而漂移——因为
SWND 是 SOUI 自己的稳定句柄（`SWindowMgr` 管理），而不是随对象地址变化的
`this` 指针。这也是平台桥（macOS 的 `SwinxAccElement`、Linux 的 `AccNode`）
能靠 `(hwnd, child id 链)` 做"身份壳"的前提。

**(c) MSAA 的复杂约定全部在这里落地。** 具体有：

| MSAA 约定 | `SAccessible` 的处理 |
|---|---|
| `CHILDID_SELF`(0) 表示"对象自身"，1..N 表示子元素 | `GetAccTarget()`：0 → `m_pWnd`；>0 → `GetAccProxy()->GetAccChild(lVal)` |
| 简单元素共享父对象的 `IAccessible`，`get_accChild` 须返回 `S_FALSE` | `get_accChild()`：`if (VirtualChildIndex(varChild) > 0) return S_FALSE;` |
| `VT_DISPATCH`（有独立对象）vs `VT_I4`（简单元素）二态返回 | `get_accChild` 真实子窗口返回 `IDispatch*`；虚拟子项由上述 `S_FALSE` 表达 |
| 子元素编号：先真实子、后虚拟子 | `VirtualChildIndex(v) = (v > real && v <= real+virtual) ? v - real : 0` |
| `get_accName` 对虚拟子项需区分实现 | `vi>0` → `GetAccSubItemName(vi)`；否则 → `GetAccTarget()->GetAccProxy()->get_accName()` |
| `get_accRole` / `get_accState` 同理 | `GetAccSubItemRole(vi)` / `GetAccSubItemState(vi)` |
| 虚拟子项不支持 help/description | 直接返回 `E_INVALIDARG` |
| `accDoDefaultAction` / `accSelect` 对虚拟子项 = 选中该子项 | `SetAccSubItemSel(vi)` |
| `accLocation` 坐标系转换 | 虚拟子项：`GetAccSubItemRect` + `ClientToScreen`；真实子窗口：`GetWindowRect` + `FrameToHost` + `ClientToScreen` |
| `accHitTest` 的 z 序语义 | **从 `nCount` 倒序遍历**（`GSW_LASTCHILD` 最顶层优先），避免垫底的全窗口背景节点抢答；未命中真实子窗口再试 `HitTestAccSubItem`；仍无则 `CHILDID_SELF` |
| `get_accSelection` 的两种来源 | `GetAccSelIndex()`，<=0 时回退 `GetAccSubItemSel()`（返回 `RealChildCount() + vs`） |
| `get_accFocus` 的三种结果 | SWND 焦点是自己 → `CHILDID_SELF`；是直接子窗口 → 数 `GSW_PREVSIBLING` 得 1-based index；否则返回子对象的 `IDispatch` |
| `accNavigate` 的空参/`DISP_E_PARAMNOTFOUND` → `CHILDID_SELF` | `accValidateNavStart()`；校验范围覆写为 `[0, TotalChildCount()]` |
| `accNavigate` 对 dispatch / 简单元素的不同寻址 | `FIRSTCHILD/LASTCHILD` → 1/total；`NEXT/DOWN/RIGHT` 与 `PREVIOUS/UP/LEFT` 对 `VT_DISPATCH` 走 `GSW_NEXTSIBLING/GSW_PREVSIBLING`，对 `VT_I4` 做环绕加减 |
| `get_accParent` 到顶时的兜底 | 无父窗口 → `AccessibleObjectFromWindow(hostHwnd, OBJID_WINDOW, IID_IAccessible, ...)`；有父 → 父的 `GetAccessible()->QueryInterface(IID_IDispatch, ...)` |
| `IDispatch` 方法 | 全部 `E_NOTIMPL`（MSAA 客户端只用 `IDispatch` 做身份传递，不用名称调用） |

**这整张表就是"污染"的量。** 它的所有行都写在 `SwndAccessible.cpp`（607 行）里，
而控件系统一行都没沾。

### 3.3 `SApplication::CreateAccProxy`：集中工厂

```cpp
// SOUI/src/SApp.cpp:380
#ifdef SOUI_ENABLE_ACC
IAccProxy *SApplication::CreateAccProxy(IWindow *pWnd) const
{
    if (pWnd->IsClass(SProgress::GetClassName()))
        return new SAccProxyProgress(pWnd);
    else if (pWnd->IsClass(SSliderBar::GetClassName()))
        return new SAccProxySlideBar(pWnd);
    else if (pWnd->IsClass(SButton::GetClassName()) ||
             pWnd->IsClass(SImageButton::GetClassName()))
        return new SAccProxyButton(pWnd);
    else if (pWnd->IsClass(SComboBox::GetClassName()))
        return new SAccProxyCombobox(pWnd);
    else if (pWnd->IsClass(SRichEdit::GetClassName()))
        return new SAccProxyEdit(pWnd);
    // ... 约 160 行 if-else，覆盖 40+ 个控件类 ...
    else
        return new SAccProxyWindow(pWnd);     // 兜底：通用窗口语义
}

IAccessible *SApplication::CreateAccessible(IWindow *pWnd) const
{
    return new SAccessible(pWnd);
}
#endif // SOUI_ENABLE_ACC
```

**为什么用集中工厂而不是虚函数？**

- 若用虚函数（"每个控件 override `CreateAccProxy`"），SWindow 的**每个子类的
  vtable 都会因为 ACC 而变化**（新增虚函数），并且要在每个控件类里加代码——
  这正是"污染"。
- 集中工厂把分派逻辑放在**一个与控件无关的类**（`SApplication`）里。控件类不需要
  知道自己是"按钮"还是"进度条"——proxy 端通过 `IsClass()` 自己判断。
- 副作用可控：ACC 关闭时整段工厂代码不编译，`SApplication` 的其它部分不受影响。
- `CreateAccProxy` / `CreateAccessible` 都是 `virtual`，业务可以继承
  `SApplication` 覆盖它们，注入自定义 proxy（例如第三方控件库）。

### 3.4 SWindow 上的三个接口点

`SOUI/include/core/SWnd.h`：

```cpp
#ifdef SOUI_ENABLE_ACC
    IAccessible *GetAccessible();     // 惰性创建 SAccessible
#endif
    IAccProxy   *GetAccProxy();       // 惰性创建 IAccProxy（ACC 关闭返回 NULL）
    void         accNotifyEvent(DWORD dwEvt);   // 事件上报

#ifdef SOUI_ENABLE_ACC
    SAutoRefPtr<IAccessible> m_pAcc;       // 惰性缓存
    SAutoRefPtr<IAccProxy>   m_pAccProxy;  // 惰性缓存
#endif
```

实现（`SOUI/src/core/Swnd.cpp`）：

```cpp
#ifdef SOUI_ENABLE_ACC
IAccessible *SWindow::GetAccessible()
{
    if (!m_pAcc)
        m_pAcc.Attach(SApplication::getSingleton().CreateAccessible(this));
    return m_pAcc;
}
#endif

IAccProxy *SWindow::GetAccProxy()
{
#ifdef SOUI_ENABLE_ACC
    if (!m_pAccProxy)
        m_pAccProxy.Attach(SApplication::getSingleton().CreateAccProxy(this));
    return m_pAccProxy;
#else
    return NULL;
#endif
}

void SWindow::accNotifyEvent(DWORD dwEvt)
{
#ifdef SOUI_ENABLE_ACC
    if (GetContainer())
        NotifyWinEvent(dwEvt, GetContainer()->GetHostHwnd(), GetSwnd(), CHILDID_SELF);
#endif
}
```

注意：这两个指针**只挂在基类 `SWindow` 上**，且是惰性的。一个有 5000 个窗口对象的
界面，如果 AT 从未查询过，就一个 `SAccessible` / `IAccProxy` 都不会创建——零开销。

### 3.5 `IAccHelper`：从 IAccessible 反查 owner

`SOUI/include/interface/sacchelper-i.h`：

```cpp
DEFINE_SGUID(IID_IAccHelper, 0x9FAF3067, 0x6723, 0x40DA, 0xA5, 0x6B, 0xCD, 0xCB, 0x11, 0xB8, 0x90, 0x2B);

#undef  INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_(IAccHelper, IUnknown)
{
    DECLARE_CLASS_SIID(IID_IAccHelper)   // ← 提供 static GetIID()，见第 7 章
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD_(void, SetOwner)(THIS_ IWindow *pOwner) PURE;
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST PURE;
};
```

用途：SOUI 内部有时只拿到一个 `IAccessible*`，需要反查它属于哪个窗口。用法在
`Swnd.cpp` 中出现两处：

```cpp
// 窗口销毁时（Swnd.cpp:2418）——断开 owner，防止悬垂
accNotifyEvent(EVENT_OBJECT_DESTROY);
if (m_pAcc) {
    SComPtr<IAccHelper> accHelper;
    if (m_pAcc->QueryInterface(__suidof(IAccHelper), (void**)&accHelper) == S_OK)
        accHelper->SetOwner(NULL);
}

// 析构时（Swnd.cpp:658）——断言 owner 已断开
if (m_pAcc) {
    SComPtr<IAccHelper> accHelper;
    if (m_pAcc->QueryInterface(__suidof(IAccHelper), (void**)&accHelper) == S_OK)
        SASSERT(accHelper->GetOwner() == NULL);
}
```

另有 `Swnd.cpp:793` 的 `SetWindowText` 里发 `EVENT_OBJECT_NAMECHANGE` 等事件通知。

### 3.6 该设计带来的隔离效果：一张对照表

| 关注点 | 放在哪 | 控件系统是否可见 |
|---|---|---|
| MSAA `IAccessible`/`IDispatch` 实现 | `SAccessible`（`SWndAccessible.h/.cpp`） | ✗ 不可见 |
| child id ↔ 子元素映射 | `SAccessible` 的 4 个翻译助手 | ✗ 不可见 |
| 各控件的角色/名字/状态语义 | `SAccProxy*`（`msaa/` 目录） | ✗ 不可见 |
| 控件→proxy 分派 | `SApplication::CreateAccProxy` | ✗ 不可见 |
| ACC 事件上报 | `SWindow::accNotifyEvent`（1 个方法） | ✓ **唯一可见点** |
| 惰性对象缓存 | `SWindow::m_pAcc` / `m_pAccProxy` | ✓ 2 个成员 |
| 访问入口 | `GetAccessible` / `GetAccProxy` | ✓ 2 个方法 |

**控件基类上 ACC 相关的接口面合计：2 个方法 + 1 个事件方法 + 2 个成员。**
这就是全部"染污"。

---

## 4. 子元素模型：AccChild 与 AccSubItem

MSAA 只有一套 `(VARIANT varChild)` 寻址机制，但 SOUI 的"子元素"实际上有两类，
语义差别很大：

### 4.1 两类子元素

| | **AccChild**（真实子窗口） | **AccSubItem**（自绘子项） |
|---|---|---|
| 是什么 | 一个真正的 `SWindow` 子窗口 | 容器内部画出来的条目（Tab 页签、表头列、列表项…） |
| 有独立 `IAccessible` 吗 | **有**（每个 SWindow 都有自己的 `SAccessible`） | **没有**（不是 SWindow，没有自己的对象） |
| `get_accChild` 返回 | `IDispatch*`（子对象） | **`S_FALSE`**（表示"简单元素，请用父对象 + childId"） |
| 客户端如何查询 | 先取子对象，再 `child->get_accName(SELF)` | 直接 `parent->get_accName({VT_I4, childId})` |
| 编号来源 | `GetAccProxy()->GetAccChildCount()` / `GetAccChild(i)` | `GetAccProxy()->GetAccSubItemCount()` / `GetAccSubItemName(i)` |
| 编号区间 | `[1, RealCount]` | `[RealCount+1, RealCount+VirtualCount]` |
| 典型例子 | 面板里的子控件、列表中的可见 `SItemPanel` | `STabCtrl` 的页签、`SHeaderCtrl` 的列头、`SListBox` 的项、`STreeCtrl` 的节点 |

**为什么要分两类？**

因为虚拟列表（MVC）控件的数据是**千上万条**，但屏幕上只渲染**十几个**
`SItemPanel`。如果每个数据项都要有独立的 `IAccessible` 对象，AT 一查询就要
创建上千个 proxy——内存与时延都不可接受。用"简单元素"（`VT_I4` child id）
表达，AT 只需要"父对象 + 序号"就能问出名字/角色/状态，成本是常数。

反过来，`STabCtrl` 的页签虽然也是自绘的，但数量固定且少——用 AccSubItem 表达
最自然。

### 4.2 基类的默认策略：AccChild = 子窗口，AccSubItem = 空

`SAccProxyWindow`（`SOUI/include/msaa/SAccProxyWindow.h`）是**所有未特化窗口的默认
proxy**，它体现了"最保守但正确"的默认语义：

```cpp
// 真实子窗口：默认就是 SWindow 的子窗口，一一对应
int    STDMETHODCALLTYPE SAccProxyWindow::GetAccChildCount() { return m_pWnd ? m_pWnd->GetChildrenCount() : 0; }
IWindow*STDMETHODCALLTYPE SAccProxyWindow::GetAccChild(int iChild) { return ((SWindow*)m_pWnd)->GetChild(iChild); }
int    STDMETHODCALLTYPE SAccProxyWindow::GetAccSelIndex() { return 0; }

// 自绘子项：默认没有（Composite controls override these to expose their drawn sub-items.）
int     STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemCount() { return 0; }
BSTR    STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemName(int) { return NULL; }
long    STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemRole(int) { return 0; }
DWORD   STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemState(int) { return 0; }
void    STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemRect(int, RECT *prc)
            { prc->left = prc->top = prc->right = prc->bottom = 0; }
int     STDMETHODCALLTYPE SAccProxyWindow::GetAccSubItemSel() { return -1; }
STDMETHODIMP              SAccProxyWindow::SetAccSubItemSel(int) { return E_NOTIMPL; }
int     STDMETHODCALLTYPE SAccProxyWindow::HitTestAccSubItem(long, long) { return -1; }
```

窗口级的默认属性语义（`SOUI/src/msaa/SAccProxyWindow.cpp`）：

```cpp
STDMETHODIMP SAccProxyWindow::get_accName(BSTR *pszName)
{
    SStringA strTextA;
    m_pWnd->GetWindowTextU8(&strTextA, TRUE);           // 优先用窗口文本
    SStringW strText;
    if (strTextA.IsEmpty())
    {
        // No window text: fall back to the object type (class name) so that
        // elements such as "window"/"img" show their type instead of "(nothing)".
        LPCWSTR pszClass = m_pWnd->GetObjectClass();
        strText = pszClass ? pszClass : L"";
        if (strText.IsEmpty()) return E_INVALIDARG;
    }
    else
        strText = S_CA2W(strTextA, CP_UTF8);
    *pszName = ::SysAllocString(strText);
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_WINDOW;                // 默认角色
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accState(VARIANT *pvarState)
{
    DWORD dwState = m_pWnd->GetState();
    pvarState->vt = VT_I4;
    pvarState->lVal = 0;
    if (!m_pWnd->IsVisible(TRUE))
        pvarState->lVal = STATE_SYSTEM_INVISIBLE;
    else if (m_pWnd->IsDisabled(TRUE))
        pvarState->lVal = STATE_SYSTEM_UNAVAILABLE;
    else
    {
        if (dwState & WndState_PushDown) pvarState->lVal  = STATE_SYSTEM_PRESSED;
        if (dwState & WndState_Check)    pvarState->lVal |= STATE_SYSTEM_CHECKED;
        if (m_pWnd->GetContainer()->GetFocus() == m_pWnd->GetSwnd())
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        if (m_pWnd->IsFocusable())
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;
    }
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::get_accKeyboardShortcut(BSTR *pszKeyboardShortcut)
{
    // 只有按钮有加速键；其它控件 E_NOTIMPL
    ...SAccelerator(dwAccel).FormatHotkey()...
}

STDMETHODIMP SAccProxyWindow::get_accDefaultAction(BSTR *pszDefaultAction)
{
    *pszDefaultAction = ::SysAllocString(L"cmd");
    return S_OK;
}

STDMETHODIMP SAccProxyWindow::accDoDefaultAction()
{
    m_pWnd->FireCommand();
    return S_OK;
}
```

### 4.3 覆写契约：可访问子级职责在 accproxy 侧

"可访问子级暴露"的职责完全在 accproxy，`SWindow` 不参与：

- **`SWindow` 没有任何 ACC 专属虚函数。** 控件基类的 vtable 不受 ACC 影响。
- **默认实现**在 `SAccProxyWindow`（基于 `GetChildrenCount` / `GetChild` / 0），
  即"真实子 = 普通子窗口，自绘子项 = 无"。
- **MVC 视图**（`SListView` / `SMCListView` / `STileView` / `STreeView`）由对应的
  视图代理（`SAccProxyListView` / `SAccProxyMCListView` / `SAccProxyTileView` /
  `SAccProxyTreeView`）覆写，数据统一取自 `SViewBase` 的 `GetVisibleAccChild*`。

`SViewBase` 侧的三个方法（`SOUI/include/control/SViewBase.h:192`）：

```cpp
    /** @brief Accessible 子级（当前可见项）数量。 */
    int GetVisibleAccChildCount() const { return (int)m_lstItems.GetCount(); }

    /** @brief 按 1 基序号取当前可见的 SItemPanel 作为 Accessible 子级。 */
    SWindow *GetVisibleAccChild(int iChild) const;     // 越界返回 NULL

    /** @brief 计算当前选中项对应的可见 SItemPanel 序号（1 基）。 */
    int GetVisibleAccSelIndex() const;
```

注意这三个方法**不含任何 ACC 类型**（没有 `VARIANT`、没有 `IAccessible`），只是
"可见项窗口 / 选中项序号"的普通查询——它们同时也是滚动定位、绘制优化可能需要的数据。
这是"语义下沉但不污染"的范例。

---

## 5. 控件如何零 ACC 代码接入

### 5.1 最简情形：只换角色

`SAccProxyCmnCtrl.h/.cpp` 里有 40+ 个 proxy，绝大多数**只覆写 `get_accRole` 一个方法**。
注意两点：(1) 它们继承 `SAccProxyWindow`（而非直接 `TObjRefImpl<IAccProxy>`）以复用默认语义；
(2) 方法签名里**没有 `VARIANT varChild` 参数**——proxy 已经知道自己属于哪个窗口，不需要
child id 寻址，这正是"MSAA 语义已被剥离"的直接证据。

```cpp
// SOUI/include/msaa/SAccProxyCmnCtrl.h:15
class SOUI_EXP SAccProxyButton : public SAccProxyWindow
{
  public:
    SAccProxyButton(IWindow *pWnd) : SAccProxyWindow(pWnd) {}
    virtual STDMETHODIMP get_accRole(VARIANT *pvarRole);
};

// SOUI/src/msaa/SAccProxyCmnCtrl.cpp:16
STDMETHODIMP SAccProxyButton::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
    return S_OK;
}
```

角色映射表（部分）：

| 控件 | ROLE_SYSTEM_* | 附加覆写 |
|---|---|---|
| SButton / SImageButton | `PUSHBUTTON` | 加速键（基类已处理） |
| SCheckButton | `CHECKBUTTON` | — |
| SRadioButton | `RADIOBUTTON` | — |
| SComboBox / SDateTimePicker / SDropDown | `COMBOBOX` | `get_accValue` = 当前项文本 |
| SEdit / SHotKeyCtrl | `TEXT` | — |
| SProgress | `PROGRESSBAR` | `get_accValue` = 百分比；`get_accName` 复用 value |
| SSliderBar / SSpinButtonCtrl | `SLIDER` / `SPINBUTTON` | `get_accValue` = 数值 |
| SStatic | `STATICTEXT` | — |
| SLink | `LINK` | — |
| SImageWnd / SIconWnd | `GRAPHIC` | — |
| SHrLine | `SEPARATOR` | — |
| SGroup | `GROUPING` | — |
| SCaption | `TITLEBAR` | — |
| SListCtrl / SListView / SListBox / SMCListView / STileView / SSearchDropdownList | `LIST` | 见 5.2 / 5.3 |
| SDockBar / SToolBar | `TOOLBAR` | — |
| SMenuBar | `MENUBAR` | — |
| SScrollBar | `SCROLLBAR` | — |
| SSplitWnd / SStackView | `PANE` | — |
| SSwitch | `CHECKBUTTON` | — |
| STabCtrl | `PAGETABLIST` | 见 5.2 |
| STreeCtrl / STreeView | `OUTLINE` | 见 5.2 / 5.3 |
| SActiveX / SCalendar / SFrame / SHeaderCtrl / SRealWnd | `CLIENT` | — |

> ⚠️ `SCalendar` 用 `CLIENT` 而非 `ROLE_SYSTEM_CALENDAR`：swinx 的 `oleacc.h`
> 未定义 `ROLE_SYSTEM_CALENDAR`，且其取值与本地 `RADIOBUTTON`(0x2D) 冲突。
> 详见第 9 章。

### 5.2 覆写 AccSubItem（自绘子项）

以 `STabCtrl` 为例（`SOUI/src/msaa/SAccProxyCmnCtrl.cpp:474` 起，此处略去空指针防御）：

```cpp
STDMETHODIMP SAccProxyTabCtrl::get_accRole(VARIANT *pvarRole)
{
    pvarRole->vt = VT_I4;
    pvarRole->lVal = ROLE_SYSTEM_PAGETABLIST;
    return S_OK;
}

int STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemCount()
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    return pTab ? pTab->GetItemCount() : 0;
}
BSTR STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemName(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab) return NULL;
    STabPage *pPage = pTab->GetItem(iChild - 1);      // ← 1-based → 0-based
    if (!pPage) return NULL;
    return ::SysAllocString(pPage->GetTitle());       // 调用方负责 SysFreeString
}
long STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemRole(int iChild)
{
    return ROLE_SYSTEM_PAGETAB;
}
DWORD STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemState(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    DWORD dwState = STATE_SYSTEM_FOCUSABLE | STATE_SYSTEM_SELECTABLE;
    if (pTab && iChild - 1 == pTab->GetCurSel())
        dwState |= STATE_SYSTEM_SELECTED;
    return dwState;
}
void STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemRect(int iChild, RECT *prc)
{
    prc->left = prc->top = prc->right = prc->bottom = 0;
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab) return;
    ...  // 取第 iChild-1 页的 CRect，写入 prc
}
int STDMETHODCALLTYPE SAccProxyTabCtrl::GetAccSubItemSel()
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab) return -1;
    int s = pTab->GetCurSel();                        // -1 表示无选中
    return s >= 0 ? s + 1 : -1;                       // ← 0-based → 1-based
}
STDMETHODIMP SAccProxyTabCtrl::SetAccSubItemSel(int iChild)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab) return E_INVALIDARG;
    return pTab->SetCurSel(iChild - 1) ? S_OK : E_INVALIDARG;
}
int STDMETHODCALLTYPE SAccProxyTabCtrl::HitTestAccSubItem(long x, long y)
{
    STabCtrl *pTab = sobj_cast<STabCtrl>(m_pWnd);
    if (!pTab) return -1;
    CPoint pt((int)x, (int)y);
    int count = pTab->GetItemCount();
    ...  // 逐项 GetItemRect + PtInRect，命中返回 i+1，否则 -1
}
```

**注意 `sobj_cast<T>`**：SWND 上的 `m_pWnd` 是 `IWindow*`（接口指针），
`sobj_cast` 做安全向下转型并按需 AddRef。这是 accproxy 访问具体控件 API 的标准方式。

`SHeaderCtrl`（列头）、`SListBox`（列表项，含滚动偏移 `m_siVer.nPos` 手算）、
`STreeCtrl`（节点，用 `GetAccVisibleItem(iChild-1)` + `GetItemText`，状态补
`STATE_SYSTEM_EXPANDED`）都是同一套模式。

### 5.3 覆写 AccChild（MVC 可见项）

```cpp
// SAccProxyListView / SAccProxyMCListView / SAccProxyTileView / SAccProxyTreeView
// （SAccProxyCmnCtrl.cpp:699 起，四者同构，仅控件类型不同）
int STDMETHODCALLTYPE SAccProxyListView::GetAccChildCount()
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChildCount() : 0;
}
IWindow *STDMETHODCALLTYPE SAccProxyListView::GetAccChild(int iChild)
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccChild(iChild) : NULL;
}
int STDMETHODCALLTYPE SAccProxyListView::GetAccSelIndex()
{
    SListView *pView = sobj_cast<SListView>(m_pWnd);
    return pView ? pView->GetVisibleAccSelIndex() : 0;
}
```

### 5.4 控件侧唯一的 ACC 代码：事件上报

控件（及容器）在状态变化处调用 `accNotifyEvent`，通知 AT 更新缓存/重新朗读。
全库范围内的调用点：

| 位置 | 事件 | 触发时机 |
|---|---|---|
| `Swnd.cpp:793` | `EVENT_OBJECT_NAMECHANGE` | `SetWindowText` 文本变化 |
| `Swnd.cpp:2372` | `EVENT_OBJECT_CREATE` | 窗口创建（OnInit） |
| `Swnd.cpp:2416` | `EVENT_OBJECT_DESTROY` | 窗口销毁前 |
| `Swnd.cpp:2752` / `2757` | `EVENT_OBJECT_SHOW` / `HIDE` | 显示/隐藏 |
| `Swnd.cpp:2999` | `EVENT_OBJECT_FOCUS` | 获得焦点 |
| `Swnd.cpp:3828` | `EVENT_OBJECT_STATECHANGE` | 状态变化（enable/check 等） |
| `SHeaderCtrl.cpp:61/189/199` | `EVENT_OBJECT_REORDER` | 列增删/顺序变化 |
| `STabCtrl.cpp:400/415/681` | `EVENT_OBJECT_REORDER` | 页签增删/移动 |
| `STabCtrl.cpp:579` | `EVENT_OBJECT_SELECTION` | 当前页切换 |
| `SListbox.cpp:79` | `EVENT_OBJECT_SELECTION` | 选中项变化 |
| `SViewBase.cpp:83/95` | `EVENT_OBJECT_REORDER` | 可见集合/数据变化（读屏器重建子级树） |
| `SViewBase.cpp:391` | `EVENT_OBJECT_SELECTION` | 选中项变化 |

这就是控件层全部的 ACC 接触面：**12 处一行调用，没有一行 MSAA 类型**。

---

## 6. 经 swinx 实现三平台 ACC

### 6.1 策略：一套 MSAA 面，三种"下层"

SOUI 的 ACC 实现**只写了一份**——就是第 2–5 章讲的 `SAccessible` + `IAccProxy`。
它按标准 MSAA 语义实现，调用标准 oleacc API：

- `LresultFromObject`（`SHostWnd::OnGetObject` 里）
- `NotifyWinEvent`（`SWindow::accNotifyEvent` 里）
- `__suidof` / `IID_IAccessible` / `CHILDID_SELF` / `ROLE_SYSTEM_*` / `STATE_SYSTEM_*`

在 Windows 上这些由系统 `oleacc.dll` / `oleaut32.dll` 提供；在 macOS/Linux 上，
**由 swinx 提供同一组 API 的头文件与实现**。于是 SOUI 侧代码三平台完全一致。

```
                      SOUI 的 SAccessible / IAccProxy（一份代码）
                                     │
                    标准 oleacc API 面（同一套声明与语义）
                                     │
        ┌────────────────────────────┼────────────────────────────┐
        ▼                            ▼                            ▼
   Windows 原生              swinx: macOS 桥              swinx: Linux 桥
   oleacc.dll                SNsAccessibility.mm          SAtSpi.cpp
   （swinx 不介入）           ↓ NSAccessibility            ↓ AT-SPI2 / D-Bus
                             VoiceOver                    Orca / accerciser
```

### 6.2 swinx 提供的进程内 MSAA 运行时（`swinx/src/oleacc.cpp`）

这是三个平台桥的**公共下游**。它提供：

**(a) `LresultFromObject` / `ObjectFromLresult` —— 句柄往返**

见 2.2 节。要点：弱登记句柄表（容量 256，FIFO 淘汰最旧）、不 AddRef、
每次产生新句柄、按 `(句柄, wParam)` 匹配、失败以 HRESULT 作 LRESULT 返回。
代码注释明确写了所有权约定：

> **swinx 不持有 IAccessible**。SHostWnd::OnGetObject 在响应 WM_GETOBJECT 时按需取
> 对象（SOUI_ENABLE_ACC 关闭时直接返回 0），swinx 只把它"弱登记"进句柄表用于同一次
> 同步往返的解析，不 AddRef、不延长生命周期。

**(b) `AccessibleObjectFromWindow(hwnd, dwObjectID, riid, ppv)` —— 触发 WM_GETOBJECT**

```cpp
LRESULT lResult = ::SendMessageA(hwnd, WM_GETOBJECT, (WPARAM)-1,
                                 (LPARAM)(LONG)dwObjectID);
if (lResult == 0) return E_FAIL;
return ObjectFromLresult(lResult, riid, (WPARAM)-1, ppv);
```

**(c) `AccessibleObjectFromPoint` / `...FromEvent` / `AccessibleChildren`**

`AccessibleChildren` 从 child id 1 开始枚举、永不产生 `CHILDID_SELF`；
`get_accChild` 失败则回退 `VT_I4` 简单元素；`cChildren < 0` 时不写 `*pcObtained`。

**(d) `NotifyWinEvent` + `SetWinEventHook` —— 事件通道**

```cpp
void WINAPI NotifyWinEvent(DWORD event, HWND hwnd, LONG idObject, LONG idChild)
{
    // 与 user32 一致：只投递、立即返回
    PendingWinEvent ev;
    ev.event = event; ev.hwnd = hwnd; ev.idObject = idObject; ev.idChild = idChild;
    ev.idThread = ::GetCurrentThreadId(); ev.dwmsEventTime = ::GetTickCount();
    ev.matched = SnapshotWinEventMatches(ev);      // ← 事件产生时刻快照钩子
    if (ev.matched.empty()) return;                // 无人接收：立即丢弃
    // 入队（容量 512，满则丢最旧）
    ...
}
```

两个**必须照抄 user32** 的语义：

1. **接收者在事件发出那一刻确定**（入队时快照），而非派发时再匹配。这是硬要求：
   AT-SPI 桥持有全区间钩子，"无接收者则丢弃"的检查在派发时匹配会恒为假，
   无钩子期间发出的事件将滞留队列并泄漏给之后注册的钩子。
   → 事件产生时若无匹配钩子，**立即丢弃，不占队列**；已入队的事件不受随后注销影响。
2. **回调异步执行**：`NotifyWinEvent` 立即返回，钩子回调在消息泵里由
   `SwinxDispatchPendingWinEvents()`（`sysapi.cpp` 的 `GetMessage`/`PeekMessage` 调用）
   派发，与 `OUTOFCONTEXT` 钩子一致。`t_inDispatchWinEvents` 做同线程防重入。
   `WINEVENT_SKIPOWNPROCESS` 钩子注册成功但永远收不到（进程内所有事件都来自本进程）；
   `WINEVENT_SKIPOWNTHREAD` 按注册线程过滤。

**(e) 角色/状态文本**：`GetRoleTextW/A`、`GetStateTextW/A`（查表 + UTF-8 转码）。

**(f) `SwinxAccResolvePath` —— 桥共享的"下钻内核"**

声明在 `swinx/src/SwinxAccGlue.h`（**实现细节，不是公共 API**——公共头
`swinx/include/oleacc.h` 保持与 Windows SDK 相同的 API 面，不放 swinx 扩展）：

```cpp
HRESULT WINAPI SwinxAccResolvePath(HWND hwnd, const LONG *pChain, LONG cChain,
                                   IAccessible **ppAcc, LONG *pChildId);
```

语义：从 `hwnd` 的可访问根（`OBJID_CLIENT`，回退 `OBJID_WINDOW`）出发，沿
child id 链逐级 `get_accChild` 下钻。输出：

- `*pChildId == CHILDID_SELF` → `*ppAcc` 就是目标对象；
- `*pChildId != CHILDID_SELF` → 目标是**简单元素**，`*ppAcc` 是其父对象，
  查询属性时须带 childId；
- 路径中途断链（非末级的简单元素）→ `E_FAIL`（控件已销毁/重建）。

调用方负责 `Release(*ppAcc)`。**每次调用都重新向窗口发 `WM_GETOBJECT`**，
所以平台桥从不缓存接口指针，天然免疫悬垂。

### 6.3 macOS：NSAccessibility 桥

`swinx/src/platform/cocoa/SNsAccessibility.h/.mm`。

**问题**：SOUI（以及所有 swinx 承载的窗口内容）整体绘制在一个 `NSView`（`SNsWindow`）
里。VoiceOver 默认只能看到一个不透明的大视图。

**方案**：把 IAccessible 暴露的控件树映射成 `NSAccessibilityElement` 树，挂到内容视图下：

```
NSWindow
  └─ contentView(SNsWindow)
       └─ SwinxAccElement（根：OBJID_CLIENT / OBJID_WINDOW）
            └─ SwinxAccElement ...（递归）
```

**核心机制——"身份壳"（identity shell）**：

```objc
@interface SwinxAccElement : NSAccessibilityElement
@property (nonatomic, readonly) HWND swinxHwnd;
@property (nonatomic, readonly) NSArray<NSNumber *> *swinxChain;  // child id 链
- (void)swinxInvalidateChildren;
@end
```

- 元素位置由 `(hwnd, childId 链)` 唯一确定：链为空 = 窗口客户区根；
  链 `[i, j, ...]` = 根的第 i 个孩子的第 j 个孩子……
- **壳不持有任何 COM 引用**（不 AddRef）。每次属性/动作访问都经
  `SwinxAccResolvePath` 重新发 `WM_GETOBJECT`，拿到接口**现查现用现放**。
- 壳按需构建并缓存，用于维持 **NSAccessibility 对象身份稳定**——VoiceOver 依赖
  "同一个元素对象"来追踪焦点。结构变化事件只丢弃壳缓存（零成本，因为不持 COM 引用）。
- 控件销毁 → 解析失败 → 表现为空属性，**绝不访问悬垂指针**。

**坐标**：MSAA `accLocation` 返回 swinx 全局像素坐标（主屏左上角、y 向下），
经 `SNsCoord.h` 换算成 Cocoa 屏幕坐标（point、y 向上）。

**事件**：`NotifyWinEvent` 经标准 `SetWinEventHook` 转发到桥，映射为
`NSAccessibility` 通知；事件可能来自任意线程，统一 dispatch 到主队列。

**macOS 编译约束（两处必须遵守）**——见 `SNsAccessibility.h` 头注释：

1. **Cocoa 必须先于 swinx 头导入**：swinx 的 `gdi.h` 有
   `#define Polygon Polygon_Priv`（避开 Quickdraw 的 `Polygon` typedef）。
   若先定义，`Quickdraw.h` 的 `typedef MacPolygon Polygon` 会被宏破坏而编译失败。
2. **进入 ObjC 世界前必须 `#undef interface`**：swinx 的 `basetyps.h` 把
   `interface` 宏定义为 `struct`，而 macOS SDK 头文件用到 `interface` 这个词。
   ```objc
   #import <Cocoa/Cocoa.h>
   #include <windows.h>
   #include <oleacc.h>
   #undef interface
   ```
3. 该头只被 `.mm`（Objective-C++）包含——`IAccessible` 是 C++ 类型。

### 6.4 Linux：AT-SPI2 桥

`swinx/src/platform/linux/SAtSpi.h/.cpp`。

**问题**：swinx 在 Linux 上已提供完整 MSAA 面，SOUI 也照 Win32 方式响应
`WM_GETOBJECT`。但 Linux 的屏幕阅读器（Orca / accerciser）只认 AT-SPI2，通过
无障碍总线（a11y bus）上的 D-Bus 接口拉取对象树。

**方案**：在 MSAA 树与 AT-SPI2 对象树之间架一层桥：

```
MSAA 树（进程内）  <-->  AT-SPI2 对象树（a11y bus 上的 D-Bus 对象路径）
```

**协议要点**（与 at-spi2-core 一致）：

1. **总线地址**：先取环境变量 `AT_SPI_BUS`，否则向 session bus 的
   `org.a11y.Bus`（`/org/a11y/bus`）调 `GetAddress()`。
2. **注册**：连接 a11y bus 后，向 `org.a11y.atspi.Registry` 的
   `/org/a11y/atspi/accessible/root` 调 `org.a11y.atspi.Socket.Embed(plug=(so))`，
   拿到 registry 根对象的 `(bus name, object path)`。
3. **对象树**：应用必须有唯一根 `/org/a11y/atspi/accessible/root`（实现
   `org.a11y.atspi.Application`），其余对象挂在该路径下。
4. **事件**：D-Bus 信号，`interface = org.a11y.atspi.Event.<Class>`，
   `member` = 事件名（CamelCase），签名固定为 `"siiva{sv}"`。

**设计取舍**：

- **对象路径是确定性编码的**。这样不需要在桥里长期保存 `IAccessible*`
  （SOUI 控件销毁后指针失效，缓存必然悬垂）；每次请求按路径从窗口根重新导航
  （经 `SwinxAccResolvePath`），代价 O(深度) 次调用，换来绝不访问野指针。
- **所有 D-Bus 收发都在创建连接的那个线程**（通常是 UI 线程）完成：桥用
  `SetTimer(NULL, 0, 50, proc)` 挂进 swinx 的消息循环，定时器回调里驱动
  `dbus_connection_read_write_dispatch`。因此处理 D-Bus 请求时可以直接调用 MSAA
  （含 `SendMessage(WM_GETOBJECT)`），不需要跨线程 marshal。

**惰性兜底初始化**（`oleacc.cpp`）：

```cpp
#if defined(__linux__) && !defined(__ANDROID__) && !defined(__OHOS__)
extern "C" void SwinxAtSpiInit(void) __attribute__((weak));   // 弱符号
#define SWINX_HAVE_ATSPI 1
#endif
```

弱符号的意义：某些同样定义 `__linux__` 的平台（如 OpenHarmony）走 `mobile.cmake`、
不编译 `SAtSpi.cpp`，弱引用保证链接仍然通过。初始化点有两处：

- **主初始化点**：`SConnection.cpp` 的 `SConnMgr::instance()` 里，在 `inst`
  完整构造之后、**且不持有锁**时调用 `SwinxAtSpiInit()`。
  **约束：不能在 `SConnMgr` 构造函数里调用**——桥内 `EnsureInit` 会经
  `SetTimer(NULL,...)`（`wnd.cpp`）回到 `instance()`，而 C++ 静态局部变量的初始化
  守卫（`__cxa_guard`）不可重入，在同一守卫的动态范围内递归进入会对该 guard
  第二次 acquire，**永久阻塞**（与 `s_cs` 的 `recursive_mutex` 无关）。
  放在 `inst` 构造完成之后即可让重入路径直接返回。
- **惰性兜底**：`LresultFromObject` 与 `NotifyWinEvent` 里调 `EnsureAtSpiInit()`
  （`std::call_once` 保证一次），覆盖不经 `SConnMgr` 的调用路径（如单元测试直接调
  MSAA API）。`SOUI_ENABLE_ACC` 关闭时桥完全不启动（不开 D-Bus 连接、不占 timer）。



### 6.5 三平台对照

| 维度 | Windows | macOS | Linux |
|---|---|---|---|
| MSAA API 提供方 | 系统 `oleacc.dll` / `oleaut32.dll` | swinx `src/oleacc.cpp` + `src/objbase.cpp` + `src/variant.cpp` | 同左 |
| SOUI 侧代码 | 同一份 `SAccessible` / `IAccProxy` | 同左 | 同左 |
| WM_GETOBJECT 往返 | 系统跨进程 marshalling | swinx 进程内句柄表 | swinx 进程内句柄表 |
| 下层协议 | MSAA/COM（原生） | `NSAccessibility` (Objective-C) | `AT-SPI2` over D-Bus |
| 桥文件 | 无（原生） | `platform/cocoa/SNsAccessibility.mm` | `platform/linux/SAtSpi.cpp` |
| 对象身份表达 | `(hwnd, OBJID)` / COM 对象指针 | `SwinxAccElement`：`(hwnd, child 链)` | `AccNode`：`(hwnd, child 链)` + D-Bus 路径 |
| 桥是否持有 IAccessible | 不适用（系统管引用） | **不持有**（身份壳） | **不持有**（确定性路径） |
| 事件通道 | 系统 `NotifyWinEvent` → 消息泵 | `SetWinEventHook` → `NSAccessibility` 通知 | `SetWinEventHook` → D-Bus 信号 |
| 桥初始化时机 | 不适用 | 惰性、幂等（`SwinxNsAccInit`） | `SConnMgr` 构造后 + `LresultFromObject`/`NotifyWinEvent` 兜底 |
| 条件编译 | 无 | `__APPLE__` | `__linux__ && !__ANDROID__ && !__OHOS__` |

---

## 7. gcc / clang 上的 COM：无 `__uuidof`、无 COM 运行时

这是三平台支持里**技术含量最高**的一层。SOUI 的 ACC 代码按 MSVC 风格写成：

```cpp
class SOUI_EXP SAccessible : public IAccHelper, public SUnkImpl<IAccessible>
{
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj) override { ... }
    IUNKNOWN_BEGIN(IAccessible)       // ← 展开出 AddRef/Release/QueryInterface
    IUNKNOWN_ADD_IID(IAccHelper)
    IUNKNOWN_ADD_IID(IDispatch)
    IUNKNOWN_END()
};

// 使用点
m_pAcc->QueryInterface(__suidof(IAccHelper), (void **)&accHelper);

// 接口声明
DEFINE_SGUID(IID_IAccHelper, 0x9FAF3067, 0x6723, 0x40DA, 0xA5, 0x6B, 0xCD, 0xCB, 0x11, 0xB8, 0x90, 0x2B);
DECLARE_INTERFACE_(IAccHelper, IUnknown) { DECLARE_CLASS_SIID(IID_IAccHelper) ... };
```

这些在 MSVC 上由 `<unknwn.h>` / `<objbase.h>` / `__declspec(uuid)` 支持。
在 gcc / clang 上，`__uuidof` **不是关键词**、`__declspec(uuid)` **不存在**、
MSAA/OLE 运行时**也没有**。swinx 用一组小而完整的兼容头把这一层补齐。

### 7.1 问题分解

| MSVC 提供的能力 | gcc/clang 的缺口 | swinx 的补法 |
|---|---|---|
| `__uuidof(IFace)` 关键词（编译期类型→IID 映射） | 无此关键词 | `guiddef.h`：`__uuidof(X)` / `__suidof(X)` 都定义为 `X::GetIID()` |
| `__declspec(uuid("..."))` 给接口挂 IID | 无此扩展 | `DECLARE_CLASS_SIID(iid)` 宏在每个接口内定义 `static GetIID()` |
| `GUID`/`IID`/`REFIID` 类型与 `IsEqualIID` | 无 | `guiddef.h` 自带类型与 `memcmp` 实现 |
| `DECLARE_INTERFACE_` / `STDMETHOD` / `PURE` 等 | 无 | `basetyps.h` 完整宏体系（C 与 C++ 两套展开） |
| `DEFINE_GUID` 产生 GUID 实例 | 无 | `guiddef.h` 重定义 + `SELECT_ANY` 弱符号 |
| `IUnknown` 与引用计数实现 | 无 | `SUnkImpl<T>` + `IUNKNOWN_BEGIN/ADD_IID/END` |
| `BSTR` / `SysAllocString` | 无 | `objbase.cpp`（`SysAllocString/Len/LenByteLen` 等） |
| `VARIANT` / `VariantInit/Clear/Copy` | 无 | `oaidl.h` + `variant.cpp` + `safearray.cpp` |
| `IDispatch` | 无 | `oaidl.h` 声明 + 各实现类 `E_NOTIMPL` |
| `IID_IAccessible` 等 MSAA 常量 | 无 | `swinx/include/oleacc.h`（照 Windows SDK API 面） |

### 7.2 IID 机制：把"编译期类型映射"变成"静态函数"

**核心 trick**——`swinx/include/guiddef.h`：

```cpp
#ifdef __cplusplus
#define DECLARE_CLASS_SIID(cls_id) \
    static const GUID &GetIID()    \
    {                              \
        return cls_id;             \
    }
#else
#define DECLARE_CLASS_SIID(cls_id)
#endif

#define __suidof(ClassName) ClassName::GetIID()
#define __uuidof(ClassName) ClassName::GetIID()
```

**MSVC 的 `__uuidof(IFace)` 是编译期关键词**，编译器从类型的 `__declspec(uuid(...))`
属性里取出 IID，生成一个编译期常量引用。gcc/clang 没有这个能力，
swinx 就把它**降级为一次静态函数调用**：每个接口类内部用 `DECLARE_CLASS_SIID`
定义 `static const GUID &GetIID()`，`__suidof` / `__uuidof` 都展开成 `X::GetIID()`。

效果：

- **调用点写法与 MSVC 完全一致**（`__suidof(IAccHelper)`）→ 源码零改写。
- 返回值是 `const GUID&`（引用），与 MSVC `__uuidof` 返回 `const GUID&` 一致，
  所以 `QueryInterface(__suidof(IAccHelper), ...)` 的 `REFIID` 参数类型吻合
  （`REFGUID` 在 C++ 下定义为 `const GUID &`，见 `guiddef.h`）。
- 唯一约束：**接口必须用 `DECLARE_INTERFACE_` 定义并带 `DECLARE_CLASS_SIID`**。
  SOUI 的 ACC 接口（`IAccHelper`）就是这么写的：

```cpp
// SOUI/include/interface/sacchelper-i.h
#undef  INTERFACE
#define INTERFACE IAccHelper
DECLARE_INTERFACE_(IAccHelper, IUnknown)
{
    DECLARE_CLASS_SIID(IID_IAccHelper)     // ← 落地为 IAccHelper::GetIID()
    STDMETHOD_(HRESULT, QueryInterface)(THIS_ REFIID riid, void **ppvObject) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
    STDMETHOD_(void, SetOwner)(THIS_ IWindow *pOwner) PURE;
    STDMETHOD_(IWindow *, GetOwner)(CTHIS) SCONST PURE;
};
```

### 7.3 GUID 类型与比较：不依赖 Windows

`swinx/include/guiddef.h`：

```cpp
typedef struct _GUID {
    uint32_t Data1;  uint16_t Data2;  uint16_t Data3;  uint8_t Data4[8];
} GUID;

typedef GUID IID;   typedef IID   *LPIID;
typedef GUID CLSID; typedef CLSID *LPCLSID;
typedef GUID FMTID; typedef FMTID *LPFMTID;

#define IsEqualGUID(id1, id2)          (memcmp(&id1, &id2, sizeof(GUID)) == 0)
#define IsEqualIID(riid1, riid2)       IsEqualGUID(riid1, riid2)
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID(rclsid1, rclsid2)

#ifdef __cplusplus
inline bool operator==(const GUID &id1, const GUID &id2) { return IsEqualGUID(id1, id2); }
#endif

#ifdef __cplusplus
#define REFGUID const GUID &
#else
#define REFGUID const GUID *
#endif
#define REFIID   REFGUID
#define REFCLSID REFGUID
```

**用 `memcmp` 比较 GUID** 是这里最省事的正确做法：GUID 是 POD，逐字节比较等价于
语义比较。不依赖任何 Windows API。

**`REFIID` 的 C/C++ 差异**：C++ 下是 `const GUID&`，C 下是 `const GUID*`。
这与 Windows SDK 一致（`basetyps.h` 的 C 展开也需要 `This` 指针形式）。

### 7.4 `DEFINE_GUID` 与弱符号

```cpp
#ifdef DEFINE_GUID
#undef DEFINE_GUID
#endif

#ifndef SELECT_ANY
#define SELECT_ANY __attribute__((weak))
#endif

#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID SELECT_ANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID name
#endif
```

`SELECT_ANY = __attribute__((weak))` 的作用：允许在**多个翻译单元**里各有一个
弱定义的同名 GUID，链接器任选其一，不会报"重复定义"。这在头文件里写
`DEFINE_GUID`（同时被多个 TU 包含）时是必需的。

### 7.5 接口声明宏体系：`basetyps.h`

MSVC 下 `DECLARE_INTERFACE_` / `STDMETHOD` / `PURE` / `THIS_` 由
`<basetyps.h>`（Windows SDK）提供。swinx 自带一份等价实现
（`swinx/include/basetyps.h`，196 行），关键是**给 C 与 C++ 两套展开**：

```cpp
#if defined(__cplusplus) && !defined(CINTERFACE)
#define __STRUCT__                struct
#define interface                 __STRUCT__
#define STDMETHOD(method)         virtual COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type, method)  virtual COM_DECLSPEC_NOTHROW type STDMETHODCALLTYPE method
#define PURE                      = 0
#define THIS_
#define THIS                                          void
#define DECLARE_INTERFACE(iface)                      interface DECLSPEC_NOVTABLE iface
#define DECLARE_INTERFACE_(iface, baseiface)          interface DECLSPEC_NOVTABLE iface : public baseiface
#define STDMETHODIMP        HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#else  // C 语言展开
#define interface struct
#define STDMETHOD(method)         HRESULT(STDMETHODCALLTYPE *method)
#define STDMETHOD_(type, method)  type(STDMETHODCALLTYPE *method)
#define PURE
#define THIS_ INTERFACE FAR *This,
#define THIS  INTERFACE FAR *This
#define DECLARE_INTERFACE(iface)  typedef interface iface { struct iface##Vtbl FAR *lpVtbl; } iface; ...
#endif
```

于是同一份接口定义，在 C++ 下展开成抽象基类（虚函数表），在 C 下展开成
`{ vtable 指针 }` 结构体 + `IfaceVtbl` 函数指针表——这正是 `ioleauto`/`oleacc`
接口头文件能在两种语言下通用的原因。

**`interface` 在 C++ 下被定义为 `struct` 而非 `class`**，注释解释了原因：

> NOTE: Our documentation says '#define interface class' but we use 'struct' instead
> of 'class' to keep a lot of 'public:' lines out of the interfaces.

**注意副作用**：`interface` 变成宏之后，会污染 macOS SDK 头里同名标识符——
所以 `SNsAccessibility.h` 必须在 `#import <Cocoa/Cocoa.h>` 之后 `#undef interface`
（见 6.3 节）。

### 7.6 `IUnknown` 的实现：`SUnkImpl<T>` + `IUNKNOWN_*` 宏

**第一层：引用计数模板**（`swinx/src/SUnkImpl.h`，与
`SOUI/include/helper/SUnkImpl.h` 同源）：

```cpp
template <class T>
class SUnkImpl : public T {
  public:
    SUnkImpl() : m_cRef(1) {}
    virtual ~SUnkImpl() {}
    virtual void OnFinalRelease() { delete this; }   // 可覆写做差异化销毁

  protected:
    STDMETHODIMP_(ULONG) _AddRef(void) { return (ULONG)InterlockedIncrement(&m_cRef); }
    STDMETHODIMP_(ULONG) _Release(void) {
        LONG ret = InterlockedDecrement(&m_cRef);
        if (ret == 0) OnFinalRelease();
        return ret;
    }
    LONG GetRef() const { return m_cRef; }

  private:
    LONG m_cRef;
};
```

用法：`class SAccessible : public IAccHelper, public SUnkImpl<IAccessible>`——
`SUnkImpl<IAccessible>` 提供 `AddRef`/`Release`（经 `_AddRef`/`_Release`）。

> ⚠️ 注意：这里**没有 `QueryInterface`**。`QueryInterface` 必须由派生类自己实现，
> 因为一个类可能实现多个接口（`SAccessible` 同时是 `IAccessible`、`IAccHelper`、
> `IDispatch`），只有它知道"我是谁、支持哪些 IID"。

**第二层：`QueryInterface` 声明式定义**（`IUNKNOWN_*` 宏）：

```cpp
#define IUNKNOWN_BEGIN2(iid, cls)                                    \
    STDMETHODIMP_(ULONG) AddRef(void) override { return _AddRef(); }  \
    STDMETHODIMP_(ULONG) Release(void) override { return _Release(); }\
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj) override {\
        if (ppvObj == NULL) return E_INVALIDARG;                      \
        *ppvObj = NULL;                                               \
        if (IsEqualIID(riid, iid) || IsEqualIID(riid, IID_IUnknown))  \
            *ppvObj = (cls *)this;

#define IUNKNOWN_ADD_IID2(iid, cls) \
    if (IsEqualIID(riid, iid)) *ppvObj = (cls *)this;

#define IUNKNOWN_END()                                    \
    if (*ppvObj) { AddRef(); return S_OK; }                \
    return E_NOINTERFACE; }

#define IUNKNOWN_BEGIN(iface)    IUNKNOWN_BEGIN2(__suidof(iface), iface)
#define IUNKNOWN_ADD_IID(iface)  IUNKNOWN_ADD_IID2(__suidof(iface), iface)
```

`SAccessible` 的用法：

```cpp
IUNKNOWN_BEGIN(IAccessible)      // 接受 IID_IAccessible 与 IID_IUnknown，this 作为 IAccessible*
IUNKNOWN_ADD_IID(IAccHelper)     // 额外接受 IID_IAccHelper
IUNKNOWN_ADD_IID(IDispatch)      // 额外接受 IID_IDispatch
IUNKNOWN_END()
```

展开后是一个完整的 COM 三件套：`AddRef`（转发 `_AddRef`）、`Release`（转发
`_Release`）、`QueryInterface`（逐个 `IsEqualIID` 比较，命中则 `*ppvObj = (cls*)this`
并 `AddRef()`，否则 `E_NOINTERFACE`）。

**注意三个细节**：

1. **`IUNKNOWN_BEGIN` 的第一个分支同时接受 `IID_IUnknown`**——这是 COM 铁律
   （每个对象都必须响应 `IID_IUnknown`）。
2. **命中后必须 `AddRef()`**——因为 `(cls*)this` 是裸指针，但返回给调用方的是一份
   新引用。`IUNKNOWN_END` 统一处理。
3. **`AddRef`/`Release`/`QueryInterface` 都带 `override`**——swinx 版加了 `override`，
   SOUI 版（`SOUI/include/helper/SUnkImpl.h`）用 `#ifndef IUNKNOWN_BEGIN` 守卫，
   谁先定义谁生效。这保证两个版本不会冲突，且都指向同一套语义。

### 7.7 接口 IID 的实例定义：`DEFINE_SGUID` + `INITGUID`

`DECLARE_CLASS_SIID(IID_IAccHelper)` 里的 `IID_IAccHelper` 必须**在某处有唯一定义**。
SOUI 用自己的 `DEFINE_SGUID` 机制（`SOUI/include/SGuidDef.h`）：

```cpp
#include <soui_exp.h>
#include <guiddef.h>

#ifdef INITGUID
#define DEFINE_SGUID(name, l, w1, w2, b1, ..., b8) \
    EXTERN_C const GUID SOUI_EXP name = { l, w1, w2, { b1, ..., b8 } }
#define DEFINE_SGUID2(name, l, w1, w2, b1, ..., b8) \
    EXTERN_C const GUID name = { l, w1, w2, { b1, ..., b8 } }
#else
#define DEFINE_SGUID(name, l, w1, w2, b1, ..., b8) \
    EXTERN_C const GUID SOUI_EXP name          // 仅声明
#endif
```

- **接口头文件**里用 `DEFINE_SGUID`（不带 `INITGUID`）→ 只产生声明。
- **专门一个 .cpp** 里先 `#define INITGUID` 再包含，产生唯一定义：

```cpp
// SOUI/src/core/SGuids.cpp
#include <souistd.h>
#include <SInitGuid.h>          // #define INITGUID / #include <SGuidDef.h>

DEFINE_SGUID(IID_ICtrl,       0xBCB520F9, 0x3C81, 0x4AD7, ...);
DEFINE_SGUID(IID_IAccHelper,  0x9FAF3067, 0x6723, 0x40DA, 0xA5, 0x6B, 0xCD, 0xCB, 0x11, 0xB8, 0x90, 0x2B);
DEFINE_SGUID(IID_IListView,   0x107ed2ea, 0x6c95, 0x4cec, ...);
// ... 约 50 个接口 IID 集中在此 ...
```

这个模式与 MSVC 的 `DEFINE_GUID` + `initguid.h` 完全相同，好处是：
- 接口头可以随意被包含，不会重复定义；
- 全框架的 IID 定义集中在一个文件，容易审计；
- `SOUI_EXP` 保证定义在共享库构建下正确导出（Windows 上 `__declspec(dllexport)`）。

**注意**：`IID_IAccHelper` 在两个地方出现——`sacchelper-i.h` 里
`DEFINE_SGUID(...)`（声明），`SGuids.cpp` 里 `DEFINE_SGUID(...)`（定义）。
两者必须**值一致**（当前是 `{0x9FAF3067, 0x6723, 0x40DA, {0xA5, 0x6B, 0xCD, 0xCB, 0x11, 0xB8, 0x90, 0x2B}}`）。

### 7.8 BSTR / VARIANT / IDispatch 的实际提供方

ACC 的接口大量使用 OLE 自动化类型。swinx 提供：

| 类型 / API | 头文件 | 实现 |
|---|---|---|
| `BSTR`、`SysAllocString`、`SysAllocStringLen`、`SysAllocStringByteLen`、`SysFreeString`、`SysStringLen` | `swinx/include/oaidl.h` / `oleauto.h` | `swinx/src/objbase.cpp`（`bstr_internal.h` 内部结构） |
| `VARIANT`、`VARIANTARG`、`VariantInit`、`VariantClear`、`VariantCopy` | `swinx/include/oaidl.h` | `swinx/src/variant.cpp` |
| `SAFEARRAY` | `swinx/include/oaidl.h` | `swinx/src/safearray.cpp` |
| `IDispatch`、`DISPID`、`DISPPARAMS`、`EXCEPINFO` | `swinx/include/oaidl.h` | 由各实现类提供（ACC 里全部 `E_NOTIMPL`） |
| `IID_IAccessible`、`ROLE_SYSTEM_*`、`STATE_SYSTEM_*`、`OBJID_*`、`CHILDID_SELF`、`EVENT_OBJECT_*` | `swinx/include/oleacc.h` | 常量/声明（64 个 `ROLE_SYSTEM_*`） |
| `HRESULT`、`S_OK`、`E_NOTIMPL`、`E_INVALIDARG`、`E_NOINTERFACE`、`E_FAIL`、`S_FALSE` | `swinx/include/winerror.h` / `ctypes.h` | 常量 |

`variant.cpp` 的头注释点明了 ACC 实际依赖的那一小部分语义，值得引用：

```
 * The COM surface SOUI actually depends on (see SwndAccessible.cpp, compiled
 * when SOUI_ENABLE_ACC is on) requires:
 *  - VariantCopy with VT_VARIANT|VT_BYREF source dereferences the pointer and
 *    copies the referenced variant (accValidateNavStart loops on it);
 *  - VariantClear must release interface references (VT_DISPATCH/VT_UNKNOWN)
 *    and destroy SAFEARRAYs, so VARIANT-held COM references are balanced;
 *  - VT_BYREF payloads are pointers owned by the caller: neither Clear nor
 *    Copy may free/reallocate the pointee.
```

**注意 `VariantClear` 释放 `VT_DISPATCH`/`VT_UNKNOWN` 引用**——这与
`SAccessible` 里"返回子对象时 `QueryInterface` 已经 AddRef 过一次"配平，是引用计数
不泄漏的关键。

**`BSTR` 在非 Windows 的表示**：`swinx` 的 `WCHAR` 在 macOS 是 `wchar_t`（4 字节），
所以 `BSTR` 载荷是 UTF-32。cocoa 桥里的转换注释：

```objc
/* swinx 的 WCHAR 在 macOS 是 wchar_t（4 字节），BSTR 载荷即 UTF-32 */
static NSString *SwinxAccStringFromBstr(BSTR s) { ... }
```

### 7.9 小结：非 MSVC 上的 COM 支持全景

```
应用/框架代码（写 MSVC 风格 COM）
    __uuidof(IFace) / __suidof(IFace)
    DECLARE_INTERFACE_(I, Base) { DECLARE_CLASS_SIID(iid) STDMETHOD(...) PURE; }
    class Impl : public IFace, public SUnkImpl<IFace> {
        IUNKNOWN_BEGIN(IFace) IUNKNOWN_ADD_IID(Other) IUNKNOWN_END()
    };
    DEFINE_GUID(...) / DEFINE_SGUID(...)
            │
            ▼
┌──────────────────────────────────────────────────────────────────┐
│ swinx 兼容头（gcc / clang 上等价于 Windows SDK 的对应头）           │
│                                                                  │
│ guiddef.h      GUID/IID/CLSID/REFIID + IsEqualIID(memcmp)        │
│                DECLARE_CLASS_SIID → static GetIID()              │
│                __suidof / __uuidof = X::GetIID()                 │
│                DEFINE_GUID + SELECT_ANY(__attribute__((weak)))   │
│                                                                  │
│ basetyps.h     DECLARE_INTERFACE_/STDMETHOD/STDMETHODIMP/        │
│                PURE/THIS_/interface=struct（C 与 C++ 两套展开）    │
│                                                                  │
│ unknwn.h       IUnknown 声明 + IID_IUnknown                      │
│ oaidl.h        BSTR / VARIANT / IDispatch / SAFEARRAY 类型与声明   │
│ oleauto.h      SysAllocString 族声明                              │
│ objbase.h      SysAllocString 族实现                              │
│ variant.cpp    VariantInit / VariantClear / VariantCopy          │
│ safearray.cpp  SAFEARRAY 生命周期                                 │
│ oleacc.h       IAccessible / 64 个 ROLE_SYSTEM_* / 角色状态文本    │
│ winerror.h     HRESULT 常量                                       │
└──────────────────────────────────────────────────────────────────┘
            │
            ▼
┌──────────────────────────────────────────────────────────────────┐
│ SUnkImpl<T>（引用计数）+ IUNKNOWN_* 宏（QueryInterface）           │
│ → 等价于 MSVC 上的 ATL CComObject/IDispatchImpl，但只有 80 行      │
└──────────────────────────────────────────────────────────────────┘
            │
            ▼
┌──────────────────────────────────────────────────────────────────┐
│ swinx/src/oleacc.cpp  —— 进程内 MSAA 运行时                       │
│ LresultFromObject / ObjectFromLresult /                          │
│ AccessibleObjectFromWindow / ...FromPoint / ...FromEvent /       │
│ AccessibleChildren / NotifyWinEvent / SetWinEventHook /          │
│ GetRoleText / GetStateText / SwinxAccResolvePath                 │
└──────────────────────────────────────────────────────────────────┘
```

**总结一句话**：gcc/clang 上"实现 COM"并不需要 COM 运行时，只需要
**(1) 一套与 MSVC 同形的接口声明宏**（`basetyps.h`）、
**(2) 把 `__uuidof` 从编译期关键词降级为静态函数**（`DECLARE_CLASS_SIID` + `GetIID()`）、
**(3) 一个 80 行的引用计数 + QueryInterface 模板**（`SUnkImpl<T>` + `IUNKNOWN_*`）、
**(4) OLE 自动化类型的最小实现**（`BSTR`/`VARIANT`/`SAFEARRAY`）。
四者齐备，SOUI 的 ACC 代码就能**一字不改**地在三平台编译运行。

---

## 8. 构建开关、启用与验证

### 8.1 构建开关

`CMakeLists.txt:128`：

```cmake
option(SOUI_ENABLE_ACC "Enable MSAA/IAccessible (provided by swinx)" OFF)
```

默认 **OFF**。这是一条贯穿全局的闸门：

| 位置 | 效果 |
|---|---|
| `SWnd.h` | `GetAccessible()` 声明、`m_pAcc`/`m_pAccProxy` 成员被剔除 |
| `SWndAccessible.h` / `.cpp` | 整个文件内容不编译 |
| `msaa/SAccProxyWindow.*` / `SAccProxyCmnCtrl.*` | 整个文件不编译 |
| `SApp.h/.cpp` | `CreateAccProxy` / `CreateAccessible` 不编译 |
| `shostwnd.cpp:OnGetObject` | 走 `SetMsgHandled(FALSE); return 0;`——对 `WM_GETOBJECT` 直接拒绝 |
| `Swnd.cpp` 各处 | `accNotifyEvent` 空实现；`IAccHelper` 回查不编译 |
| `swinx/src/oleacc.cpp` | AT-SPI 初始化被 `#ifdef SOUI_ENABLE_ACC` 排除——**桥完全不启动**（不开 D-Bus、不占 timer） |

> ⚠️ **OHOS 特殊**：`CMakeLists.txt:115` 里 `if(IS_OHOS) set(SOUI_ENABLE_ACC OFF)`，
> OpenHarmony 构建强制关闭。

### 8.2 打开 ACC

```bash
cmake -DSOUI_ENABLE_ACC=ON ...
```

打开后 `build/config/config.h` 会出现 `#define SOUI_ENABLE_ACC`。
Linux 上同时确保 swinx 编译了 `SAtSpi.cpp`（默认 desktop 构建会编译）。

### 8.3 应观察到的行为

以下都是**当前实现的预期行为**，不是缺陷：

| 现象 | 说明 |
|---|---|
| Windows：Inspect.exe / Narrator 能看到完整控件树 | `SHostWnd::OnGetObject` 对 `OBJID_CLIENT` 返回控件树根的 `IAccessible`；对其他 `lParam`（SOUI 传的 SWND，MSAA 允许的 custom object ID）经 `SWindowMgr::GetWindow` 返回该窗口的 `IAccessible` |
| macOS：VoiceOver 能看到控件树而非单个大视图 | `SwinxNsAccInit()` 注册事件钩子；`SNsAccessibility.mm:552` 做 `WM_GETOBJECT` 探测后建壳 |
| Linux：Orca / accerciser 能看到控件树 | 需 `SOUI_ENABLE_ACC=ON`、在桌面会话内启动（`DBUS_SESSION_BUS_ADDRESS` 有效）、且 `toolkit-accessibility` 为 true |
| 列表只暴露可见项 | **设计意图**——MVC 只暴露当前可见的 `SItemPanel`（`GetVisibleAccChild*`），以常数成本支持海量数据 |
| tooltip / 隐藏窗口不出现在 AT 树 | **设计意图**——隐藏窗口不暴露，符合 AT-SPI 语义 |
| 无专门 proxy 的控件只有通用属性 | 走 `SAccProxyWindow` 默认（角色 `ROLE_SYSTEM_WINDOW`、`GetAccSubItemCount()` = 0）。需要更精确语义的控件要配专门 proxy（见 `SApp.cpp` 的 `CreateAccProxy` 分派表） |
| 非 Windows 平台事件归因到窗口级 | **当前实现未使用 `idObject`**，只按 `hwnd` 归因；SWND 本身是可经 `AccessibleObjectFromEvent` 解析的（见第 9 章第 1 条） |

### 8.4 环境检查命令（Linux）

```bash
# 查 a11y bus 地址
dbus-send --session --print-reply --dest=org.a11y.Bus /org/a11y/bus org.a11y.Bus.GetAddress

# 确认 at-spi 进程
ps aux | grep at-spi

# 用 accerciser 或 pyatspi 遍历树
python3 -c "import pyatspi; ..."
```

---

## 9. 已知限制与注意事项


1. **`ROLE_SYSTEM_CALENDAR` 在 swinx 的 `oleacc.h` 里未定义，且与本地
   `ROLE_SYSTEM_RADIOBUTTON`(0x2D) 取值冲突。** 因此 `SAccProxyCalendar` 使用
   `ROLE_SYSTEM_CLIENT`。若要恢复为 `CALENDAR`，需在 `swinx/include/oleacc.h` 里
   按 Windows 真实取值补齐角色常量表并解决冲突。

2. **C API（`IAccProxy_*` / `IAccHelper_*` 宏）只在 Windows 侧提供。**
   `SOUI/include/interface/capi/saccproxy-capi.h` 与 `sacchelper-capi.h` 整文件包在
   `#ifdef _WIN32` 内。非 Windows 平台若要给 C 调用者用，需要额外补齐（C 展开的
   调用宏本身与平台无关，纯粹是当前未导出）。

3. **`SAccProxyWindow::get_accDefaultAction` 固定返回 `L"cmd"`。**
   这意味着所有未特化控件的默认动作名都是 "cmd"——语义较弱，但至少让 AT 知道
   "这个元素可执行默认动作"。需要精确动作名的控件应覆写。

4. **`get_accDescription` / `get_accHelp` / `get_accHelpTopic` 默认 `E_NOTIMPL`。**
   这是有意的：MSAA 的 help 机制在实践中很少被 AT 使用，实现它会带来额外的字符串
   管理负担。

5. **`SAccessible` 的 `IDispatch` 四方法（`GetTypeInfoCount`/`GetTypeInfo`/
   `GetIDsOfNames`/`Invoke`）全部 `E_NOTIMPL`。** MSAA 客户端只用 `IDispatch` 做
   对象身份传递，不做名称调用，所以安全。

6. **MVC 视图只暴露可见项。** `GetVisibleAccChildCount()` 返回
   `m_lstItems.GetCount()`（当前渲染的 `SItemPanel` 数），不是数据总条数。
   这是性能与语义的折衷——AT 通过滚动触发 `EVENT_OBJECT_REORDER`
   （`SViewBase.cpp:83/95`）重建子级。

7. **`swinx` 的公共 API 面严格等于 Windows SDK。** 平台桥需要的额外助手
   （`SwinxAccResolvePath`、`SwinxDispatchPendingWinEvents`）放在内部头
   `swinx/src/SwinxAccGlue.h`，**不导出**。新增桥功能时不要把扩展塞进
   `swinx/include/oleacc.h`。

---

## 10. 关键代码索引

### SOUI 侧（ACC 门面与隔离层）

| 功能 | 文件 : 位置 |
|---|---|
| WM_GETOBJECT 汇合点 | `SOUI/src/core/shostwnd.cpp:1775` `SHostWnd::OnGetObject` |
| MSAA 门面类定义 | `SOUI/include/core/SWndAccessible.h:19` `class SAccessible` |
| MSAA 语义实现（607 行） | `SOUI/src/core/SwndAccessible.cpp` |
| └ 翻译助手 `GetAccTarget` | 同上，见 `GetAccTarget()` |
| └ `get_accChild` 的 `S_FALSE` 约定 | 同上，`get_accChild()` |
| └ `accHitTest` 倒序 z 序 | 同上，`accHitTest()` |
| └ `accNavigate` 校验范围 | 同上，`accNavigate()` |
| `IAccProxy` 接口定义 | `SOUI/include/interface/saccproxy-i.h` |
| `IAccHelper`（反向 owner 查询） | `SOUI/include/interface/sacchelper-i.h` |
| `SAccessible` 的 IID 支持表 | `SOUI/include/core/SWndAccessible.h:309-312` |
| proxy 工厂（集中分派，160 行） | `SOUI/src/SApp.cpp:380` `CreateAccProxy` |
| `CreateAccessible` | `SOUI/src/SApp.cpp:543` |
| 工厂声明 | `SOUI/include/SApp.h:776/783` |
| `SWindow` 的三个接口点 | `SOUI/include/core/SWnd.h:1253/1259/1265` |
| `SWindow` ACC 成员 | `SOUI/include/core/SWnd.h:2866-2867` |
| `GetAccessible` / `GetAccProxy` / `accNotifyEvent` | `SOUI/src/core/Swnd.cpp:4152 / 4161 / 4173` |
| `IAccHelper` 回查（销毁断开） | `SOUI/src/core/Swnd.cpp:2418`、`658` |
| 窗口级默认 proxy | `SOUI/include/msaa/SAccProxyWindow.h` / `SOUI/src/msaa/SAccProxyWindow.cpp` |
| 40+ 控件 proxy | `SOUI/include/msaa/SAccProxyCmnCtrl.h`（1048 行）/ `SOUI/src/msaa/SAccProxyCmnCtrl.cpp`（774 行） |
| MVC 可见子级数据源 | `SOUI/include/control/SViewBase.h:192-208` / `SOUI/src/control/SViewBase.cpp:115 / 130` |
| `DEFINE_SGUID` 机制 | `SOUI/include/SGuidDef.h` |
| `INITGUID` 引导头 | `SOUI/include/SInitGuid.h` |
| IID 集中定义 | `SOUI/src/core/SGuids.cpp:5` `DEFINE_SGUID(IID_IAccHelper, ...)` |
| `SUnkImpl` / `IUNKNOWN_*`（SOUI 版） | `SOUI/include/helper/SUnkImpl.h` |
| C API 宏（仅 Windows） | `SOUI/include/interface/capi/saccproxy-capi.h`、`sacchelper-capi.h` |
| 构建开关 | `CMakeLists.txt:128`（`option(SOUI_ENABLE_ACC ...)`）、`:115`（OHOS 强制 OFF） |

### swinx 侧（MSAA 运行时 + COM 兼容层 + 平台桥）

| 功能 | 文件 : 位置 |
|---|---|
| 进程内 MSAA 运行时（733 行） | `swinx/src/oleacc.cpp` |
| └ `LresultFromObject` | 同上 `:267` |
| └ `ObjectFromLresult` | 同上 `:292` |
| └ `AccessibleObjectFromWindow`（含 OBJID 符号扩展） | 同上 `:306` |
| └ `AccessibleObjectFromPoint` | 同上 `:329` |
| └ `AccessibleChildren` | 同上（`:380` 附近） |
| └ `NotifyWinEvent` | 同上 `:423` |
| └ `SnapshotWinEventMatches`（事件产生时快照） | 同上 `:202` |
| └ `SwinxDispatchPendingWinEvents` | 同上 `:241` |
| └ `SAccHandleTable`（弱登记句柄表） | 同上 `:90` |
| └ `AccGetRootAccessible` | 同上 `:665` |
| └ `SwinxAccResolvePath`（桥共享下钻内核） | 同上 `:674` |
| └ `GetRoleTextW/A`、`GetStateTextW/A` | 同上 `:453` 起 |
| 桥共享内部头 | `swinx/src/SwinxAccGlue.h` |
| COM 兼容：GUID 与 `__suidof` | `swinx/include/guiddef.h`（`DECLARE_CLASS_SIID` :36，`__suidof/__uuidof` :46-47，`DEFINE_GUID`+`SELECT_ANY` :62-73） |
| COM 兼容：接口声明宏 | `swinx/include/basetyps.h`（196 行） |
| COM 兼容：引用计数 + QI 宏 | `swinx/src/SUnkImpl.h`（83 行） |
| `BSTR` 实现 | `swinx/src/objbase.cpp:49/70/89`（`SysAllocStringByteLen/Len/String`） |
| `VARIANT` 实现 | `swinx/src/variant.cpp`（`VariantInit` :18） |
| `SAFEARRAY` 实现 | `swinx/src/safearray.cpp` |
| MSAA 常量与接口声明 | `swinx/include/oleacc.h`、`swinx/include/oaidl.h`、`swinx/include/unknwn.h` |
| macOS 桥 | `swinx/src/platform/cocoa/SNsAccessibility.h` / `.mm` |
| └ 身份壳 `SwinxAccElement` | `SNsAccessibility.h`（@interface） |
| └ 按需解析（`ResolvePath`） | `SNsAccessibility.mm:181/195` |
| └ 命中测试链 | `SNsAccessibility.mm`（`SwinxAccHitTestChain`，:617） |
| └ 事件钩子 | `SNsAccessibility.mm`（`SwinxAccEventHook`，:790） |
| Linux 桥 | `swinx/src/platform/linux/SAtSpi.h` / `.cpp` |
| └ 桥设计说明 | `SAtSpi.h` 文件头 |
| └ `ResolveNode`（用 `SwinxAccResolvePath`） | `SAtSpi.cpp:359` |
| └ `EnsureInit` / `SwinxAtSpiInit` | `SAtSpi.cpp:974 / 2704` |
| └ 惰性兜底初始化 | `swinx/src/oleacc.cpp:54`（弱符号）、`:277`（`LresultFromObject` 内）、`:427`（`NotifyWinEvent` 内） |
| └ 主初始化点（须在 `SConnMgr` 构造完成后调用） | `swinx/src/platform/linux/SConnection.cpp:80-96` |

---

## 附：一页速览

```
SOUI ACC = 三层

【1】门面层：SAccessible（唯一认识 MSAA 的类）
      IAccHelper + SUnkImpl<IAccessible>
      ├ 所有 MSAA 语义（child id / CHILDID_SELF / VT_DISPATCH vs VT_I4 / S_FALSE）
      ├ 无状态：只持一个 SWindow*，属性现查现用
      └ 由 SWindow::GetAccessible() 惰性创建 → 挂在 m_pAcc

【2】隔离层：IAccProxy（完全不认识 MSAA）
      继承 IObjRef，不继承 IAccessible/IDispatch
      ├ Child 组：GetAccChildCount / GetAccChild(IWindow*) / GetAccSelIndex
      ├ SubItem 组：GetAccSubItem* / SetAccSubItemSel / HitTestAccSubItem
      └ 由 SApplication::CreateAccProxy() 集中工厂按类名分派（40+ proxy）

【3】控件层：SWindow + 具体控件（零 ACC 代码）
      ├ GetAccessible() / GetAccProxy() / accNotifyEvent()  ← 仅 3 个接口点
      ├ m_pAcc / m_pAccProxy                                  ← 仅 2 个成员
      └ 12 处 accNotifyEvent 事件上报，无一行 MSAA 类型

平台接入 = 一套 MSAA 面 + 三种下层

      SOUI（三平台上同一份代码）
             │ 标准 oleacc API
      ┌──────┼──────────────────┬──────────────────┐
      ▼      ▼                  ▼                  ▼
   Windows  swinx oleacc.cpp   macOS 桥           Linux 桥
   原生     进程内运行时         NSAccessibility    AT-SPI2/D-Bus
   oleacc   LresultFromObject   SwinxAccElement    AccNode + D-Bus 路径
            SwinxAccResolvePath （身份壳，不持 COM）（确定性路径，不持 COM）
            NotifyWinEvent
            SetWinEventHook

非 MSVC 的 COM = 4 件小事

  ① basetyps.h          接口声明宏（C/C++ 两套展开）
  ② guiddef.h           __suidof(X) = X::GetIID()   ← 替代 __uuidof
                        DECLARE_CLASS_SIID / IsEqualIID(memcmp) / 弱符号
  ③ SUnkImpl<T>         80 行引用计数
     IUNKNOWN_BEGIN/ADD_IID/END   QueryInterface 声明式定义
  ④ objbase.cpp / variant.cpp / safearray.cpp
                        BSTR / VARIANT / SAFEARRAY 最小实现
```
