cmake_minimum_required(VERSION 3.16)
project(fundholdings-wasm)

# 本文件是基金持仓 wasm 工具的顶层 CMake 工程入口；由 build-fundholdings.sh 以
# `cmake -S demos/fundholdings/wasm` 调用。SOUI4_DIR 指向仓库根，
# wasm-lib-config.cmake 把 SOUI 库本身编成 wasm 形态，随后把仓库根作为子目录
# add_subdirectory 进来（仅编库）。httpclient 等 wasm 下默认关的组件由构建脚本 -D 打开。
set(SOUI4_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH
    "soui4 repo root (three levels up from demos/fundholdings/wasm)")
set(SOUI4_BUILD "${CMAKE_BINARY_DIR}/soui4" CACHE PATH
    "soui4 build dir (library only)")
include("${SOUI4_DIR}/__cmake/wasm-lib-config.cmake")
add_subdirectory(${SOUI4_DIR} ${SOUI4_BUILD})

# =============================================================================
# demos/fundholdings/wasm/CMakeLists.txt -- 基金持仓工具的 wasm 入口
#
# 本文件是基金持仓 wasm 工具的顶层 CMake 工程入口（不再由根 CMakeLists 选择）。
#
# 与桌面目标（../CMakeLists.txt）的差别：
#   1. 桌面 main.cpp 不进构建，改用本目录的 fundholdings_main.cpp（浏览器主循环）；
#   2. 渲染器从 Skia 换成 render-gdi（wasm 的 canvas2d 后端），图片解码仍是 stb；
#   3. IHttpClient 换成 components/httpclient/wasm（同步 XHR，跑在 TaskLoop 的
#      worker 线程上），请求经本目录 serve-fundholdings.py 的同源代理转发；
#   4. 资源用桌面版 uires（1440x860），不加载系统资源。
# =============================================================================

if(SOUI_WASM_PTHREADS)
    set(_soui_fund_out_default "${CMAKE_CURRENT_SOURCE_DIR}/out_pt")
else()
    set(_soui_fund_out_default "${CMAKE_CURRENT_SOURCE_DIR}/out")
endif()
set(SOUI_WASM_FUNDHOLDINGS_OUT "${_soui_fund_out_default}" CACHE PATH
    "fundholdings wasm: output directory for fundholdings.{html,js,wasm,data}")

# 与 soui-wasm / cnchess 用同一个默认栈：SHostWnd::Create() 的建窗递归按桌面线程栈
# 深度写，5 MB 下会直接踩爆。
set(SOUI_FUNDHOLDINGS_STACK "32MB" CACHE STRING "fundholdings wasm: -sSTACK_SIZE")

foreach(_req
        "${CMAKE_CURRENT_SOURCE_DIR}/../uires/uires.idx")
    if(NOT EXISTS "${_req}")
        message(FATAL_ERROR "fundholdings wasm: 缺少 ${_req}（桌面版资源没检出？）")
    endif()
endforeach()

# 挂载点必须和 fundholdings_main.cpp 里的 kAppResDir 一致。
set(_soui_fund_preload
    "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/../uires@/opt/soui/uires")

# 观测口 / 输入口与 soui-wasm、cnchess 同一套契约。
set(_soui_fund_exports
    "_main,_FundStat,_FundPump,_FundMouse,_FundWheel,_FundKey,_FundText,_FundCaretPos,_FundTextOf,_FundClick,_SwinxClipboardSetBrowserText,_malloc,_free,_SwinxFbBits,_SwinxFbWidth,_SwinxFbHeight,_SwinxFbStride,_SwinxFbLogicalWidth,_SwinxFbLogicalHeight,_SwinxFbRequests,_SwinxFbPresents")

# 宿主页与 wasm-demo 共用 soui-host.js（下载进度、上屏、键鼠/滚轮注入、IME 代理、
# 剪贴板、崩溃上报）。壳页 fundholdings.html 只写排版与启动参数。
set(_soui_host_js_default "${SOUI4_DIR}/demos/wasm-demo/soui-host.js")
set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH
    "fundholdings wasm: shared soui-host.js, copied next to the artifacts")
if(NOT EXISTS "${SOUI_HOST_JS}" AND EXISTS "${_soui_host_js_default}")
    set(SOUI_HOST_JS "${_soui_host_js_default}" CACHE FILEPATH
        "fundholdings wasm: shared soui-host.js" FORCE)
endif()
if(NOT EXISTS "${SOUI_HOST_JS}")
    message(WARNING "SOUI_HOST_JS 不存在: ${SOUI_HOST_JS} —— 产物目录里不会有 soui-host.js")
endif()

# SGridCtrl 与桌面构建同口径：直接把 controls.extend/gridctrl 的源文件编进来，
# 不走 ExtendCtrls（那会把 hellocharts/Scintilla 等无关控件一起拉进 wasm）。
set(_soui_fund_src
    ../MainDlg.cpp
    ../SFundWidgets.cpp
    ../applog.cpp
    ../fund_eastmoney.cpp
    ../fund_estimate.cpp
    ../stdafx.cpp
    ${SOUI4_DIR}/controls.extend/gridctrl/SGridCtrl.cpp
    ${SOUI4_DIR}/controls.extend/gridctrl/SSelRangeMgr.cpp
    fundholdings_main.cpp)

add_executable(fundholdings-wasm ${_soui_fund_src})

set_target_properties(fundholdings-wasm PROPERTIES
    OUTPUT_NAME "fundholdings"
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY "${SOUI_WASM_FUNDHOLDINGS_OUT}")
foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(fundholdings-wasm PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${SOUI_WASM_FUNDHOLDINGS_OUT}")
endforeach()

# 应用源码按 -O0 编译，链接期整模块优化才是体积/速度来源；出错时行号可读。
target_compile_options(fundholdings-wasm PRIVATE -O0 -g -Wall -Wno-unused-variable)
target_compile_definitions(fundholdings-wasm PRIVATE
    SWINX_FB_WIDTH=${SWINX_FB_WIDTH}
    SWINX_FB_HEIGHT=${SWINX_FB_HEIGHT})

target_include_directories(fundholdings-wasm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${SOUI4_DIR}
    ${SOUI4_DIR}/config
    ${SOUI4_DIR}/utilities/include
    ${SOUI4_DIR}/SOUI/include
    ${SOUI4_DIR}/components
    ${SOUI4_DIR}/components/httpclient
    ${SOUI4_DIR}/components/httpclient/wasm
    ${SOUI4_DIR}/controls.extend
    ${SOUI4_DIR}/controls.extend/gridctrl
    ${SOUI4_DIR}/swinx/include
    ${SOUI4_DIR}/swinx/src/platform/wasm
    ${SOUI4_BUILD}/config)

target_link_libraries(fundholdings-wasm PRIVATE
    soui4 utilities4 swinx
    render-gdi imgdecoder-stb aupng
    httpclient taskloop msftedit msls31
    brotlidec brotlicommon
    m)

target_link_options(fundholdings-wasm PRIVATE
    "-sMODULARIZE=1"
    "-sEXPORT_NAME=createFundHoldings"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sSTACK_SIZE=${SOUI_FUNDHOLDINGS_STACK}"
    "-sEXPORTED_RUNTIME_METHODS=HEAPU32,HEAP32,HEAPU8"
    "-sEXPORTED_FUNCTIONS=${_soui_fund_exports}"
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/fundholdings.html"
    # swinx 的 canvas2d 后端用 emscripten::val 与 Canvas 通信，运行时由 embind 提供。
    "-lembind"
    # 数据段必须落在 0x10000 以上，否则串字面量会被 IS_INTRESOURCE 判成原子号。
    "-Wl,--stack-first"
    ${_soui_fund_preload})

file(GLOB_RECURSE _soui_fund_res "${CMAKE_CURRENT_SOURCE_DIR}/../uires/*")
set_property(TARGET fundholdings-wasm APPEND PROPERTY LINK_DEPENDS
    ${_soui_fund_res}
    "${CMAKE_CURRENT_SOURCE_DIR}/fundholdings.html"
    "${SOUI_HOST_JS}"
    "${CMAKE_CURRENT_SOURCE_DIR}/serve-fundholdings.py"
    "${CMAKE_CURRENT_SOURCE_DIR}/start-server.bat")

# 产物目录要能整体拷走独立部署：静态页 + wasm 产物 + 自带的服务脚本与启动脚本。
add_custom_command(TARGET fundholdings-wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SOUI_HOST_JS}"
        "${SOUI_WASM_FUNDHOLDINGS_OUT}/soui-host.js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/serve-fundholdings.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/start-server.bat"
        "${SOUI_WASM_FUNDHOLDINGS_OUT}"
    COMMENT "Copying soui-host.js / server scripts for fundholdings-wasm")

message(STATUS "fundholdings-wasm: ${SWINX_FB_WIDTH}x${SWINX_FB_HEIGHT}, "
               "stack=${SOUI_FUNDHOLDINGS_STACK}, taskloop=${SOUI_WASM_PTHREADS} "
               "(ON=worker / OFF=pump) -> ${SOUI_WASM_FUNDHOLDINGS_OUT}")
