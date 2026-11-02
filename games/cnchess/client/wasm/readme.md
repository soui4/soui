# 中国象棋客户端的 WebAssembly 版

把 `games/cnchess/client` 的桌面客户端整体搬进浏览器：同一套 `CMainDlg` / `ChessGame` /
`SGameTheme` 源码，换成 wasm 宿主 + 浏览器原生 WebSocket + 单线程 TaskLoop。产物是
`out/cnchess-wasm.{html,js,wasm,data}` 四个文件，`.data` 里预载了客户端资源和系统皮肤。文本不再打包字体——wasm 走 canvas2d 绘制后端，字形由浏览器的内置字体渲染（见下文"字体"一节）。

## 与桌面版的三处差异

1. **网络：`ws` 组件 → 浏览器原生 WebSocket。**
   桌面版 `ws` 组件是 libwebsockets 的封装，自带收包线程。wasm 里没有线程也没有 BSD socket，
   所以 `components/network/ws` 在 wasm 下改走 Emscripten 的 `<emscripten/websocket.h>`，
   底层就是页面的 `WebSocket` 对象：`emscripten_websocket_new` 建连、四个回调挂到 JS 侧、
   `send_binary` 把 `HEAPU8.subarray(...)` 交给浏览器。客户端代码（`WebSocketClient` /
   `WsClientListener`）一行没改，仍然是二进制帧、`dwType + dwSize` 包头。
   消息回调里的数据只在回调期间有效，所以 `ws` 组件在回调里就把载荷拷进自己的缓冲区，不在
   wasm 堆上留悬垂指针。

2. **线程：TaskLoop 变成单线程队列。**
   桌面版 TaskLoop 是线程池（`components/taskloop`），`WebSocketClient` 的收包线程、
   主题包的解压都往它上面投任务。wasm 后端（`components/taskloop/backend_wasm.cpp`）把它们
   压成一条主线程队列：`postTask` 只入队（按优先级稳定排序），`waitUntilDone` 直接内联执行
   （不能阻塞，否则死锁），真正的执行由宿主的 `STaskLoopPump(预算毫秒)` 在每帧泵里抽干。
   所以 `ThemeDownloadManager::ExtractZipAsync` 的调用点不用改，"异步解压"在浏览器里等于
   "下一帧解压"，进度回调照常打，只是不再跨线程。

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
| `out/` | 默认（调试）产物（未纳入版本控制） |
| `release/` | 上线档产物（`--release`，整模块 `-O2 -DNDEBUG`，未纳入版本控制） |

## 页面宿主：复用 `soui-wasm/soui-host.js`

上屏、键鼠/滚轮注入、输入法代理、剪贴板双向桥、WebAudio 音效、崩溃上报、加载遮罩
这一整套，全在 `soui-wasm/soui-host.js` 里——和 SOUI 演示页是**同一份文件**，构建时由
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

| 脚本 | 主机 | 调用 | 构建类型 | 产物目录 |
| --- | --- | --- | --- | --- |
| `build-cnchess.sh` | Linux / macOS | `./build-cnchess.sh [构建目录]` | `RelWithDebInfo` | `wasm/out` |
| `build-cnchess.sh` | Linux / macOS | `./build-cnchess.sh --release [构建目录]` | `Release`（整模块 `-O2 -DNDEBUG`） | `wasm/release` |
| `build-cnchess.bat` | Windows | `build-cnchess.bat [构建目录]` | `RelWithDebInfo` | `wasm\out` |
| `build-cnchess.bat` | Windows | `build-cnchess.bat release [构建目录]` | `Release` | `wasm\release` |

`release` 也可以写成 `-r` / `--release` / `/release`，与构建目录参数的先后顺序无关。实测两档
的体积（本机）：`out/cnchess-wasm.wasm` 约 40 MB（带 DWARF，链接期 binaryen 只能做有限优化），
`release/cnchess-wasm.wasm` 约 5.1 MB；`.data` 两边都是同一份预载资源，约 20 MB。

```bash
source "$EMSDK/emsdk_env.sh"

# 调试档 -> games/cnchess/client/wasm/out（构建目录没配置过就现场 emcmake 配一个）
games/cnchess/client/wasm/build-cnchess.sh /path/to/build-wasm

# 上线档 -> games/cnchess/client/wasm/release（默认自己一个目录，不用指）
games/cnchess/client/wasm/build-cnchess.sh --release
```

Windows 主机：

```bat
call <emsdk>\emsdk_env.bat
games\cnchess\client\wasm\build-cnchess.bat D:\build-soui-wasm
games\cnchess\client\wasm\build-cnchess.bat release
```

构建目录取第一个位置参数，其次是同名环境变量，最后是每档的默认值；`/tmp` 会被清空的机器
就用 `SOUI_WASM_BUILD`（调试档）与 `SOUI_CNCHESS_SHIP_BUILD`（上线档）指到持久目录：

| 环境变量 | 调试档默认 | 上线档默认 |
| --- | --- | --- |
| `SOUI_WASM_BUILD` / `SOUI_CNCHESS_SHIP_BUILD` | `/tmp/build-soui-wasm`（与演示页共用一个目录也行） | `/tmp/build-cnchess-ship` |

已经配好的目录只把下面的缓存变量透传进去重新配置，其余选项保持不变，所以增量构建不会被
脚本毁掉；档位切换（`CMAKE_BUILD_TYPE`）会触发整树重编，脚本不替用户做这个决定——上线档
指到一个非 `Release` 的目录时直接报错退出，调试档沿用目录里已有的档位、只打一行提示（产物
的优化档位跟着缓存走，不再假装是 `RelWithDebInfo`）。

缓存变量（都可以用同名环境变量由脚本透传）：

| 变量 | 默认 | 说明 |
| --- | --- | --- |
| `SOUI_CNCHESS_STACK` | `32MB` | `-sSTACK_SIZE`。SOUI 在 wasm 下走 canvas2d 绘制后端（render-gdi 接 swinx 的 canvas2d 实现，没有 freetype/cairo 递归绘制链），但 `SHostWnd::Create()` 的建窗递归仍按桌面线程栈深度写，5 MB 下直接踩爆 |
| 字体 | 浏览器内置 | canvas2d 后端用浏览器内置字体渲染文本，不再有 `SOUI_CNCHESS_FONT` 选项，也不预载 TTF（原选项已废弃） |
| `SOUI_WASM_CNCHESS_OUT` | `wasm/out` | 产物目录；`--release` / `release` 档固定成 `wasm/release`，不接受环境变量覆盖 |
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

> `build-cnchess.bat` 保持纯 ASCII：`cmd.exe` 按活动 OEM 代码页逐行解码批处理文件，UTF-8 的
> 中文注释会被切成幽灵命令（`chcp 65001` 也修不干净，分块读取仍会截断多字节序列）。中文说明
> 写在 `build-cnchess.sh` 与本文件里。它同时保持 CRLF 行尾：脚本用 `goto` 跳转，而 `cmd.exe`
> 是按块扫描批处理文件里的标签的，LF 行尾的标签正好跨块边界时会被找不到；线性的
> `soui-wasm\build.bat` 没有这个暴露面，所以那个文件是 LF。

## 运行

wasm 模块要通过 HTTP 加载（`file://` 读不了 `.data`），所以需要一个静态服务器：

```bash
# 1) 页面
python3 -m http.server 8080 -d games/cnchess/client/wasm/out

# 2) 棋服务器：主题包必须放在可执行文件旁边（wasm 客户端报的是 OS_ID_WINDOWS，
#    服务器找的是 pc_theme.zip）
cd /tmp/cnchess_stage
LD_LIBRARY_PATH=<soui4>/build/bin ./cnchess_server config/config.xml
```

Linux 下 `cnchess_server` 不带参数时走 `run(NULL)`，只认默认端口 3311、不读 `config/config.xml`；
要改端口/配置就把配置路径当第一个参数传进去。

浏览器打开 `http://localhost:8080/cnchess-wasm.html`，在登录框里把服务器地址改成
`127.0.0.1:3311`（默认是作者的公网地址），点「登 录」。

### 加载进度遮罩

`.wasm`（上线档约 5 MB，调试档约 40 MB）与 `.data`（约 20 MB 预载资源）合计几十 MB，
必须先从服务器拉完才能建控件树，公网部署下这段等待很长。壳页和 `soui-wasm/soui.html`
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

## 观测点

页面状态行每 250 ms 读一次 `CnchessStat`，排障时不用开控制台：

```
pump 1295 · present 34 · theme 1 · board 167 · taskloop run/pending/live 1/0/0
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
| 12 | 上一帧泵是否超预算被截断 |
| 14 | 主题进度弹窗可见 |
| 16 / 17 | 根窗口直接子节点数 / 主对话框已初始化 |

浏览器里也能直接读：页面把实例挂在 `window.cnchess` 上，控制台里
`cnchess._CnchessStat(4)` 就是主题状态。

## 无头烟测

```bash
node games/cnchess/client/wasm/smoke.js games/cnchess/client/wasm/out/cnchess-wasm.js
```

它不需要浏览器：自己起一个真 `cnchess_server`（临时目录里放 `pc_theme.zip` 与 `config/`，
端口取空闲端口），用自带的 RFC6455 客户端冒充 `global.WebSocket`，然后跑完整条链——
开会话、注入登录、收主题包（约 4.4 MB / 73 个二进制帧）、解压、加载、插棋盘，27 条断言，
退出码即结论。

环境变量：`CNCHESS_SERVER`（服务器二进制）、`CNCHESS_THEME_ZIP`（主题包）、
`CNCHESS_TIMEOUT`（默认 30 s）、`CNCHESS_KEEP=1`（保留临时目录）。

三条变异用来证明断言不是摆设：

| 变异 | 预期失败 |
| --- | --- |
| `CnchessPump` 里删掉 `STaskLoopPump` | 主题不落地、棋盘不插、进度弹窗不关、TaskLoop 计数全 0（6 条） |
| 删掉 `IMessageLoop::ExecutePendingTask()` | 连 `onConnected` 都进不去，收发都是 0（6 条） |
| 拼 URL 时丢掉端口 | 连接 URL 断言逐字符不符（8 条） |

## 手动验证清单（Chromium，一次约 1 分钟）

1. 打开页面 → 遮罩先给出下载进度（百分比、已下/总 MB、速度与剩余时间），随后显示
   「正在编译并初始化中国象棋…」，第一帧出来后淡出；状态行出现
   `pump N · present N · theme 0 · board 1`，画面是登录弹窗。想看清进度中间态就把产物
   放到带宽受限的机器上，或用 `python3 -m http.server` 之外会分块回写的服务器。
   反向验证：临时把 `.data` 改名再刷新，遮罩应转红并停在
   `加载失败：cnchess-wasm.data 下载失败：HTTP 404`（这条在 2026-10-31 本机实测过）。
2. 把服务器地址改成自己的 `ip:port`，点「登 录」。
3. 主题进度弹窗出现，依次显示「正在下载…」「正在解压…」，然后在 1~2 秒内关闭。
4. 进入大厅：顶部四个页签（大厅 / 残局 / 开心象棋 / 关于），大厅里 6 张桌子，右上「在线: 1」。
   此时状态行 `theme 1 · board 103`、`taskloop run/pending/live 1/0/0`。
5. 点一张空桌的「空位」→ 自己的头像落到座位上，底部出现一排操作按钮；再点「准备」（猪头图标）
   → 进入棋局页。
6. 棋局页：90 个棋子、两侧头像、右侧「求和 / 认输 / 悔棋」、中间 60 秒回合计时器在走；
   落子后机器人会还着，棋盘实时刷新。
7. 键盘：在登录框里用输入法打中文昵称（组合输入代理 `#ime-proxy`），提交后文本进编辑框。
8. 控制台没有 `[FATAL]`，也没有 `memory access out of bounds`。

已知未覆盖：Windows 主机上的 `build-cnchess.bat`（本机没有 `cmd.exe`，只做过静态检查：纯
ASCII + CRLF、标签与 `goto` 配对、引号成对；两档实测结论都来自 `.sh`）、`macOS` 的 `.sh`
（bash 3.2 的空数组写法已按注释规避，但没在真机上跑过）、移动端浏览器（触摸事件走的是
pointer 事件，但没有在真机上跑过）、`https://` + `wss://` 部署、多客户端对局（烟测只起一个
客户端）。
另外 `.github/workflows/soui-wasm-gate.yml` 只构建 `soui-wasm-demo`（它的烟测不需要服务器），
`cnchess-wasm` 与 `smoke.js` 目前不在 CI 里，需要在本机按上面的命令各跑一次。

## 清理

`out/` 与 `release/` 都是产物目录，可以随时删；重新跑一次 `build-cnchess.sh`（Windows 上
`build-cnchess.bat`）的对应档位即可。构建目录本身也可以删——脚本认得空目录，会现场重新
`emcmake cmake` 配置。
