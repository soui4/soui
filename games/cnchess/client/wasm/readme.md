# 中国象棋客户端的 WebAssembly 版

把 `games/cnchess/client` 的桌面客户端整体搬进浏览器：同一套 `CMainDlg` / `ChessGame` /
`SGameTheme` 源码，换成 wasm 宿主 + 浏览器原生 WebSocket + TaskLoop 的 wasm 后端（默认
`-pthread` 的 worker 线程，不带 `-pthread` 时退回主线程泵）。产物是
`out_pt/cnchess-wasm.{html,js,wasm,data}` 四个文件（泵后端在 `out/`），`.data` 里预载了客户端资源和系统皮肤。文本不再打包字体——wasm 走 canvas2d 绘制后端，字形由浏览器的内置字体渲染（见下文"字体"一节）。

## 与桌面版的三处差异

1. **网络：`ws` 组件 → 浏览器原生 WebSocket。**
   桌面版 `ws` 组件是 libwebsockets 的封装，自带收包线程：`IConnListener` 的四个回调发生在那
   条线程上，应用侧靠 `CMainDlg::OnMessage` 里的 `STaskHelper::post` 切回 UI。wasm 里 socket
   对象本身搬不进 worker —— Emscripten 的 `src/lib/libwebsocket.js` 中 `createOnMainThread==false`
   仍是 TODO，四个 `*_set_on*_callback_on_thread` 直接忽略 thread 参数、回调一律在主线程的 JS
   事件循环里 `makeDynCall`。所以搬的边界放在**回调之后**：
   `components/ws/src/wasm/WsClientWasm.cpp` 在 `connectTo` 时给这条连接开一条 ITaskLoop 投递
   队列（与桌面版每连接一条工作线程同形态），主线程的 C 回调只做一次拷贝 + 入队，拆 `dwType +
   dwSize` 包头和调 `IConnListener` 都在队列的执行线程上做。拷贝必须留在回调里 —— 回调返回后
   `data` 指向的 JS 缓冲就失效了。
   `send_binary` 等发送侧仍由调用线程直接走 `emscripten_websocket_send_binary`，客户端代码
   （`WebSocketClient` / `WsClientListener`）一行没改。
   队列建不起来（组件没链进来、或 `postTask` 失败）时就地交付，退回"主线程同步回调"的旧形态，
   不静默丢事件。`wait()` 在这个平台上没有可等的队列，恒返回 `FALSE`。

2. **线程：TaskLoop 换成 wasm 后端，默认仍是 worker 线程。**
   桌面版 TaskLoop 是线程池（`components/TaskLoop/thread`），主题包的解压往它上面投任务。wasm
   后端（`components/TaskLoop/wasm/TaskLoopWasm.cpp`）有两条路，由根 CMakeLists 的
   `SOUI_WASM_PTHREADS` 决定，`build-cnchess.sh` 默认打开：
   - **worker 后端（默认，带 `-pthread`）**：`start()` 建一条 wasm worker 线程，队列归它消费，
     UI 线程只入队 + `Atomics.notify` 唤醒，与桌面版同口径 —— 解压整条离开 UI 线程，一帧都
     不被它拖住。代价是产物要 `crossOriginIsolated`（COOP+COEP 响应头）才建得起线程，
     起页面必须用 `tools/wasm/serve-isolated.py`。
   - **泵后端（`--no-pthreads`）**：不建线程，`postTask` 只入队（按优先级稳定排序），
     `waitUntilDone` 内联执行（浏览器主线程等 worker 等于冻住标签页），真正的执行由宿主的
     `STaskLoopPump(预算毫秒)` 在每帧泵里抽干 —— "异步解压"在浏览器里等于"下一帧解压"。
     这条路的产物不需要隔离头，`python3 -m http.server` 就能跑。
   两条后端的调用点（`ThemeDownloadManager::ExtractZipAsync`）一行没改，进度回调都照常打；
   区别只在跑在哪条线程上，用 `CnchessStat` 的 6 与 19 相等与否判断（见「观测点」）。
   结果回抛走 `IMessageLoop::PostTask` + 宿主每帧 `ExecutePendingTask()`。
   **缺隔离头时不报错**：线程拿不到 `SharedArrayBuffer` 就静默退回泵后端，表现照常、只是没
   有并行 —— 状态行的 `worker N/M` 停在 `0/0` 是唯一的线索。

3. **弹窗：`SModalRoot` 而不是 `SHostDialog::DoModal()`。**
   `DoModal` 是嵌套消息循环，浏览器里拿不到新事件。登录、确认、求和/应答四个弹窗各有一个
   `*_modal.xml`，走 `SModalRoot`（`LoginDlg` / `PeaceReqDlg` / `PeaceAckDlg` 因此不参与 wasm 构建）。
   宿主负责把 DOM 事件注入消息队列并每帧泵一次，模态视图就能正常收发。

## 文件

| 文件 | 作用 |
| --- | --- |
| `cnchess_main.cpp` | wasm 宿主入口：`SApplication` + `SHostWnd`、帧缓冲上屏、`CnchessPump`（消息泵 + 任务泵）、导出给 JS 的统计量与输入注入口 |
| `cnchess.html` | Emscripten `--shell-file`，现在只是**薄壳**：标题 + 状态行 + 配色 + 一段 `SouiHost.start(...)`（宿主逻辑见下） |
| `CMakeLists.txt` | `cnchess-wasm` 目标：源码清单、导出函数、预载资源、链接选项 |
| `build-cnchess.sh` | Unix 主机的 CMake 包装器：查 `em++`、按档位把构建目录配好、构建目标；`--release` 出上线档 |
| `build-cnchess.bat` | Windows 主机的同名包装器（同样的档位、同样的环境变量；文件保持纯 ASCII + CRLF，见下） |
| `smoke.js` | 无头端到端烟测（node，不需要浏览器）：起真服务器 → 登录 → 收主题 → 解压 → 棋盘 |
| `node-websocket.js` | 烟测自带的 RFC6455 客户端（node 18 没有全局 `WebSocket`，也不引 `ws` 依赖） |
| `out_pt/` | 调试档 + **worker 后端**（默认，带 `-pthread`）的产物（未纳入版本控制） |
| `out/` | 调试档 + 泵后端（`--no-pthreads`）的产物（未纳入版本控制） |
| `release_pt/` | 上线档 + worker 后端（`--release`，整模块 `-O2 -DNDEBUG`，未纳入版本控制） |
| `release/` | 上线档 + 泵后端（`--release --no-pthreads`，未纳入版本控制） |

两条后端的 `.js/.wasm/.data` 是三件套，混在同一目录里换构建会出现"新 wasm 配旧胶水"的
`LinkError`，所以产物目录按后端分开；`.data` 只装 `uires` 与 `soui-sys-resource` 两份预载资源
（约 0.5 MB，字体不再打包，见下文"字体"一节）。

## 页面宿主：复用 `demos/wasm-demo/soui-host.js`

上屏、键鼠/滚轮注入、输入法代理、剪贴板双向桥、WebAudio 音效、崩溃上报、加载遮罩
这一整套，全在 `demos/wasm-demo/soui-host.js` 里——和 SOUI 演示页是**同一份文件**，构建时由
`CMakeLists.txt` 的 POST_BUILD 拷到产物目录（`SOUI_HOST_JS` 可覆盖路径），并进了
`LINK_DEPENDS`，所以改宿主 js 会重链重拷。

`cnchess.html` 里只剩下三行有信息量的东西：

```js
SouiHost.start({
  container: document.getElementById('app'),
  wasm: 'cnchess-wasm.wasm', factory: 'createCnchess', width: 900,
  api: { pump:'_CnchessPump', mouse:'_CnchessMouse', key:'_CnchessKey',
         wheel:'_CnchessWheel', text:'_CnchessText', caret:'_CnchessCaretPos' },
  onFatal: (msg) => { statusEl.textContent = msg; }
});
```

`api` 覆盖是因为本程序的导出名是 `Cnchess*` 前缀，宿主缺省用演示页的 `SwinxDemo*`；
剪贴板那个（`_SwinxClipboardSetBrowserText`）两边同名，不用覆盖。`C++` 侧的 fatal 钩子
现在叫 `Module.souiFatal`（旧名 `cnchessFatal` 保留兜底）。

页面上保留的 DOM id 是历史约定，`smoke.js`/无头脚本按 id 找节点：`#screen`（画布）、
`#screen-wrap`、`#loading`、`#load-phase`、`#ime-proxy`、`#log`、`#status`——宿主建的元素
沿用同一批 id，改名前先看 `tools/diag/` 里的脚本。

## 构建

编译与链接的命令行全部在 `CMakeLists.txt`（目标 `cnchess-wasm`）里；两个脚本都只是
CMake 包装器——查 `em++`/`emcmake`/`ninja`、把构建目录按档位配好、构建目标，流程与环境
变量一一对应，选哪个只取决于主机：

| 脚本 | 主机 | 调用 | 构建类型 | TaskLoop 后端 | 产物目录 |
| --- | --- | --- | --- | --- | --- |
| `build-cnchess.sh` | Linux / macOS | `./build-cnchess.sh [构建目录]` | `RelWithDebInfo` | worker（`-pthread`） | `wasm/out_pt` |
| `build-cnchess.sh` | Linux / macOS | `./build-cnchess.sh --no-pthreads [构建目录]` | `RelWithDebInfo` | 泵 | `wasm/out` |
| `build-cnchess.sh` | Linux / macOS | `./build-cnchess.sh --release [构建目录]` | `Release`（整模块 `-O2 -DNDEBUG`） | worker | `wasm/release_pt` |
| `build-cnchess.bat` | Windows | `build-cnchess.bat [构建目录]` | `RelWithDebInfo` | worker | `wasm\out_pt` |
| `build-cnchess.bat` | Windows | `set SOUI_WASM_PTHREADS=OFF && build-cnchess.bat [构建目录]` | `RelWithDebInfo` | 泵 | `wasm\out` |
| `build-cnchess.bat` | Windows | `build-cnchess.bat release [构建目录]` | `Release` | worker | `wasm\release_pt` |

实测体积（本机，调试档，2026-11-04）：`out_pt/cnchess-wasm.wasm` 约 31 MB、`out/` 约 30 MB
（带 DWARF，链接期 binaryen 只能做有限优化；worker 那份额外带 pthread 运行时与线程栈），
`.data` 两份都是约 0.5 MB 的预载资源。上线档本轮未重编，`release/` 里还是 2026-10-31 那份
带字体的旧产物（`.wasm` 约 4.9 MB、`.data` 约 20 MB），要用 `-O2 -DNDEBUG` 的结论就重跑
`--release` 再测。

`release` 也可以写成 `-r` / `--release` / `/release`，与构建目录参数的先后顺序无关；
`--no-pthreads`（或 `-P`）同理，两个开关可以一起给（`--release --no-pthreads` → `wasm/release`）。

```bash
source "$EMSDK/emsdk_env.sh"

# 调试档 + worker 后端 -> games/cnchess/client/wasm/out_pt（构建目录没配置过就现场 emcmake 配一个）
games/cnchess/client/wasm/build-cnchess.sh /path/to/build-wasm-pt

# 同一份源码要泵后端的产物
games/cnchess/client/wasm/build-cnchess.sh --no-pthreads /path/to/build-wasm

# 上线档 -> games/cnchess/client/wasm/release_pt（默认自己一个目录，不用指）
games/cnchess/client/wasm/build-cnchess.sh --release
```

Windows 主机：

```bat
call <emsdk>\emsdk_env.bat
games\cnchess\client\wasm\build-cnchess.bat D:\build-soui-wasm
games\cnchess\client\wasm\build-cnchess.bat release
```

构建目录取第一个位置参数，其次是同名环境变量，最后是每档的默认值：

| 环境变量 | 调试档默认 | 上线档默认 |
| --- | --- | --- |
| `SOUI_WASM_BUILD` / `SOUI_CNCHESS_SHIP_BUILD` | `wasm/build` | `wasm/build-release` |

默认落在**本 app 自己的 wasm 目录下**，四个 wasm app 各自一棵独立的 CMake 构建树。
以前几个 app 共用 `%TEMP%\build-soui-wasm` / `/tmp/build-soui-wasm` 时，先编过别的 app
的目录里残留 `CMakeCache.txt`，再编 cnchess 就会报 `The source ... does not match the
source used to generate cache`（只能删掉整个目录重配）。需要指到别处（更快的盘、CI
持久缓存）时再用这两个环境变量或位置参数覆盖。

已经配好的目录只把下面的缓存变量透传进去重新配置，其余选项保持不变，所以增量构建不会被
脚本毁掉；档位切换（`CMAKE_BUILD_TYPE`）会触发整树重编，脚本不替用户做这个决定——上线档
指到一个非 `Release` 的目录时直接报错退出，调试档沿用目录里已有的档位、只打一行提示（产物
的优化档位跟着缓存走，不再假装是 `RelWithDebInfo`）。
**换后端（`SOUI_WASM_PTHREADS`）等于整树重编**，两个脚本每次都把它写进缓存：这是有意的，
让"这条命令要哪条后端"和"产物是哪条后端"永远对得上。

缓存变量（都可以用同名环境变量由脚本透传）：

| 变量 | 默认 | 说明 |
| --- | --- | --- |
| `SOUI_WASM_PTHREADS` | `ON` | TaskLoop 后端。`ON` 给 `-pthread`，主题解压跑在真正的 worker 线程上，产物要 `crossOriginIsolated`（`tools/wasm/serve-isolated.py`）才起得来线程；`OFF` 退回 `STaskLoopPump` 的泵后端，页面不再需要隔离头 |
| `SOUI_CNCHESS_STACK` | `32MB` | `-sSTACK_SIZE`。SOUI 在 wasm 下走 canvas2d 绘制后端（render-gdi 接 swinx 的 canvas2d 实现，没有 freetype/cairo 递归绘制链），但 `SHostWnd::Create()` 的建窗递归仍按桌面线程栈深度写，5 MB 下直接踩爆 |
| 字体 | 浏览器内置 | canvas2d 后端用浏览器内置字体渲染文本，不再有 `SOUI_CNCHESS_FONT` 选项，也不预载 TTF（原选项已废弃） |
| `SOUI_WASM_CNCHESS_OUT` | 按后端取 `wasm/out_pt` / `wasm/out` | 产物目录；`--release` / `release` 档取 `wasm/release_pt` / `wasm/release`，不接受环境变量覆盖 |
| `SOUI_WASM_JOBS` | 交给 Ninja / `2` | 并行度。`.bat` 默认 2：emcc 在 Windows 上 `-j` 开大会卡死（2026-10-28 见过），确认宿主行为正常再调高 |

客户端源码按 `-O0 -g` 编译，体积/速度来自链接期的整模块优化；调试符号留着，出错时行号可读。

## 字体（canvas2d 浏览器内置字体）

wasm 产物走 swinx 的 canvas2d 绘制后端（`render-gdi` 在 wasm 下接 canvas2d 实现），
文本直接交给浏览器的 Canvas 2D `ctx.fillText` 渲染，字形由**浏览器内置字体**提供——
不再像旧版那样把一份 TTF/TTC 预载进 `.data` 的 `/opt/soui/fonts/demo.ttf`，也不再调用
`AddFontResource`。因此：

- 构建时无需准备 CJK 字体文件，`SOUI_CNCHESS_FONT` 选项已废弃（去掉了自动探测与
  `--preload-file` 两处逻辑），`.data` 体积也少了约 20 MB。
- 控件 XML 里写的字体名（如 `宋体` / `微软雅黑`）在浏览器里若没有对应字体，会按浏览器的
  默认字体回退；汉字/象棋棋子字形（車馬炮…）由浏览器的 CJK 回退字体负责，无需 SOUI 干预。
- 烟测里 `CnchessStat(3)`（字体）固定返回 1，含义从"字体注册成功"改为"文本字形由浏览器提供"。

这与 `demos/wasm-demo` 的 route C 行为一致；需要离线/可控字形时再考虑把 canvas2d 后端换成
带字体子集化的 cairo 分支（那是另一条线，不在本目录的默认构建里）。

> 所有 wasm 的 `.bat`（各 app 的薄包装与 `__cmake\wasm-build-common.bat` 共享驱动）都保持纯
> ASCII + CRLF：`cmd.exe` 按活动 OEM 代码页逐行解码批处理文件，UTF-8 的中文注释会被切成幽灵命令
> （`chcp 65001` 也修不干净，分块读取仍会截断多字节序列）；共享驱动用 `goto` 跳转，而 `cmd.exe`
> 按块扫描标签，LF 行尾的标签正好跨块边界时会被找不到。中文说明写在同目录的 `.sh` 包装器和
> 本文件里。

## 运行

wasm 模块要通过 HTTP 加载（`file://` 读不了 `.data`），所以需要一个静态服务器。**默认产物
（`out_pt/`，带 `-pthread`）还必须带 COOP+COEP 响应头**，否则 `SharedArrayBuffer` 拿不到、
worker 建不起来：

```bash
# 1) 页面 —— worker 产物用隔离服务，并把它反代到棋服务器（口径见下面第 3 条）
python3 tools/wasm/serve-isolated.py games/cnchess/client/wasm/out_pt \
    --port 8080 --ws-proxy 127.0.0.1:3311
#    泵产物（out/）不需要隔离头，但同样需要这条反代才能用默认地址：
python3 tools/wasm/serve-isolated.py games/cnchess/client/wasm/out \
    --port 8081 --no-isolation --ws-proxy 127.0.0.1:3311

# 2) 棋服务器：主题包必须放在可执行文件旁边（wasm 客户端报的是 OS_ID_WINDOWS，
#    服务器找的是 pc_theme.zip）
cd /tmp/cnchess_stage
LD_LIBRARY_PATH=<soui4>/build/bin ./cnchess_server config/config.xml
```

Linux 下 `cnchess_server` 不带参数时走 `run(NULL)`，只认默认端口 3311、不读 `config/config.xml`；
要改端口/配置就把配置路径当第一个参数传进去。

浏览器打开 `http://localhost:8080/cnchess-wasm.html`，登录框里的地址**不用改**（默认就是
`ws://localhost:8080`），点「登 录」即开始下载主题。要手工指定别的服务器就在框里写
`127.0.0.1:3311` 这种 `ip:port`，那条走浏览器直连，不经过反代。

三条容易踩空的口径：

- **缺隔离头不报错**：`window.crossOriginIsolated` 为 false 时页面照跑，只是静默退回泵后端，
  状态行的 `worker N/M` 会一直停在 `0/0`。`tools/wasm/serve-isolated.py` 带 `--no-isolation` 就是专门用来
  复现这条退出的。
- **默认地址是同源，不是 3311**：`MainDlg.cpp` 的 `__EMSCRIPTEN__` 分支把登录框默认值填成
  `ws://` + `location.host`，跟线上"页面和 `/cnchess/` 同域名、由 Apache/nginx 反代到同机
  cnchess_server"的部署形态一致。所以只起静态服务（`python3 -m http.server`、或
  `tools/wasm/serve-isolated.py` 没给 `--ws-proxy`）时，点了登录不会有任何下载 —— 握手打到了静态服务
  自己身上，症状是状态行 `ws 0/0`、`theme 0`，而 `cnchess_server` 日志里连一条新连接都没有。
  `--ws-proxy HOST:PORT` 就是把这一层反代在本机顶上：带 `Upgrade: websocket` 的请求原样隧道
  给上游，拿到 101 后双向搬字节。
- **主题包条目的根必须是主题内容本身**：`config.xml` 要在 zip 顶层，不能带 `pc_theme/` 前缀
  （客户端按 `<缓存目录>/def_theme/config.xml` 判缓存、按同一目录加载主题，`NormalizeZipPath`
  只做分隔符换算、不剥根目录）。`smoke.js` 的打包口径是对的：
  `cd games/cnchess/client/pc_theme && zip -r <stage>/pc_theme.zip .`。手搓一份带前缀的包，
  症状是解压报成功（`extracted 46/46 files`）但主题加载失败，随后 `InitGame` 里
  `RuntimeError: null function` —— 这条在 2026-11-04 实测过，与 `-pthread` 无关。

### 主题与配置的落地目录

wasm 端没有"个人目录"，主题缓存与登录配置都放在 `/tmp` 下面（`MainDlg.cpp` 的
`__EMSCRIPTEN__` 分支）：主题包落 `/tmp/theme_cache/def_theme.zip`，解压到
`/tmp/theme_cache/def_theme/`，配置写 `/tmp/cnchess_cfg.xml`。不能放 `/opt/soui`：worker 后端
必须开 `-sWASMFS=1`，而它把 `--preload-file` 铺出来的整棵树当成只读资源，在里面建目录直接
`EACCES`。浏览器没有跨会话磁盘，所以这份配置只在当前页面生命周期内有效，刷新一次就回到默认值。

### 加载进度遮罩

`.wasm`（调试档约 30 MB，上线档约 5 MB）与 `.data`（约 0.5 MB 预载资源）要先从服务器拉完才能
建控件树，公网部署下这段等待很长。壳页和 `demos/wasm-demo/soui.html`
用同一套做法：页面自己用 `fetch` 带进度地读这两个文件，再经加载器的两个官方钩子交进去
——`Module.getPreloadedPackage` 给 `.data`（同步返回 `ArrayBuffer`，跳过加载器自己的下载）、
`Module.instantiateWasm` 给 `.wasm`（自己编译并回调，跳过它自己的 fetch）。遮罩盖在画布上，
显示合计百分比、已下/总字节、实时速度（EMA 平滑，200 ms 一测）与剩余时间估计；DOM 每
100 ms 才刷一次，每个 chunk 都改 DOM 会拖慢下载本身。

两处容易忽略的口径：

- `fetch` 带 `cache: 'no-cache'`。产物文件名是固定的，不加的话换构建后浏览器还在复用旧
  缓存，跑的是上一版，容易被当成"控件缺陷"去查。
- 启动期失败（下载 404、`WebAssembly` 实例化异常、`DoConfig` 失败）同时写进遮罩和状态行，
  遮罩转成红色并停住。这些失败都发生在画面出来之前，只写状态行的话用户看到的是"一直卡在
  加载"。

遮罩在第一帧真正 blit 到画布之后才淡出（`onWasmReady` 里 `presentDirtyRect` 之后），早一步
会露出空白画布。

> 跨域：`ws://` 连接不经过 CORS，但页面必须是 `http://`（或 `https://`）加载的；
> 用 `https://` 部署页面时服务器那端要是 `wss://`，浏览器不允许 `https` 页面连明文 `ws`。

> **HTTPS 站点不要给默认地址补 `:3311`**：登录框默认填的是 `wss://` + `location.host`
> （例如 `wss://www.soui.com.cn`），端口留空即 443；`WebSocketClient.cpp` 会把请求路径写成
> `/cnchess/?...`，由 Apache/nginx 把 `/cnchess/` 反代到同机 `cnchess_server:3311`（配置见
> `conf/extra/httpd-cnchess.conf`）。后端 3311 监听的是**明文 ws**，写 `wss://host:3311` 会
> 在 TLS 握手就失败（`ERR_SSL_PROTOCOL_ERROR`），写 `ws://host:3311` 又会被 HTTPS 页面按混合
> 内容拦掉。两种都是「点登录没反应」。要验证反代是否通，用 curl 带 `Upgrade: websocket` 打
> `https://<域名>/cnchess/`，期望 101。

## 观测点

页面状态行每 500 ms 读一次 `CnchessStat`，排障时不用开控制台：

```
pump 335 · present 56 · theme 1 · board 103 · taskloop 75/0/1 · worker 1/75 · ws 73/73 (pt=1)
```

`CnchessStat` 的下标是烟测与页面共享的契约（见 `cnchess_main.cpp` 的注释）：

| 下标 | 含义 |
| --- | --- |
| 0 / 1 | 泵次数 / 上屏次数 |
| 2 / 3 | 宿主窗口是否建好 / 字体是否可用（canvas2d 浏览器字体） |
| 4 | 主题是否加载（服务器下发的主题包走完了 ws 收包 + 落盘 + 解压 + 加载） |
| 5 | `chessboard` 的子节点数（布局里自带 1 个 `btn_test`，`InitGame` 后是 103） |
| 6 / 7 / 8 | TaskLoop 累计执行数 / 排队数 / 存活实例数 |
| 9 / 15 | 登录弹窗可见 / 存在 |
| 10 / 11 | 根窗口宽 / 高 |
| 12 | 上一帧泵是否超预算被截断 |
| 13 | `GetTickCount()` |
| 14 | 主题进度弹窗可见 |
| 16 / 17 | 根窗口直接子节点数 / 主对话框已初始化 |
| 18 / 19 / 20 | worker 后端：当前由 worker 驱动的队列数 / 在 worker 上执行掉的任务数 / 这份产物带没带 `-pthread` |
| 21 / 22 | 交给 `IConnListener` 的收包帧数 / 其中不在主线程上的帧数 |

后三个是 worker 后端的取证三口：20 说构建能力，18 说运行时真起过几条 worker（页面没有
`crossOriginIsolated` 时退回泵后端，这里就是 0），19 与 6 **相等**才是"解压整条离开了 UI 线程"
—— 光看 `6 > 0` 两条后端都给得出，区分不了。

21 / 22 是同一口径用在收包链上：21 与页面实际收到的帧数对齐，说明没有帧被投递链吞掉；22 与 21
**相等**才是"拆包和回调都不在主线程上"。6 里现在也含这 21 条投递任务，所以整轮登录下来的典型形状是
`taskloop 75/0/1 · worker 1/75 · ws 73/73 (pt=1)`：73 条投递 + 1 条解压 + 1 条杂项，`live` 收尾是 1
而不是 0 —— 每条连接的那条投递队列跟着页面活着（桌面版也是每连接一条线程活到断开），
临时队列（解压那条）释放掉才是这里要看的。

浏览器里也能直接读：页面把实例挂在 `window.cnchess` 上，控制台里
`cnchess._CnchessStat(4)` 就是主题状态。

## 无头烟测

```bash
# worker 后端（默认产物）
node games/cnchess/client/wasm/smoke.js games/cnchess/client/wasm/out_pt/cnchess-wasm.js
# 泵后端
node games/cnchess/client/wasm/smoke.js games/cnchess/client/wasm/out/cnchess-wasm.js
```

它不需要浏览器：自己起一个真 `cnchess_server`（临时目录里放 `pc_theme.zip` 与 `config/`，
端口取空闲端口），用自带的 RFC6455 客户端冒充 `global.WebSocket`，然后跑完整条链——
开会话、注入登录、收主题包（约 4.4 MB / 73 个二进制帧）、解压、加载、插棋盘、放一次音效，
退出码即结论。断言条数按后端分流：**worker 产物 44 条、泵产物 42 条** —— worker 腿四条专属
（起过 worker 线程、`offUi==run`、逐帧采到活的实例、`ws 交付 offUi==frames`），泵腿一条专属
（`workers==0 && offUi==0`，外加它的 `ws 交付 offUi==0`），净差两条。node 没有 `crossOriginIsolated` 这道前提，两条后端都能真的跑起来，所以这份 A/B 是本地
最便宜的证据；2026-11-04 实测两边各自全绿。

弹窗相关的两条断言（进度弹窗已关闭 / 登录弹窗已销毁）走 `settle()` 而不是"注入 + 泵一帧"：
SOUI 的模态入场/出场动画只由 `kPulseTimer` 的 `WM_TIMER` 推进，而 `WM_TIMER` 按**真实时间**到期，
node 里连续空泵几乎不花钱（实测 32 帧只走 1 ms），一帧动画都推不动。同一道理在
`CnchessClick` 里按 1.5 s 墙上时间放开命中等待。

环境变量：`CNCHESS_SERVER`（服务器二进制）、`CNCHESS_THEME_ZIP`（主题包）、
`CNCHESS_TIMEOUT`（默认 30 s）、`CNCHESS_KEEP=1`（保留临时目录）。

变异用来证明断言不是摆设。前三条是既有记录（本轮未重跑），最后一条是本轮新跑的：

| 变异 | 预期失败 |
| --- | --- |
| `CnchessPump` 里删掉 `STaskLoopPump` | 主题不落地、棋盘不插、进度弹窗不关、TaskLoop 计数全 0（6 条） |
| 删掉 `IMessageLoop::ExecutePendingTask()` | 连 `onConnected` 都进不去，收发都是 0（6 条） |
| 拼 URL 时丢掉端口 | 连接 URL 断言逐字符不符（8 条） |
| 把 worker 后端的分流断言改成"只看 `run>0`" | 泵产物也会绿 —— 正是本轮要避免的假证；现在的写法是 worker 腿要求 `offUi==run`、泵腿要求 `offUi==0 && workersPeak==0`，两条腿不可能同时给绿 |
| `WsClientWasm.cpp` 的 `postOne()` 改成永远 `runOne()`（投递退回主线程） | worker 腿只红新加的那一条：`FAIL worker 后端：交付全部发生在非主线程 [offUi=0/73]`，其余 43 条照绿；`taskloop` 回到 `run=1`（只剩解压那条），`ws frames` 仍是 73 —— 证明"帧交出去了"和"交出去时在哪条线程"是两条独立的判据（2026-11-06 本机实测） |

## 手动验证清单（Chromium，一次约 2 分钟）

前提：`cnchess_server` 跑着、`pc_theme.zip` 按上面的口径打包（`config.xml` 在 zip 顶层），
页面用 `tools/wasm/serve-isolated.py` 起（worker 产物必须要 COOP+COEP，并且要带
`--ws-proxy 127.0.0.1:<棋服务器端口>`，否则默认的同源地址打不到棋服务器）。

1. 打开页面 → 遮罩先给出下载进度（百分比、已下/总 MB、速度与剩余时间），随后显示
   「正在编译并初始化中国象棋…」，第一帧出来后淡出；状态行出现
   `pump N · present N · theme 0 · board 1 · … (pt=1)`，画面是登录弹窗。想看清进度中间态就把产物
   放到带宽受限的机器上，或用 `python3 -m http.server` 之外会分块回写的服务器。
   反向验证：临时把 `.data` 改名再刷新，遮罩应转红并停在
   `加载失败：cnchess-wasm.data 下载失败：HTTP 404`（这条在 2026-10-31 本机实测过）。
2. 控制台确认 `crossOriginIsolated` 为 `true`、`cnchess._CnchessStat(20)` 为 `1`。
   少了隔离头时页面不报错，只是静默退回泵后端 —— 第 4 步的 `worker` 停在 `0/0`、`ws` 第二段停在
   `0/N` 是唯一线索。
3. 登录框里的地址保持默认（同源，`ws://localhost:8080`），点「登 录」。这条走
   `--ws-proxy` 反代，与线上同域部署形态一致；反代没起时改成 `127.0.0.1:3311` 这种 `ip:port`
   直连也能登，只是绕过了要验的这条路。
   反向验证：只起静态服务（不带 `--ws-proxy`）时用默认地址点登录，状态行停在
   `theme 0 · board 1 · ws 0/0`，`cnchess_server` 日志里没有新连接（2026-11-06 本机实测）。
4. 主题进度弹窗出现，依次显示「正在下载…」「正在解压…」，然后在 1~2 秒内关闭。
   进入大厅：顶部四个页签（大厅 / 残局 / 开心象棋 / 关于），大厅里 6 张桌子，右上「在线: 1」。
   此时状态行应为 `theme 1 · board 103 · taskloop 75/0/1 · worker 1/75 · ws 73/73 (pt=1)`：
   `worker` 末尾那个数与前面的总数相等，是"解压与收包交付整条跑在 worker 上"的直接证据；
   `ws` 两段相等说明 73 帧都交到了监听器手里、且都不在主线程；`taskloop` 三段收尾回到 `x/0/1`
   表示队列跑空、只剩每条连接那条长驻投递队列（解压那条临时队列已释放，这里若是 2 就是漏了）。
   无头烟测在 2026-11-06 本机两条腿各自全绿（worker 腿 44 条 / 泵腿 42 条，数字见上）。
   **浏览器侧这一条本轮没重跑**：上次实测（2026-11-04，改动前）是 `iso=true pt=1 run=1 workerRun=1`、
   峰值 `live/pending/worker实例 = 1/1/1`、最大帧间隔 230 ms；对照腿（`out/` +
   `python3 -m http.server`）`pt=0 run=1 workerRun=0`、峰值 `worker=0`、最大帧间隔 298 ms。
   复跑时要按上面的新数字逐段核对，`ws` 第二段在隔离页里必须是 `73/73` 这个形状。
   这条链上占大头的是 ws 收包、MD5、落盘与主题加载本身，所以帧间隔只当观测、不当判据。
5. 点一张空桌的「空位」→ 自己的头像落到座位上，底部出现一排操作按钮；再点「准备」（猪头图标）
   → 进入棋局页。
6. 棋局页：90 个棋子、两侧头像、右侧「求和 / 认输 / 悔棋」、中间 60 秒回合计时器在走；
   落子后机器人会还着，棋盘实时刷新。
7. 键盘：在登录框里用输入法打中文昵称（组合输入代理 `#ime-proxy`），提交后文本进编辑框。
8. 音效：走一步棋能听到落子声（`sounds.xml` 查名 → `/tmp/theme_cache/def_theme/Sound/…` →
   WebAudio）。这条在无头烟测里只证明字节交到了钩子上（RIFF/WAVE + 19518 字节），出声要在这里看。
9. 控制台没有 `[FATAL]`，也没有 `memory access out of bounds`、没有
   `must not be resizable`（后者是 growable SAB 直接喂给 `TextDecoder`/`ImageData` 的症状，
   见 `swinx/doc/swinx-wasm-design.md` §5.4）。

已知未覆盖：Windows 主机上的 `build-cnchess.bat`（本机没有 `cmd.exe`，只做过静态检查：纯
ASCII + CRLF、标签与 `goto` 配对、引号成对；两档实测结论都来自 `.sh`）、`macOS` 的 `.sh`
（bash 3.2 的空数组写法已按注释规避，但没在真机上跑过）、上线档 `release_pt`（本轮没重编，
`-O2 -DNDEBUG` 会改掉断言与栈行为，结论不能从调试档推过去）、Safari/Firefox 的 SAB+COEP 组合、
移动端浏览器（触摸事件走的是 pointer 事件，但没有在真机上跑过）、`https://` + `wss://` 部署、
多客户端对局（烟测只起一个客户端）。
另外 `.github/workflows/soui-wasm-gate.yml` 只构建 `soui-wasm-demo`（它的烟测不需要服务器），
`cnchess-wasm` 与 `smoke.js` 目前不在 CI 里，需要在本机按上面的命令各跑一次。

## 清理

`out/`、`out_pt/`、`release/`、`release_pt/` 都是产物目录，可以随时删；重新跑一次
`build-cnchess.sh`（Windows 上 `build-cnchess.bat`）的对应档位与后端即可。构建目录本身也可以删
——脚本认得空目录，会现场重新 `emcmake cmake` 配置。
