# =============================================================================
# games/cnchess/client/wasm/CMakeLists.txt -- 中国象棋客户端的 wasm 入口
#
# 只在 Emscripten 下、且根 CMakeLists 打开 SOUI_BUILD_WASM_CNCHESS 时进入。
#
# 和 soui-wasm 的区别只有三处：
#   1. 客户端自身的源码（../*.cpp）要一起编进这个模块；桌面入口 main.cc 和
#      android_entry.cc / ohos_entry.cc 不进构建（它们自带 _tWinMain / JNI 入口）。
#   2. 弹窗走 SModalRoot（登录、确认、求和/应答），不走 LoginDlg/PeaceXxxDlg 的
#      SHostDialog::DoModal()：那两个类不进构建，因为 DoModal 是嵌套消息循环，
#      在浏览器里拿不到新事件。
#   3. 多链三个组件：taskloop（任务队列，两条后端）、ws（浏览器原生 WebSocket）、
#      resprovider-zip（解压服务器下发的主题包，wasm 下用 swinx 的 zlib_static）。
#
# TaskLoop 的后端由根 CMakeLists 的 SOUI_WASM_PTHREADS 决定，cnchess 的构建脚本默认
# 打开它：主题包的解压因此跑在一条真正的 worker 线程上，与桌面版"工作线程解压"同口径，
# 不再占 UI 帧。代价是产物要 crossOriginIsolated（COOP+COEP 响应头）才起得来线程 ——
# 用 demos/wasm-demo/serve-isolated.py 起页面，python3 -m http.server 那一套不行。
# 关掉它（build-cnchess.sh --no-pthreads）就退回泵后端：解压仍在 UI 线程的
# STaskLoopPump 里跑完，页面不再需要隔离头。两种产物的 .js/.wasm/.data 是三件套，
# 混在同一目录里换构建会出现"新 wasm 配旧胶水"的 LinkError，所以产物目录按后端分开。
# =============================================================================

# 后端决定默认产物目录（out / out_pt），与演示那边同一套口径。显式给
# SOUI_WASM_CNCHESS_OUT 时以它为准；构建脚本总会按请求的后端把它算好再传进来，
# 免得老构建目录里缓存的 out 把 worker 产物盖到泵产物上。
if(SOUI_WASM_PTHREADS)
    set(_soui_cnchess_out_default "${CMAKE_CURRENT_SOURCE_DIR}/out_pt")
else()
    set(_soui_cnchess_out_default "${CMAKE_CURRENT_SOURCE_DIR}/out")
endif()
set(SOUI_WASM_CNCHESS_OUT "${_soui_cnchess_out_default}" CACHE PATH
    "cnchess wasm: output directory for cnchess.{html,js,wasm,data}")

# 与 soui-wasm 用同一个默认栈：SOUI 在 wasm 下走 canvas2d 绘制后端（render-gdi 接
# swinx 的 canvas2d 实现，文本由浏览器内置字体渲染），没有 freetype/cairo 的递归绘制链，
# 但 SHostWnd::Create() 的建窗递归仍按桌面线程栈深度写，5 MB 下会直接踩爆。
set(SOUI_CNCHESS_STACK "32MB" CACHE STRING "cnchess wasm: -sSTACK_SIZE")

# 字体：canvas2d 后端用浏览器的内置字体渲染文本（与 demos/wasm-demo 的 route C 一致），
# 不再从 MEMFS 预载 TTF，也不再调用 AddFontResource。原 SOUI_CNCHESS_FONT 选项已废弃。

foreach(_req
        "${CMAKE_CURRENT_SOURCE_DIR}/../uires/uires.idx"
        "${PROJECT_SOURCE_DIR}/soui-sys-resource/uires.idx")
    if(NOT EXISTS "${_req}")
        message(FATAL_ERROR "cnchess wasm: 缺少 ${_req}（资源目录没检出？）")
    endif()
endforeach()

# 资源挂载点必须和 cnchess_main.cpp 里的 kAppResDir/kSysResDir 一致
# （文本走 canvas2d 的浏览器内置字体，不再挂载 /opt/soui/fonts）。
set(_soui_cnchess_preload
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/../uires@/opt/soui/uires"
    "--preload-file=${PROJECT_SOURCE_DIR}/soui-sys-resource@/opt/soui/soui-sys-resource")

# _malloc/_free：JS 侧要把控件名/文本写进线性内存再传进来。
# 观测口（CnchessStat/CnchessChildXxx）是无头烟测唯一的取证方式：
#   登录弹窗是否建出来、连接后主题是否落地、game_container 下棋盘是否插入、
#   TaskLoop 跑在哪条后端上（执行数/排队数/存活实例数，以及 worker 实例数与
#   worker 上执行掉的任务数 —— 后者是"解压真的离开了 UI 线程"的唯一直接证据）。
# 输入口按 soui-wasm 的同一套契约给全（漏一项，页面调用就是
# "m._CnchessXxx is not a function" 直接抛异常）：
#   _CnchessWheel        滚轮 -> WM_MOUSEWHEEL（wParam 高字是 ±120 的整数倍）
#   _CnchessCaretPos     插入符位置，页面把输入法代理框钉到光标旁
#   _CnchessPlayEffectSound 主题音效链路取证（sounds.xml -> 前缀 -> PlaySound）
#   _SwinxClipboardSetBrowserText 浏览器 paste 事件把文本写进 swinx 的 SClipboard，
#     随后注入的 Ctrl+V 才读得到（符号在 swinx/platform/wasm/SConnection.cpp 里，
#     不是演示专有，cnchess 直接复用）
set(_soui_cnchess_exports
    "_main,_CnchessStat,_CnchessPump,_CnchessMouse,_CnchessWheel,_CnchessKey,_CnchessText,_CnchessCaretPos,_CnchessPlayEffectSound,_CnchessClick,_CnchessChildRect,_CnchessChildText,_CnchessChildVisible,_CnchessChildCount,_CnchessSetText,_SwinxClipboardSetBrowserText,_malloc,_free,_SwinxFbBits,_SwinxFbWidth,_SwinxFbHeight,_SwinxFbStride,_SwinxFbLogicalWidth,_SwinxFbLogicalHeight,_SwinxFbRequests,_SwinxFbPresents")

# 页面宿主：与 demos/wasm-demo/soui.html 共用同一份 soui-host.js（下载进度、上屏、
# 键鼠/滚轮注入、IME 代理、剪贴板、WebAudio 音效、崩溃上报都在里面）。壳页
# cnchess.html 只写自己的排版与启动参数，不再维护第二份宿主实现。
# 产物目录里必须有它：页面用相对路径 <script src="soui-host.js">，缺了直接白屏。
set(_soui_host_js_default "${PROJECT_SOURCE_DIR}/demos/wasm-demo/soui-host.js")
set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH
    "cnchess wasm: shared soui-host.js, copied next to the artifacts")
# CACHE 变量只写一次：搬迁（soui-wasm/ -> demos/wasm-demo/）之前配置过的构建目录里
# 还留着旧路径，文件已经不在了。这里回落到新默认值，免得 cnchess 产物没有宿主 js。
if(NOT EXISTS "${SOUI_HOST_JS}" AND EXISTS "${_soui_host_js_default}")
    set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH "cnchess wasm: shared soui-host.js" FORCE)
endif()
if(NOT EXISTS "${SOUI_HOST_JS}")
    message(WARNING "SOUI_HOST_JS 不存在: ${SOUI_HOST_JS} —— 产物目录里不会有 soui-host.js，页面打不开")
endif()

# LoginDlg/PeaceReqDlg/PeaceAckDlg 的 DoModal 版本在 wasm 里没有入口（见文件头
# 第 2 条），不参与编译也就不会把 SHostDialog 的第二条消息循环链进来。
set(_soui_cnchess_src
    ../ChessBoard.cpp
    ../ChessGame.cpp
    ../ChessPiece.cpp
    ../CnchessSkin.cpp
    ../EndgameHandler.cpp
    ../LobbyHandler.cpp
    ../MainDlg.cpp
    ../myprofile.cpp
    ../SGameTheme.cpp
    ../stdafx.cpp
    ../ThemeDownloadManager.cpp
    ../utils.cpp
    ../WebSocketClient.cpp
    ../WsClientListener.cpp
    cnchess_main.cpp)

add_executable(cnchess-wasm ${_soui_cnchess_src})

set_target_properties(cnchess-wasm PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY "${SOUI_WASM_CNCHESS_OUT}")
foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(cnchess-wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${SOUI_WASM_CNCHESS_OUT}")
endforeach()

# 客户端源码按 -O0 编译：链接期整模块优化（-O2）才是体积/速度的来源，把 -O0 留
# 在源码层，出错时行号可读。
target_compile_options(cnchess-wasm PRIVATE -O0 -g -Wall -Wno-unused-variable)
target_compile_definitions(cnchess-wasm PRIVATE
    SWINX_FB_WIDTH=${SWINX_FB_WIDTH}
    SWINX_FB_HEIGHT=${SWINX_FB_HEIGHT})

target_include_directories(cnchess-wasm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../algorithm
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/utilities/include
    ${PROJECT_SOURCE_DIR}/SOUI/include
    ${PROJECT_SOURCE_DIR}/components
    ${PROJECT_SOURCE_DIR}/controls.extend
    ${PROJECT_SOURCE_DIR}/soui-sys-resource
    ${PROJECT_SOURCE_DIR}/swinx/include
    ${PROJECT_SOURCE_DIR}/swinx/src/platform/wasm
    ${PROJECT_BINARY_DIR}/config)

# taskloop/ws/resprovider-zip 由 components/CMakeLists.txt 的 SOUI_WASM_MINIMAL
# 分支在 SOUI_BUILD_WASM_CNCHESS 打开时加入；zlib_static 是 swinx 自带的那份，
# 不能再链 third-part/zlib（同一批符号的第二份拷贝）。
target_link_libraries(cnchess-wasm PRIVATE
    cnchess_algorithm
    soui4 utilities4 swinx
    render-gdi imgdecoder-stb aupng
    ExtendCtrls msftedit msls31
    taskloop ws resprovider-zip zlib_static
    brotlidec brotlicommon
    m)
if(SOUI_BUILD_WASM_SCINTILLA)
    target_link_libraries(cnchess-wasm PRIVATE Scintilla)
endif()

target_link_options(cnchess-wasm PRIVATE
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createCnchess"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sSTACK_SIZE=${SOUI_CNCHESS_STACK}"
    "-sEXPORTED_RUNTIME_METHODS=HEAPU32,HEAP32,HEAPU8"
    "-sEXPORTED_FUNCTIONS=${_soui_cnchess_exports}"
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/cnchess.html"
    # swinx 的 canvas2d 后端（gdi.cpp）用 emscripten::val 与 Canvas 通信，它的运行时
    # （_emval_incref/decref/invoke/new_cstring…）由 embind 提供。不是 EM_JS 就不需要
    # 链接期这一项 —— 缺了会在最终 wasm 链接期报一屏 _emval_* undefined。
    "-lembind"
    # 数据段必须落在 0x10000 以上：swinx 用 Win32 的 IS_INTRESOURCE 判"字符串还是
    # 原子号"，判据是 (ULONG_PTR)p >> 16 == 0。--no-stack-first 布局下数据段从
    # 1024 开始，串字面量会落进低 64 KB，窗口类名被当成原子号，CreateWindowEx 失败。
    "-Wl,--stack-first"
    ${_soui_cnchess_preload})

file(GLOB_RECURSE _soui_cnchess_res
    "${CMAKE_CURRENT_SOURCE_DIR}/../uires/*"
    "${PROJECT_SOURCE_DIR}/soui-sys-resource/*")
set_property(TARGET cnchess-wasm APPEND PROPERTY LINK_DEPENDS
    ${_soui_cnchess_res}
    "${CMAKE_CURRENT_SOURCE_DIR}/cnchess.html"
    "${SOUI_HOST_JS}")

# 宿主 js 拷到产物目录（与 cnchess-wasm.html 同目录）。同时进了上面的
# LINK_DEPENDS，所以改 soui-host.js 会重链并重拷；这里再 copy_if_different，
# 换目标不重编时也能补上。
add_custom_command(TARGET cnchess-wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SOUI_HOST_JS}"
        "${SOUI_WASM_CNCHESS_OUT}/soui-host.js"
    COMMENT "Copying soui-host.js for cnchess-wasm")

message(STATUS "SOUI_WASM_CNCHESS: ${SWINX_FB_WIDTH}x${SWINX_FB_HEIGHT}, stack=${SOUI_CNCHESS_STACK}, "
               "taskloop=${SOUI_WASM_PTHREADS} (ON=worker / OFF=pump), "
               "font=canvas2d(browser) -> ${SOUI_WASM_CNCHESS_OUT}")
