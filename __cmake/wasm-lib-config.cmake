# =============================================================================
# __cmake/wasm-lib-config.cmake -- SOUI 库在 WebAssembly(Emscripten) 下的构建配置
#
# 由各 wasm app 的顶层 CMakeLists 在 add_subdirectory(<soui4>) 之前 include 进来，
# 把 SOUI 库本身编成 wasm 形态（最小静态库：swinx + utilities4 + SOUI 核心 +
# 最小组件）。根 CMakeLists 不再感知 wasm，也不决定编译哪个 app —— app 选择由
# 各自的 wasm CMakeLists 负责。
#
# 静态形态是必须的：浏览器没有 dlopen，组件库无法按 SOUI_ENABLE_CORE_LIB=OFF 的
# DLL 方式在运行时加载，只能编进最终二进制。
#
# 组件白名单不写在这里，而是由 third-part/CMakeLists.txt 和
# components/CMakeLists.txt 各自的 SOUI_WASM_MINIMAL 分支决定——那两个文件才是
# 自己内容的所有者。
# =============================================================================

# 最小库构建总开关：被 third-part / components / 根 CMakeLists 的 SOUI_WASM_MINIMAL
# 分支消费，决定哪些 third-part / 组件进图。
set(SOUI_WASM_MINIMAL ON CACHE INTERNAL
    "wasm build: swinx + utilities4 + SOUI core + minimal components")

# 库编成静态 core + com（wasm 下必须静态）。
set(SOUI_ENABLE_CORE_LIB ON CACHE BOOL "Enable compile 'core' as static lib")
set(SOUI_ENABLE_COM_LIB ON CACHE BOOL "Enable compile 'components' as static lib")

# wasm 下这些不进库。HTTPCLIENT 默认 OFF，需要 HTTP 能力的 wasm app 由自己的
# 构建脚本 -DSOUI_BUILD_HTTPCLIENT=ON 打开。
set(SOUI_BUILD_DEMOS OFF CACHE BOOL "Build builtin demos")
set(SOUI_BUILD_GAMES OFF CACHE BOOL "Build soui games")
set(SOUI_BUILD_TOOLS OFF CACHE BOOL "Build soui tools")
set(SOUI_BUILD_HTTPCLIENT OFF CACHE BOOL "Build HTTP client component")
set(SOUI_BUILD_WS OFF CACHE BOOL "Build Websocket module")

# 页面上真正能打字的 <edit> 需要 richedit：SOUI 的 SRichEdit 有两条路，桌面构建
# 默认走 LoadLibrary 加载系统 riched20/msftedit，wasm 没有 dlopen，只能走
# INIT_RICHEDIT 的静态链接版本（SOUI/CMakeLists.txt 里由本开关决定）。
set(SOUI_BUILD_RICHEDIT ON CACHE BOOL "using richedit from source code")

# SVG 在 wasm 下也开：Lua 游戏页（消消乐/俄罗斯方块/坦克大战）的皮肤全是
# src="svg:..."（多帧雪碧图 + 九宫药丸底），关掉之后这些 skin 拿不到 ISvgObj，
# 整页画成空白。实现只多一份 header-only 的 nanosvg（SOUI/src/core/Svg.cpp）
# 加 render-gdi 的 SVG 绘制分支。
set(SOUI_ENABLE_SVG ON CACHE BOOL "Enable SVG support")

set(SOUI_ENABLE_SPY OFF CACHE BOOL "Enable Swnd Spy")
set(SOUI_ENABLE_ACC OFF CACHE BOOL "Enable MSAA/IAccessible (provided by swinx)")

# 可选能力开关（仅 wasm 最小构建图有意义，桌面构建不受其约束）：
#   SOUI_BUILD_WASM_SCINTILLA —— Scintilla + controls.extend 的 <scintilla> 控件，
#       关掉直接从 .wasm 里省掉 Scintilla 的 .cxx 体积；被 controls.extend 与
#       各 wasm app 的 CMakeLists 消费。
option(SOUI_BUILD_WASM_SCINTILLA
       "wasm: build Scintilla and the <scintilla> control from controls.extend" ON)

# 可选能力开关：worker 线程。打开后整个 wasm 构建图带上 -pthread：
#   components/TaskLoop 的 worker 后端（ITaskLoop::start 建一条 worker 线程，UI 只
#   入队 + Atomics 唤醒）需要它；同时必须 -sWASMFS=1 —— 默认 JS 文件系统下，worker
#   里的任何 stdio 都要浏览器主线程在 futex 忙等里代为处理请求，而本端口的主线程
#   从不阻塞（实测：主线程不睡时 worker 的 fopen 永不返回；WASMFS=1 后 47ms 跑完）。
#   代价是 SharedArrayBuffer，也就是站点必须 crossOriginIsolated（COOP+COEP 响应
#   头），python -m http.server 那套起法跑不了。
option(SOUI_WASM_PTHREADS
       "wasm: build with -pthread so ITaskLoop can run on a worker thread" OFF)
if(SOUI_WASM_PTHREADS)
    # swinx 与 components 都按这个宏分派后端（不靠编译器 __EMSCRIPTEN_PTHREADS__：
    # 那个宏只有编译单元自己带 -pthread 时才定义，而宿主需要知道链接期能力是否可用）。
    add_compile_options("-pthread")
    add_compile_definitions(SOUI_WASM_PTHREADS=1)
    # 链接期三条设置放在这里，让每个 wasm 可执行目标（演示 / cnchess / fundholdings）
    # 拿到同一份配置 —— 只在某个 app 的 CMakeLists 里补，别的目标一开这个开关就会
    # 在 wasm-ld 期报错（cnchess 实测：INITIAL_MEMORY 默认 16MB < STACK_SIZE 32MB）。
    #   -sWASMFS=1        文件系统搬进线性内存。默认的 JS 文件系统下，worker 里的
    #                     每次 stdio 都要浏览器主线程在 futex 忙等里代跑，而本端口
    #                     的主线程从不阻塞（实测：主线程不睡时 worker 的 fopen 永不
    #                     返回，换 WASMFS=1 后同一个读 47ms 跑完）。
    #   -sINITIAL_MEMORY  Emscripten 硬性要求它大于 STACK_SIZE；栈是主线程的静态栈，
    #                     worker 栈另从堆里按 4MB 一条分配（TaskLoopWasm.cpp）。
    #   GrowableArrayBuffers 1 = 有 resizable buffer 就用，没有照旧。pthreads 与
    #                     ALLOW_MEMORY_GROWTH 并存时，不用它的构建要走一段很慢的胶水。
    add_link_options("-sWASMFS=1" "-sINITIAL_MEMORY=64MB" "-sGROWABLE_ARRAYBUFFERS=1")
    message(STATUS "SOUI_WASM_PTHREADS: -pthread + -sWASMFS=1，产物需要 crossOriginIsolated 才能跑")
endif()

# Emscripten 用 clang 编译，而根 CMakeLists 的 Darwin 分支告警抑制只按目标平台判断，
# 交叉编译到 wasm 时不生效：同一批头文件会在 macOS/Linux 主机上各刷上万条
# -Winconsistent-missing-override。这里按编译器口径补一条，与 Darwin 分支一致。
add_compile_options(-Wno-inconsistent-missing-override)
