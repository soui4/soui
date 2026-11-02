# WebAssembly 演示：swinx 裸窗口 与 SOUI XML

> **目录位置**：`demos/wasm-demo/`（原先在仓库根的 `soui-wasm/`）。下面命令里的 `<soui4>` 指仓库根；
> 两个构建脚本都用 `HERE/../..` 自己解析仓库根，所以从任何工作目录调用都行，不用先 `cd` 到根。
> 根 `CMakeLists.txt` 在 `SOUI_BUILD_WASM_DEMO=ON` 时 `add_subdirectory(demos/wasm-demo)`。

本目录有两个演示，共用同一套宿主接缝（帧缓冲 / 上屏回调 / 输入注入 / 消息泵）：

- **阶段 0：swinx 裸窗口**（`main.cpp` + `index.html` + `build.sh`）——只用 swinx（`libswinx.a` +
  cairo/pixman/fontconfig/freetype/expat/png/zlib/brotli），不链 SOUI 上层，在浏览器里跑一个 Win32
  风格的窗口：`RegisterClassA`/`CreateWindowExA` 建窗、`PeekMessage` 泵、`BeginPaint`/`EndPaint` 里画
  GDI 图元、JS 宿主把脏矩形 blit 到 canvas、DOM 事件注入回窗口队列。它同时是 wasm 移植四个宿主接缝
  的最小可运行样例。
- **阶段 2 起：SOUI XML 展示页**（`soui_main.cpp` + `soui.html` + `build-soui.sh` + `uires/`）——
  同一套宿主接缝之上换成 `SApplication` + `SAppCfg::DoConfig()` + `SHostWnd`，布局来自
  `uires/xml/dlg_main.xml`，渲染器 `render-gdi` 与图片解码器 `imgdecoder-stb` 静态编进模块。
  页面按 SOUI 的能力分成十二页 `<tabctrl>`（概览 / 基础控件 / 更多控件 / MVC / 列表与树 / 布局 /
  图形控件 / 表格 / 图表 / 编辑器 / 扩展控件 / Lua 游戏），可以整包发到官网上当在线演示。其中非核心的几页分别靠
  `richedit41`、`Scintilla`、`controls.extend`、`ScriptModule-LUA` 静态链入；后两项各有一个独立开关
  `SOUI_BUILD_WASM_LUA` / `SOUI_BUILD_WASM_SCINTILLA`，见"两个可选能力的编译开关"一节。

## 文件

| 文件 | 作用 |
| --- | --- |
| `main.cpp` | 阶段 0 演示程序：窗口过程、GDI 绘制、脏矩形失效、导出给 JS 的统计量与注入入口 |
| `index.html` | 阶段 0 的 Emscripten `--shell-file`，即 JS 宿主：canvas、上屏 blit、输入注入、数据面板 |
| `build.sh` | 调 `em++` 链接 `libswinx.a` 与 vendor 静态库，产出 `out/swinx-wasm-demo.{html,js,wasm}` |
| `soui_main.cpp` | 阶段 2 演示程序：`SApplication` + `SHostWnd` 子类 + 事件映射表 + 数据控件填充（MVC / gridctrl / HelloChart / grouplist），以及「宿主接线」一节的 6 个导出 |
| `soui-host.js` | **可复用的浏览器宿主**（本仓库两个 wasm 程序通用）：下载进度、上屏、鼠标/键盘/滚轮注入、输入法代理、剪贴板、WebAudio、崩溃上报。新程序引这一个 JS 即可，见下面「复用宿主」 |
| `soui.html` | 阶段 2 的 `--shell-file`，现在只剩页面排版 + 一行 `SouiHost.start({...})`（宿主逻辑都在 `soui-host.js`） |
| `build-soui.sh` | CMake 包装器：查 `em++`、确保构建目录打开了 `SOUI_BUILD_WASM_DEMO`、`cmake --build --target soui-wasm-demo`。编译与链接命令本身在 `demos/wasm-demo/CMakeLists.txt` |
| `uires/` | SOUI 资源树：`xml/` 布局与控件、`values/` 字符串/颜色/皮肤/模板、`anim/` 与 `animator/` 动画、`svg/` 皮肤、`lua/` 三个游戏脚本，以及生成的 `uires.idx`；预载范围受"两个可选能力的编译开关"影响（关掉 Lua/Scintilla 时对应脚本与源文件不进 `.data`），页面片段说明见各节 |
| `out/` | 默认（调试）构建的产物，即 `-O2 -g`（未纳入版本控制） |
| `release/` | 上线档的产物（`-O2 -DNDEBUG` 或 `MinSizeRel`，见下面"上线前的编译参数"）；`.data`/`.html`/`.js`/`.wasm` 四个文件必须放在一起，`.data` 按当前工作目录找（未纳入版本控制） |

## 复用宿主：soui-host.js（新程序只写 C++）

页面侧的东西全部收在 `soui-host.js` 里，一个 Emscripten `MODULARIZE` 产物只要在页面上写三行：

```html
<script src="soui-host.js"></script>
<div id="app"></div>
<script>
  SouiHost.start({
    container: document.getElementById('app'),
    wasm: 'myapp.wasm',       // 胶水 .js 与 .data 由它推导（.wasm → .js / .data）
    factory: 'createMyApp',   // emcc -sEXPORT_NAME=createMyApp
  });
</script>
```

它会自己建画布、加载遮罩、输入法代理框和错误行，并注入对应的 `.soui-*` 样式（`injectCss:false`
可关）。宿主做的事：

| 能力 | 说明 |
| --- | --- |
| 下载与进度 | 自己 `fetch(…,{cache:'no-cache'})` 拉 `.wasm`/`.data`，走 `ReadableStream` 显示百分比/速度/剩余时间，再经 `getPreloadedPackage` + `instantiateWasm` 交给加载器 |
| 胶水载入 | 动态 `<script>` 带 `?v=<时间戳>`，避开"新 wasm 配旧胶水"的启发式缓存；实例化前用 `WebAssembly.Module.imports()` 预检，缺项时报人话而不是裸 `LinkError` |
| 上屏 | 三条路径按 `Module.fbFormat` 选：`'canvas'`（canvas2d 后端已由 C++ `drawImage` 合成，JS 只统计）/ `'rgba'`（零拷贝 `ImageData` + 单次 `putImageData`）/ 兜底 ARGB 换序 |
| 输入 | 鼠标（含 `pointerup` 挂 window、失焦清按键）、滚轮（攒满 `WHEEL_DELTA` 才注入）、键盘（按 `e.code` 算 VK、每次注入后同步抽干消息队列） |
| 输入法 | 透明 `textarea` 代理 + `compositionend`/`input(insertText)` 双路提交，代理框每帧钉在 C++ 报的插入符坐标上 |
| 剪贴板 | `paste` 同步读 `clipboardData` 再注入 Ctrl+V；反向走 `Module.souixClipboardWrite` |
| 音效 | `Module.swinxAudioPlay` → WebAudio，autoplay 未放行时排队、`onstatechange` 冲刷 |
| 崩溃 | `window.onerror` / `onunhandledrejection` 把 `err.stack` 打到控制台，遮罩显示一句人话 |

C++ 侧要提供的导出（名字可用 `api:{...}` 覆盖，缺了对应功能自动降级、不抛异常）：

```
_SwinxDemoPump _SwinxDemoMouse _SwinxDemoKey _SwinxDemoWheel
_SwinxDemoText _SwinxDemoCaretPos _SwinxClipboardSetBrowserText
_malloc _free _SwinxFbBits _SwinxFbWidth _SwinxFbHeight _SwinxFbStride
```

外加两条 `EM_ASM` 回调：`Module.swinxPresent(l,t,r,b)`（脏矩形上屏）与 `Module.souiFatal(str)`
（启动失败）。`soui_main.cpp` 的「宿主接线」一节就是这部分的完整实现，新程序抄过去即可。

常用配置项：`container` / `canvas` / `canvasId`（默认 `screen`）/ `glue` / `wasm` / `data` /
`factory` / `width`、`height`（CSS 显示宽度，缺省取 C++ 的逻辑尺寸）/ `injectCss` / `loading` /
`log` / `ime` / `audio` / `api` / `loadingTitle` / `onFatal` / `onLog`。
`SouiHost.start()` 返回 `{canvas, module, fail, whenReady, injectText, requestFocus}`，
`whenReady` 在第一帧上屏后 resolve；`window.Module` 与 `SouiHost.current` 也留着给控制台和
`tools/diag/*` 用。

## 构建与运行（阶段 0）

```bash
source "$EMSDK/emsdk_env.sh"

# 1) 先出 swinx 的 wasm 静态库（必须 -DSOUI_ENABLE_CORE_LIB=ON，见下文"静态库"）
emcmake cmake -S <soui4>/swinx -B /tmp/build-swinx-wasm \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_PLATFORM=wasm -DSOUI_ENABLE_CORE_LIB=ON
cmake --build /tmp/build-swinx-wasm --target swinx -j

# 2) 链接演示（构建目录也可用 SWINX_WASM_BUILD 环境变量指定）
demos/wasm-demo/build.sh /tmp/build-swinx-wasm

# 3) 起静态服务，浏览器打开 http://localhost:8090/swinx-wasm-demo.html
python3 -m http.server 8090 -d demos/wasm-demo/out
```

必须经 HTTP 打开：`file://` 下浏览器不允许 `fetch`/`streaming` 取 `.wasm`，页面会停在空白。

## 页面上能看到什么（阶段 0）

画布是 1:1 像素（CSS 尺寸跟着帧缓冲走），里面几组图形分别对应不同的验证点：

- 顶部标题带：配色随按键/右键切换，验证 `FillRect` 与调色板。
- 左上进度条 + 转盘：由 `SetTimer(hWnd,1,50,NULL)` 驱动，验证 wasm 的 `WM_TIMER` 是从
  `GetTickCount()` 在 `peekMsg()` 里补进队列的，纯 `PeekMessage` 泵也能拿到定时消息。
- 十字光标：跟随 `WM_MOUSEMOVE`，每次只在旧位置和新位置各失效一块 24×24。
- 圆点标记：左键 `WM_LBUTTONDOWN` 落下，攒满 24 个整窗重绘一次。
- 右下圆角方块：方向键移动，验证 `WM_KEYDOWN`/`WM_KEYUP` 与 `GetKeyState` 一致性。

面板三块数据：

- **wasm 侧计数**（`SwinxDemoStat`）：泵轮数、`WM_PAINT`/`WM_TIMER`/鼠标/按键/`WM_CHAR` 次数、
  `GetTickCount`、上一次泵是否被 100ms 预算截断（`17`）。
- **JS 宿主侧**（脏矩形上屏）：上屏次数、本次脏矩形、本次像素数、累计拷贝像素、**节省比例（相对整帧拷贝）**、面板 FPS。
  动画稳定运行时节省比例约 84%（300×250 条带 vs 800×600 整帧）。
- **操作**：页面上鼠标和键盘直接作用到画布，另有"全窗重绘 / 注入一次中心点击 / 注入一次方向键"三个按钮。

## 四个宿主接缝（两个演示共用）

1. **帧缓冲**：`SwinxFbBits()/Width()/Height()/Stride()`，ARGB32（内存字节序 B,G,R,A，小端读成
   `0xAARRGGBB`），尺寸是 `swinx/wasm.cmake` 里的 `SWINX_FB_WIDTH/HEIGHT` 缓存变量，编译期烘进
   `libswinx.a` 的 `.bss`。JS 侧只按导出函数取值，不写死尺寸；canvas 尺寸必须与之一致。
2. **上屏**：`EndPaint -> ReleaseDC -> commitCanvas()` 请求一次上屏——端口只在这一步解析裁剪并把
   窗口局部矩形并进待上屏队列，真正的 blit 延到最外层消息派发结束（`AfterProcMsg`）或下一次进泵时，
   再调 `SwinxSetFbPresentCb()` 注册的回调，`EM_ASM` 把 `(l,t,r,b)` 交给 `Module.swinxPresent`，
   JS 逐行 swizzle 成 RGBA 后 `putImageData` 局部上屏。一次绘制周期里落在同一块区域的多次请求
   合并成一次回调；互不相交的区域仍分别上屏（并起来是更大的包围盒，见"坑与已知限制"）。
3. **输入**：`SwinxInjectMouse/SwinxInjectKey`（演示里由 `SwinxDemoMouse/SwinxDemoKey` 包一层）
   收**屏幕坐标**，内部按 `wndObj->rc` 换算 client，并把消息投进目标窗口所在线程的队列，
   所以宿主在哪个线程回调都不影响投递目标。滚轮另包一层 `SwinxDemoWheel`：它和鼠标共用注入
   入口，但 `WM_MOUSEWHEEL` 的 `wParam` 高字是带符号滚动量、低字才是 `MK_` 键位，坐标系
   仍是屏幕坐标（`SwinxInjectMouse` 把这个值原样交给窗口，打包得在调用前做）。
4. **消息泵**：`emscripten_set_main_loop(SwinxDemoPump, 0, 1)` 由 requestAnimationFrame 驱动，
   泵内只做 `PeekMessage + TranslateMessage + DispatchMessage`。浏览器主线程不能阻塞，
   所以演示不用 `GetMessage`；也因为一次调用必须返回，`SwinxDemoPump` 只工作 100ms 预算就带着
   “队列还有消息”的标记返回，剩下的留给下一帧——不限预算时，页面上任何一个
   `repeatCount="-1"` 的动画都会把这条泵变成永不返回的死循环，见"阶段 7 特有的坑"。

## 已验证（阶段 0）

Linux x64 + Emscripten 6.0.13：

- `build.sh` 无警告链接（除 swinx 既有的 sign-compare/format 警告）。
- 无头 node 跑通：`main()` 建窗即重绘、`WM_TIMER` 连续触发、鼠标/键盘注入计数正确、
  脏矩形分布为 `800x600`（启动整帧）→ `300x250`（动画条带）→ `24x24`（指针点）→
  `76x60`（方块移动），emscripten 的栈/堆金丝雀 `HEAPU32[0] == 'emsc'` 全程完好。
- Chromium 实测：加载后无异常中止，动画运行，鼠标移动/点击与方向键都有响应，
  节省比例约 84%。控制台只有一条 `readlink: No such file or directory`，见下文。

无头验证脚本（不需要浏览器，能覆盖渲染链之外的全部逻辑）：

```js
const create = require('./out/swinx-wasm-demo.js');
let presents = 0;
create({ noExitRuntime: true, swinxPresent: () => ++presents,
         onRuntimeInitialized: () => {} })
  .then((m) => {
    m._SwinxDemoMouse(0x0200, 300, 400, 1);   // WM_MOUSEMOVE
    m._SwinxDemoMouse(0x0200, 420, 460, 1);
    m._SwinxDemoMouse(0x0201, 500, 500, 1);   // WM_LBUTTONDOWN
    m._SwinxDemoKey(0x0100, 0x27, 0);         // WM_KEYDOWN VK_RIGHT
    for (let i = 0; i < 20; i++) m._SwinxDemoPump();
    console.log('paints', m._SwinxDemoStat(1), 'timers', m._SwinxDemoStat(2), 'presents', presents);
  });
```

## 坑与已知限制（阶段 0）

- **导出函数的"假可用"**：Emscripten 在实例化前就把 `Module._Xxx` 全部装成
  `makeInvalidEarlyAccess` 占位函数，`typeof === 'function'` 和真值判断都会通过，调用时才抛
  "reference taken before Wasm module initialization"。`index.html` 因此用一个显式 `ready`
  标志（由 `Module.onRuntimeInitialized` 置位）守住上屏、输入和面板三条入口，并在 ready 时
  主动做一次整帧 blit——帧缓冲里 `main()` 已经画好，宿主是"接管"而不是"重启"。
- **像素格式**：帧缓冲是预乘 ARGB32，`ImageData` 要非预乘 RGBA，JS 侧做
  `out = (v & 0xFF00FF00) | ((v & 0xFF) << 16) | ((v >> 16) & 0xFF)`。本演示像素全部
  alpha=255，预乘与直色等价；引入半透明图层后这行换算必须改成先除 alpha。
- **没有文字**：浏览器里没有 `/etc/fonts` 和系统字体目录，fontconfig 找不到任何字体，
  `TextOut` 只会画空白。这个演示刻意用形状 + JS 面板表达状态；预置 TTF 的做法见下文
  阶段 2 一节（`build-soui.sh` 的 `--preload-file` 与字体候选列表）。
- **`libswinx.a` 必须是静态库**：brotli 会强制 `BUILD_SHARED_LIBS OFF`，此时共享 cairo 在
  wasm 链接期报 `relocation R_WASM_MEMORY_ADDR_LEB cannot be used against symbol kBrotliBitMask`。
  另外 freetype 只 `PRIVATE` 链 `brotlidec`，而 brotli 自己在 Emscripten 下省略
  `brotlidec -> brotlicommon` 这条边，`libbrotlicommon.a` 得由 `swinx/wasm.cmake` 显式拉起来。
- **两侧尺寸必须同源**：帧缓冲大小改 `SWINX_FB_WIDTH/HEIGHT` 后要重配 swinx 构建目录并重跑
  `build.sh`（它从 `CMakeCache.txt` 读回同一组值），否则 canvas 与帧缓冲错位。
- **首条 `WM_MOUSEMOVE` 不生效**：swinx 的 hover 进入/离开状态机在 `htCode` 首次变成
  `HTCLIENT` 时置 `bSkipMsg`，把这条消息转成一次 `WM_NCMOUSELEAVE`，与 Win32 一致。
  演示的"鼠标移动次数"因此从第二条起才涨。
- **~~方向键会产生一条 `WM_CHAR`~~ 已修**：wasm 的 `SConnection::TranslateMessage` 原先把
  `vk ∈ [0x20,0x7e]` 直接当字符发出去，而 `VK_LEFT..VK_DOWN` 是 `0x25..0x28`，于是方向键、
  `PageUp`、`Insert/Delete`、`F1..F4`、小键盘数字全都会伪造字符，`Enter/Tab/Backspace`
  反而一个都不发。现在按 Win32 的分工做：注入侧报**物理键位 VK**，端口按 US 布局结合
  Shift/CapsLock 状态算字符（当初由无头烟测逐键断言并 A/B；烟测已移除）。仍缺的是中文组合输入
  （`composition* → ImmGetCompositionString`），见 `doc/soui-wasm-port-plan.md` §12。
- **~~一次重绘有三次同矩形上屏~~ 已修**：框架的 `WM_PAINT` 默认处理、应用的 `BeginPaint/EndPaint`
  和 `WM_NCPAINT` 各走一次 `ReleaseDC -> commitCanvas`，memcpy 幂等所以画面正确，但拷贝与 JS
  回调是重复的。现在 `commitCanvas()` 只在请求期解析裁剪、把窗口局部矩形并进待上屏队列，
  blit + 回调延到最外层消息派发结束（`AfterProcMsg` 递归深度归零）与各条泵入口，一次绘制
  周期收敛成一次上屏（实测每帧 commit 请求 2 次、上屏 1 次）；`SwinxFbRequests()/SwinxFbPresents()`
  可被宿主读出来，当初由无头烟测做增量断言并 A/B（烟测已移除）。
  **合并只对重叠的矩形做**：无条件取并集会让互不相交的小块（动画条带 300×250 + 光标 24×24 两块）
  并成一个空旷包围盒，同一输入脚本下 18 次上屏的累计拷贝从 1,461,708 像素涨到 2,258,856（+54.5%）
  而次数一次没省，所以现在按面积判据合并，不相交时先上屏已欠的那块再重新记账。
  见 `doc/soui-wasm-port-plan.md` §13。
- **`hInstance` 只是占位**：演示传 `(HINSTANCE)0x1000`，wasm 上 `RegisterClassA` 不校验它。
- **控制台那条 `readlink`**：`GetModuleFileNameA(NULL,...)` 在无 `/proc` 的浏览器里必然失败并
  `perror`，属预期输出；`DllLoader` 的构造已经容忍这个失败（拿不到可执行文件路径时只保留
  "按当前目录搜索"），不要再当成崩溃线索。

---

## SOUI XML 演示（阶段 2）

同一套宿主接缝之上跑真正的 SOUI：`SApplication` + `SAppCfg::DoConfig()`（渲染器
`components/render-gdi`、图片解码器 `components/imgdecoder-stb`，都是静态编进模块）+
`SHostWnd` 装载 `uires/xml/dlg_main.xml`，资源由 `SResProviderFiles` 从 MEMFS 里的 `uires.idx`
读取。移植侧的完整实施记录、构建图取舍与跨平台待复验清单在
`doc/soui-wasm-port-plan.md` §11，这一节只写"怎么跑起来"和"页面上哪一块证明哪条链"。

### 构建与运行

```bash
source "$EMSDK/emsdk_env.sh"

# 1) 配置 + 构建。一条 CMake 命令出全部静态库与演示目标（链接命令、--preload-file、
#    字体探测、导出符号都在 demos/wasm-demo/CMakeLists.txt 里，不依赖 bash）
emcmake cmake -S <soui4> -B /tmp/build-soui-wasm -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_DEMO=ON
cmake --build /tmp/build-soui-wasm --target soui-wasm-demo

# 2) 起静态服务，浏览器打开 http://localhost:8081/soui-wasm-demo.html
python3 -m http.server 8081 -d demos/wasm-demo/out
```

`demos/wasm-demo/build-soui.sh /tmp/build-soui-wasm` 是上面第 1 步的包装器（老构建目录没开
`SOUI_BUILD_WASM_DEMO` 时它会补一次配置），保留它只是为了不改变既有的脚本用法。
Windows 主机用 `demos\wasm-demo\build.bat`（同样的流程与环境变量：`SOUI_DEMO_FONT` /
`SOUI_DEMO_STACK` / `SOUI_WASM_DEMO_OUT` / `SOUI_BUILD_WASM_LUA` / `SOUI_BUILD_WASM_SCINTILLA`，
另加 `SOUI_WASM_JOBS` 覆盖并行度——Windows 上 emcc 高并发会挂死，默认 `-j2`）。
该文件**必须保持纯 ASCII**：cmd 按当前代码页逐行解码批处理，UTF-8 中文注释会被拆成
幽灵命令，`chcp 65001` 也修不干净（分块读取仍会截断多字节序列）；中文说明看本文件和 build-soui.sh。

`SOUI_BUILD_WASM_DEMO` 默认关：只有 Emscripten 构建并且显式打开才进入 `demos/wasm-demo`，
桌面构建图与改动前一致。演示目标不在 `SOUI_BUILD_DEMOS` 里——那个开关在 EMSCRIPTEN 下恒关。

三个和阶段 0 不同的前提：

- **字体是演示的一部分**。CMake 在构建机上按候选顺序找一份**同时覆盖拉丁与 CJK** 的
  TTF/TTC（Windows 主机的候选是 `%WINDIR%\Fonts\msyh.ttc` → `simsun.ttc` → `Deng.ttf` →
  `arial.ttf`；macOS 主机是 `PingFang.ttc` → `STHeiti Medium.ttc` → `Supplemental/Arial Unicode.ttf`），
  `--preload-file` 成 `/opt/soui/fonts/demo.ttf`，程序里用 `AddFontResource()` 交给 fontconfig。
  找不到也能出包，只是文字没有字形（面板"字体注册"停在 0）。别用
  `DroidSansFallbackFull.ttf`：它是纯 CJK 兜底字体，cmap 里一个 ASCII 都没有，整页英文会变成豆腐块。
  用 `-DSOUI_DEMO_FONT=C:/Windows/Fonts/msyh.ttc`（或环境变量 `SOUI_DEMO_FONT`）指定别的字体。
- **`.data` 按当前工作目录找**，浏览器侧无感（`fetch` 按同目录解析），node/无头用法才需要 `locateFile`。
- **内存布局是钉住的，而且不合法时会当场报出来**：链接期显式加 `-Wl,--stack-first`。`-O1` 及以上
  emcc 默认换成 `--no-stack-first --global-base=1024`，数据段会落到 64 KB 以下，swinx 的
  `IS_INTRESOURCE` 判据把窗口类名当成原子号，`SHostWnd::Create()` 直接失败且 `err=0`。根因与 A/B 见
  `doc/soui-wasm-port-plan.md` §15.3。swinx 在第一次登记窗口类 / 第一次建 `SConnection` 时会自检这一前提，
  不满足就打印数据段下界与修法然后 `abort()`——看到 `[swinx/wasm] 致命：静态数据落在低 64 KB`
  就去查链接选项，别去查 XML。改链接选项时注意 `--global-base=` **必须带 `-Wl,` 前缀**，
  裸写会被 emcc 的默认值覆盖（等于没写）。

### 壳页：顶栏信息与加载进度

> 下面这些行为都实现于 `soui-host.js`（`soui.html` 只剩排版与一行 `SouiHost.start`），
> 复用到别的程序时原理不变。

`soui-host.js` 在画面出来之前先自己把两个产物文件拉进内存，再交给 Emscripten 加载器，所以下载阶段
有真实的百分比、速度与剩余时间；这一步不是纯 UI，加载器本身拿不到这些进度：

- **为什么不能等加载器报进度**：`.wasm` 的下载在工厂闭包里（`wasmBinary` 是局部变量，没有 `Module`
  桥），`.data` 只暴露 `dataFileDownloads` 这种"开始/结束"事件，没有分块回调。壳页因此先
  `fetch(name, {cache: 'no-cache'})` 带 `ReadableStream` 逐块累加，再把 `ArrayBuffer` 通过两个
  官方钩子交进去：`Module.getPreloadedPackage = () => dataBuf`（同步返回，跳过加载器自己的 `.data`
  下载）与 `Module.instantiateWasm = (imports, cb) => Promise`（跳过它自己的 `.wasm` fetch）。
  两个钩子都在 MODULARIZE 工厂内、`onRuntimeInitialized` 之前求值，必须在 `createSouiDemo()`
  之前挂上。
- **`cache: 'no-cache'` 是必须的**：产物文件名固定，浏览器会直接复用旧缓存，换构建后仍跑上一版
  （这一条曾被误判成控件缺陷）。注意这是"仍会走缓存但先校验"，不是禁用缓存。
- **遮罩本身在 `#screen-wrap` 里、盖住 canvas**，而不是另开一层页面：失败信息（下载失败、实例化
  失败、`DoConfig` 失败）同时写进遮罩的红色状态与 `#log`，避免"一直卡在加载"和"画面黑屏"两种
  现象被混为一谈。下载完成后阶段文字切到"正在编译并初始化 SOUI…"，`onWasmReady()` 结束时才淡出。
- **顶栏的版本号来自 C++**：XML 里 `app_brand` 只写占位词，`main()` 在 `ShowWindow` 之后调
  `fillBrand()`，用 `SOUI_VER1..4`（`SOUI/include/soui-version.h`）格式化成 `SOUI 5.3.2.1`；
  QQ 群与渲染信息是 `values/string.xml` 里的静态串。顶栏宽度是按 800px 帧缓冲排过的，改文案时
  注意一行放得下。

### 上线前的编译参数（体积）

默认那条配置命令是 `-DCMAKE_BUILD_TYPE=RelWithDebInfo`，也就是 `-O2 -g`：产物里 **87.4% 是调试信息**
（`.debug_*` + `name` 共 48,021,903 / 54,933,142 B），真正的代码只有 5.5 MB。上线换一档就行：

```bash
# 推荐：MinSizeRel（阶段 6 末实测整包 gzip 32.95 MB → 17.79 MB，本机测不出相对 -O2 的重绘代价）
emcmake cmake -S <soui4> -B /tmp/build-soui-min -G Ninja \
    -DCMAKE_BUILD_TYPE=MinSizeRel -DSOUI_BUILD_WASM_DEMO=ON \
    -DSOUI_WASM_DEMO_OUT=<soui4>/demos/wasm-demo/release
cmake --build /tmp/build-soui-min --target soui-wasm-demo

# 备选：不想动优化级别，只去掉 -g、补上 -DNDEBUG（整包 17.95 MB，比上面多 0.9% gz）
emcmake cmake -S <soui4> -B /tmp/build-soui-ship -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O2 -DNDEBUG" \
    -DCMAKE_C_FLAGS_RELWITHDEBINFO="-O2 -DNDEBUG" \
    -DSOUI_BUILD_WASM_DEMO=ON -DSOUI_WASM_DEMO_OUT=<soui4>/demos/wasm-demo/release
cmake --build /tmp/build-soui-ship --target soui-wasm-demo
```

两档都往 `demos/wasm-demo/release/` 写，**跑第二条会覆盖第一条的产物**——按选定的那一档只跑一遍。
出包后原地验一次：`python3 -m http.server 8088 -d demos/wasm-demo/release` 后打开 `http://localhost:8088/soui-wasm-demo.html`。

实测对照（gzip/brotli 是 `.wasm`+`.data`+`.js`+`.html` 四个文件的整包，本机 emsdk）：

| 配置 | `.wasm` | 整包 gz | 备注 |
| --- | --- | --- | --- |
| `-O2 -g`（默认，调试用，**含 SVG + 三个游戏页 + Lua 计时器派发修复 + MVC 500 行富模板与子页签 + MVC 行内进度条 SVG 胶囊皮肤 + waveProgress 双层波 + scrolltext 两种模式 + 壳页加载进度 + 滚轮交给 SOUI**） | 54,933,142 | 33.31 MB | 87.4% 是调试信息，别直接上线 |
| 同上但 `SOUI_ENABLE_SVG` OFF（阶段 7 早前，主循环预算修复之前） | 54,215,867 | — | SVG 的代码代价 +570,403 B（+1.05%）；`.data` 与上面**逐字节相同**（`md5 c198940b…`，`uires/svg/` 不管开关都预载）。**这行要重测必须改根 `CMakeLists.txt:171`**，见下面"SVG 开关在 wasm 下被强制打开" |
| `-O2 -DNDEBUG` | 6,315,766 | 18.23 MB | **当前 `demos/wasm-demo/release/` 里放的就是这一档**，含 SVG、三个游戏页、主循环预算与 Lua 计时器派发修复、MVC 500 行富模板与子页签、MVC 行内进度条 SVG 胶囊皮肤、waveProgress 双层波、scrolltext 两种模式、壳页加载进度与滚轮接收权；2026-10-31 迁移到根目录 `demos/wasm-demo/` 后重测（gz 一律按 `wasm+data+js+html` 四个文件各自 `gzip -9` 相加；`.data` 20,012,101 B） |
| `Release`（`-O3 -DNDEBUG`） | 6,318,469 | 17.97 MB | 比 `-O2` 还多 66 KB，不推荐；阶段 6 末实测 |
| `MinSizeRel`（`-Os -DNDEBUG`） | 5,356,946 | **17.79 MB** | 阶段 6 末实测（当时 65/66） |
| `MinSizeRel -flto` | 5,597,872 | 17.84 MB | **更差，别开**；阶段 6 末实测 |
| `MinSizeRel` + 链后 `wasm-opt -Oz` | 5,284,679 | 17.78 MB | 只省 0.1%，不加这道工序；阶段 6 末实测 |

表里除前三行外都是**阶段 6 末**（没有 SVG 皮肤与三个游戏页时）的快照，用来定"选哪一档编译参数"
的相对结论；那部分结论不受新增代码影响（新增的是 Lua 游戏脚本 + nanosvg 分支，量级见前两行的
1%）。各档的**绝对** `.wasm` 要以重新构建当次为准，`demos/wasm-demo/release/` 会用推荐的档位刷新。

四件事要知道：

- **换档位不为难性能**：11 页轮转整帧重绘的基准里，`-O2` / `-O3` / `-Os` 中位数是 15.68 / 15.45 /
  16.15 ms 每页，相差 4%，而同一档位重复跑的内部极差就有 39%——本机测不出差异，按体积选就行。
  真机（手机浏览器 + 移动端 SoC）没测。

- **`-flto`、链后 `wasm-opt`、`--closure 1` 都不划算**：前者 +0.3% gz，中者 −0.1%，后两者只动
  `.js`/`.html`（占整包 gz 的 0.19%）。`-fno-exceptions`/`-fno-rtti` 不要用——`components` 有 36 处
  `catch`、`controls.extend` 有 12 处 `dynamic_cast`，全局关会在运行期出事。
- **`-O1` 及以上 emscripten 会压缩 `--shell-file` 生成的 html**，包括里面那段宿主 JS。这暴露过一个真缺陷：
  压缩器把 `commitImeText` 里的 `'\0'` 落成裸 0x00 字节，浏览器按 HTML 规范把 script 数据中的 NUL 换成
  U+FFFD，输入法每提交一次就往文档里多塞一个乱码（`compositionend('你好')` 实测回落
  `[20320,22909,65533]`）。现在终止符改成写进堆（多分配一字节再写数值 0）。**往 wasm 传 C 字符串时
  别在壳页里用 NUL 字面量**；这类问题只在浏览器里能发现（node 脚本不经过 HTML 解析器）。
- **改完之后上线档整包 gzip 的 86.8% 是 `.data`**（20,012,101 B，其中 97.4% 是构建机自动探测到的
  `NotoSansCJK-Regular.ttc`，字体自己 gzip 后 15.55 MB，占 `.data` gz 的 98.4%）。编译参数已经没有数量级空间，再要降只有换
  字体子集一条路，那由 `-DSOUI_DEMO_FONT=` 决定。

赶时间又不想重编时，可以对现有产物剥一次调试段（本机验证过功能不变）：

```bash
/home/flyhigh/emsdk/upstream/bin/llvm-objcopy \
    --remove-section='.debug_*' --remove-section='name' \
    soui-wasm-demo.wasm soui-wasm-demo.strip.wasm   # 54.8 MB → 6.90 MB（整包 gz 33.13 MB → 18.19 MB，−45.1%）
```

### 两个可选能力的编译开关

`SOUI_BUILD_WASM_LUA` 与 `SOUI_BUILD_WASM_SCINTILLA`（根 `CMakeLists.txt:179-180`，默认都 ON）
把两块"体积大、但不是所有人都要"的能力做成独立开关：

```bash
# 只要核心 + 控件，不要 Lua 与 Scintilla
SOUI_BUILD_WASM_LUA=0 SOUI_BUILD_WASM_SCINTILLA=0 \
    bash demos/wasm-demo/build-soui.sh /tmp/build-soui-wasm
```

`build-soui.sh` 把这两个环境变量（以及 `SOUI_WASM_DEMO_OUT`）转成 `-D` 写进缓存，其余选项保持不变。
关掉的 effects 是三处一起生效的：

- **构建图**：`lua-54` / `ScriptModule-LUA` / `translator` 与 `Scintilla` + `controls.extend` 的
  `<scintilla>` 不再进入链接；`uires/lua/` 也不再有 `--preload-file`。
- **页面**：Lua 页留一行 `lua_hint` 说明"本构建没带 Lua 脚本模块"——游戏页的三张子页是 XML，
  照常建出来，但棋盘是空的（格子全部由脚本在 `on_init` 里建，脚本没编进来就没有格子）；
  编辑器页的 `<scintilla>` 标签被静默跳过（SOUI 查不到注册名就是跳过，不报错），由 `edit_hint`
  写明"本构建没带 Scintilla"。**没有这两行说明，"开关没生效"和"开关生效了"在画面上长得一样。**
- **验证**：当初的 `smoke.js` 用 `SwinxDemoFeatureMask()` 探针切换断言集，并在关掉的分支上
  也有断言（`lua_hint`/`edit_hint` 说明行存在）；烟测与探针已随测试设施移除。

阶段 5 末实测两种构建（同一份 XML/资源，只差这两个开关）：

| 构建 | mask | `.wasm` | `.data` | 断言 |
| --- | --- | --- | --- | ---|
| 默认（Lua + Scintilla） | 3 | 54,528,681 B | 19,598,121 B | 61 条全 PASS |
| 两个都关 | 0 | 43,461,196 B | 19,596,302 B | 60 条全 PASS（少的是 Lua 那 3 条换成 2 条说明断言） |

那张表定的是**关掉这两个能力的相对代价**，之后没有重测：现在（阶段 7，含 SVG 皮肤、游戏页与主循环
预算修复，外加 Lua 计时器派发修复）默认档是 74 条、`.wasm` 54,779,018 B，把 Lua 关掉后游戏页那 10 条与「提示」无限动画那 1 条
都不执行、只剩 2 条说明断言，
所以两种构建各自的**绝对**条数与体积都变了，`-20.3%` 这个量级只能当阶段 5 的参考值。

`.wasm` 减 11,067,485 B（−20.3%），`.data` 基本不变（字体才是大头）。产物体积治理仍是未收的账，
见 `doc/soui-wasm-port-plan.md` 的"已知缺口"。

### Windows 主机构建

移植本身不依赖 bash：链接命令在 `demos/wasm-demo/CMakeLists.txt` 里，`--preload-file` 的宿主路径
由 CMake 给出正斜杠形式（`file_packager` 按 `@` 切分，反斜杠会被当成转义）。Emscripten 在
Windows 上是交叉编译（`CMAKE_SYSTEM_NAME=Emscripten`，`_WIN32` 不成立），所以走的仍是
`swinx/wasm.cmake` 那条 POSIX 分支，产物一样是 `.html/.js/.wasm/.data`。

```bat
:: 1) 装 emsdk 与 CMake/Ninja，然后（cmd 或 PowerShell）
C:\emsdk\emsdk_env.bat

:: 2) 配置。Ninja 是硬要求：Emscripten 工具链不支持 Visual Studio 生成器
emcmake cmake -S <soui4> -B C:\bw -G Ninja ^
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_DEMO=ON

:: 3) 构建
cmake --build C:\bw --target soui-wasm-demo

:: 4) 起服务并打开 http://localhost:8081/soui-wasm-demo.html
py -m http.server 8081 -d demos\wasm-demo\out
```

Windows 主机上容易踩的三点：

- **`ninja.exe` 要在 `PATH` 里**。`emcmake` 只负责注入工具链文件，生成器还得自己指定。
- **构建目录要短**（如 `C:\bw`）。`emcc` 的响应文件、`ninja` 的中间路径和 `--preload-file`
  都是长路径，仓库又深，容易出现 `MAX_PATH` 溢出或路径超长报错。
- **字体路径不能有中文/空格时更省事**；`-DSOUI_DEMO_FONT` 传正斜杠路径。

阶段 0 的 `build.sh` 仍是 POSIX 脚本（bash 数组、`sed`、`find`），Windows 主机上跑不了；
要它就在 Git Bash / WSL 里跑，或者直接用上面这条 CMake 路径（SOUI XML 演示）。

### macOS 主机构建

macOS 主机与 Linux 用同一条命令，不需要额外参数：

```bash
source "$EMSDK/emsdk_env.sh"
emcmake cmake -S <soui4> -B /tmp/build-soui-wasm -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_DEMO=ON
cmake --build /tmp/build-soui-wasm --target soui-wasm-demo
```

字体探测按 `CMAKE_HOST_APPLE` 走 macOS 候选列表（不能用 `APPLE`：Emscripten 交叉编译时
`CMAKE_SYSTEM_NAME=Emscripten`，`APPLE` 为假，会误落到 Linux 路径并报警告）。2026-10-29
在 macOS 12.7.6 (x86_64) 上实测：配置 0 报错、776 步构建成功（当时的烟测 27/27 PASS，烟测现已移除）。

Retina（dpr=2）下 canvas 的 CSS 尺寸与 backing store 都仍是 800x600，由浏览器合成器放大到
约 1602x1204，文字边缘会发虚——这是"帧缓冲 1:1 上屏"的固有表现，不是渲染错误。


### 画面上每一块对应哪条链

| 画面元素 | 证明的链路 |
| --- | --- |
| 顶栏品牌（`SOUI 5.3.2.1`，版本号由宿主 `fillBrand()` 从 `SOUI_VER1..4` 填）/ QQ 群 / 各页状态行（中英混排） | 文本测量 + cairo/freetype 字形链（没有预置字体时这里没字形，其余照常） |
| 根窗口的深色底 | `colorBkgnd="@color/panel"` 的纯色填充（**不挂** `_skin.sys.wnd.bkgnd`：那是 light 主题的白色九宫图，配近白文字就是白底白字） |
| 各页功能按钮（打乱数据/加一行/换一批数据）的渐变与圆角边框 | `render-gdi` 的 `GradientFill` / Path 系列，纯矢量；文字用深色是因为 `_skin.sys.btn.normal` 皮肤本身是白底 |
| 勾选框方块（基础控件页与 gridctrl 复选列） | imglist / imgframe 皮肤（`states=8` / `sys_icons.png` 横向 12 帧），位图链 |
| 点 MVC 页「打乱数据」 | DOM `pointerdown/up` → `SwinxInjectMouse` → 消息队列 → SOUI 命中测试 → 宿主 `REFLECT_NOTIFICATIONS_EX` 反射 → `notifyDataSetChanged()` 三个视图一起变 |
| 编辑框里敲键盘出字 | DOM `keydown`（按 `e.code` 映射成物理键位 VK）→ `SwinxInjectKey` → `TranslateMessage`（结合 Shift/CapsLock 算字符）→ `WM_CHAR` → 焦点控件（richedit41 插文档） |
| Lua 游戏页方块自动下落 | `::SetTimer` → `emscripten_set_main_loop` 驱动的 `PeekMessage` 补出 `WM_TIMER` → `ITimer` 回调进脚本 |

| 脏矩形上屏 | 脏矩形合并后局部上屏：一次绘制周期里的多次 `ReleaseDC -> commitCanvas` 被合并成一次 blit + 一次 JS 回调（见“合并上屏的人工验证步骤”） |
| 用输入法往编辑框里打汉字 | DOM `compositionend` → `SwinxDemoText` → swinx `SwinxInjectText` 逐码点 `WM_IME_CHAR` → `SRichEdit::OnImeChar` → richedit41 插文档；候选词窗的位置来自 `SwinxDemoCaretPos`（见"输入法（CJK 组合输入）"） |

十二页 `<tabctrl>` 里每一页各自证明什么，见下一节。

### 十二页控件：控件、布局、数据与脚本进 wasm 的证据

下半部的十二页才是控件演示。**页签切换本身**也走 SOUI 自己的命中测试 → 事件 → `SetCurSel` → 布局 → 重绘，不是 JS 直接改 DOM。

| 页 | 控件 | 编译前提（缺了会怎样） |
| --- | --- | --- |
| 概览 | 2×2 能力卡（`gridLayout` + `rowWeight`/`xGravity=fill`） | 核心布局；顺带证明 `size` 不能写 0（0 是 `SIZE_SPEC`，不是自适应） |
| 基础控件 | `progress`×2（含 `showText` 的百分比）、`sliderbar`、`switch`、`toggle`、`radio`×3、`combobox`（下拉列表）、`listbox`、`calendar` | 核心控件 + `GETBUILTINSKIN` 内置皮肤，阶段 2 就有；下拉列表是 `SDropDownWnd` 的第二个软件宿主窗口（见"更多控件"一行原来的说明） |
| 更多控件 | `hotkey`、`ipctrl`、`ratingbar`（SVG 星星）、`lightbutton`×2、`verificationCodeEdit`、`grouplist`（分组标题模板 + 宿主事件喂数据、折叠箭头） | 前两样在核心 `SCmnCtrl`；`ratingbar`/`lightbutton`/`verificationCodeEdit`/`grouplist` 在 `controls.extend`（依赖 `SCtrlsRegister`）；星星皮肤走 `SOUI_ENABLE_SVG` |
| MVC | 一个子 `<tabctrl name="tab_mvc">` 里四个视图：`<listview>` + `<tileview>` + `<mclistview>` 共用一份 `SAdapterBase` 的 **500 行**数据，行模板是"头像块 + 两行标题 + 说明 + 热度进度条 + 下载量"的控件树（`getView` 按名字逐个填）；`<treeview>`（`STreeAdapterBase` 的另一棵树，`hasLines`/`indent=14dp`，三组共 15 个节点、含一层收起的三级节点） | 核心 MVC 控件；行窗口只在 `SListView::OnPaint` 里按 `m_bDatasetInvalidated` 物化，所以要看行内容必须**先切页**；500 行只建可视区的十余个行窗口，滚动时回收复用；热度条不套系统 progress 皮肤，改用 `values/skin.xml` 里两个 16×16 圆角 SVG（`margin=4`），8dp/10dp 高度下九宫格仍能拼出完整胶囊；树节点的展开框由 `STreeView::OnLButtonDown` 在 `hasLines` 下自己算，不需要额外的按钮皮肤 |
| 列表与树 | `treectrl`（内联四级 `<item>`，`hasLines`/`fullRowSel`）、`listctrl`（表头 + 4 行，数据由宿主喂） | 同上；`listctrl` 的 `<item>` 只支持表头，行必须 `InsertItem/SetSubItemText` |
| 布局 | 3×2 网格六张卡，每张卡里是**真的按该属性排布的子窗口**：`weight=1:2:3`、`interval=14dp`、子块 `layout_gravity` 左/中/右、容器 `gravity`（hbox 里管垂直轴）、`pos` 的 `|0`+offset 居中 / `%25` 百分比 / 四元组负值右锚拉伸、hbox 套 vbox | 核心布局引擎；每张卡宽度写死（页宽 776 − padding 24 − 列间距 16）/ 3 = 245dp。六张卡的色块真的按属性排布，肉眼可逐条核对
| 图形控件 | `progressRing`（`controls.extend`）、`SpeedMeter`（sweep 渐变描边）、`waveProgress`（timeline 驱动、两层波反方向漂移）、`qrctrl`（纯算法生成） | `controls.extend` 静态链入并注册；`SpeedMeter` 还依赖 cairo 的 sweep 渐变与"路径转区域"两个后端能力，见本节末尾 |
| 表格 | `<gridctrl>` 固定行列网格（1 表头 + 6 行 × 5 列，第 4 列是复选单元格）、`<propgrid>` 分组属性树 | 两者都在 `controls.extend`；`gridctrl` 的单元格只能 `SetItemText`/`SetCellType`，**XML 里没有内容**；`propgrid` 少一个 `<cmdbtnstyle>` 会在 `CreateChildren` 里 `SASSERT` **当场终止整个模块**（见下一节） |
| 图表 | `<lineChart>` / `<columnChart>` / `<pieChart>`（hellocharts），数据由宿主 `SetXxxChartData` 喂 | `controls.extend` 的 hellocharts 分支；三个控件的点数/组数/扇区数由 `m_nChartSeq` 决定，状态行读的是控件**自己的** `GetLineChartData()` 等，不是宿主里的变量 |
| 编辑器 | `<edit>`（richedit41，含一个 `number="1"` 的数字框）、`<scintilla>`、`<hexedit>` | 分别依赖 `third-part/richedit41`、`third-part/Scintilla`、`controls.extend`；缺任何一项，标签被**静默跳过**（树里没有、画面上也没有，只留一行日志）。数字框存在的理由见"输入法（CJK 组合输入）" |
| 扩展控件 | `colorizeText`、`scrolltext`×2（一行内容宽于窗口的水平跑马灯 + 一行按 `\n` 拆成三段逐行向上过渡的多行滚动） | `controls.extend` 静态链入 **且**在 `Create()` 之前调过 `SCtrlsRegister::RegisterCtrls(app)`——扩展控件不是自注册的 |
| Lua 游戏 | 消消乐 / 俄罗斯方块 / 坦克大战三个子页（`<tabctrl name="tab_game">`），棋盘与 LED 数字位全部由 `games.lua` 在 `on_init` 里按 `values:template` 批量建；按钮 `on_command="xxl_on_restart"` 这类属性直接把 `EVT_CMD` 路由到 lua 全局函数 | `ScriptModule-LUA` + `lua-54` + `translator` 静态链入，宿主要在 `SHostWnd::Create()` 之前 `SetScriptFactory`；缺了则脚本根本不执行，三张子页只剩空白和一行 `lua_hint`。**还要求 `SOUI_ENABLE_SVG=ON`**：皮肤全是 `src="svg:..."`，关掉后 nanosvg 那条链没编进来，整页空白而不报错（见下一节） |

`<hexedit>` 是 `controls.extend` 里纯 GDI 的 `SPanel`（自己画地址/十六进制/ASCII 三栏），**不依赖 Scintilla**；数据要宿主喂，演示里 `CWasmHostWnd::fillHexDemo()` 送 128 字节 0..127 并设三栏颜色。`<listctrl>` 的行同理，见 `fillListDemo()`。

Scintilla 在 wasm 下关掉了异常：`third-part/Scintilla/CMakeLists.txt` 的 `EMSCRIPTEN` 分支加 `-fno-exceptions -include ScintillaNoExcept.h`，后者把 `try/catch` 降级成 `if(true)/if(false)`、`throw` 映射到 `abort()`。不开全模块 `-fexceptions` 的理由与代价见 `doc/soui-wasm-port-plan.md` §14.2。

图形控件那一页的两条后端前提（都是"画得不报错、就是不对"的类型）：

- **sweep（锥形）渐变**：`SpeedMeter` 的外圈是 `grad_sweep` + `DrawGradientPath`。swinx 的 cairo 后端补了 `CreateSweepGradientPattern`（把 180 段扇形拼成一张 mesh 图案），`GRADIENTINFO.sweep` 的成员顺序也改成和 SOUI 的 `GradientInfo::sweep` 一致（`startAngle/centerX/centerY`，SOUI 直接强转结构体交给 `CreateGradientBrush`）。apple 后端目前退回"第一个色标的纯色"并打一次警告，属已知缺口（见 §16.6）。
- **路径转区域**：`SpeedMeter`/`SProgressRing` 会 `SetClipPath`，这要求 `PathToRegion` 对**非矩形路径**（圆弧）返回真实区域。cairo 原先用 `cairo_copy_clip_rectangle_list` 导出裁剪矩形，遇到非轴对齐路径直接返回 `CAIRO_STATUS_CLIP_NOT_REPRESENTABLE`（错误码 22）而不是空列表——表现为仪表盘外圈整块不画。现在两个后端都改成"把路径栅格化成 1bpp 掩码、再按行合并成 RECT 填 `RGNDATA`"。回归用例在 `demos/fun_test/test_dc.cpp`：`PathToRegion_roundrect_keeps_shape` 与 `PathToRegion_pie_covers_sector`，都在改前实现上会失败（A/B 见 `doc/soui-wasm-port-plan.md` §16.5）。


### 三页"数据控件"的宿主契约（MVC / 表格 / 图表）

这三页共同的风险是**空壳看不出来**：控件建出来了、边框和格线都画得对，但数据一行都没进去，
截图上和"内容正确"几乎分不开。所以每一条演示都必须在页面上留一个能被读回来的痕迹：

- **MVC**：`mvc_lv`/`mvc_tile`/`mvc_mclv` 共用一个 500 行的 `g_lvRows`，listview 与 tileview 还共用
  一个适配器（同一套子控件名，一份 `getView` 喂两个视图），状态行显示的是从三个控件自己的
  `GetAdapter()` 读回来的行数。要点是行窗口**只在 `SListView::OnPaint` 里、且
  `m_bDatasetInvalidated` 为真时才物化**（`SOUI/src/core/SListView.cpp:212-218`），
  `GetItemPanel()` 对未物化的行返回 NULL（同文件 886-898）——所以"能读到第 0 行的文字"同时证明了
  行被建出来、`getView` 把模板填成了内容，而"适配器报 500 行"两件事都证明不了；500 行只建可视区的
  十余个行窗口，滚动时回收复用。行模板不是一行文本：头像块里还有子 `text`、热度是每行自己的
  `<progress>`（`getView` 里 `FindChildByName2<SProgress>` 后 `SetValue`），说明富行模板同样走
  `SItemPanel::InitFromXml` 物化整棵子树。刷新只调一次 `LvAdatperImpl::notifyDataSetChanged()`，
  三个视图一起变。`mvc_tree` 走另一套 `CTreeDemoAdapter`（`STreeAdapterBase` 派生），数据有三组
  15 个节点、其中"构建与渲染 → 平台适配"是一层默认收起的三级节点；折叠/展开是基类行为，没有
  `getView` 之外的自定义逻辑；它的控件名不能叫 `tree_demo`——「列表与树」页的 `treectrl` 已经占了
  那个名字，重名会让 `FindChildByName2` 拿错控件。
- **表格**：`gridctrl` 的行/列数、单元格文字、复选状态全部只能从宿主侧
  `SetRowCount/SetColCount/SetItemText/SetCellType(CELL_TYPE_CHECKBOX)` 写进去，
  翻转要逐格 `GetCellOp<IGridCellCheck>`；状态行回读的是 `GetRowCount/GetColCount` 与真实勾选数。
- **图表**：hellocharts 的三个控件数据由 `SetLineChartData/SetColumnChartData/SetPieChartData` 喂，
  参数由 `m_nChartSeq` 推出来（`nPt=5+seq%4`、`nCol=4+seq%3`、`nSlice=3+seq%3`），断言读的是控件自己的
  `GetLineChartData()` 等，而不是宿主里那份变量。

**`<propgrid>` 有一条硬前提，写错不是"这一页没内容"而是整个模块起不来**：
`SPropertyGrid::CreateChildren` 无条件 `child(L"cmdbtnstyle")` 并对结果 `SASSERT`
（`controls.extend/propgrid/SPropertyGrid.cpp:341-342`），所以 `<propgrid>` 里必须至少有一个
`<cmdbtnstyle>`；另外两个就地样式名是 `editStyle` 与 `optionStyle`（注册处见
`propitem/SPropertyItem-Text.cpp:54`、`propitem/SPropertyItem-Option.cpp:63-64`）。
本轮第一次把 `<propgrid>` 加进 XML 时，wasm 在 `SHostWnd::Create()` 阶段直接
`Assertion failed: xmlCmdBtn` 终止，其余各页一起没了画面——这类“一个标签炸掉整页”的坑当初是无头烟测在上线前拦住的（烟测已随测试设施移除，记录见 `doc/soui-wasm-port-plan.md`）。

### Lua 游戏页：三个脚本游戏与 SVG 皮肤（阶段 7）

第 11 页「Lua 游戏」原来是 `uires/lua/demo.lua` 建一个按钮的最小样例，现在换成 `demos/demo`
的三个 Lua 游戏（消消乐 / 俄罗斯方块 / 坦克大战）：脚本 `uires/lua/games.lua`
（`<script src="lua:games_script"/>`），布局 `uires/xml/page_game.xml`
（`<include src="layout:XML_PAGE_GAME"/>`，页内再套一个 `<tabctrl name="tab_game">` 三张子页），
棋盘格子全部由脚本在 `on_init` 里 `CreateChildrenFromXml` 批量建（消消乐 64、俄罗斯方块 200、
下一块预览 16、坦克大战 195），LED 数字位和棋子走 `values/template.xml` 的两个模板。

这一页同时也是 **SVG 支持的验收页**：`values/skin.xml` 里游戏皮肤全是 `src="svg:..."`
（多帧雪碧图 + 九宫药丸底 + 键帽），`SOUI_ENABLE_SVG` 因此在 wasm 分支被根 `CMakeLists.txt`
FORCE 成 ON。关掉它不报错，症状是**整页空白**——`CreateSvgFromResId` 返回 NULL，皮肤对象
照样建得出来、格子照样有矩形、页面照样"可见"，矩形/可见性/事件那几条断言一条都不会红。
这段验证当初靠无头烟测逐格读帧缓冲：每格在中心加上下左右各偏 1/5 边长的五个采样点取像素，
取到任意非棋盘底色才算画上，64 格全中才 PASS。A/B 实测把 `SOUI_ENABLE_SVG` 改 OFF 重链后恰好
32/64 格变底色、其余断言不红——矩形/可见性断言盯不住 SVG，只有像素盯得住（关掉时那 32 格采到
的是 `<img>` 拿不到皮肤剩下的像素，不是糖果）。烟测已随测试设施移除；现在判定 SVG 是否正常
直接看页面：糖果有图形、棋盘完整即可。

这一页踩过、并且已经写进 XML 头注释的几条约定：

- **命中测试要求父窗口矩形包住子控件**（`SWindow::SwndFromPoint` 先判父矩形再往下找）。
  棋盘上的浮层 `wnd_*_aniframe` / `wnd_tank_sprites` 铺满整页（`pos="0,0,-0,-0"`）却必须
  `msgTransparent="1"`，否则它盖在棋盘上面，鼠标点到的永远是它，`EVT_CMD` 一次都不发。
- **脚本在启动时建的子控件初始矩形是 `[0,0,0,0]`、不可见**，因为 `on_init` 发生在第一次布局
  之后，而它们所在的页当时还没被选中；任何验证都必须"先切页、再读/再点"。
- **`<tabctrl>` 写了 `animateSteps` 时，切页过渡期新旧两页都被 `SetVisible(FALSE)`**（只画
  滑块）。内层 `tab_game` 因此不设动画，否则"切到子页 + 子页控件可见"这条断言必红，浏览器里
  也要等动画跑完才看得见棋盘。
- **纵向预算只有 343dp**：页区 `[20,188,780,558]`（tabctrl 的 tab 在左侧占 90dp）减 padding。
  `gridlayout` 的列宽/行高取"该列最大请求尺寸"，放不下时**不会压缩**（权重只加不减，
  `SOUI/src/layout/SGridLayout.cpp` pass 2），所以 `demos/demo` 的原尺寸（消消乐格子按 SVG
  原始 64、方块 24、坦克 26）在这里会把第 7、8 列和最后几行直接排到画布外——控件还在树里、
  仍然"可见"，只是点不到、看不见。现在改成消消乐 38（`values/template.xml`）、方块 15、
  坦克 18（`games.lua` 的 `tetris_cell` / `tank_cell`；坦克的精灵步长是从格子实测矩形算的，
  改一个数就够）。回归盯法：坦克棋盘底边不得越过页区底 558（改之前是 616）。

### 无头烟测（已随测试设施移除）

这版 demo 曾带一套 node 无头烟测（`smoke.js` + 一批 `SwinxDemo*` 探针导出，最多 74 条断言），覆盖：
控件树建立与逐页布局、布局几何逐条比矩形、MVC/表格/图表数据、图形控件像素、Lua 游戏的脚本事件与
`ITimer` 回调、键盘逐键字符、输入法逐码点、脏矩形与合并上屏。按“demo 不带测试性质内容”的要求，
`smoke.js` 与全部探针导出已删除，宿主只留五个功能口
（`SwinxDemoPump/Mouse/Key/Text/CaretPos`）与帧缓冲/上屏导出；断言清单与各轮变异 A/B 数据
见 git 历史与 `doc/soui-wasm-port-plan.md` §19、§21。

两条从烟测里沉淀下来、并且仍然成立的时序约定：

- **每注入一条按键就泵一次**：`TranslateMessage` 读的是**翻译时刻**的按键状态。一次回调里连投
  down/up 再统一泵，Shift 的 up 会先清掉状态位，`Shift+A` 就变成小写。`soui.html` 里每次注入后
  同步调用 `_SwinxDemoPump()` 就是这个原因。
- **别给 `tabctrl` 设 `animateSteps`/`animateTime`**：切页过渡按时长推进，浏览器标签页被挂起
  （后台标签 rAF 停摆）时会停在一页都不显示的状态（见 `dlg_main.xml` 顶部的注释）。

### 键盘的人工验证步骤（Chromium）

1. `python3 -m http.server 8083 -d demos/wasm-demo/out`，打开 `http://localhost:8083/soui-wasm-demo.html`。
2. 点「编辑器」页签，用真鼠标点两下 `<edit>`（第一下聚焦、第二下定位光标——rich edit 在
   `WM_SETTEXT` 后会把整段文本留成选中态，不点第二下字符会替换选区），敲 `a`、`Shift+a`、`1`、
   `Shift+1`。预期：框里依次出现 `a A 1 !`。
3. 敲方向键、`F5`、`PageUp`、`Insert`、`Delete`。预期：文字不变（这些键不承载字符；方向键移动插入符）。
4. 敲 `Enter`、`Tab`。预期：`Enter` 换行；`Tab` 页面刻意**不**拦截（留给页面导航），敲完焦点可能已经
   离开画布，继续敲之前重新点一下画布。
5. 按一次 `CapsLock` 再敲 `a`。预期：出 `A`（页面只观察它收到的 CapsLock 按键，**加载页面前**系统
   CapsLock 已开时第一个字母大小写会反，按一次 CapsLock 后一致）。
6. Scintilla 里点一下再敲字母。预期：字符进代码区——同一条键盘链对每个拿焦点的控件都成立。

### 滚轮的人工验证步骤（Chromium）

目标行为：光标在画布上时，滚轮归 SOUI（滚动光标下的列表/树），页面正文不跟着滚。

1. `python3 -m http.server 8085 -d demos/wasm-demo/out`，打开 `http://localhost:8085/soui-wasm-demo.html`。
2. 切到「MVC」页，点子页签 `mclistview`（五列表格）。把光标放到表格中间滚一格。预期：行内容随滚轮
   上下走（首列的仪表盘名字变化），页面本身**停在原位**——正文既不抖也不慢慢下沉。
3. 反向滚同样格数。预期：回到原来的行（同样的首行文字与进度条长度），说明一格滚轮恰好换一行。
4. 光标移到画布外的正文或留白上再滚。预期：这时才是页面正文滚动（监听只挂在 canvas 上）。
5. 按住 `Ctrl`/`⌘` 滚轮。预期：浏览器缩放，画布内容不滚（组合键事件不拦也不注入）。
6. 触控板两指小幅滑动：预期连续、平滑地滚，不会"滑很多下才动一格"——页面按小数累积
   `WHEEL_DELTA`，攒满一格才注入。

这一轮在 Chromium 里用脚本代跑过第 2、3 步：在画布上派发 `WheelEvent`，`defaultPrevented` 为真，
页面 `scrollY` 不变，帧缓冲里 mclistview 的进度条行有 43 行发生变化，反向再滚 43 行后逐像素回到
原状态。A/B：把根节点 `sendWheel2Hover` 改回 `0` 重建，同一脚本下滚动行数为 0——证明这条属性是
滚轮能落到列表上的必要条件，而不是"加了个监听就碰巧好了"。实现细节见
`doc/soui-wasm-port-plan.md` §29。

**没覆盖的**：Windows Edge、Firefox（`deltaMode=1` 的分支只按 3 行/格换算，没在真机 Firefox 上跑）、
Safari 触控板、手机浏览器；滚动惯性与真实手感也只能人手判断。wasm 产物没有无头断言，
`ctest -L '^soui-headless$'` 覆盖不到这一层。

### 输入法（CJK 组合输入）

需求原话是"编辑器里打开输入法的情况下也只能输入字母"。canvas 元素拿不到浏览器的组合输入，
所以键盘焦点固定在页面上一个透明的代理 `<input>`（`#ime-proxy`）上，链路三段：

1. **代理框跟着光标走**。宿主每帧交出 SOUI 自己的插入符坐标（`SwinxDemoCaretPos`），
   `syncProxyPos()` 把它换算成 canvas 的页面坐标设到代理框的 `left/top`。浏览器的候选词窗
   挂在焦点元素旁边，代理在光标处，候选窗就在正在输入的那行字边上。
2. **提交文本进 wasm**。`compositionend` 拿到成品字符串后按 UTF-8 送进 `SwinxDemoText`，
   swinx 的 `SwinxInjectText` 解码后**逐码点**投一条 `WM_IME_CHAR`（一条消息一个码点；
   不是按键，所以没有 `WM_KEYDOWN`/`WM_KEYUP`，`lParam` 为 0）。
3. **控件消费**。`SRichEdit::OnImeChar` 把消息交给 richedit41 的 `WM_IME_CHAR` 分支插入文档。

手机软键盘、表情面板、某些"直接上屏"的输入法不产生组合事件，只发 `input(insertText)`，
代理框也接这条路；`insertCompositionText` 明确不接——那是组合过程的文本，成品已经在
`compositionend` 交过一次，再交就是双写。

**组合进行中不能碰代理框的 `value`**。真实输入法每敲一个拼音字母都会发一次
`input(insertCompositionText)`，`textarea` 里那串拼音就是输入法的预编辑缓冲。在这条事件里
清空它，等于把缓冲抹掉：浏览器的表现是**候选窗刚弹出就关闭、`compositionend` 再也不来、
一个字都上不了屏**。所以 `input` 监听的第一句是 `if (composing || e.isComposing) return;`，
清空只发生在"没有组合在进行"的分支里（以及 `compositionend` 之后）。

**为什么是 `WM_IME_CHAR` 而不是 `WM_CHAR`**：swinx 在每个非 Windows 后端都按这个约定投递
"键盘打不出来的字符"——Linux 是 XIM 的 commit 回调（`swinx/src/platform/linux/SConnection.cpp:235`）、
macOS 是 `NSTextInputClient` 的 `insertText:`（`SNsWindow.mm:1181`）加 `TranslateMessage` 里
`c<127 ? WM_CHAR : WM_IME_CHAR` 那条规则（`SConnection.mm:601`）、iOS 同构，wasm 这次补齐同一条约定。
Windows 不用应用操心：`DefWindowProc` 先把 `WM_IME_CHAR` 转成 `WM_CHAR` 再派发，所以风格过滤
（`ES_NUMBER` 等）在 `OnChar` 里做一次就够。如果把 wasm 的提交改成发 `WM_CHAR`，非 Unicode 构建下
`SRichEdit::OnChar` 里那个 ANSI DBCS 累加器会按 `wParam` 低字节调 `IsDBCSLeadByte`，实测
`你好，输入法人` 上屏成 `你好，鍥햺`——四个字的低字节 0x93/0x65/0xD5/0xBA 里两个落在 GBK 前导区
`0x81-0xFE`，被两两拼成乱码、最后一个整条吞掉。

**因此 `SRichEdit` 侧那处改动不是 wasm 专属**：它补的是整个非 Windows 平台缺的
`ES_NUMBER` 闸门（`number="1"` 的编辑框以前能用输入法塞进汉字），编辑器页因此多了一个
`number="1"` 的数字框。可证伪证据见 `doc/soui-wasm-port-plan.md` §19.4 与
`demos/fun_test/test_e2e_richedit_ime.cpp`。

人工验证步骤（Chromium，需要本机装一个中文输入法）：

1. `python3 -m http.server 8085 -d demos/wasm-demo/out`，打开 `http://localhost:8085/soui-wasm-demo.html`。
2. 切到「编辑器」页，点进第一个编辑框（那行"点进来就能打字…"），把系统输入法切到拼音。
3. 敲 `nihao` 并选词。预期：候选窗出现在这一行字旁边（不是屏幕角落），选定的汉字逐字出现在光标处，
   插入符右移，后续仍可继续输入；用退格能逐字删掉。
4. 点进第二个框（`number="1" 只收数字`），用输入法提交一串汉字。预期：**汉字不进文档**，
   数字与 `.`/`-`/`,` 照常进；这条同时证明键盘路径的过滤没被绕开。
5. 手机/平板浏览器：软键盘的中文与 emoji 走 `input(insertText)`，预期同样上屏，且不出现双写
   （组合型输入法只该收到一次成品）。

本机（Linux + headless node）能自动化的部分当初在 `smoke.js`（逐码点 `WM_IME_CHAR`、整串逐字进文档、
不伪造键盘链、插入符前进、数字框丢弃 `人`）；烟测已随测试设施移除，A/B 数据见
`doc/soui-wasm-port-plan.md` §19.4。

上面 1–4 步里**不依赖真实输入法**的部分，本轮在 Chromium 里用脚本代跑过一遍（`evaluate_script`
派发 `CompositionEvent` 到 `#ime-proxy`，见 `doc/soui-wasm-port-plan.md` §19.6 的表）：代理框落在
插入符的换算坐标上（行首 `[24,210]` → `[44,203]`、行尾 `[378,210]` → `[399,203]`，误差 0）；组合期间
文档不变、面板显示拼音原文、期间的 `keydown` 被丢弃；`compositionend('输入法验证')` 后码点序列
`[36755,20837,27861,39564,35777]` 逐字相等、文档按插入点精确拼接、`WM_CHAR` 计数仍为 0；组合结束后
同一个 `keydown('a')` 立刻恢复注入（`WM_CHAR 0 → 1`），证明"被丢弃"是组合状态造成的；数字框收到
`12人3` 的四个码点但文档只多 `123`；帧缓冲里那 5 个汉字有墨列 65 / 有墨像素 407，同行空白区为 0。
**脚本代跑不了的**：候选窗的真实外观与时序、marked text、退格逐字删（第 3 步的后半）、
Safari 与手机软键盘（第 5 步）。

**这条脚本代跑一度给出过错误结论**，值得记下来：上面那轮合成事件只发了
`compositionstart → compositionupdate → compositionend`，**没发中间那些
`input(insertCompositionText)`**，而恰恰是那些事件把预编辑缓冲清掉了。也就是说脚本验的
是"成品能不能进文档"，真输入法挂的是"组合能不能活到成品"——两者不是同一件事。
补上逐键 `insertCompositionText` 之后的 A/B（同一份探针，两个页面）：
修复前 `n/i/h/a/o` 五键之后代理框 `value` 全是 `""`（缓冲被清，真实输入法会在这里断）；
修复后依次是 `n → ni → nih → niha → nihao`，组合期间文档与 `WM_CHAR` 计数都不动，
`compositionend('你好')` 后按插入点精确拼接、框清空、`WM_IME_CHAR` +2。
复现用的探针与判据见 `doc/soui-wasm-port-plan.md` §19.8。

真输入法下如果候选窗仍然一闪就没了，下一个嫌疑是**第一个按键的 `keydown` 被
`preventDefault()`**（组合还没被输入法认领时，`isComposing` 与 `keyCode===229` 两个信号都还没亮，
而阻止默认动作就等于让浏览器别把这个键交给输入法）。用这段贴进 DevTools 看真实事件顺序即可判定：

```js
for (const t of ['keydown','keyup','compositionstart','compositionupdate','compositionend','input'])
  window.addEventListener(t, e => console.log(t, e.type === 'input' ? e.inputType : '',
    'key=' + e.key, 'keyCode=' + e.keyCode, 'isComposing=' + e.isComposing,
    'proxy.value=' + JSON.stringify(document.getElementById('ime-proxy').value)), true);
```

判读：正常拼音应该看到 `keydown(isComposing=true, keyCode=229)` 开头，或 `keydown` 之后紧跟
`compositionstart`；若看到 `keydown('n', isComposing=false)` 后 `proxy.value` 一直是 `""` 且没有
`compositionstart`，就是第一个键被吞了。

**没覆盖的**：画布里不显示组合期的预编辑文本（marked text / 带下划线的拼音串）——浏览器只在
候选窗里显示它，wasm 侧没有把它画进 `SRichEdit`；真机移动端浏览器、真实 HIMC/
`ImmGetCompositionString` 那条 Windows 路径也没验证过。

### 合并上屏的人工验证步骤（Chromium）

1. `python3 -m http.server 8084 -d demos/wasm-demo/out`，打开 `http://localhost:8084/soui-wasm-demo.html`。
2. 加载后什么也不做地放几秒：画面应完全静止，没有多余的重绘（稳定时没有排队中的脏矩形）。
3. 点 MVC 页「打乱数据」（或任何功能按钮）。预期：按下态**立刻**出现（延迟上屏不延迟到下一帧——
   flush 挂在最外层消息派发结束，在同一次 `DispatchMessage` 返回前完成），三个视图的首行同时变化。
4. 连续敲键、移动鼠标、切页：画面应始终跟手、不残留旧内容。合并只对重叠区域生效，裁剪在请求期解析，
   所以脏矩形既不会退化成整帧，也不会因为取并集而变大——这是这条验证真正要看的东西。
5. 若画面出现“少了一块”（某次改动始终不上屏），先查是否新增了绕过 `ReleaseDC()` 的绘制路径，
   或窗口在 pending 期间被销毁（`flushPendingCommits()` 会跳过已销毁窗口）。

### 十二页控件的人工验证步骤（Chromium）

画得对不对、点了有没有反应必须人工看。这一轮的操作都在画布内（canvas 坐标 = 帧缓冲坐标）：

1. `python3 -m http.server 8137 -d demos/wasm-demo/out`，打开 `http://localhost:8137/soui-wasm-demo.html`。
2. 「概览」页：四张能力卡按 2×2 铺满、间距一致、没有叠字。这一页专门盯 `gridLayout` 的
   `rowWeight`/`xGravity="fill"`，以及"`size` 写 0 不是自适应而是 0 像素"（写 0 时四张卡会叠到左上角）。
3. 点「列表与树」页签。预期：页签切换由 SOUI 自己完成（选中项出现蓝色下划线），树完整出现、
   四级缩进与连接线都在；点 `SOUI 核心` 行预期折叠，再点展开；点某一行预期该行有选中底色。
   下面的 `listctrl` 应有表头加四行（`SWindow/STreeCtrl/SScintillaView/SHexEdit`）。
4. 点「图形控件」页签。预期：`progressRing` 是一圈**环带**（不是实心扇形）、中间有 `68%` 文字；
   `SpeedMeter` 的外圈是**彩色渐变的环带**（sweep 渐变+描边，见上一节的"图形控件"两条前提），
   **缺口在正下方、左端绿右端红橙**、指针指向 180；`waveProgress` 是**矩形填充 + 贝塞尔波浪顶边**
   （不是楔形/三角形），波纹在动（靠 SOUI timeline 心跳，静止截图里看不出）；
   `qrctrl` 是一个能扫出 `https://github.com/soui4/soui` 的二维码。若外圈变成实心扇形或整块消失，
   先查 `PathToRegion` 与 `DrawGradientPath` 两条路径（§16.2、§16.3）；若环带镜像、波浪缺角，
   查"图形页的两处镜像缺陷"那一节（`LineTo` 记录期与 `PATH_CMD_ADDARC`）。
5. 点「编辑器」页签。预期：三块都存在——`<edit>` 一行带底色的可编辑框、`<hexedit>` 的
   地址/十六进制/ASCII 三栏（数据是 00..7F，ASCII 栏是绿底）、`<scintilla>` 带行号的代码。
   点进 `<edit>` 敲几个字母，预期文字出现在框里。注意 `WM_SETTEXT` 之后 rich edit 会把整段文本
   留成**选中**状态：用真鼠标点两下（第一下聚焦、第二下定位光标）再敲，字符才会插到光标处。
   在 Scintilla 里点一下再敲，预期字符进代码区。
6. 点「扩展控件」页签。预期：`colorize` 那一行的 `SOUI` 三个字母是绿色（`key`/`colorKey`）；
   「水平滚动」那行的文字宽于它 420dp 的窗口，于是自己从右向左循环，滚到末尾从头接上；
   「多行滚动」那行只显示一行，每行停留约 1.6 秒后逐像素向上滚到下一行，第三行之后回到第一行。
7. 点「Lua 游戏」页签，操作步骤与预期见下一节。这一步切换本身经过外层 `tab_main` 的
   `STabSlider` 动画（约半秒；动画期间点击会被过渡窗口吃掉，这是 SOUI 的正常行为），
   内层 `tab_game` 没设 `animateSteps`，所以三个子页之间切换是立刻可见的。
8. 回到「基础控件」页签。预期：两个 progress 条一个带 `62%` 文字、一个不带；slider 的滑块位置在
   40%；switch 是开的；三个 radio 只有 A 选中；日历是深底浅字、有"年月/星期/日期"三块，
   点某一天预期该天被选中，翻页箭头预期切月；`combobox` 点开后弹出五项列表（`SDropDownWnd`
   的第二个软件宿主窗口），选一项后框内文字跟着变；下面的 `listbox` 六项，点一项出现选中底色。
9. 点「MVC」页签。预期：状态行是 `共 500 行 · 列表500 / 平铺500 / 多列500 ｜ 首行 …`；子页签
   `listview` 的行是"头像块 + 名称/来源两行 + 说明 + 热度条 + 下载量"，拖动滚动条到中间，
   行内容按需换、不在可视区里的行没有行窗口；切 `tileview` 是同一份数据排成卡片（头像、名称、
   来源、热度条，列数随宽度变）；切 `mclistview` 是五列表格（控件/来源/版本/说明/热度），
   最后一列是每行的进度条，表头分隔线可拖动；切 `treeview` 是另一棵树（三组共 15 个节点，
   `构建与渲染 → 平台适配` 默认收起），点缩进区的方块展开/收起。
   三处热度条在 10dp（`listview` / `mclistview`）与 8dp（`tileview`）高度下都应是**两端收圆的
   胶囊**：上下边距各 4dp，目标高度刚好由两个圆角半片拼成；如果看到的是上下两条硬边、圆角被
   压平，说明又退回了边距 8/5 的系统 progress 皮肤。
   点「打乱数据」→ 三个列表视图的首行同时变成同一个新名字、状态行 `首行` 跟着变（树不参与，
   它读的是另一份数据）；点「追加一行」→ 状态行变成 `共 501 行 …`。若只有状态行变、视图里文字不变，
   说明 `notifyDataSetChanged()` 没被三个控件的适配器都收到（这是这一页最容易坏的一条）。
10. 点「表格」页签。预期：`gridctrl` 是"1 行表头 + 6 行 × 5 列"的固定网格、最后一列是复选框且有
    3 项勾着；点「加一行」→ 状态行 `8 行 … 勾选 4 项`；点「翻转勾选」→ 逐格取反、变回 3 项。
    下面的 `propgrid` 应出现分组（点分组名预期折叠），值列里能看到文本框与下拉两种编辑器。
11. 点「图表」页签。预期：折线、柱状、饼三张图**都有数据**（空数据时只剩坐标轴与图例，很容易和
    "控件坏了"混淆）；点「换一批数据」→ 状态行从 `折线 5 点 ｜ 柱 4 组 ｜ 扇区 3 块` 变成
    `折线 6 点 ｜ 柱 5 组 ｜ 扇区 4 块`，三张图形状同时变化。
12. 点「布局」页签。这一页六张卡里的色块是**真的按该属性排布的**，肉眼逐条对：
    ①三块宽度 1:2:3；②三块之间各留 14px；③三块分别左/中/右贴容器（同宽）；④两个不同高度的块
    **共用垂直中线**（hbox 的容器 `gravity` 管的是垂直轴，`right` 是底对齐）；⑤`|0 + offset -0.5`
    那行水平居中、`%25` 那块左边在父宽 1/4 处、最后一块左右各留 2/6px；⑥hbox 里套 vbox、右侧
    `weight=1` 的块撑满剩余宽度。把浏览器窗口改宽，⑤最后一块与⑥右侧块会跟着拉伸。
13. 观察：切页/点按这些动作仍然只重绘局部矩形（整页切过去那一瞬接近整帧，属预期）。
14. 点「更多控件」页签。预期：快捷键框显示 `Ctrl+Shift+F2`；IP 框分四段、只能输数字和点；
    评分条默认 3.5 星、点/划可改（含半星）；两个灯按钮 `RUN`/`STOP` 悬停与按下变色；验证码框
    点一下出数字；右边的分组列表两组各 3 项，点组名折叠/展开（箭头翻转），点子项出现选中底色。

第 5 步与第 9~11 步是这一版最值得看的三条：前者覆盖 richedit41 的 `ITextHost` 创建、Scintilla 的
headless 绘制与输入、键盘消息要经过控件焦点这三段；后三条覆盖"宿主把数据交给适配器/表格模型/图表
数据源，控件自己重画"这条回路。

### Lua 游戏页的人工验证步骤（Chromium）

“格子建出来了、事件回脚本、`ITimer` 回调被派发”这些链路当初由无头烟测验证（已移除，记录见
`doc/soui-wasm-port-plan.md`）；**能不能玩**必须人工在浏览器里看。前提：这次构建的 `SOUI_ENABLE_SVG` 是 ON（根 `CMakeLists.txt` 的 wasm 分支里
`FORCE`，不用额外传参）；关掉时这些 `.wasm` 照样链得出来，但整页只剩背景色——SVG 皮肤拿不到
`ISvgObj` 时 SOUI 不报错，只画空。

1. `python3 -m http.server 8137 -d demos/wasm-demo/out`，打开 `http://localhost:8137/soui-wasm-demo.html`，
   点「Lua 游戏」页签。预期：左侧是三个子页签（消消乐 / 俄罗斯方块 / 坦克大战，竖排在左），
   默认显示消消乐。
2. **消消乐**：8×8 棋盘每格是一颗彩色糖果（SVG 雪碧图逐帧切换，不是纯色块）。鼠标点一颗再点相邻
   一颗，预期两者交换；能凑成 3 连就消除、上方的棋子掉下来补位、`得分`/`金币` 的 LED 数字翻页滚动、
   连击时中间出现橙色 `xN`。点「提示」预期一对棋子高亮并**持续脉冲缩放**（`xxl_scale_select` 是
   `repeatCount="-1"` 的无限动画，一直跳到下一次点选/消除为止），关键验收是**标签页不能冻住**——
   这一条踩过整页假死，根因见"阶段 7 特有的坑"里 pump 预算那条。点「洗牌(5)」预期全盘重排且金币减 5，
   金币不足时按钮不使棋盘变化。棋盘右下角第 7、8 列必须**完整可见**——这一条盯的是"格子尺寸写回
   `demos/demo` 原值时列会排到画布外"的回归。
3. **俄罗斯方块**：点子页签后点「开始游戏」→ 状态行变"进行中"、上方开始落下方块——**不按任何键也要落**，
   约 0.8s 一格（一等 800ms 重力，这是 `ITimer` 在驱动；本轮修的正是这条回调在 wasm 里从未被调用，
   症状是方块悬在半空、消行后整局卡死）；键盘 ← → 移动、
   ↑ 旋转、↓ 软降、空格硬降、`P` 暂停/继续（预期状态行同步）。要先用鼠标点一下棋盘拿到焦点
   （棋盘 `focusable="1"` 且有 tip）。消一行时预期：整行闪白约 300ms（两次明暗交替）→ 光波/碎片动画 →
   `得分`/`行数` LED 增加并接着出下一块，**闪白不会停住不动**（这段 300ms 倒计时同样由 `ITimer` 推进）。
   堆到顶部时预期出现半透明遮罩 + "游戏结束"卡片（遮罩是页内静态 XML + anim 模板，不是模态框），
   点「重新开始」卡片消失、能再来一局。
4. **坦克大战**：点开始后 13×15 战场出现砖墙/钢墙/基地和我方坦克，右侧 `剩余敌人` LED 有数。
   方向键移动并转向、空格开炮、`P` 暂停；炮弹命中砖墙预期砖块消失、命中钢墙预期弹开。
   消灭本关全部敌人后中间出现关卡提示条；基地被摧毁时预期出现"战斗结束"卡片、原因行写"基地被摧毁"。
   坦克移动是连续像素位移（精灵层按实测格子步长 `Move`），预期不会一格一格跳、也不会跑出棋盘。
5. 三个游戏都有音效：消消乐/俄罗斯方块用 `snd/tetris_*.wav`，坦克大战用 `snd/tank_*.wav`
   （开炮 / 敌开炮 / 砖墙碎裂 / 钢墙弹回 / 击毁 / 阵亡 / 出生 / 开局 / 过关 / 结束，共 10 条，
   由 `tools/sfx/gen_tank_sfx.py` 合成）。浏览器 autoplay 策略要求页面先有一次用户手势
   才能出声，所以进页面后先点一下画面（或按一次键），否则最初几条会排队等第一个手势。
6. 观察：切子页与消子时的重绘是局部矩形；把浏览器窗口拉宽，棋盘所在那一片会跟着变宽
   （消消乐棋盘是 `weight=1` + `layout_gravity="center"`，俄罗斯方块/坦克的棋盘是自适应尺寸居中，
   所以前者拉伸、后者只在中间平移）。

第 3、4 步的键盘链与动画观感是自动化覆盖不到的：当初烟测点一次「开始游戏」、按一次 `VK_P` 验证
`EVT_CMD`/`EVT_KEYDOWN` 能进 lua，再用真实时间等几个重力节拍验证 `ITimer` 在跑（烟测已移除）。
方向键→ lua → 棋盘重画、闪白的明暗节奏、光波/碎片/飞分的动画是否好看，都要人工确认。

### 阶段 2 特有的坑

- **`SCOM_MASK` 必须在 `#include <SAppCfg.h>` 之前定义**。`commgr2.h` 的默认值是"全开"，而
  `SAppCfg::DoConfig()` 的 switch 无条件引用 `CreateResProvider_ZIP` / `CreateResProvider_7ZIP`；
  全掩码会让这两个 wasm 构建图里根本不存在的组件变成真实链接依赖。按位收敛成
  `scom_mask_render_gdi | scom_mask_imgdecoder_stb` 后对应分支编译成 `return FALSE`。
- **`-sSTACK_SIZE` 不能留在默认 5 MB**。render-gdi → cairo → freetype/fontconfig 这条绘制栈会溢出，
  `build-soui.sh` 默认给 32 MB（`SWINX_WASM_STACK` 覆盖）。
- **不开 pthread 时任何 `std::thread` 构造都会 abort 整个模块**，而症状只是
  `SHostWnd::Create()` 返回 `NULL`。触发点是 swinx `src/gdi/cairo/FontFallback.cpp` 里后台构建系统
  字体表的线程，`__EMSCRIPTEN__` 下已改成同步。
- **`SApplication` 必须堆上 `new` 且不 `delete`**：`emscripten_set_main_loop(fn, 0, 1)` 会展开
  `main()` 的栈，栈上单例会在主循环刚开始时就被析构。
- **`SHostWnd` 不发 `WM_INITDIALOG`**，初始化放在 `Create()` 返回之后；`SetTimer` 要写 `::SetTimer`，
  `SWindow`/`SNativeWnd` 上有同名成员函数会遮蔽它。定时器 id 也要避开 `SHostWnd` 自己那批
  （`kPulseTimer/kTaskTimer/kNcCheckTimer` = 4321..4323）。
- **`SWindow::GetChild(i)` 是 1 基的**：`GetIChild(0)` 命中 `CHILDID_SELF` 会返回窗口自己，遍历控件树
  时写成 0 基就是无限自旋。
- **macOS 的 Cmd 不等价 Ctrl**：`soui.html` 把 `MetaLeft/Right` 映射成 `VK_LWIN`，而 SOUI 的编辑
  快捷键认 `VK_CONTROL`，所以 Cmd+C/V/A/Z 不会命中（wasm 的剪贴板本身也是桩）；物理 Control
  可用但不是 macOS 习惯。中文组合输入仍未覆盖。
- **产物体积**：默认构建 `.wasm` 约 54.5 MB、`.data` 约 19.6 MB（字体与系统资源目录都内联在 `.data` 里）。
  相对阶段 2 的 30.9 MB，增量全部来自 `richedit41` + `Scintilla` + `controls.extend` + Lua 那几个静态库，
  不是资源；不是可发布形态。`SOUI_BUILD_WASM_LUA=0 SOUI_BUILD_WASM_SCINTILLA=0` 能砍到约 43.5 MB
  （−20.3%，见"两个可选能力的编译开关"），体积治理见 §11.6 第 3 项。
  **阶段 7 之后**是 `.wasm` 54,786,558 B / `.data` 19,854,263 B：多出的 0.57 MB 代码是 nanosvg 分支，
  多出的 0.25 MB 资源是 `uires/svg|anim|animator`（三个游戏页的皮肤）。

### 阶段 7 特有的坑（SVG 皮肤 + 三个游戏页）

- **`SOUI_ENABLE_SVG` 关掉是"静默空白页"**：`CreateSvgFromResId` 返回 NULL 时 SOUI 不报错也不警告，
  皮肤对象照建、格子照有矩形、页面照样"可见"，只有帧缓冲像素能看出来。所以别拿“矩形/可见”当 SVG 的证据，要直接看像素（见“Lua 游戏页”一节）。
- **`.data` 不随 SVG 开关变**：`uires/` 是整目录 `--preload-file`，`svg/*.svg` 关着也照样打包。
  想省资源得动预载清单，动开关只省代码。
- **`-DSOUI_ENABLE_SVG=OFF` 在 wasm 下已经关不掉 SVG**：根 `CMakeLists.txt:171` 在 EMSCRIPTEN 分支
  `set(SOUI_ENABLE_SVG ON CACHE BOOL ... FORCE)`——游戏页皮肤全是 `src="svg:..."`，关掉就是空白页，
  所以移植把 SVG 定成 wasm 的必备能力并强制打开。要做体积 A/B 只能临时改这一行；踩过的坑是
  拿着旧构建目录里的 `SOUI_ENABLE_SVG:BOOL=OFF` 当真，重链后 `.wasm` 只涨不跌、SVG 像素断言照样
  PASS，量出来的"代价"是假的。
- **wrap-content 的格子拿到的是 SVG 固有尺寸**：`size="-1,-1"` 在 `gridlayout` 里等于"列宽 = SVG 原始
  64"，而 `gridlayout` 的列宽/行高**只加不减**（`SOUI/src/layout/SGridLayout.cpp:900/926`），
  放不下就整片排到画布外，症状是"点不到、看不见、断言全绿"。见"Lua 游戏页"那节的 343dp 预算。
- **按名字问不到脚本建的格子**：格子只有数字 `id`，排查时只能按下标遍历（`SWindow::GetIChild`）；
  而 `SWindow::GetIChild` 是 **1 基**（`SOUI/src/core/Swnd.cpp:3641`，0 被 `CHILDID_SELF` 占了），
  内部要 `+1`——第一次实现忘了，0 号格子读回的是棋盘自己的矩形。
- **游戏没有声音**：wasm 的 `PlaySound`（`swinx/src/platform/wasm/audio.cpp`）把 wav 字节交给宿主的
  `Module.swinxAudioPlay`，解码与播放都在 WebAudio 里。按这个顺序查：① autoplay —— 页面必须先有
  一次用户手势（点一下画面即可）才能 `resume()` 出声，否则请求一直排队；② `snd/` 有没有进
  `.data`（`CMakeLists.txt` 的 `--preload-file`，新增 wav 后要重新 configure）；③ `GetAppDir()`
  是不是 `/opt/soui`（`soui_main.cpp` 里设定），路径打不开时 `PlaySound` 返回 FALSE、静默无声。
  `tetris_play_snd`/`tank_play_snd` 判到导出为 `nil`（headless 回归环境）时同样静默跳过，那是设计内行为。
- **无限动画会把 `SwinxDemoPump` 拖成死循环**：消消乐「提示」的选中脉冲用
  `uires/anim/xxl_scale_select.xml`（`repeatCount="-1"`）。SOUI 用 10ms 脉冲定时器驱动动画，
  每帧 `OnAnimationInvalidate` → `InvalidateRect(NULL)`，而 wasm 一帧重绘要 35~107ms，定时器每次
  `PeekMessage` 都早已到期，于是 `WM_TIMER`/`WM_PAINT` 交替、消息队列永不排空。浏览器里
  `SwinxDemoPump` 就是 `requestAnimationFrame` 回调本身（`emscripten_set_main_loop(SwinxDemoPump, 0, 1)`），
  不返回等于整页冻住——一点「提示」标签页就没了。现在一次 pump 只工作 100ms 预算就带着
  "还有消息"的标记返回，剩下的留给下一帧。坑的普适形式：**别在 rAF 回调里
  写 `while (PeekMessage)` 抽干队列**，页面上只要存在 `repeatCount="-1"` 的动画就一定会踩；
  有限动画播完就停（实测 `xxl_fx_ring` 让第一次 pump 耗时 594ms、`xxl_fx_pop` 1868ms，之后队列排空），
  症状只在无限动画上出现，所以别拿有限动画验证通过当作没问题。当初由 `smoke.js` 的「提示」那条盯（已移除）；现在人工验证：点「提示」后标签页不能冻住、棋子持续脉冲。
- **带回调的 `SetTimer` 曾经在 wasm 里一次都不回调**：`SetTimer(NULL, 0, elapse, proc)` 的
  `WM_TIMER`，Win32 是在 `PeekMessage` 内部直接调用 `proc`、**不**把它交给 `DispatchMessage`；
  wasm 后端把 `proc` 存进了 `msg->lParam` 却没人取，消息照旧被泵出来，而
  `DispatchMessage` 见 `hwnd==NULL` 直接返回 `FALSE`（`swinx/src/wnd.cpp:4105`）。症状是"队列照抽干、
  定时器消息照进队列，脚本却一动不动"：方块没有重力，消行进入 `clearing` 后那 300ms
  的闪白倒计时永远走不完，`tetris_collapse()`（抹行 + 计分 + 出下一块）不执行——画面上就是整局卡死。
  三个游戏的计时器是同一族（`tetris_on_timer` / `tank_on_timer` 都走 `STimer`），消消乐没踩到只是
  因为它不靠 `ITimer`。修在 `swinx/src/platform/wasm/SConnection.cpp` 的 `peekMsg`（与
  `src/platform/linux/SConnection.cpp:1032-1041` 同一语义，回调前先放掉 `m_mutex`），
  `getMsg` 改成直接委托 `peekMsg`，两条入口不再各写一份扫描。坑的普适形式：
  **`WM_TIMER` 的计数证明不了回调跑到没有**，要看被回调方自己写的 observable；同理无头验证一定要
  **真的等墙上时间**（`sleepSync` + 泵），否则 50ms 的计时器根本不到期，坏构建也"看起来正常"。
  当初由 `smoke.js` 的对应断言盯（已移除）；现在人工验证：不按任何键方块也要落。

### 控件与主题的坑（阶段 3）

这批坑的共同点是**不报错、不警告，只是画不出来**（或白底白字）。判定方法都是同一套：按名字取子控件
矩形，再把帧缓冲对应区域取出来看像素直方图——不要凭"应该是皮肤没加载"猜。

- **根窗口别挂 light 主题的 `_skin.sys.wnd.bkgnd`**：那是 `soui-sys-resource` 里的白色九宫图，
  配近白文字就是白底白字。用 `colorBkgnd="@color/panel"` 统一深色底。
- **`_skin.sys.btn.normal` 是白底皮肤**：按钮文字必须写深色（本演示沿用 sys 资源 `normalbtn` 类的
  `#385e8b`），不能跟着 `class="row"` 写近白色。
- **`SProgress` 的百分比文字是 `showText`**（`SOUI/include/control/SCmnCtrl.h:891`）；写成
  `showPercent` 不报错，只是没人认这个属性，文字不画。
- **`SCalendar` 的背景写死在 `Init()` 里**：`SCalendar::Init()` 把 `m_crBg` 设成 `@color/thm_color`，
  这张表在 `soui-sys-resource/theme_sys_res/theme_color.xml` 里是 `rgb(255,255,255)`——不覆盖就是纯白
  方块。`m_crHoverText` 在 `Init()` 里没初始化，鼠标划过日期会拿到未初始化颜色，`colorHoverText` 要
  显式给。另外 `GetDesiredSize()` 返回 0，`size="-1"` 会被压成 0 高，必须给固定尺寸。
- **`SColorizeText` 的 `text` 必须写成属性且排在 `key` 前面**：着色区间只在 `OnAttrKey →
  OnContentChanged → UpdateTextColors` 时算一次，而标签内联文本是全部属性应用完之后才塞进
  `m_strText` 的，那一步不触发 `OnContentChanged`——排到后面就永远不着色。
- **`<edit>` 的默认字符格式继承父链文字色**：`SRichEdit::InitDefaultCharFormat` 取
  `BeforePaintEx()` 之后的 `pRT->GetTextColor()`，不写 `colorText` 会继承 tabctrl 的深色，在深底上
  几乎看不见。
- **`<hexedit>` 的 XML 只有背景色**：文字色要走 `SetAdrCol/SetHexCol/SetAsciiCol(背景, 文字)`，
  不设就是默认深色文字，深底上读不出来。
- **`STreeCtrl` 没有 `colorItemSelText`**：选中行底色来自内置 `itemSkin`（浅蓝），选中态文字走
  `colorTextPush`；不设就回落到 `colorText`（近白），浅蓝底上白底白字。
- **`tabSkin` 是浅灰图片时页签文字要深色**：`_skin.sys.tab.header` 是 `sys_tab_header.png`，
  页签文字写近白色在画面上只剩一条灰带。
- **`controls.extend` 不是自注册的**：必须在 `SHostWnd::Create()` 之前调
  `SCtrlsRegister::RegisterCtrls(app)`，漏了 `<scintilla>/<colorizeText>/<scrolltext>` 会被当成未知
  标签静默跳过（只留一行日志）。

### 阶段 4 特有的坑（展示页重写 + Lua）

这一批的共同点依然是"不报错、只是不对"，而且多数错在**命中测试**或**后端契约**，不在 XML 解析：

- **`SXmlDoc::root()` 是 document 节点，不是根元素**。`CreateChildrenFromResId` /
  `CreateChildrenFromXml` 都写 `CreateChildren(xmlDoc.root())`，于是 XML 的根元素自己也会被建成
  窗口——阶段 4 的样例是 `lua_panel.xml`（阶段 7 删掉了），现在受这条约束的是
  `values/template.xml` 的 `<g.xxl_ele>`：它的根 `<stack>` 必须写全 `size`（现在是 38,38），
  否则按内容测出一个比子控件摆放范围小得多的矩形，子控件"画得出来、点不到"。
  同一个坑在"Lua 游戏页"一节的命中测试约定里还会再咬一次。
- **`size` 里的 `0` 是 `SIZE_SPEC`（0 像素），不是自适应**。`SIZE_UNDEF=-3`、
  `SIZE_WRAP_CONTENT=-1`、`SIZE_MATCH_PARENT=-2`、`SIZE_SPEC=0`
  （`SOUI/include/interface/slayout-i.h`），与 Android 的 `layout_width` 约定相反且不警告。
  网格页的卡片靠这条：`size="0,-1"` 会让卡片测成 0 宽，四张全部叠到左上角——文字仍然画得出来
  （文本不裁剪），看起来就是"一团重叠的字"。
- **`multiLines` 才是换行开关**。`wordBreak="1"` 只决定"在哪断"，`multiLines="1"` 才允许多行；
  只写前者时文本被裁成一行，看上去像"容器宽度算错了"。
- **`STabSlider` 过渡期间点击会被过渡窗口吃掉**。`tabctrl` 配了 `animateSteps` 后，切页会插入一个
  盖住整页的过渡窗口，把新旧页都设成不可见；动画没结束时点按钮，命中落在过渡窗口上。
  浏览器里这是"第一次点没反应、第二次就正常"的典型来源；自动化里要等动画跑完（当初烟测用足够多次 `pump`）再点。
- **`PathToRegion` 不能靠 `cairo_copy_clip_rectangle_list` 实现**。cairo 对任何非轴对齐裁剪路径
  返回 `CAIRO_STATUS_CLIP_NOT_REPRESENTABLE`（22），关掉抗锯齿也没用（本机用
  `/tmp/sweeptest/clip.c`、`clip2.c` 复现过），而 `SpeedMeter`/`SProgressRing` 的 `SetClipPath`
  全是圆弧。老实现把"错误"当成"空列表"，于是仪表盘外圈整块不画；现在 cairo 与 apple 两个后端都
  改成"把路径栅格化成 1bpp 掩码、按行合并成 RECT 填进 `RGNDATA`"（WINDING 填充规则与 Win32
  `PathToRegion` 一致）。`ExtCreateRegion` 的 `dwSize` 收的是**字节数**，不是 RECT 个数。
- **`DrawGradientPath` 的契约是"描边"，不是"填充"**。Skia 的实现是
  `kStroke_Style` + `setStrokeWidth(m_curPen->GetWidth())` + 渐变 shader；`SSpeedMeter` 也是先把
  画笔宽度设成环宽再调用它。linux GDI 后端原先用"填充路径区域 + 渐变裁剪"，修完 `PathToRegion`
  之后外圈就变成一块实心扇形——现在改成用渐变画刷建一支 `PS_GEOMETRIC` 钢笔
  （`LOGBRUSH.lbStyle = BS_BRUSH`，`lbHatch` 放 HBRUSH，这是 swinx 的扩展）再 `StrokePath`。
  D2D 后端用笔刷描边但取的是笔色，属于上游差异，本轮没动。
- **`GRADIENTINFO.sweep` 的成员顺序必须和 SOUI 一致**。SOUI 把 `GradientInfo` 直接强转成
  `PGRADIENTINFO` 交给 `CreateGradientBrush`，所以 swinx `include/gdi.h` 里
  `{startAngle, centerX, centerY}` 的顺序不能改（改了是静默错位，不是编译错误）。
  cairo 后端用 180 段扇形拼 mesh 图案实现 sweep；apple 后端目前退化成"第一个色标的纯色"并打一次
  警告——这是已知缺口，见 `doc/soui-wasm-port-plan.md` §16.6。
- **cairo 后端的默认字体大小从 0 改成了 12px**。`CreateFont` 没给 `lfHeight` 时（SOUI 有些路径
  只设字体名与 weight），老实现按 0 处理，字形按 0 号字画——内容存在但不可见。现在按 12px
  兜底，apple 后端同步改了同样的兜底。
- **产物体积**：`.wasm` 54,320,077 B、`.data` 19,584,754 B、`.js` 195,747 B。比阶段 3 的
  46.4 MB 多出的 ≈7.9 MB 是 `lua-54` + `ScriptModule-LUA` + `translator` 的静态库体积；
  仍然不是可发布形态，压缩/裁剪见 §11.6 第 3 项与任务"体积治理"。

### 图形页的两处镜像缺陷（阶段 4 补丁）

两个缺陷都只错"形状"，不影响控件存在、可见性、事件，所以只能靠帧缓冲像素发现：

- **`LineTo` 在路径记录期多补了一次 `cairo_move_to`**（swinx cairo 后端）。记录期应当只追加线段，
  但老实现每条 `LineTo` 之后都补一次 `cairo_move_to`，把连续线段拆成互不相连的子路径；填充时每个
  子路径各自隐式闭合，`SWaveProgress` 的"矩形 + 贝塞尔顶边"就散成三条线，画面上的楔形即由此而来。
  apple 后端的 `LineTo` 记录期本来就提前返回，缺陷只在 cairo。
- **`PATH_CMD_ADDARC` 把角度按 y 向上换算、又没有交换 GDI `Arc` 的起止点**（`render-gdi` 的
  linux/win 两份实现）。Skia 的 `addArc(start, sweep)` 是屏幕坐标（y 向下、3 点钟为 0、顺时针为正），
  而 GDI 的 `Arc` 从起点**逆时针**画到终点；正确写法是 y 向下取点、把终点当起点传进去
  （`Arc(end, start)`），与同在文件里的 `SRenderTarget_GDI::DrawArc` 一致。写成 y 向上且不交换，
  画出来的是另一侧的 240° 圆弧——`SSpeedMeter` 的缺口从下方跑到上方。
  渐变本身没动：sweep 的 `t = (角度 − startAngle)/360` 与 Skia 的 `localMtx` 旋转等价。

验证（Linux x64 / Emscripten，两处各自回退后重链，同一台机器上 A/B）：

| 断言 | 旧实现 | 新实现 |
| --- | --- | --- |
| 仪表盘环带在上、缺口在下 | 上=`244,246,250`（底） 下=`255,69,0`（环） | 上=`255,140,0` 下=`244,246,250` |
| 渐变左中段绿 / 右中段红橙 | 左=`74,229,90`（边缘混色） | 左=`0,252,125` 右=`255,69,0` |
| 波浪底部贯穿整宽的实心带 | `[波,波,底,波,波]`（左中漏底） | 五点全为 `147,197,253` |
| 波浪未达到的顶部仍是背景 | 通过 | 通过 |

`demos/fun_test/test_dc.cpp` 另有一条不依赖渲染器的回归：`PathRecording_consecutive_lineto_stays_onefigure`
断言 `MoveToEx + 3×LineTo` 记录出 4 个点（旧实现 7 个）且 `PathToRegion` 覆盖四边形右下角。
回退 cairo 的 `LineTo` 后该用例 FAIL（`Value of: 4, Expected: n, Which is: 7`），恢复后 506 条全过。

Windows 的 `components/render-gdi/win/render-gdi.cpp` 改了同名分支，但本机没有 Windows 环境，
需要在 Windows 上按"图形控件"页人工复验环带缺口朝下；原生 Windows GDI 的 `Arc` 语义与 swinx
一致（`.wasm` 走的是 cairo，结论相同）。
