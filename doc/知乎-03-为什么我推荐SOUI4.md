# 跨平台 GUI 选型：为什么我推荐 SOUI5

*（一套 C++ 同时跑在 Windows / Linux / macOS / iOS / Android / OpenHarmony 六端，而业务源码里 `__ANDROID__` 出现 0 次。这篇文章不讲愿景，只讲能逐行核对的证据。）*

---

## 一、先问一个问题：你的业务代码里有多少 `#ifdef`

选跨平台 GUI 框架，本质上是在选"**用哪种方式消化平台差异**"。市面上的方案大致三条路：

| 路线 | 代表 | 跨平台怎么实现 | 你要付出的主要代价 |
|---|---|---|---|
| **换语言** | Flutter、Electron、React Native | 用另一门语言写 UI，再各自渲染 | 现有 C++ 资产与团队技能栈归零；还要背一个运行时（Dart VM / Chromium+Node） |
| **自建抽象 API** | Qt、wxWidgets | 定义一套框架自己的 API，各平台分别实现 | 还是 C++，但业务代码要面向框架 API 重写；不同平台的行为仍有差异 |
| **重演平台本身** | **SOUI5** | 不去抽象 Windows，而是**让 Windows 的 API 面在别的平台上成立** | 需要一个兼容层（SOUI 的兼容层叫 `swinx`，162,819 行） |

第三条路最"笨"，也最少人走——因为要重演的是一整个 API 面。但它的收益非常直接：

> **你已经会的 Win32/WTL 编程模型、以及你围绕 SOUI 写过的 Windows C++ 代码，在这里是资产，不是负债。**

但这句话有一个必须提前说清的边界：**"重演 Windows"目前重演的是 SOUI 应用所需要的那部分 Win32 面。** 如果你手上是一个用原生控件（`Edit`/`ComboBox`/公共控件）、窗口自己的菜单栏、资源对话框搭起来的**既有 Win32 程序**，直接搬过来还跑不起来——差在哪、差多少，2.3 节逐条列给你看。**这是本方案当前最大的未完成项**，也是它出现在第六节"不要选"清单里的原因。

SOUI5 的内核直接 `#include <windows.h>`，然后编译到 Linux / macOS / iOS / Android / OHOS 上。架构上就四层：

| 层 | 内容 |
|---|---|
| 业务代码 | C++ + XML，六端共享，**零平台宏** |
| SOUI5 内核 | 窗口树 / 消息路由 / 事件系统 / 布局 / 动画 / 控件 / XML 对象工厂 |
| swinx 兼容层 | **44 个**与 Windows SDK 同名同结构的头文件 + 162,819 行实现（Windows 上**不编译**，直接用系统 SDK）；**当前覆盖的是"SOUI 应用所需"的那部分 API 面**，见 2.2 / 2.3 |
| 底层 | Windows 系统 SDK · Linux XCB+Cairo · macOS Cocoa+CoreGraphics · iOS UIKit+CoreGraphics · Android/OHOS JNI·N-API + Cairo |

下面按"它做到了什么" → "代价是什么" → "你怎么验" 的顺序展开。**优点和短板我都会写清楚**，因为技术选型最怕的不是缺点，而是不知道缺点。

---

## 二、这层兼容层做到了什么程度

### 2.1 六端，而且业务代码真的零平台宏

先说平台覆盖（`README.md` 的官方声明，也已核对编译归属）：

| 平台 | 窗口系统 / 启动方式 | 2D 后端 |
|---|---|---|
| Windows | 系统 Win32 | GDI / Direct2D |
| Linux | xcb + `poll()` 事件循环 | Cairo |
| macOS | `NSApplication` | Core Graphics |
| iOS | `UIApplicationMain`（`swinx_ios_entry` 托管） | Core Graphics |
| Android | JVM 启动 + `PlatformAPI` 桥 | Cairo |
| HarmonyOS (OHOS) | ArkTS 运行时 + `PlatformAPI` 桥 | Cairo |

"零平台宏"这句话，我用仓库里那个真实项目 `games/cnchess`（中国象棋，能联机对弈、带 AI 机器人、带主题热下载）实测过：

| 宏 | 在业务源码里的出现次数 | 说明 |
|---|---|---|
| `__ANDROID__` | **0 次** | 只在 `README.md` 里被提到 1 处 |
| `__OHOS__` | **0 次** | 只在 ohos 目录的 CMakeLists / readme 里 |
| `_WIN32` | 入口与资源层少量（`main.cc`、`stdafx.h`、`cnchess.rc`…） | 平台分流集中在薄入口 |
| `__MOBILE__` | `ChessGame.cpp` 4 处、`MainDlg.cpp` 4 处等 | **仅**用于区分"移动触屏 vs 桌面弹窗"（移动端用 `SModalView` 替代对话框） |

而核心绘制与逻辑文件 `ChessBoard.cpp`、`ChessPiece.cpp`、`CnchessSkin.cpp`、`myprofile.cpp`、`algorithm/*` **完全不含平台宏**。

平台差异被压在三个"薄入口"里：桌面/iOS 由 swinx 提供事件泵并归一为 `WinMain`；Android 由 `soui-android-lib`（JNI）把 `SApplication` 托管进 `GameActivity`；OHOS 由 `soui-ohos-lib`（N-API）托管进 `EntryAbility`。**同一份业务代码，接三个宿主。**

> 这在跨平台框架里是罕见的彻底度——Flutter 要用 Dart 重写 UI 层，Qt 要适配 QWidget/QML，wxWidgets 的 API 本身随平台而变。

### 2.2 兼容面是"严格等于"，而且边界是公开的

`swinx/include/` 下的头文件（`windows.h`、`gdi.h`、`unknwn.h`、`oleacc.h`、`textserv.h`…）**与 Windows SDK 同名同结构**，共 44 个。所以内核与业务代码的感受是"我在写 Windows 程序"，而不是"我在用某个框架的移植版"。

具体的 API 面有多大：

| 指标 | 数值 |
|---|---|
| 函数声明总数 | 1,042 |
| **有定义的** | **913**（完整实现 884 / 部分实现 6 / 空实现或语义桩 23） |
| 仅保留声明（链接期失败） | 129（其中 6 项为扫描器误收录的宏，**实际未提供 123 项**） |
| 分层分布 | KERNEL32 等价 335 · USER32 等价 242 · GDI32 等价 178 · COM/OLE 61 · COMCTL·SHELL 56 · 多媒体与杂项 41 |

**这里的边界要分两层看，而两层的量级差得很远——很多讨论只盯着第一层，容易得出错误的结论。**

**第一层（声明与定义层面，小账）** —— 这一层 swinx 收得已经很干净：

- **129 项只有声明没有定义**：链接期就报错，不会静默返回一个错误结果给你；
- **23 项空实现里**：14 项是刻意的单值语义（`GetACP()` 恒返回 `CP_UTF8`、`SetStretchBltMode` 在目标后端没有对应概念），3 项是有明确边界的实现范围限制（`EnumDisplayDevicesW` 只到显示器级、不到设备级）；
- **扣掉这些，真正"该补没补"的只剩 5 个**：`ScrollWindowEx`、`SendNotifyMessageA`、`SendNotifyMessageW`、`SetWindowExtEx`、`SetMenuItemBitmaps`——占 1,042 项声明的 0.5%，影响面局限在"依赖滚动位块搬运、视口换算、跨线程投递"的少数第三方库。另有 `IsWindowUnicode` 恒返回 `FALSE`，经核对是"swinx 统一用 UTF-8"的刻意语义，不是缺陷。

**这几项不是迁移的主要障碍，我也不想把它写成风险标题。** SOUI 自绘的场景根本碰不到它们（它自己重绘、用自己的消息钩子体系）。

**第二层（接口面本身，大账）** —— 才是决定"能不能迁移"的那一层：**面向原生控件、窗口菜单栏、对话框的那半张 Win32 面孔，目前还没有建。** 它不体现在"缺口 5 项"这种口径里——因为它不是"某几个函数没实现"，而是"整类 API 还没提供"。下一节逐条列。

> 顺带说一句我认为比覆盖率更值钱的东西：在第一层上，swinx 的态度是把底牌摊开——**它不是"看起来兼容"，而是"兼容到哪一条线，写在明面上"**，链接期报错而不是静默给错结果。这个态度值得肯定。

### 2.3 如果目标是"迁移现有 Windows 程序"：还差什么

这一节是全文最该细看的一节，因为它直接决定"你的存量代码能不能搬"。

先说结论：**swinx 让"用 SOUI 写的新应用跨端"这件事已经成立（cnchess 就是活证据），但让"既有 Win32 程序零成本搬过来"还没有成立。** 差的不在那 5 个 API，而在下面几类**整块缺失**的接口面——每一条都回到源码核过。

| 缺什么 | 当前状态 | 迁移时会怎样 |
|---|---|---|
| **窗口自己的菜单栏** | `SetMenu` / `GetMenu` / `DrawMenuBar` **在 swinx 里连声明都没有**；`CreateWindowEx(..., hMenu, ...)` 的菜单句柄被当作控件 ID 存进 `GWLP_ID`（`src/wnd.cpp:358`），不会挂出菜单栏。反差是：**弹出菜单机制本身是完整的**（`src/cmnctl32/menu.cpp`，1,992 行，`CreateMenu`/`AppendMenu`/`InsertMenu`/`TrackPopupMenu`/`CheckMenuItem` 一应俱全） | 调 `SetMenu` 的程序**编译期就过不去**。SOUI 应用不受影响——它用自己自绘的 `SMenuBar`/`SMenu` 控件（XML 描述） |
| **原生基础控件** | 只注册了 **3 个**窗口类：`Static`、`Button`（`src/cmnctl32/cmnctl32.cpp`，124 行的最简实现，本质是给 `MessageBox` 当零件）、`ListBox`（`src/cmnctl32/listbox.cpp`，**3,023 行，从 Wine 完整移植**）。**`Edit` / `ComboBox` / `ScrollBar` 都没有** | ⚠️ 这里比"少几个函数"更麻烦：类名宏（`WC_EDITA` 等）在 `commctrl.h` 里是有的，所以 `CreateWindowEx(0, "EDIT", …)` **能编译、能链接**，运行时却**找不到窗口类 → 返回 `NULL`**。**这种失败不会在编译期提醒你** |
| **公共控件（comctl32）** | `commctrl.h` 里常量、结构体、`WC_*` 类名都在，但 `InitCommonControls` / `InitCommonControlsEx` **只有声明、没有定义**（`commctrl.h:106`、`:139`）；`SysListView32` / `SysTreeView32` / `SysTabControl32` / `SysHeader32` / `SysIPAddress32` 等**全无实现** | 调 `InitCommonControls()` **直接链接失败**（这种反而"友善"）；建 `SysListView32` 这类窗口则是运行时返回 `NULL` |
| **资源对话框** | **没有任何对话框实例化 API**：`DialogBoxParam` / `DialogBoxIndirectParam` / `CreateDialogIndirectParam` / `EndDialog` 全部缺失。`.rc` 里的 `RT_MENU`(4) / `RT_DIALOG`(5) 只是类型宏，**没有对应运行时** | "用资源编辑器画对话框和菜单、`DialogBox(hInst, MAKEINTRESOURCE(IDD_X), …)`"这条最经典的老路走不通；`SOUI/include/helper/souidlgs.h` 那套 WTL 派生的资源对话框 helper 在 swinx 平台上链接不过 |
| **`MessageBox`** | **有**：`src/cmnctl32/msgbox.cpp`（250 行）自绘了一个模态窗口，用 `Static` + `Button` 拼出来 | 能用，但按钮文案是**硬编码简体中文**（确定/取消/是/否/中止/重试/忽略/继续），图标用内置位图；**不跟随系统语言与主题**，也没有 `MessageBoxIndirect`。别指望外观一致 |
| **文件 / 颜色 / 字体对话框** | **有**：`GetOpenFileName(A/W)`、`GetSaveFileName(A/W)`、`ChooseColor(A/W)`（`src/cmmmdlg.cpp`）；`ChooseFont` 三个桌面端都有实现（Linux `platform/linux/dlghelper.cpp:1843`、macOS `platform/cocoa/dlghelper.mm:712`；iOS 版明确返回 `FALSE`）。另有 swinx 私有的 `PickFolder(A/W)` 选目录扩展 | 这一块比预期完整，反而不是问题 |

**怎么读这张表：**

- 前三类是**同一件事**：swinx 已经建起了"窗口 / 消息 / 类注册 / DC / GDI / 资源 / COM"这套**地基**，但没有在地基上建"Win32 原生控件库"这一层。地基比控件库难得多；而控件库恰恰是最能从 Wine 现成搬运的部分——`ListBox` 那 3,023 行就是活样板。
- 这些缺口的**失败方式**要单独留心：既有链接期报错（`InitCommonControls`），也有运行时静默返回 `NULL`（建 `EDIT` / `SysListView32` 窗口）。后者最容易漏——**搬家时如果不检查 `CreateWindowEx` 的返回值，你会看到"程序起来了，但界面是空的"。**
- 有一条现成的绕行通道：移动端的 `SRealWnd` + `IRealWndHandler` 能把窗口类名映射到**宿主原生控件**（Android demo 里的 `<realwnd wndclass="edit"/>` 走的就是 `NativeEditView`，见 `SouiPlatformBridge.java`）。也就是说**建不出来的控件可以借宿主的原生控件顶上**——只是这属于"宿主 App 提供的能力"，不是 swinx 自己提供的 Win32 控件。

**本节结论**：swinx 的目标——"让基于它的 Windows 程序零成本迁移到多平台"——**目前还没有达到**。它已经达到的是另一个、也确实好用的目标：**让用 SOUI 写的应用全平台原生运行**。两者只差"原生控件库 + 窗口菜单栏 + 资源对话框"这三块，但确实还差着。

**而这件事的能力是具备的**：`ListBox` 已经证明"从 Wine 整体搬一个控件"这条路走得通，消息 / DC / GDI / 类注册这套地基也已经稳。所以只要**项目需要、时间允许**，"把另外半张面孔补齐"是个可达的目标——**它是工作量，不是架构上的不确定性。**

### 2.4 三个"它居然也搬过来了"的案例

兼容层好不好，看它啃过什么硬骨头最直接：

| 案例 | 难在哪 | 怎么解决 |
|---|---|---|
| **RichEdit 富文本编辑** | 这是 Windows 上最深的坑之一：依赖 Text Services（`ITextHost` / `ITextServices`）+ OLE + GDI+ 一整套私有契约 | Windows 上走系统 `msftedit.dll`（RICHEDIT50W）；非 Windows 上由 swinx 提供 `richedit.h` / `textserv.h`，底层引擎是移植的 Microsoft RichEdit 4.1 源码（`third-part/richedit41`，**499 个受控文件**）。**同一份 `SRichEdit.cpp` 在六端编译** |
| **Scintilla 语法着色编辑器** | Scintilla 官方本来就提供 Win32 与 GTK 两套后端 | SOUI **只取 Win32 那套**：`Scintilla_RegisterClasses` 注册窗口类、`CreateWindowEx(..., "Scintilla", ...)` 建窗、`SendMessage(m_hWnd, SCI_*, ...)` 发消息——全靠 swinx 把 `CreateWindowEx` / `SendMessage` / GDI 整体模拟出来 |
| **`.rc` 资源文件跨桌面端编译** | 别的框架每端一套资源管线（Qt `.qrc`、Flutter pubspec assets、Electron asar） | 一份 Windows `.rc`，在 Windows / Linux / macOS **三端同源编译**进 PE / ELF / Mach-O。流水线为 `windres` → COFF → `ld -r -b binary`（或 `.incbin`）→ `objcopy --redefine-syms` 统一符号 → `-rdynamic` + `--version-script` 防符号被 GC 抹掉 |

第二和第三个案例我认为特别有说明力：

- Scintilla 那个例子证明了一个**可推广的结论**——**只要某个组件存在 Windows 实现，swinx 就足以让它跨端，不必为每个平台另写后端**。这个结论有个前提值得点明：**Scintilla 是自绘组件**，它需要的只是 `CreateWindowEx` / `SendMessage` / GDI 这层公共设施。它**不**意味着"系统自带的那批控件会自动可用"——那部分见 2.3。
- `.rc` 那个例子则是把"用熟悉的资源编辑器写一份 .rc"这个 Windows 开发习惯直接带到了 Linux/macOS。配合 `ENABLE_BUILD_RESOURCE` 把资源内嵌进可执行文件，还能做到**单文件分发**。同样要补一句：这里编进资源的是图标、位图、清单这类**数据型资源**；`.rc` 里的 `DIALOG` / `MENU` **模板**虽然也能被编进资源段，但目前没有运行时可以把它们实例化出来（见 2.3）。

---

## 三、框架自身：几个会直接影响你日常的点

跨平台能力之外，框架本身的设计决定你**每天写代码顺不顺手**。挑四个最相关的说。

### 3.1 体积：核心约 2 MB，组件按需

- 编译出的核心动态库**约 2 MB**（官方口径）；
- `components/` 下 **16 个组件各自是独立 target**，没用到的（skia / wic / zip / 7zip / lua / network / httpclient…）**根本不链进来**；
- 资源可内嵌，静态模式下产物就**一个可执行文件**。

放一起对比（同类框架横向参考）：

| 方案 | 运行时依赖 | 核心体积量级 |
|---|---|---|
| **SOUI5** | **无 VM、无 JS 引擎** | **~2 MB** |
| Qt 6 | 无 VM，但较重 | 数十 MB |
| Flutter | Dart VM + 自绘引擎 | 10 MB – 数十 MB |
| Electron | Chromium + Node | 100 MB+ |
| wxWidgets | 借用系统原生控件 | 小 |

> 需要限定语境：说"最小"是**相对 Qt / Flutter / Electron 这类"提供原生渲染 + 完整控件 + XML 布局的全功能跨平台 GUI 框架"**成立；跟即时模式、游戏 GUI 库不是一个赛道。另外实际体积取决于配置——静态全链接 + 内嵌全部资源 + Skia 静态捆绑时，单体 exe 会变大，"2 MB"指的是动态链接核心。

### 3.2 内存：把 C++ 最大的坑收进框架

C++ 写 GUI 最常被诟病的就是"泄漏"和"悬空指针"。SOUI 的解法是一套**类 COM 引用计数**：

```
IObjRef                      AddRef / Release / OnFinalRelease（刻意不带 QueryInterface）
  └─ TObjRefImpl<T>          构造 m_cRef=1；Interlocked 增减；归零 → delete this
       └─ SObjectImpl<T>
            └─ SWindow : TObjRefImpl<SObjectImpl<IWindow>>
SAutoRefPtr<T>               构造/拷贝 AddRef，析构/赋值 Release，另有 Attach / Detach 精确控制
```

两个我认为做对了的决定：

- **资源对象一律引用计数**：皮肤、字体池、位图、渐变、布局都用 `SAutoRefPtr` 持有；连事件参数与事件槽也走计数——**订阅即增引用，回调期间对象不会悬空**。
- **控件树反而用裸指针**：`m_pFirstChild` / `m_pParent` / `m_pNextSibling` 都是裸指针，由父窗口以 owner 语义统一析构。这是**刻意**的——DirectUI 树如果父子互相引用计数，就永远不会释放。

也就是在"自动回收资源对象"和"避免控件树循环引用"之间取了正确的平衡，把 C++ 最常见的两类泄漏都堵上了。

> 但要诚实说一句：**引用计数不防跨线程生命周期误用，也不防你在外部缓存裸指针**。它降低风险，不消除风险。框架给了称手的工具，纪律还得自己守。

### 3.3 开发效率：XML 描述 UI，皮肤还能热换

- UI 用 XML 描述（`pugixml` 封装为 `SXml`），控件由对象工厂按节点名映射到 C++ 类——**换布局、换皮肤不改 C++ 代码**；
- 支持 `<include>`、模板、命名空间；
- `SOUI/include/control/` 下 **38 个控件头文件**（含 1 个聚合头 `SouiCtrls.h`），含 MVC 虚拟列表 `SListView` / `SMCListView` / `STreeView` / `STileView` / `SComboView`（大数据集只渲染可见项）；
- 自带可视化设计器 `uieditor`（在 `demos/` 里，构建 demo 即得）；
- 除了 C++，还有 **Lua / JS / Python 三种脚本绑定**（SOUI5JS、SOUI5PY 支持 Windows / Linux / macOS），团队里不是所有人都是 C++ 手时有用。

**皮肤热换**这块做得比我预期的彻底：cnchess 的主题不是打包进安装目录的，而是**运行时从服务器下载**的——WebSocket 分块传输、带 MD5 命中判断（服务器发现本地 MD5 一致就直接回"总长 0"，一个字节都不传）、增量 MD5 校验（逐块 `MD5_Update`，不合并成大缓冲，所以**峰值内存只是分块之和**）、校验通过才解压生效。整套机制在 `client/ThemeDownloadManager.{h,cpp}`，可以直接当范例读。

### 3.4 渲染：SVG 是"渲染原语"，不是"图片格式"

多数框架把 SVG 当成"另一种要解码的图片"——先栅格化成某个固定尺寸的位图，再拉伸。SOUI 是把它当**矢量绘制原语**：

- `IRenderTarget::DrawSVG` 是渲染目标的绘制接口之一，**四个后端全部实现**（Skia / Direct2D / GDI-Windows / GDI-Linux）；Skia 后端在**绘制时刻**把 nsvg 路径转成 SkPath，整条链路是"矢量 → 原生路径 → GPU/光栅"；
- 皮肤走 `SSkinImgList::SetSvg`（XML 皮肤类型 `"svg"`），`DrawSVG9Patch` 让 SVG 像 9-Patch 一样按任意尺寸拉伸**而不糊边**；
- 因为绘制发生在**目标 DPI 的像素网格上**，Retina / 4K / 移动端高分屏天然清晰，**不需要准备 @1x/@2x/@3x 多套位图**；
- 外加一个在 2026 年才显得重要的好处：SVG 是文本化、可程序化生成的格式，**对 AI 设计/生成工作流天然友好**——LLM 或设计工具可以直接产出、改写它的路径数据。

---

## 四、性能：C++ 的价值在"密集计算"上才真正拉开

> 这一节我特意写具体一点。因为"**C++ 比 Java 快**"是句正确的废话——关键在**什么负载上、快在哪、以及算了之后怎么不卡界面**。

### 4.1 差距在哪

- **交互式 UI**：差异是**毫秒级抖动**。Android 原生跑在 ART 上（Java/Kotlin），OHOS 原生跑在 ArkCompiler 上（ArkTS），都有 GC；SOUI 的 UI 树遍历、事件分发、计时器与动画调度全部是 AOT 编译的原生 C++，**没有托管堆的 GC 抖动**，帧时序更可控。
- **CPU 密集型任务**（棋盘搜索、路径规划、物理碰撞、图像处理、编解码与加解密、批量数据计算）：差异是**吞吐与可预测性**。托管语言在这里要同时交三笔税——① 分配频率高导致 GC 更频繁；② 并发 GC 的停顿在"多线程 + 大堆"下不可控；③ 内存布局被运行时接管（对象头、引用间接、装箱）。

C++ 没有这三重税：**布局自己定、线程自己分、时间预算自己算。**

| 维度 | SOUI5（纯 C++） | Android 原生（Java/Kotlin） | OHOS 原生（ArkTS） |
|---|---|---|---|
| 执行模型 | 原生 AOT，无 VM | ART（托管） | ArkCompiler（托管） |
| 内存管理 | 类 COM 引用计数（**确定性回收**） | GC（**有停顿**） | GC（**有停顿**） |
| 运行时体积 | 无 VM，轻 | 含 ART 运行时 | 含 ArkCompiler 运行时 |
| 冷启动 | 快（无 VM 预热） | 受 ART 初始化影响 | 受 ArkCompiler 初始化影响 |
| **密集计算** | 紧凑布局 + 自有线程池，**多核可跑满、时间可预算** | 分配频繁、并发 GC 停顿不可控，须刻意绕开 GC | 同左 |
| **重计算与 UI 协作** | `ITaskLoop` 工作线程 + `PostTask` 回 UI 线程，**一次调用、无锁** | 机制等价，但结果对象仍活在托管堆上 | 机制等价，同上 |
| 内存安全 | 引用计数大幅缓解，**仍需谨慎** | 托管天然防越界/悬空 | 托管天然防越界/悬空 |
| 移动 SDK 生态 | **需自行桥接（门槛高）** | 官方 SDK 完备 | 官方 SDK 完备 |

### 4.2 一个可以逐行核对的样本：cnchess 的机器人 AI

`games/cnchess/algorithm/ChsAIEngine.cpp` 就是纯 CPU 密集负载（PVS + 置换表 + 杀手/历史启发 + 静态搜索）。它的三个关键做法，恰好是托管语言最难对位的地方：

**① 内存布局自己定**

90 格棋盘压成 `int8_t` 一维数组（正红负黑，类型 1..7），Zobrist 键随 `Make/Unmake` **增量异或**维护——不在每个结点重算哈希。

**② 并行自己分，共享数据无锁**

```cpp
// algorithm/ChessAI.cpp:22 —— 引擎含置换表等有状态资源，按 thread_local 隔离既避免加锁……
static thread_local ChsAI::SearchEngine s_engine(18);   // 2^18 条置换表
```

每个池线程各持一份置换表（`2^18 = 262,144` 条 × 16 B ≈ **4 MB/线程**，池上限 16 线程约 64 MB），**线程之间零同步**。

**③ 时间自己兜底**

```cpp
// algorithm/ChsAIEngine.cpp:799 —— 每 4096 个结点检查一次时间预算
if ((m_nodes & 4095) == 0 && TimeUp(false))
```

到点立即放弃本层（本层结果作废），层间再按 60% 预算软停——保证"**到点必返回，且返回的一定是某个完整层的搜索结果**"。搜索深度 3 / 5 / 7 三档，时间预算 400 / 800 / 1500 ms（`server/config/config.xml`）。

还有一个细节值得单独说：**这个引擎只依赖标准 C++ 头，不依赖 `windows.h`** —— 所以同一份算法在服务端和 Android NDK / OHOS clang 里直接复用。"**算得快**"和"**到处能算**"，在 C++ 下可以是同一个文件。

### 4.3 "算完怎么安全回到界面"：一行 `post`

C++ 只解决"算得快"，不解决"算完之后怎么把结果安全地交给 UI 线程"。SOUI 在这里的抽象成本低到出乎我意料——**网络线程把活交给 UI 线程，全部代码就一行**：

```cpp
// games/cnchess/client/MainDlg.cpp:270
BOOL CMainDlg::OnMessage(DWORD dwType, std::shared_ptr<std::vector<BYTE>> data)
{
    STaskHelper::post(GetMsgLoop(), this, &CMainDlg::_OnMessage, dwType, data);
    return TRUE;
}
```

网络线程那边只做拆帧（8 字节头 = type + size，载荷装进 `shared_ptr<vector<BYTE>>`），**完全不碰 UI 对象**。

底下的两层原语：

| 原语 | 用途 |
|---|---|
| `IMessageLoop::PostTask(IRunnable*)` | 从**任意线程**把任务投到 **UI 线程**执行；入队后 `StartTimer()` 唤醒消息循环 |
| `ITaskLoop::postTask(runnable, waitUntilDone, priority)` | 把任务投到**专属工作线程**（`components/TaskLoop`，每线程一条队列），返回 taskId 供取消 |

`STaskHelper::post(...)` 是这两者的薄封装：把"对象 + 成员函数 + 参数"或 lambda 绑成 `IRunnable`，按第一个参数是 `IMessageLoop*` 还是 `ITaskLoop*` 自动选路。

**而"它为什么安全"这件事，我去核了实现** —— 安全性来自 `clone` 语义，不是"引用调用方的对象"：

- `SMessageLoop::PostTask` 的实现是 `m_runnables.AddTail(runable->clone())`（`SOUI/src/core/SMsgLoop.cpp:257`）；
- `ITaskLoop::postTask` 与 `IWebsocket::postServiceTask` 同样先 `clone`。

所以：**投递方在栈上构造的 functor，返回后立刻析构也不影响执行——你不需要 `new`，也不需要写锁。**

对照一下裸 Win32 的等价写法：要自己 `new` 一个载荷结构、`PostMessage(hwnd, WM_APP+n, wParam, lParam)`、在窗口过程里 `delete`，还得处理"窗口先销毁、消息后到"的悬空指针。在 SOUI 下，这一整类样板与陷阱被 `post` 一行吸收了。

**服务端方向**也一样干净：`RobotAIPool` 用 `getTaskCount()` 挑排队最少的 loop 投递，`RunTask` 只对**棋盘深拷贝快照**计算，结果回主线程落子，并校验 `generation` 丢弃过期结果（悔棋 / 新局场景）——**共享状态始终只有一个线程在写，业务代码里连一把显式锁都不需要。**

> 顺手把边界也写清楚，免得误用：`IMessageLoop::PostTask` 是纯异步、无等待语义；`ITaskLoop::postTask` 的 `waitUntilDone=TRUE` 才同步等待（跨线程用信号量）——**同线程 + `waitUntilDone` 已被实现短路为直接 inline 执行**（`components/TaskLoop/TaskLoop.cpp:69`），所以不会自锁。

---

## 五、和主流方案放一起看

| 维度 | SOUI5 | Qt 6 | Flutter | Electron | wxWidgets |
|---|---|---|---|---|---|
| 语言 | C++ | C++ | Dart | JS/HTML | C++ |
| 编程模型 | Win32/WTL 式 + XML | 自有 Q* 体系 | 声明式 Widget | Web DOM | 原生控件 + 跨平台 API |
| 跨平台代码复用 | **高（零平台宏）** | 高 | 高（但需 Dart） | 高 | 中（API 随平台） |
| 原生观感 | 自绘（Skia/Cairo/GDI） | 自绘 / QPA | 自绘（Impeller） | Web | **真原生控件** |
| 运行时依赖 | 无 VM，轻 | 无 VM，较重 | Dart VM | Chromium + Node | 系统原生 |
| 核心体积 | **~2 MB** | 数十 MB | ~10 MB – 数十 MB | 100 MB+ | 小（借系统） |
| 内存管理 | 类 COM 引用计数 | 父子树 / 智能指针 | GC（Dart） | GC（V8） | 引用计数 / 手动 |
| 许可 | 自有商业（非商用免费） | LGPL / 商业 | BSD（引擎含限制） | MIT | LGPL / 商业 |
| 移动端 | **六端含 OHOS** | 有（OHOS 较弱） | 强（无 OHOS 官方支持） | 借 Capacitor | 弱 |
| 学习曲线（对 Win32 程序员） | **低** | 中 | 高（换语言） | 低（Web） | 中 |

**定位判断**：SOUI5 的甜区是"**熟悉 Win32/WTL、想用一套 C++ 覆盖桌面 + 移动（含鸿蒙），且重视体积与原生渲染**"的团队。它最大的差异点在于：**把 Windows 程序员的知识资产直接复用到全平台，而不是要求你换一门语言或换一套范式。**

---

## 六、什么情况下不要选它

推荐文如果只说好话，就没有参考价值了。下面这几条，我认为你在做选型之前就应该知道：

1. **许可要先看清**：SOUI5 是**自有商业条款——非商用免费，商用收费**（见 `LICENSE-zh.md` / `LICENSE-en.md`）；`swinx` 内含 Wine 派生代码并单独披露（`swinx/license_CN.txt`）。**如果你的项目是商用闭源产品，先把授权谈清楚，再谈技术。** 这是本清单里唯一的硬门槛。
2. **移动端目前没有自动构建与回归**：仓库里 3 个 GitHub Actions 工作流只覆盖 Windows / Linux / macOS 桌面三端；**iOS / Android / OHOS 是零自动构建、零自动测试**，全靠人工。而这三端恰恰是 vendor 依赖最多、平台 API 最不标准的端。若你要长期依赖移动端，最好自己补一层"交叉编译不回归"的 CI。
3. **单作者项目（bus factor = 1）**：知识高度集中在一人身上。对长周期项目，这是真实的维护与交接风险——要么接受，要么投入人力参与共建。
4. **迁移"既有 Windows 程序"目前不是零成本**（见 2.3）：窗口自己的菜单栏（`SetMenu`/`GetMenu`/`DrawMenuBar`）未提供；原生控件只有 `Static`/`Button`/`ListBox` 三个，`Edit`/`ComboBox`/公共控件都没有；也没有资源对话框（`DialogBoxParam` 那套）。**如果你要做的是"把既有 Win32 程序搬到多平台"，先按 2.3 逐条自查**——尤其注意 `CreateWindowEx` 建原生控件失败时是**返回 `NULL` 而不是编译报错**。若你想的是"用 SOUI 写一套新程序覆盖多端"，这一条不影响你。（声明层面的小缺口另见 2.2。）
5. **OHOS 无障碍在构建层被强制关闭**：`CMakeLists.txt:118-120` 里写着 `if(IS_OHOS) set(SOUI_ENABLE_ACC OFF)`。桌面三平台的无障碍是可用的（Windows 走系统 `oleacc`、macOS 走 NSAccessibility 桥、Linux 走 AT-SPI2 桥，约 6,400 行自研代码），但**如果鸿蒙端的无障碍是硬性合规要求，目前还不满足**。
6. **OHOS 深度适配仍需真机验证**：沙箱路径、共享内存等系统行为在不同 OHOS 版本与设备上的差异尚未穷尽。
7. **团队技能栈要对得上**：这套东西的收益建立在"你懂 Win32/WTL"之上。`README` 自己也是这么写的——"SOUI requires basic knowledge of C++ and Win32"。如果团队全是 Web / Kotlin / Swift 背景，学习曲线会比 Flutter/Electron 陡。
8. **vendor 代码体量大**：`third-part/` 3,921 + `swinx/thirdparty/` 7,843 = **11,764 个受控文件**，占两个仓库受控文件的 **67.6%**（完整检出工作区约 78.6%）。换来的是**零版本兼容风险**（cairo / fontconfig / freetype / xkbcommon / dbus / skia 全部按源码 vendor 化，构建不依赖发行版给什么版本），代价是 clone 体积与全文检索体验。

这 8 条里，**第 1 条是"能不能用"的问题，第 2、3 条是"你要接受或参与改善"的工程现实，4–8 条是可以规划的具体事项。**

---

## 七、如果我要评估它，两天怎么验

不用听完介绍就下结论，两天足够判断：

**第 1 天：验证"它能不能跑起来"（约 1–2 小时）**

- 拉仓库，按 `README` 构建桌面端；
- **跑测试门禁**——这一步我强烈建议做，因为它一次同时验证了两件事："构建链是通的"和"框架自身有回归保障"：
  ```
  ctest --test-dir build -L '^soui-headless$'
  ```
  当前应当是 **325 条全部通过**（323 单元 + 1 集成 + 1 无窗口 E2E）；源码里共 31 个 `test_*.cpp`、328 处 `TEST` 声明；
- 跑 `games/cnchess` 桌面端，联机下一盘棋，**顺便观察 AI 思考时界面是否依然流畅**——那就是 4.3 那套异步任务在干活。

**第 2 天：验证"它能不能装下你的需求"（这才是重点）**

1. **写一个你自己的窗口**：加一段 XML 布局、两个控件、一个点击事件。这一步验的是"XML + 事件映射表"这套开发模型你顺不顺手。
2. **把你现有代码里最"Windows"的那部分拿来试编译**——这是 SOUI 相对其他方案最大的潜在收益点，也是最该提前验证的点。**重点盯三件事**：① 有没有 `SetMenu` / `GetMenu` / `DrawMenuBar`（有 → 编译期就过不去）；② 有没有 `CreateWindowEx` 建 `EDIT` / `COMBOBOX` / `SCROLLBAR` / `SysListView32` 这类**原生或公共控件**（有 → 编译能过、运行时窗口建不出来，返回 `NULL`）；③ 有没有 `.rc` 里的 `DIALOG` 模板配 `DialogBoxParam`（有 → 找不到符号）。**这三类只要中一条，就不是"改改就能编"，而要评估重写量**——把这部分 UI 改写成 SOUI 的 XML + 自有控件，通常是可行且推荐的方向（2.3 节末段讲了为什么这件事是可达的）。
3. **在你的目标平台上跑通构建**：如果目标是移动端或鸿蒙，直接看 `games/cnchess/client/{android,ohos}` 是怎么接的——它就是标准的宿主接入样板，三个薄入口，照抄即可。
4. **回到第六节第 1 条，核对许可与你的商用场景。**

---

## 八、结语

我不认为 SOUI5 是"通吃"的方案，它的选择很明确：**不去抽象 Windows，而是让 Windows 成立。**

这带来一个别家给不了的收益——**Win32 程序员的知识与代码是资产**；也带来一个别家没有的成本——162,819 行的兼容层，以及一个**目前只覆盖了"SOUI 应用所需"那半张、还没覆盖原生控件 / 窗口菜单栏 / 资源对话框的 API 面**（2.3）。它对"最懂 Windows 的人"最友好，也正因此，它对"不懂 Windows 的人"最不友好。

所以它适合的场景也很明确：

> **团队熟悉 Win32/WTL，需要一套 C++ 代码同时覆盖桌面与移动（含鸿蒙），并且在意体积、启动速度与原生渲染。**

再补一句限定：这里的"需要一套 C++ 覆盖多端"，指的是**用 SOUI 来写这套程序**。如果你指的是"把手上那套既有 Win32 程序原样搬过来"，请先读完 2.3——**那件事目前还做不到；但它是一项工作量，不是一条死路。**

如果你正好在这个交集里，我认为它值得你花上面那两天——尤其是第 2 天的第 2 步。

---

**SOUI5 项目主页：**

- GitHub：<https://github.com/soui4/soui>
- Gitee：<https://gitee.com/setoutsoft/soui4>
- GitCode：<https://gitcode.com/setoutsoft/soui4>
- 官方网站：<https://www.soui.com.cn>

---

*本文所有数据取自 2026-09-19 当前源码实测（详见仓库内 `doc/SOUI5项目评估报告-2026-09-19.md`）：代码规模用 `wc -l` 统计工作区文件，文件数用 `git ls-files` 统计受控文件，`swinx` 的 API 计数取自 `swinx/doc/tools/api_scan.json`，测试与门禁数取自以当前源码重新 configure 生成的 `CTestTestfile.cmake`。文中引用的每一处源码位置（`MainDlg.cpp:270`、`ChessAI.cpp:22`、`ChsAIEngine.cpp:799`、`TaskLoop.cpp:69`、`CMakeLists.txt:118-120` 等）均已逐条核对。*
