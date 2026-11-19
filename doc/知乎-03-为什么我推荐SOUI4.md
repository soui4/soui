# 跨平台 GUI 选型：为什么我推荐 SOUI

*（仓库目录叫 `soui4`，内核版本 5.3.2.x，官网对外标 5.3 / 有时写 SOUI5，本文统称 SOUI，指同一个东西。同一套 C++ 跑在 **8 个运行目标**上——Windows / Linux / macOS / iOS / Android / OpenHarmony / FreeRTOS 裸机 / 浏览器 WebAssembly——而两个真实应用的业务源码里 `__ANDROID__` 出现 **0 次**。这篇只讲能逐行核对的证据，短板一并写清。）*

---

## 〇、先说结论，以及这两个月变了什么

九月份我写过一版推荐，那时它还是"六端"。十一月这版最大的变化不是又多了几个平台，而是**同一个抽象被推到了两个极端**：没有操作系统的 MCU，和没有原生窗口的浏览器沙箱。

| 维度 | 2026-09 | 2026-11（当前） |
|---|---|---|
| 运行目标 | 6 | **8**（+ FreeRTOS 真机 + WebAssembly） |
| `swinx` 兼容层 | 162,819 行 / 44 个公共头 | **209,615 行 / 57 个公共头** |
| API 面 | 1,042 声明 / 913 有定义 | **1,153 声明 / 1,143 有定义（99.1%）** |
| 测试门禁（ctest） | 325 条 | **638 条** |
| CI 工作流 | 3 个（桌面三端） | **7 个，八个目标全在门禁里** |
| 移动端 | 零自动构建 | Android / OHOS / iOS **交叉编译进 PR 门禁** |
| 新增能力面 | — | 裸机适配、浏览器双后端、WGL/OpenGL 兼容面、鼠标手势、UI 自动化测试库、单时间轴动画调度 |

先说一句避免误会的话：**"8 个目标"不等于"8 个能直接交付的目标"**。六端是产品级，FreeRTOS 与 WASM 是"有 CI、有真机或真实应用、但有明确缺口"的能力证明级。第二节会把这张分档表摊开——这是我判断它是否适合你的第一把尺子。

---

## 一、选型其实是在选"用哪种方式消化平台差异"

市面上的方案大致三条路：

| 路线 | 代表 | 跨平台怎么实现 | 你要付出的主要代价 |
|---|---|---|---|
| **换语言** | Flutter、Electron、React Native | 用另一门语言写 UI，再各自渲染 | 现有 C++ 资产与团队技能栈归零；还要背一个运行时（Dart VM / Chromium+Node） |
| **自建抽象 API** | Qt、wxWidgets | 定义一套框架自己的 API，各平台分别实现 | 还是 C++，但业务代码要面向框架 API 重写；不同平台的行为仍有差异 |
| **重演平台本身** | **SOUI** | 不去抽象 Windows，而是**让 Windows 的 API 面在别的平台上成立** | 需要一个兼容层（`swinx`，209,615 行） |

第三条路最"笨"，也最少人走——因为要重演一整个 API 面。但收益非常直接：

> **你已经会的 Win32/WTL 编程模型、以及围绕 SOUI 写过的 Windows C++ 代码，在这里是资产，不是负债。**

代价也很具体：**兼容层的代码量是内核的 1.11 倍**。框架自有源码 547,951 行里，`SOUI/` + `utilities/` 只有 189,536 行，`swinx` 单独占 209,615 行。这套方案的核心成本不在 UI 框架本身，而在"让 Win32 语义在八个目标上成立"。这不是缺陷，是设计选择的必然结果——换来的东西是：**内核里 0 处平台 `#ifdef`**。

架构上就四层：

| 层 | 内容 |
|---|---|
| 业务代码 | C++ + XML（+ 可选 Lua），八端共享，**零平台宏** |
| SOUI 内核 | 窗口树 / 消息路由 / 事件系统 / 六类布局 / 两套动画 / 控件 / XML 对象工厂 / 手势归属链 |
| swinx 兼容层 | **57 个**公共头（绝大多数与 Windows SDK 同名同结构）+ 209,615 行实现（Windows 上**不编译**，直接用系统 SDK）+ 7 个平台目录 × 4 个渲染后端 |
| 底层 | 系统 Win32 · xcb+Cairo · Cocoa/CoreGraphics · UIKit · Android/OHOS JNI·N-API + Cairo · FreeRTOS 任务 + 帧缓冲 · 浏览器 Canvas2D |

这里有一个必须提前说清的边界，否则后面全是误会：

> **"重演 Windows"目前重演的是"SOUI 应用所需要的那部分 Win32 面"。** 如果你手上是一个用原生控件（`Edit`/`ComboBox`/公共控件）、窗口自己的菜单栏、资源对话框搭起来的**既有 Win32 程序**，直接搬过来还跑不起来——差在哪、差多少，第四节逐条列给你看。

---

## 二、八端不是一回事：先看分档

"8 个目标"这个数字很容易被读成"8 个都能上生产"。用四把尺子（有无 CI / 有无示例 / 有无运行期证据 / 已知缺口）量一遍，实际是三档：

| 档位 | 目标 | 判据 | 还缺什么 |
|---|---|---|---|
| **产品级** | Windows / Linux / macOS | 门禁 + 完整示例 + 系统级能力（无障碍、菜单、原生控件、对话框）齐备 | — |
| **产品级（移动）** | Android / OHOS / iOS | 交叉编译进 PR 门禁、宿主库齐备、应用有对应壳 | **无运行期回归**；OHOS 的无障碍被构建层强制关闭 |
| **能力证明级+** | FreeRTOS | QEMU + **ESP32-S3 真机**、四道门禁、有完整性能参数 | 字形只烘了 ASCII、只有一块板、双动画仍低于单动画、无输入法 |
| **能力证明级+** | WebAssembly | 3 个消费端、双后端、可选真线程、双后端烟测门禁 | 触摸/移动端浏览器未验证、系统级输入法与拖放未做、多窗口合成缺应用层自动化 |

这张表是全篇最重要的读法。**"能构建、`SApp` 能端到端跑通"与"能交付项目"之间的差，全在最后一列。**

平台适配层与渲染后端是两个正交维度，可以任意组合：

| 目标 | 平台适配层（文件 / 行） | 渲染后端 | 宿主形态 |
|---|---|---|---|
| Windows | （不编译 swinx） | GDI / Direct2D | 系统窗口系统 |
| Linux | `platform/linux`（20 / 14,979） | `gdi/cairo` | xcb + `poll()` 事件循环 |
| macOS | `platform/cocoa`（26 / 10,708） | `gdi/apple` | `NSApplication` |
| iOS | `platform/ios`（23 / 6,218） | `gdi/apple` | `UIApplicationMain` |
| Android | `platform/mobile`（15 / 2,797） | `gdi/cairo` | JVM 启动 + `PlatformAPI` 桥 |
| OpenHarmony | `platform/mobile` | `gdi/cairo` | ArkTS 启动 + `PlatformAPI` 桥 |
| FreeRTOS | `platform/freertos`（23 / 6,452） | `gdi/freertos` → 内存帧缓冲 | **无 OS**：任务跑消息泵，宿主取帧缓冲上屏（ESP32 LCD / QEMU / SDL2） |
| WASM | `platform/wasm`（17 / 4,298） | **`gdi/canvas2d`（默认）/ `gdi/cairo`（可选）** | **浏览器**：`requestAnimationFrame` 驱动泵，JS 宿主按脏矩形 blit |

> 两个维度的组合自由度不是装饰：WASM 能在不改一行业务代码的前提下切 canvas2d / cairo（`-DSWINX_WASM_GDI_BACKEND=canvas2d|cairo`），这个自由度正是"跨后端像素对拍"的前提。

---

## 三、两个极端：裸机上的 56 fps，和浏览器里的真线程

这两块是这次最重要的增量，也是"重演 Win32"这条路能不能外推的关键证据。

### 3.1 FreeRTOS：一块 320×240 的屏，跑的是完整 `SApp`

不是"点亮一个色块"的移植，而是把桌面那套 demo 原样搬上去——标题栏、PNG 图、下拉框、按钮、两个值动画，全部走 SOUI 自己的路径，**没有一行 LVGL 代码**：

| 项 | 实测值 |
|---|---|
| SoC / 屏 / 触摸 | ESP32-S3 rev0.2，16 MB Flash，8 MB PSRAM / ILI9341V SPI 320×240 / FT6X36 电容触摸 |
| 固件 | **4,098,192 字节（≈3.91 MB，6 MB app 分区余 35%）** |
| 冷启动 → 首帧 | 约 **4.4 s**（含 COFF 资源解析、布局 + SVG 皮肤、cairo 首帧、整屏 SPI） |
| 稳态刷新（单动画） | **56~58 fps**（frame 17.3 ms / paint 14.2 ms，探针关） |
| 稳态刷新（双动画，当前设备版本） | 约 **40 fps** |
| 资源 | windres COFF **87,352 B / 59 条**，桌面同一条 `SResProviderPE` / `FindResource` 路径 |
| 渲染链路 | `render-gdi` → `gdi/cairo` → PSRAM 帧缓冲（ARGB32）→ RGB565 → SPI |
| 上游改动 | **4 处**（配置宏、临界区分支、线程 ID、`dlerror()` 判空），**内核零移植改动** |

裸机载体累计约 13,881 行，其中框架适配 6,976 行、ESP32-S3 整套板级 2,394 行。也就是说：**把抽象从"有窗口系统的 OS"退到"没有 OS"，框架侧的代价只有 7 千行，其余全是工程胶水。**

优化过程里有三条结论对别的目标也成立，我单独摘出来：

1. **一帧只脏约 5,700 像素，paint 仍要十几毫秒**——受限目标上要算的是"固定开销 / 像素开销"的比值，不是像素数。
2. **`-O2` 与 `-Os` 只差 2 ms**——结构性问题不能靠编译选项解决。早期 8 fps 到 56 fps 的跨越全部来自结构优化（脏区合并、减少合成次数、单时间轴）。
3. **layered 窗口的成本在"每帧一次离屏合成"，与面积无关**——预算是"合成次数 / 帧"，不是"窗口数"。这也是双动画 40 fps 与单动画 56 fps 之间的差额来源。

### 3.2 WebAssembly：双后端，而且能开真线程

浏览器这条线复用了一个几乎一模一样的蓝图：WASM 与 FreeRTOS 同属"没有原生窗口服务器、软件渲染到内存、宿主负责呈现"，所以移植 ≈ 一个平台后端 + 一个宿主 JS 壳。四个宿主接缝（帧缓冲 / 上屏 / 输入 / 泵）与裸机同构——**这是第二、第三个消费端能几乎零成本起来的原因**。

两个后端是一个明码标价的交换：

| 维度 | `gdi/canvas2d`（默认） | `gdi/cairo`（可选） |
|---|---|---|
| 代码量 | 4 文件 / 6,336 行 | 9 文件 / 8,633 行（与桌面共用） |
| 字体 | **浏览器内置字体** | 内嵌 TTF + freetype/fontconfig |
| C 库依赖 | **零** | 整族（cairo/pixman/freetype/fontconfig/zlib/png） |
| 演示总计 | **5.5 MB**（`.wasm` 4.53 MiB + `.data` 0.77 MiB） | **26 MB**（6.18 + 19.6） |
| 适用 | 发布（体量优先） | 字形可控 / 离线字形 / 像素对拍 |

**差 4.7 倍，其中 19.6 MB 全是内嵌字体与 C 库。** 这组数字说明"为什么默认 canvas2d"，也说明"为什么保留 cairo"：当你要字形可控时，代价就是 20 MB——这个交换应该能被显式选择，而不是被删掉。

三个真实消费端，不是三个 demo：

| 消费端 | 性质 | 产物 |
|---|---|---|
| `demos/wasm-demo/` | 演示（十二页控件 / 编辑器 / 图表 / Lua 小游戏） | 5.5 MB |
| `games/cnchess/` | 真实应用：登录、大厅、棋局、WebSocket 联网、主题包下载解压、音效、中文输入法 | 上线档 ~5 MB + 0.5 MB 资源 |
| `demos/fundholdings/` | 真实应用：基金持仓工具，行情代理、列表、图表，**默认开 pthreads** | 3.77 MB + 45 KB 资源 |

三者共用同一份 `soui-host.js`（54 KB：下载进度、脏矩形上屏、键鼠/滚轮注入、IME 代理、剪贴板桥、WebAudio、崩溃上报）。新程序在页面里 `SouiHost.start({container, wasm, factory, api})` 三五行即可。

线程这块值得单独说，因为它把一句常见的架构判断降级成了部署约束：

| 配置 | `ITaskLoop` 后端 | 前提 | 结果 |
|---|---|---|---|
| `SOUI_WASM_PTHREADS=OFF`（默认） | `STaskLoopPump` | 无 | 主线程队列，宿主每帧按预算抽干 |
| `SOUI_WASM_PTHREADS=ON` | `TaskLoopWasm` worker | `-pthread` + `-sWASMFS=1` + `crossOriginIsolated`（COOP+COEP） | worker 真跑，UI 只入队 + Atomics 唤醒 |

象棋的主题包解压真跑在 worker 上（烟测断言 `offUi==posted`，泵后端必然给不出这个值）。代价是站点必须带 COOP/COEP 头，调试心智从"单线程"变成"双线程 + 隔离"——**这是真实的复杂度增加，不是免费升级**。

性能上，同机 A/B：整窗重绘每帧 **1,773 ops / 11.8 ms**（其中帧内 JS 5.3 ms），Lua 游戏页（坦克大战）**57 fps**，idle `rafP95` 16.8 ms。相对同机基线：ops −9%、span −14%、fps ×2.6。

### 3.3 为什么这两件事能成立

把三块新能力（FreeRTOS / WASM / OpenGL）并排看，会发现它们的位置和形状完全一样：

| 维度 | FreeRTOS | WebAssembly | OpenGL/WGL |
|---|---|---|---|
| **落点** | `platform/freertos` + `gdi/freertos` | `platform/wasm` + `gdi/canvas2d` | `include/wgl.h` + 两个平台实现 |
| **复用谁** | `gdi/cairo` + `platform/posix` | **复用裸机的帧缓冲蓝图（四个接缝）** | Win32 WGL 语义 |
| **内核改动** | 若干显式 include + 一个连接点注册 | 一个字体回退分支 + `TaskLoop` 的 wasm 后端 | **零** |
| **宿主职责** | 帧缓冲上屏 + 输入注入 | canvas blit + DOM 事件注入 + 泵 | 无（建在原生子窗口上） |
| **代价形态** | 工程胶水（构建 / 资源打包 / 板级驱动） | 宿主壳 + 单线程心智模型 | 平台语义差异的显式降级契约 |

结论：**三个新目标都是"同一抽象下的叶子"，不是并列的框架分支。** 这是 8 个目标能共享一套内核的结构性原因，也是我判断"再吸收一个新宿主形态也不会推翻设计"的依据。

---

## 四、兼容层：它把底牌摊开了

### 4.1 99.1% 这个数字要按"声明即承诺"读

当前口径：**1,153 条声明 / 1,143 条有定义**（完整实现 1,094 / 部分实现 9 / 空实现或语义桩 40），另有 10 条未定义——全部是宏、编译器内建或系统已提供的入口（`LOWORD`、`memcpy`、`DEFINE_GUID` 这类），**没有一条是"该有却没有"的真实 API 缺口**。

这个 99.1% 的含义是"**声明出来的函数都有定义**"，**不等于**"Windows 有的这里都有"。分母是主动裁剪过的：不打算实现的声明会被删掉，而不是留着返回 NULL。所以它说明的是"承诺与交付一致"，而不是"实现面接近 Win32 全集"。

比覆盖率更值钱的是它的维护方式：`swinx/doc/tools/api_scan.py --check` 在 PR 上比对声明面清单，**还带一层能力基线**——记录 1,120 个函数的最低实现等级，任何函数被悄悄从 `impl` 降级成 `partial`/`stub` 都会让门禁变红。这一层防的是比"忘更新清单"更隐蔽的退化。

### 4.2 迁移面：这才是决定"能不能搬"的那一层

**原生控件、窗口菜单栏、资源对话框**这半张 Win32 面孔还没建。它不体现在"缺口几条 API"这种口径里，因为那不是"某几个函数没实现"，而是"整类 API 还没提供"：

| 缺什么 | 当前状态 | 迁移时会怎样 |
|---|---|---|
| **窗口自己的菜单栏** | `SetMenu` / `GetMenu` / `DrawMenuBar` **连声明都没有**。反差是**弹出菜单机制本身完整**（`CreateMenu`/`AppendMenu`/`TrackPopupMenu`/`CheckMenuItem` 一应俱全） | 调 `SetMenu` 的程序**编译期就过不去**。SOUI 应用不受影响——它用自绘的 `SMenuBar`/`SMenu`（XML 描述） |
| **原生基础控件** | 只注册了 3 个窗口类：`Static`、`Button`（给 `MessageBox` 当零件）、`ListBox`（从 Wine 完整搬运）。**`Edit` / `ComboBox` / `ScrollBar` 都没有** | **注意**：这里比"少几个函数"更麻烦：类名宏（`WC_EDITA` 等）在头文件里是有的，所以 `CreateWindowEx(0, "EDIT", …)` **能编译、能链接**，运行时却**找不到窗口类 → 返回 `NULL`**。**这种失败不会在编译期提醒你** |
| **公共控件（comctl32）** | 常量、结构体、`WC_*` 类名都在，但 `InitCommonControls(Ex)` **只有声明没有定义**；`SysListView32` / `SysTreeView32` 等全无实现 | 调 `InitCommonControls()` 直接链接失败（这种反而"友善"）；建 `SysListView32` 则是运行时返回 `NULL` |
| **资源对话框** | **没有任何对话框实例化 API**：`DialogBoxParam` / `DialogBoxIndirectParam` / `CreateDialogIndirectParam` / `EndDialog` 全缺。`.rc` 里的 `RT_MENU` / `RT_DIALOG` 只是类型宏，没有对应运行时 | "用资源编辑器画对话框、`DialogBox(hInst, MAKEINTRESOURCE(IDD_X), …)`"这条最经典的老路走不通 |
| **`MessageBox`** | **有**：自绘模态窗口，用 `Static` + `Button` 拼出来 | 能用，但按钮文案是硬编码简体中文，图标用内置位图，**不跟随系统语言与主题** |
| **文件 / 颜色 / 字体对话框** | **有，而且比预期完整**：`GetOpenFileName` / `GetSaveFileName` / `ChooseColor` / `ChooseFont` 这一族在桌面、移动、WASM、FreeRTOS 四个平台都有实现（个别如 iOS 的 `ChooseFont` 明确返回 FALSE），还带异步回调（浏览器里 `<input type=file>` 的结果只能在 JS 栈返回后拿到） | 这一块反而不是问题 |

怎么读这张表：

- 前三类是**同一件事**：swinx 已经建好了"窗口 / 消息 / 类注册 / DC / GDI / 资源 / COM"这套**地基**，但没在地基上建"Win32 原生控件库"这一层。地基比控件库难得多；而控件库恰恰是最能从 Wine 现成搬运的部分——`ListBox` 那 3,000 多行就是活样板。
- 失败方式要单独留心：既有链接期报错，也有**运行时静默返回 `NULL`**。后者最容易漏——搬家时如果不检查 `CreateWindowEx` 的返回值，你会看到"程序起来了，但界面是空的"。
- 有一条现成绕行通道：移动端的 `SRealWnd` + `IRealWndHandler` 能把窗口类名映射到**宿主原生控件**（Android demo 里的 `<realwnd wndclass="edit"/>` 就是 `NativeEditView`）。也就是说建不出来的控件可以借宿主的原生控件顶上——但那是"宿主 App 提供的能力"，不是 swinx 提供的 Win32 控件。

**本节结论**：swinx 的目标——"让既有 Windows 程序零成本迁移"——**目前还没达到**。它已经达到的是另一个、也确实好用的目标：**让用 SOUI 写的应用全平台原生运行**。两者差的正是"原生控件库 + 窗口菜单栏 + 资源对话框"这三块。

而这件事**能力是具备的**：`ListBox` 已经证明"从 Wine 整体搬一个控件"这条路走得通，地基也稳。所以只要项目需要，"把另外半张面孔补齐"是工作量问题，不是架构上的不确定性。

### 4.3 三个"它居然也搬过来了"的案例

| 案例 | 难在哪 | 怎么解决 |
|---|---|---|
| **RichEdit 富文本编辑** | Windows 上最深的坑之一：依赖 Text Services（`ITextHost`/`ITextServices`）+ OLE + GDI+ 一整套私有契约 | Windows 上走系统 `msftedit.dll`；非 Windows 上由 swinx 提供 `richedit.h` / `textserv.h`，底层引擎是移植的 Microsoft RichEdit 4.1 源码（499 个受控文件）。**同一份 `SRichEdit.cpp` 在八端编译，包括浏览器** |
| **Scintilla 语法着色编辑器** | 官方本来就提供 Win32 与 GTK 两套后端 | SOUI **只取 Win32 那套**：注册窗口类、`CreateWindowEx(..., "Scintilla", ...)`、`SendMessage(SCI_*)`——全靠 swinx 把窗口/消息/GDI 整体模拟出来 |
| **`.rc` 资源跨端编译** | 别的框架每端一套资源管线（`.qrc`、pubspec assets、asar） | 一份 Windows `.rc` 在 Windows / Linux / macOS **三端同源编译**进 PE / ELF / Mach-O；FreeRTOS 侧换载体：`uires.idx` → `windres` COFF → `.incbin` 数据段（`.balign 8`，Cortex-M 非对齐会 fault） |
| **WKE 浏览器引擎** | 最复杂的客户端形态 | `SWkeWebkit` 控件在非 Windows 已脱离条件编译，swinx 补齐引擎所需 API；引擎本体在独立仓库编译，运行期绑定 |

第二、第三个案例特别有说明力，但要点明它们**各自的前提**：

- Scintilla 证明了"**只要某组件存在 Windows 实现，swinx 就足以让它跨端**"。前提是——**Scintilla 是自绘组件**，它只要 `CreateWindowEx` / `SendMessage` / GDI 这层公共设施。它**不**意味着"系统自带的那批控件会自动可用"（见 4.2）。
- `.rc` 那个例子把"用熟悉的资源编辑器写一份 .rc"带到了 Linux/macOS，配合 `ENABLE_BUILD_RESOURCE` 还能单文件分发。同样要补一句：这里编进资源的是图标、位图、清单这类**数据型资源**；`DIALOG` / `MENU` **模板**虽然也能编进去，但目前没有运行时能实例化它们。

---

## 五、工程能力：内存、资源、体积、矢量、可观测性

### 5.1 内存：把 C++ 最大的坑收进框架

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

即在"自动回收资源对象"与"避免控件树循环引用"之间取了正确的平衡，把 C++ 最常见的两类泄漏都堵上了。

> 但要诚实说一句：**引用计数不防跨线程生命周期误用，也不防你在外部缓存裸指针**。它降低风险，不消除风险。

### 5.2 资源：三层职责，运行期只有字符串键

把"从哪里取字节"（提供器）、"谁覆盖谁"（提供器链）、"取到之后怎么变成运行时对象"（UI 定义池）切三层，运行期定位键只有**字符串**——`类型:逻辑名`，不含语言维度、不含数值 ID。

| 框架 | 运行期定位键 | 覆盖 / 热替换语义 |
|---|---|---|
| Qt | `:/路径`（`.qrc` 编译进二进制） | 内置资源不可被外部包覆盖 |
| Android | 数值 `R.id` + 限定目录 | 变体必须打包时枚举完 |
| Flutter | `pubspec.yaml` 声明的包内路径 | 资源清单是构建输入，追加文件要重编 |
| Electron | `asar` 归档内相对路径 | 归档可整体替换，主题语义要应用层自建 |
| **SOUI** | `类型串:逻辑名` + 手写 `uires.idx` | **后挂载的提供器覆盖先挂载的**——主题包/热更目录天然盖住内置资源，各目标同一语义 |

提供器与平台的对应关系也不是"每端一套专用实现"：iOS 用的是 `GetAppleBundlePath()` + **桌面同一条** `SResProviderFiles`；只有 Android / OHOS 需要专用提供器，因为那两端的资源**根本不是文件系统里的普通文件**。另有 `ResType_Mem` + `SetAppResMem`：调用方可以把自己 `Init` 过的 `IResProvider` 直接交给 `SAppCfg`——资源"从哪里来"不再被限定为"文件 / PE / 内存块"，而是**任何实现了 `IResProvider` 的对象**。FreeRTOS 上那 59 条资源走的是 `SResProviderPE`（读链接进固件的 COFF），与桌面同一条 `FindResource` 路径。

代价如实记：字符串键把拼写错误推到运行期；**没有条目级失效/版本机制**，可用清除粒度只有整池级；`uires.idx` 手写无生成工具。

### 5.3 体积：核心 2.3 MB，且能给不同目标换配置

- 最小编译核心约 **2.3 MB**（动态链接核心库，Release x86）；
- 常规配置：Windows ~4 MB / Linux ~10 MB / macOS ~7 MB；
- **17 个组件各自是独立 target**，没用到的（skia / wic / zip / 7zip / lua / network / httpclient…）根本不链进来；
- 资源可内嵌（PE/ELF/Mach-O），静态模式下产物就一个可执行文件；
- 新增一个开关：`SOUI_EDIT_LITE`（默认 OFF）——`<edit>` 用 1,728 行自研轻量实现替代 richedit41，**整个富文本引擎可以不链**。

放一起对比：

| 方案 | 运行时依赖 | 核心体积量级 |
|---|---|---|
| **SOUI** | **无 VM、无 JS 引擎** | **~2.3 MB**（常规 4~10 MB） |
| Qt 6 | 无 VM，但较重 | 数十 MB |
| Flutter | Dart VM + 自绘引擎 | 10 MB – 数十 MB |
| Electron | Chromium + Node | 100 MB+ |
| wxWidgets | 借用系统原生控件 | 小 |

> 限定语境：说"最小"是**相对 Qt / Flutter / Electron 这类"原生渲染 + 完整控件 + XML 布局的全功能跨平台 GUI 框架"**成立。另外 wasm 的 cairo 分支 26 MB 是"字形可控"的明码价，不能拿 2.3 MB 去对比它。

### 5.4 SVG 是"渲染原语"，不是"图片格式"

多数框架把 SVG 当成"另一种要解码的图片"——先栅格化成固定尺寸位图再拉伸。SOUI 把它当矢量绘制原语：

- `IRenderTarget::DrawSVG` 是渲染接口之一，**四个后端全部实现**；
- 皮肤走 `SSkinImgList::SetSvg`（XML 类型 `"svg"`），`DrawSVG9Patch` 让 SVG 像 9-Patch 一样任意拉伸而不糊边；
- 绘制发生在**目标 DPI 的像素网格上**，Retina / 4K / 移动高分屏天然清晰，**不需要 @1x/@2x/@3x 多套位图**；
- 落地痕迹：系统主题里的 `sys_focus_checkbox.svg` / `sys_focus_radio.svg` 说明它已用在**框架级视觉状态**上，不只是皮肤素材；
- 附带一个在 2026 年才显得重要的好处：SVG 是文本化、可程序化生成的格式，**对 AI 设计/生成工作流天然友好**。

### 5.5 可观测性：零成本绘制探针

`SOUI/include/core/SPaintProbe.h` 代表一种工程取向：**性能结论必须由可复现的观测支撑，而不是推测**。三个设计点值得作为范式：

- **零开销默认**：`log=NULL` 时每个挂点只做一次指针判空，可常驻发布代码；
- **header-only + 函数内静态**：内核与固件应用共享同一实例，不需要为每个平台改一次 `CMakeLists.txt`；
- **回答"账是否闭合"而非单点耗时**：它同时记录脏区并集、进入/通过/实际绘制的窗口数、阶段耗时，能验证"没有整窗重绘""没有多画窗口"这类**否定性结论**。

这套探针在 ESP32 上抓出过一个真实缺陷：`UpdateLayout()` 原先只把 `dirty_child` 清回 clean，`dirty_self` 不清——root 一旦被置脏就再也没干净过，每帧白跑一遍布局。修复后探针显示 `ld=` 从"每帧脏"变成 `1/1219`（1219 帧里只有第一帧脏），`layout` 耗时 **550 µs → 5 µs**。

---

## 六、开发能力：从 XML 到一条共享时间轴

### 6.1 UI 描述与迭代

- UI 用 XML 描述（`pugixml` 封装为 `SXml`），控件由对象工厂按节点名映射到 C++ 类——**换布局、换皮肤不改 C++ 代码**；
- 支持 `<include>`、模板、命名空间；
- 自带可视化设计器 `uieditor`（在 `demos/` 里，构建 demo 即得）；
- **皮肤热换**做得比预期彻底：cnchess 的主题不是打包进安装目录，而是运行时从服务器下载——WebSocket 分块传输、MD5 命中判断（服务器发现本地一致就回"总长 0"，一个字节都不传）、增量 MD5 校验（逐块 `MD5_Update`，**峰值内存只是分块之和**）、校验通过才解压生效。整套在 `client/ThemeDownloadManager.{h,cpp}`，可以直接当范例读。

### 6.2 布局：`pos` 是一套坐标系，不是四个数字

六类布局：`SouiLayout`（默认，自有锚点语法）、`Anchor`（9 宫格）、`linearLayout`/`vbox`/`hbox`（线性 + weight）、`flowLayout`（流式换行）、`frameLayout`（叠放）、`gridLayout`（行列）。其中 `pos` 的前缀字符本身就是一套完整坐标系：`|` 父中心、`[`/`]` 前/后兄弟近边、`{`/`}` 远边、`%` 父窗口百分比、`@` 直接给尺寸；还能 `sib.left@id:offset` 引兄弟做相对定位。表达力最强，学习成本也最高——这是老实话。

比"有六种布局"更重要的是：

1. **布局是"标脏 + 一次重排"，不是"改属性立刻重排"**：改 20 个子控件的 `width` 只产生**一次**布局 pass。真正的 `UpdateChildrenPosition` 由宿主在每次绘制前调一次。
2. **布局参数本身就是可动画的属性**：`width`/`height`/`offsetX`/`weight`/`left`/`right`/`posX` 这些键名被逐个导出成常量，正是属性动画写属性时用的键。布局与动画因此不是两个孤岛——**动画改的是布局输入，重排与重绘照旧走同一条合并路径**。

### 6.3 动画：两套动画 + 一条共享时间轴

**A. 视图动画**——"只改变这一次怎么画"：`alpha` / `rotate` / `scale` / `translate` 四原语 + `set` 串接，**27 个插值器**（Linear、Accelerate/Decelerate、Anticipate/Overshoot、Bounce、Cycle，以及 Quad/Cubic/Sine/Expo/Quint/Back 各三档）。作用点在**绘制**而非**几何**：每帧产出一个变换矩阵，绘制时沿祖先链逐级 `preConcat`，`GetWindowRect()` 始终不变。

**B. 属性动画**——"改的是属性，布局与绘制都跟上"：时间引擎 `SValueAnimator`（duration / repeat / interpolator / reverse）、值域 `TValueAnimator<T>`（Int / Float / Color / Size / Point / Rect）、绑定 `SPropertyAnimator`（绑目标 + 关键帧数组）。其中 `ofLayoutSize` / `ofPosition` 是 DirectUI 特有的一条：动画的是**布局输入**，兄弟窗口会跟着让位——视图动画做不到这件事。

**C. 一条时间轴**。`ITimelineHandler` 只有一个 `OnNextFrame()` 和一个常量 `kPulseInterval = 10`；一次脉冲先跑所有 handler（插入符闪烁、滚动条渐隐、视图动画、窗口进出场），再跑所有 value animator。**两类共用同一帧时间，多个并发动画不会相位漂移。**

由此得到三个可验证的结论：

1. **没有动画在跑时，宿主一个定时器都没有**。注册/注销时做"空 ↔ 非空"边界检测，第一个 handler 进来才 `SetTimer(10ms)`，最后一个走掉就 `KillTimer`。这一层还向下递归，隐藏或无请求的面板自动注销登记。**对移动端和 ESP32 是直接的功耗收益，不是纸面优化。**
2. **一个时间轴被多类任务共用**：新增一类动效不需要再申请一个定时器。
3. **集中刷新是设计结果**：动画每帧只写属性；写入经 `InvalidateRect` 逐级上溯到根，根把矩形合并进一个脏区；再配合绘制前那一次 `UpdateLayout()`。于是**一帧内 N 个控件的 N 次修改 = 一次脏区并集 + 一次布局 + 一次 `WM_PAINT` + 一次上屏**。

第 3 条是性能设计而不是实现细节：**一帧的绘制成本只与脏区并集有关，与"有多少动画在同时跑"无关**。这正是 ESP32 上 56 fps 成立的前提。

### 6.4 控件：40 个内置 + 37 个扩展，深度靠用例兜

- `SOUI/include/control/` 下 **40 个控件头**，含 MVC 虚拟列表 `SListView` / `SMCListView` / `STreeView` / `STileView` / `SComboView`（大数据集只渲染可见项）；
- `controls.extend/` **37 个窗口类**经集中注册，另有 Scintilla、hellocharts、gridctrl、propgrid、qrcode 等重组件；
- **行为深度**（"40 个头"说明不了的部分）：六列表控件共享一套由手势层 + 状态层实现的多选 / 框选 / 键盘导航，`test_rubberband.cpp` **37 条**用例是这部分的行为基线；页签拖动切页 24 条；`SListCtrl`/`STreeCtrl` 的 SPACE 切 checkbox 也有用例。

> 一个刻意的取舍：焦点框动画每帧都要重绘，在弱目标（PSRAM 帧缓冲 / 浏览器）上直接换算力。"能力提供 + 默认不启用"是正确取舍，不是"做了一半"。

### 6.5 事件：一张映射表 + 一套订阅

| 机制 | 解决的问题 | 对开发者的价值 |
|---|---|---|
| **事件映射表**：一个窗口类一张表 | "这个窗口负责哪些消息与事件"必须一眼可枚举 | **维护性**：处理器集中，删功能就是删一行 |
| **事件订阅**：控件暴露事件，任意对象挂槽/撤槽 | 框架、业务、第三方在编译期互不知情时如何协作 | **扩展性**：皮肤、脚本层、业务能追加/撤掉处理器，不改控件源码 |

一条 `DEF_EVT(...)` 同时产出数值 ID、数据结构体、事件类与 `on_*` 属性名——C++ 处理器、XML 声明式绑定、Lua 回调**共享同一份定义**。派发路径固定为"本窗口订阅槽 → 脚本处理器 → owner 窗口 → 容器"，任一段可标记"已处理/停止冒泡/取消动作"。

机制核心已有 **29 条直接用例**进门禁，钉住的是"纯记账"契约：已处理不再冒泡、未处理一路冒到容器、后订阅者截断先前订阅者、返回值是 `HandleCount` 而非"最后一个槽的结果"、`isMuted` 是引用计数、`StdFunctionSlot` 对 `std::function` 槽恒等（同一事件挂第二个不同闭包会被静默拒绝——这条坑被用例显式断言，而不是留给踩坑的人）。

### 6.6 手势：拦截与消费是两个入口

原来鼠标消息只有一条路径：容器找最内层窗口，交给它，**不向父链冒泡**。于是"页面被铺满子控件时，父容器仍想感知拖动"这件事做不到。现在的解法是三层：

| 层 | 组件 | 职责 |
|---|---|---|
| ① 归属 | `DispatchTouchEvent` + `OnInterceptTouchEvent` / `OnTouchEvent` | 决定"这次拖动归谁"：DOWN/MOVE/UP/CANCEL 流转、子控件放弃捕获、嵌套优先、事件流独占 |
| ② 识别 | `SGestureDetector` + `IGestureListener` | 与窗口无关的 touchSlop、轴判定、100 ms 速度窗、fling 判定、UP/CANCEL 状态机 |
| ③ 响应 | `SPanel`（滚动/惯性/框选）、`STabCtrl`（切页） | 把识别回调映射成位移、切页与动画 |

关键设计：`OnInterceptTouchEvent` 是**纯探针**（只回答"父容器要不要接管"，绝不发回调），接管后 `OnTouchEvent` 从**同一份状态**继续，起点与速度样本不丢不重。**17 条**用例进门禁，覆盖"低于 touchSlop 保持点击语义""斜向拖动两轴同时滚动""cancel 后下一次手势干净""stray UP 不算手势"等。

### 6.7 UI 自动化测试支撑库

`soui-headless` 的用例一直是"构造对象 → 调 API → 断言状态"，一旦要验证"点一下按钮真的触发了命令"就落到需要 Xvfb 的 GUI 腿。而 SOUI 有一条被低估的能力：`SSendMessage` 能**不经原生窗口**把 `WM_*` 送进窗口处理链。

`components/souitest/souitest.h`（纯头文件，对框架零侵入）把"测试怎么写"固化成三层：**定位**（`findByName` / `findByPath`）、**注入**（`mouseClick` / `keyClick`，Tier A 走 `SSendMessage` 无头、Tier B 走真实 `PostMessage`）、**断言**（事件观测 + 几何 + 状态）。

收益是 **41 条** `soui-uitest` 用例，其中 `button_click_tierA_matches_tierB` 这类用例的价值在于**用同一份断言同时验证无头路径与真实消息路径等价**——"无头测试能不能代表真机行为"这个长期疑问，至少有了一个可执行的回答。

> 局限也要说：它仍是**框架级注入**，不经过 swinx 的窗口过程，不验证焦点/激活/DPI 缩放的真实链路；它依赖 gtest，不适合进产品二进制。

---

## 七、性能：C++ 的价值在"密集计算"上才真正拉开

> "C++ 比 Java 快"是句正确的废话——关键在**什么负载上、快在哪、以及算完之后怎么不卡界面**。

### 7.1 密集计算：布局、线程、时间预算都自己定

`games/cnchess/algorithm/ChsAIEngine.cpp` 是纯 CPU 密集负载（PVS + 置换表 + 杀手/历史启发 + 静态搜索）。三个关键做法，恰好是托管语言最难对位的地方：

**① 内存布局自己定**：90 格棋盘压成 `int8_t` 一维数组，Zobrist 键随 `Make/Unmake` **增量异或**维护——不在每个结点重算哈希。

**② 并行自己分，共享数据无锁**：

```cpp
// algorithm/ChessAI.cpp:23 —— 引擎含置换表等有状态资源，按 thread_local 隔离避免加锁
static thread_local ChsAI::SearchEngine s_engine(18);   // 2^18 条置换表
```

每个池线程各持一份置换表（2^18 = 262,144 条 × 16 B ≈ **4 MB/线程**，池上限 16 线程约 64 MB），**线程之间零同步**。

**③ 时间自己兜底**：

```cpp
// algorithm/ChsAIEngine.cpp:803 —— 每 4096 个结点检查一次时间预算
if ((m_nodes & 4095) == 0 && TimeUp(false))
```

到点立即放弃本层（本层结果作废），层间再按 60% 预算软停——保证"**到点必返回，且返回的一定是某个完整层的搜索结果**"。搜索深度 3 / 5 / 7 三档，时间预算 400 / 800 / 1500 ms。

一个细节：**这个引擎只依赖标准 C++ 头，不依赖 `windows.h`**——所以同一份算法在服务端、Android NDK、OHOS clang、甚至浏览器里都能直接复用。"算得快"和"到处能算"，在 C++ 下可以是同一个文件。

### 7.2 算完怎么安全回到界面：一行 `post`

C++ 只解决"算得快"，不解决"算完怎么把结果安全交给 UI 线程"。SOUI 这里的抽象成本低到出乎我意料：

```cpp
// games/cnchess/client/MainDlg.cpp:314
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
| `IMessageLoop::PostTask(IRunnable*)` | 从**任意线程**把任务投到 **UI 线程**执行 |
| `ITaskLoop::postTask(runnable, waitUntilDone, priority)` | 投到**专属工作线程**（每线程一条队列），返回 taskId 供取消 |

`STaskHelper::post(...)` 是薄封装：把"对象 + 成员函数 + 参数"或 lambda 绑成 `IRunnable`，按第一个参数是 `IMessageLoop*` 还是 `ITaskLoop*` 自动选路。

**它为什么安全，我去核了实现**——安全性来自 `clone` 语义，不是"引用调用方的对象"：

- `SMessageLoop::PostTask` 的实现是 `m_runnables.AddTail(runable->clone())`（`SOUI/src/core/SMsgLoop.cpp:257`）；
- `ITaskLoop::postTask` 与 websocket 的 `postServiceTask` 同样先 `clone`。

所以：**投递方在栈上构造的 functor，返回后立刻析构也不影响执行——你不需要 `new`，也不需要写锁。**

对照裸 Win32 的等价写法：自己 `new` 一个载荷结构、`PostMessage(hwnd, WM_APP+n, …)`、在窗口过程里 `delete`，还得处理"窗口先销毁、消息后到"的悬空指针。在 SOUI 下，这一整类样板与陷阱被一行 `post` 吸收。

边界也写清楚，免得误用：`PostTask` 是纯异步、无等待；`ITaskLoop::postTask` 的 `waitUntilDone=TRUE` 才同步等待（跨线程用信号量）——**同线程 + `waitUntilDone` 已被实现短路为直接 inline 执行**（`components/TaskLoop/thread/TaskLoop.cpp:69`），不会自锁；对象可能先于任务销毁时用 `cancelTasksForObject` / `cancelTask(id)`。

**跨目标一致性**：同一套 `ITaskLoop` 在五个宿主上都有后端——FreeRTOS 用任务 + 队列，桌面/移动用线程池，WASM 默认主线程按帧预算抽干、可选 worker。**"能不能用后台线程"是配置问题，不是 API 差异。**

---

## 八、和主流方案放一起看

| 维度 | SOUI | Qt 6 | Flutter | Electron | wxWidgets |
|---|---|---|---|---|---|
| 语言 | C++（+Lua） | C++（+QML/JS） | Dart | JS/HTML | C++ |
| 编程模型 | Win32/WTL 式 + XML | 自有 Q* 体系 | 声明式 Widget | Web DOM | 原生控件 + 跨 API |
| 跨平台代码复用 | **高（零平台宏）** | 高 | 高（需 Dart） | 高 | 中（API 随平台） |
| **运行目标数** | **8**（三桌面 + 三移动 + FreeRTOS + WASM） | 桌面 + 移动（无 OHOS） | 桌面 + 移动（无 OHOS） | 桌面 + 移动（借 Capacitor） | 桌面为主 |
| **裸机 MCU / 浏览器** | **都有实证**：ESP32-S3 真机 56~58 fps；浏览器跑 3 个应用（含真线程） | 均无 | 均无 | 浏览器本身 | 均无 |
| **GPU 接口** | **WGL 兼容面**（三端同源 + 契约测试） | 有（自有 API） | 有（内部） | 有（内部） | 有（wxGLCanvas） |
| **UI 自动化测试** | **有**：`souitest`（QTest 范式，41 条）+ 零成本绘制探针 | 有（QTest + Squish 商业） | 有 | 有（Playwright） | 弱 |
| 原生观感 | 自绘（Skia / Cairo / GDI / Canvas2D） | 自绘 / QPA | 自绘 | Web | **真原生控件** |
| 运行时依赖 | 无 VM，轻 | 无 VM，较重 | Dart VM | Chromium+Node | 系统原生 |
| 核心体积 | **~2.3 MB**；wasm 演示 5.5 MB | 数十 MB | 10 MB–数十 MB | 100 MB+ | 小（借系统） |
| 内存管理 | 类 COM 引用计数 | 父子树 / 智能指针 | GC | GC（V8） | 引用计数 / 手动 |
| 脚本/热更 | 内置 Lua 5.4 | QML/JS 内置 | 语言本身即脚本 | 天生 | 无内置 |
| 事件模型 | **映射表 + 订阅**，共用一份定义 | 信号槽（动态强，订阅散落） | 闭包/Builder | `addEventListener`（易泄漏） | MFC/WTL 映射表（缺订阅） |
| 资源与主题 | **运行期字符串键 + 提供器链尾部覆盖 + 内存提供器** | `.qrc` 编进二进制 | `pubspec` 声明即构建输入 | `asar` 整体替换 | 借系统 |
| 内置网页渲染 | 有：`SWkeWebkit` + WKE，运行期绑定 | 有（体积/许可成本高） | 无 | **天生** | 无 |
| 许可 | 自有商业（非商用免费） | LGPL / 商业 | BSD（引擎含限制） | MIT | LGPL / 商业 |
| 学习曲线（对 Win32 程序员） | **低** | 中 | 高（换语言） | 低（Web） | 中 |

**定位判断**：SOUI 的甜区是"**熟悉 Win32/WTL、想用一套 C++ 覆盖桌面 + 移动（含鸿蒙），并希望同一套代码还能下沉到嵌入式、上移到浏览器，且重视体积与原生渲染**"的团队。它最大的差异点是：**把 Windows 程序员的知识资产直接复用到全平台，而不是要求你换一门语言或换一套范式。**

---

## 九、什么情况下不要选它

推荐文如果只说好话，就没有参考价值。下面这些，我认为你应该在做选型之前就知道：

1. **许可要先看清**：自有商业条款——**非商用免费，商用收费**；`swinx` 内含 Wine 派生代码并单独披露。**如果你的项目是商用闭源产品，先把授权谈清楚，再谈技术。** 这是本清单里唯一的硬门槛。
2. **移动三端只有"编译门禁"**：Android / OHOS / iOS 现在都进了 PR 门禁（交叉编译 + iOS 静态库 Mach-O 复核），比九月份的"零自动构建"是一大进步；但**三端都不跑**——没有运行期回归，宿主库与 `PlatformAPI` 桥接只能靠人工验。这是当前最实的一条流程缺口。若你要长期依赖移动端，最好自己补一层"能跑起来"的回归。
3. **单作者项目（bus factor = 1）**：知识高度集中在一人身上，且边界知识（canvas2d 像素差异、GLX 时序、ESP32 内存仲裁、wasm 线程参数）进一步集中。缓解措施是 `AGENTS.md`（仓库级 + swinx 级）已把规则写成机器可读，但这是缓解，不是解决。
4. **迁移"既有 Windows 程序"目前不是零成本**（见 4.2）：窗口菜单栏未提供；原生控件只有 `Static`/`Button`/`ListBox`；没有资源对话框。**如果你要做的是"把既有 Win32 程序搬到多平台"，先按 4.2 逐条自查**——尤其注意 `CreateWindowEx` 建原生控件失败时是**返回 `NULL` 而不是编译报错**。若你想的是"用 SOUI 写一套新程序覆盖多端"，这一条不影响你。
5. **OHOS 无障碍在构建层被强制关闭**：`CMakeLists.txt:124` 里写着 `set(SOUI_ENABLE_ACC OFF)`。桌面三平台的无障碍是可用的（Windows 走系统 `oleacc`、macOS 走 NSAccessibility 桥、Linux 走 AT-SPI2 桥，约 6,240 行自研），但**如果鸿蒙端的无障碍是硬性合规要求，目前不满足**。
6. **FreeRTOS / WASM 是"能力证明级"，不是"通用发布质量"**：FreeRTOS 字形只烘了 ASCII（中文要按码位重烘）、`<edit>` 无输入法、光标不绘制、只有一块板一块屏、双动画 40 fps 仍低于单动画；WASM 的触摸与移动端浏览器未验证、系统级输入法与拖放未做、多窗口合成只有 swinx 层像素自测（没有烟测驱动下拉/菜单/tooltip）。**把它们当"能上产线"是误读，当"纸上推演"同样是误读。**
7. **OpenGL 面有明确语义上限**：无 MSAA（没有 `wglChoosePixelFormatARB`）、`wglShareLists` 只在资源创建前有效、单 drawable、macOS 只有 legacy/3.2/4.1 core 且 `NSOpenGLContext` 自 10.14 起 deprecated。
8. **vendor 代码体量大**：`third-part/` + `swinx/thirdparty/` 合计 **11,770 个受控文件，占两个仓库的 64.9%**。换来的是零版本兼容风险（cairo / fontconfig / freetype / xkbcommon / dbus / skia 全部按源码 vendor 化，构建不依赖发行版给什么版本），代价是 clone 体积与全文检索体验。
9. **配套细节的不完美**：错误码语义跨端不统一（`GetLastError`、`errno`、C++ 异常三者并存）；资源没有条目级失效；XML `on_*` 声明式绑定与脚本处理器段还缺直接用例；文档与示例偶有漂移（例如 demo 里两个从不参与编译的资源分支调用了不存在的 API 名）。

这 9 条里，**第 1 条是"能不能用"的问题，2、3 条是"你要接受或参与改善"的工程现实，4–9 条是可以规划的具体事项。**

---

## 十、如果我要评估它，两天怎么验

不用听完介绍就下结论，两天足够判断。

**第 1 天：验证"它能不能跑起来"（约 1–2 小时）**

- 拉仓库，按 `README` 构建桌面端；
- **跑测试门禁**——这一步我强烈建议做，因为它一次同时验证两件事："构建链是通的"和"框架自身有回归保障"：

```
ctest --test-dir build -L '^soui-headless$'
```

当前应当是 **633 条全部通过**（门禁总数 638 条，另外 5 条是需要显示环境的 GUI 烟测，不在这个标签里）。数字会随提交增长，九月是 325 条。

- 跑 `games/cnchess` 桌面端，联机下一盘棋，**观察 AI 思考时界面是否依然流畅**——那就是 7.2 那套异步任务在干活；
- 有浏览器的话，直接点开官网的 wasm 版三个应用，切页、拖动、在 Lua 游戏页按方向键、在编辑框里打几个汉字。**这一步不花你任何构建时间，却能最快判断"自绘 UI 的手感"你能不能接受：**

  - 控件画廊（十二页控件，最全）：<https://www.soui.com.cn/wasm/soui-wasm-demo.html>
  - 开心象棋（联网对战客户端）：<https://www.soui.com.cn/cnchess/wasm/cnchess-wasm.html>
  - 基金持仓助手（带真实在线取数）：<https://www.soui.com.cn/fundholdings/wasm/fundholdings.html>

**第 2 天：验证"它能不能装下你的需求"（这才是重点）**

1. **写一个你自己的窗口**：加一段 XML 布局、两个控件、一个点击事件。这一步验的是"XML + 事件映射表"这套开发模型你顺不顺手。
2. **把你现有代码里最"Windows"的那部分拿来试编译**——这是 SOUI 相对其他方案最大的潜在收益点，也是最该提前验证的点。**重点盯三件事**：① 有没有 `SetMenu` / `GetMenu` / `DrawMenuBar`（有 → 编译期就过不去）；② 有没有 `CreateWindowEx` 建 `EDIT` / `COMBOBOX` / `SysListView32` 这类**原生或公共控件**（有 → 编译能过、运行时窗口建不出来，返回 `NULL`）；③ 有没有 `.rc` 里的 `DIALOG` 模板配 `DialogBoxParam`（有 → 找不到符号）。**这三类只要中一条，就不是"改改就能编"，而要评估重写量**——把这部分 UI 改写成 SOUI 的 XML + 自有控件，通常是可行且推荐的方向。
3. **在你的目标平台上跑通构建**：目标是移动端或鸿蒙，直接看 `games/cnchess/client/{android,ohos}` 与 `demos/fundholdings` 是怎么接的——三个薄入口，照抄即可。目标是嵌入式，看 `soui-freertos/esp32s3/`，`BUILD_FLASH.md` 里是可粘贴的编译烧录命令，`PERFORMANCE.md` 里是帧率怎么测、怎么优化。
4. **回到第九节第 1 条，核对许可与你的商用场景。**

---

## 十一、结语

我不认为 SOUI 是"通吃"的方案，它的选择很明确：**不去抽象 Windows，而是让 Windows 成立。**

这带来一个别家给不了的收益——**Win32 程序员的知识与代码是资产**；也带来一个别家没有的成本——20 万行兼容层，以及一张**只覆盖了"SOUI 应用所需"、还没覆盖原生控件 / 窗口菜单栏 / 资源对话框的 API 面**。它对"最懂 Windows 的人"最友好，也正因此，它对"不懂 Windows 的人"最不友好。

十一月的这次进展，最有价值的不是"支持了 8 个平台"这个数字，而是它证明了两件事：

- **抽象能伸到"没有 OS"**：ESP32-S3 上 3.91 MB 固件、单动画 56~58 fps，跑的是完整 `SApp` + XML 布局 + 动画，上游只改 4 处；
- **抽象能伸到"浏览器沙箱"**：三个真实应用、双渲染后端、可开真线程，应用侧资源被压到 0.5 MB。

两端都落在 `swinx/src/platform/<name>/` 里，**内核改动都没超出显式 include 和一两个注册点**。这就是我敢说"再吸收一个新宿主形态也不会推翻设计"的依据。

它适合的场景也就很明确：

> **团队熟悉 Win32/WTL，需要一套 C++ 代码同时覆盖桌面与移动（含鸿蒙），并希望同一套代码还能下沉到嵌入式、上移到浏览器，且在意体积、启动速度与原生渲染。**

再补一句限定：这里的"需要一套 C++ 覆盖多端"，指的是**用 SOUI 来写这套程序**。如果你指的是"把手上那套既有 Win32 程序原样搬过来"，请先读完 4.2——**那件事目前还做不到；但它是一项工作量，不是一条死路。**

如果你正好在这个交集里，我认为它值得你花上面那两天——尤其是第 2 天的第 2 步。

---

**项目主页：**

- GitHub：<https://github.com/soui4/soui>
- Gitee：<https://gitee.com/setoutsoft/soui4>
- GitCode：<https://gitcode.com/setoutsoft/soui4>
- 官方网站：<https://www.soui.com.cn>

---

*本文数据取自 2026-11-17 的源码实测与真机参数（详见仓库内 `doc/SOUI4项目评估报告-2026-11-17.md`）：规模用 `git ls-files` 统计受控文件、按字节计行数；`swinx` 的 API 计数取自 `swinx/doc/tools/api_scan.json`；测试数取自 `ctest -N`；帧率取自 ESP32-S3 串口实测与 `esp32s3/PERFORMANCE.md`；wasm 性能取自同机两版交替跑的 A/B 记录。文中引用的每一处源码位置（`MainDlg.cpp:314`、`ChessAI.cpp:23`、`ChsAIEngine.cpp:803`、`TaskLoop.cpp:69`、`SMsgLoop.cpp:257`、`CMakeLists.txt:124` 等）均已逐条核对。数字会随提交变化，看结论时请以仓库当前 HEAD 为准。*
