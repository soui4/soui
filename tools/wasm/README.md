# tools/wasm —— wasm 程序的共用宿主资产

这里放的是「用 swinx 跑起来的 wasm app」在浏览器/本地开发时需要、但**不属于任何一个 app**的
两个文件。app 自己的东西（壳页 `xxx.html`、C++ 入口、资源、专用服务入口）仍然留在各自目录里。

| 文件 | 作用 | 消费方 |
| --- | --- | --- |
| `soui-host.js` | 浏览器端宿主：带进度的下载、按需载入胶水、帧缓冲上屏、鼠标/键盘/滚轮/触摸注入（触摸折算 `WM_MOUSEWHEEL`）、IME 组合输入代理、剪贴板双向桥、WebAudio 音效、崩溃上报、加载遮罩 | `demos/wasm-demo`、`demos/fundholdings/wasm`、`games/cnchess/client/wasm` 三个 wasm 程序的产物目录 |
| `serve-isolated.py` | 本地静态服务：正确的 `.wasm/.data/.js` MIME、COOP/COEP 与 `no-store`、可选的 WebSocket 隧道 | 三个程序的 `build*.sh/bat` 提示行默认指向它（`SOUI_WASM_SERVER`；fundholdings 换成了自己的入口），以及手测/CI 起页面 |

## 为什么在仓库根，而不是 `swinx/tools/` 或某个 app 目录

- **依赖方向**：`swinx/tools/` 里是「造 swinx 这个库的工具」（`bakefont`、`cross_build.py`、
  `gen_cptable.py`）；这里的两个文件反过来——是「调用 swinx 的 app 的宿主」。放回任何一侧都是
  耦合倒挂。
- **`swinx` 是独立 git 子模块**（多远端对外分发）。放进子模块意味着：改一行 js 要跨两个仓库提交、
  父仓库的 app 构建要依赖子模块内文件（子模块没初始化/指针没同步就出不了包）、app 侧的页面胶水
  会跟着库一起分发给不做 wasm 的使用者。2026-11 拆 `serve-isolated.py` 时已经比过这两个候选，
  结论同上。
- **契约在库里、实现在消费者侧**：`soui-host.js` 调用的那批导出（`_SwinxFbBits/Width/Height/Stride`、
  `SwinxInject*`、插入符位置、剪贴板、文件选择回投）是 swinx 的 wasm ABI，契约写在
  `swinx/doc/swinx-wasm-design.md`（§10 导出表、文件选择双向闭环）。库定义契约、app 拿契约写宿主，
  比「都在库里」更清楚。

## 接入方式

### 1. 宿主 js：一行 CMake

各 wasm app 的顶层 `CMakeLists.txt` 里，target 建好之后：

```cmake
include("${SOUI4_DIR}/__cmake/wasm-host-js.cmake")
soui_wasm_host_js(<target> <out_dir>)
```

`__cmake/wasm-host-js.cmake` 的 `soui_wasm_host_js()` 做四件事：定 `SOUI_HOST_JS` 缓存变量
（默认本目录的 `soui-host.js`，可用 `-DSOUI_HOST_JS=<path>` 换一份）→ 旧构建目录里指向搬迁前
路径的缓存自动回落 → 把 js 加进 target 的 `LINK_DEPENDS`（只改 js 也会重链）→ POST_BUILD
`copy_if_different` 到 `<out_dir>/soui-host.js`。文件不存在是**配置期 `FATAL_ERROR`**：拷不过去
的后果是页面白屏，早点炸掉。

**别在 app 的 CMakeLists 里再写一份 copy 命令**，三处重复正是这次合并要消掉的东西。

### 2. 开发服务器

```bash
# 产物目录里直接跑（directory 缺省是当前目录）
python3 tools/wasm/serve-isolated.py games/cnchess/client/wasm/out_pt --port 8080
python3 tools/wasm/serve-isolated.py demos/wasm-demo/out --no-isolation   # 对照：验证降级到泵后端
```

参数：`--port`（默认 8080）、`--bind`、`--no-isolation`（故意不发 COOP/COEP）、
`--ws-proxy HOST:PORT`（把带 `Upgrade: websocket` 的请求隧道进去）。`-pthread` 产物必须用带隔离
头的服务，否则 `crossOriginIsolated` 为 false，TaskLoop 静默退回泵后端（能用但不并行）。

构建脚本通过两个变量指向它：`SOUI_WASM_SERVER`（默认 `tools/wasm/serve-isolated.py`）与
`SOUI_WASM_SERVER_ARGS`，在 `__cmake/wasm-build-common.{sh,bat}` 里拼进末尾的提示行。app 有同源
专用端点时**不要改这个脚本**，在 app 目录写自己的入口并按 importlib 加载本模块
（样例：`demos/fundholdings/wasm/serve-fundholdings.py` 的 `/__soui_http` 转发）。

## 改 `soui-host.js` 要注意

- **契约**：页面三行（`<script src="soui-host.js">` + 容器 + `SouiHost.start({...})`）、导出名与
  配置项都写在文件头部注释里；C++ 侧的完整实现是 `demos/wasm-demo/soui_main.cpp` 的「宿主接线」一节。
  默认导出名是 `_SwinxDemo*`，其它 app 用 `cfg.api` 覆盖（fundholdings 用 `_Fund*`，cnchess 用
  `_Cnchess*`）。
- **生效方式**：改了 js 不需要重编 wasm；正常 `cmake --build` 会因为 `LINK_DEPENDS` 重链并把新 js
  拷进产物目录。只改了文件又不想等重链时，手工把 js 拷到 `out*/` 也等效——但下次构建仍会按
  `LINK_DEPENDS` 覆盖一遍。
- **文件属性**：无 BOM + **全 CRLF**。用会把行尾统一成 LF 的编辑器改，`git diff` 会变成 1260 行的
  全文件重写；批量替换请按字节处理并断言 BOM/行尾没变。
- **验证**：`node demos/wasm-demo/smoke.js`（无头，覆盖启动链、上屏计数、输入注入、文件选择回投）；
  触摸/IME/剪贴板这类只能真机确认的，按 `demos/wasm-demo/readme.md` 的验证清单在手机浏览器上过一遍。

## 相关文档

- `demos/wasm-demo/readme.md` —— 「复用宿主」一节：新程序只写 C++ 的接入步骤与页面侧能力清单。
- `swinx/doc/swinx-wasm-design.md` —— §3.6 链接参数与产物、§10 导出表与宿主契约。
- `games/cnchess/client/wasm/readme.md` —— 另一个 app 视角的接入说明（后端开关、隔离头、WS 隧道）。
