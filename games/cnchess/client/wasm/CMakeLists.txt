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
#   3. 多链三个组件：taskloop（单线程队列）、ws（浏览器原生 WebSocket）、
#      resprovider-zip（解压服务器下发的主题包，wasm 下用 swinx 的 zlib_static）。
# =============================================================================

set(SOUI_WASM_CNCHESS_OUT "${CMAKE_CURRENT_SOURCE_DIR}/out" CACHE PATH
    "cnchess wasm: output directory for cnchess.{html,js,wasm,data}")

# 与 soui-wasm 用同一个默认栈：SOUI 在 wasm 下走 canvas2d 绘制后端（render-gdi 接
# swinx 的 canvas2d 实现，文本由浏览器内置字体渲染），没有 freetype/cairo 的递归绘制链，
# 但 SHostWnd::Create() 的建窗递归仍按桌面线程栈深度写，5 MB 下会直接踩爆。
set(SOUI_CNCHESS_STACK "32MB" CACHE STRING "cnchess wasm: -sSTACK_SIZE")

# 字体：GDI 后端由 swinx/wasm.cmake 的 SWINX_WASM_GDI_BACKEND 决定（与 libswinx.a
# 同一次编译参数，业务层在命令行 -DSWINX_WASM_GDI_BACKEND=... 即可同时驱动 swinx
# 与 cnchess 两端）：
#   canvas2d（默认）：文本走浏览器内置字体，不预载字体、不调 AddFontResource；
#   cairo：文本/字体由 freetype + fontconfig 负责，必须把字体文件预载进 MEMFS 的
#          /opt/soui/fonts，并在 cnchess_main.cpp 里 AddFontResourceA 注册。
# 原 SOUI_CNCHESS_FONT 选项已废弃。

# SWINX_WASM_GDI_BACKEND 是 swinx/wasm.cmake 定义的 cache 变量；这里给个回落默认，
# 避免 cnchess 子目录先于 swinx 处理、而命令行又没传该变量时取到空值。
if(NOT DEFINED SWINX_WASM_GDI_BACKEND OR SWINX_WASM_GDI_BACKEND STREQUAL "")
    set(SWINX_WASM_GDI_BACKEND "canvas2d")
endif()

foreach(_req
        "${CMAKE_CURRENT_SOURCE_DIR}/../uires/uires.idx"
        "${PROJECT_SOURCE_DIR}/soui-sys-resource/uires.idx")
    if(NOT EXISTS "${_req}")
        message(FATAL_ERROR "cnchess wasm: 缺少 ${_req}（资源目录没检出？）")
    endif()
endforeach()

# 资源挂载点必须和 cnchess_main.cpp 里的 kAppResDir/kSysResDir 一致。
# canvas2d 不挂字体目录；cairo 把 fonts/ 下的字体预载进 MEMFS 的 /opt/soui/fonts
# （cnchess_main.cpp 在 CNCHESS_WASM_CAIRO_FONTS 宏下枚举并 AddFontResourceA）。
set(_soui_cnchess_preload
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/../uires@/opt/soui/uires"
    "--preload-file=${PROJECT_SOURCE_DIR}/soui-sys-resource@/opt/soui/soui-sys-resource")
set(_soui_cnchess_cairo_fonts OFF)
if(SWINX_WASM_GDI_BACKEND STREQUAL "cairo")
    set(_soui_cnchess_fonts_dir "${CMAKE_CURRENT_SOURCE_DIR}/fonts")
    if(NOT IS_DIRECTORY "${_soui_cnchess_fonts_dir}")
        message(FATAL_ERROR "cairo 后端需要字体目录 ${_soui_cnchess_fonts_dir}（放 .ttf/.ttc/.otf）")
    endif()
    list(APPEND _soui_cnchess_preload
         "--preload-file=${_soui_cnchess_fonts_dir}@/opt/soui/fonts")
    set(_soui_cnchess_cairo_fonts ON)
endif()

# _malloc/_free：JS 侧要把控件名/文本写进线性内存再传进来。
# 观测口（CnchessStat/CnchessChildXxx）是无头烟测唯一的取证方式：
#   登录弹窗是否建出来、连接后主题是否落地、game_container 下棋盘是否插入、
#   TaskLoop 是否真的只在主线程上跑（执行数/排队数/存活实例数）。
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
if(_soui_cnchess_cairo_fonts)
    # cnchess_main.cpp 据此把 /opt/soui/fonts 下的字体枚举并 AddFontResourceA。
    target_compile_definitions(cnchess-wasm PRIVATE CNCHESS_WASM_CAIRO_FONTS)
endif()

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

if(_soui_cnchess_cairo_fonts)
    set(_soui_cnchess_font_mode "cairo(bundled) -> /opt/soui/fonts")
else()
    set(_soui_cnchess_font_mode "canvas2d(browser)")
endif()
message(STATUS "SOUI_WASM_CNCHESS: ${SWINX_FB_WIDTH}x${SWINX_FB_HEIGHT}, stack=${SOUI_CNCHESS_STACK}, "
               "gdi-backend=${SWINX_WASM_GDI_BACKEND}, font=${_soui_cnchess_font_mode} -> ${SOUI_WASM_CNCHESS_OUT}")
