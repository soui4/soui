# =============================================================================
# demos/fun_test/wasm/CMakeLists.txt -- fun_test wasm 单测腿的顶层工程入口
#
# 与其它 wasm 工程（demos/wasm-demo、games/cnchess/client/wasm、
# demos/fundholdings/wasm）同一套结构：自己是 project() 顶层工程，include
# __cmake/wasm-lib-config.cmake 把 SOUI 库编成 wasm 形态，再把仓库根作为子目录
# add_subdirectory 进来（仅编库）。
#
# 源集与链接参数原先写在 ../CMakeLists.txt 的 if(EMSCRIPTEN) 分支里；重构后 wasm
# 下 SOUI_BUILD_DEMOS 恒 OFF、demos/ 不进构建图，那段成了死代码，故整体搬到这里。
#
# 顶层工程里 CMAKE_SOURCE_DIR / PROJECT_SOURCE_DIR 指向本目录，所有指向仓库根的路径
# 必须用 SOUI4_DIR，库的配置产物目录用 SOUI4_BUILD/config。
# =============================================================================
cmake_minimum_required(VERSION 3.16)
project(fun_test-wasm)

set(SOUI4_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." CACHE PATH
    "soui4 repo root (three levels up from demos/fun_test/wasm)")
set(SOUI4_BUILD "${CMAKE_BINARY_DIR}/soui4" CACHE PATH
    "soui4 build dir (library only)")

# third-part/CMakeLists.txt 的 SOUI_WASM_MINIMAL 分支靠这个开关把 gtest 拉进 wasm
# 构建图；只有 fun_test 这条腿需要它，其余 wasm app 不带 gtest。
set(SOUI_BUILD_WASM_FUN_TEST ON CACHE INTERNAL
    "wasm fun_test: pull gtest into the minimal third-part graph")

include("${SOUI4_DIR}/__cmake/wasm-lib-config.cmake")
add_subdirectory(${SOUI4_DIR} ${SOUI4_BUILD})

# ---------------------------------------------------------------------------
# GDI 后端与产物目录
# ---------------------------------------------------------------------------
# SWINX_WASM_GDI_BACKEND 由 swinx/wasm.cmake 拥有（上面的 add_subdirectory 里读），
# 取值 canvas2d（默认）/ cairo；这里只读，用 -D 在配置期选择。
if(NOT DEFINED SWINX_WASM_GDI_BACKEND OR SWINX_WASM_GDI_BACKEND STREQUAL "")
    set(SWINX_WASM_GDI_BACKEND "canvas2d")
endif()
# 产物仍落在 demos/fun_test/out.<backend>/：run-node.js 与 browser-runner.html 都按
# 这个相对路径取 ../out.<backend>/fun_test-wasm.js，不能改。
set(_fun_test_out "${CMAKE_CURRENT_SOURCE_DIR}/../out.${SWINX_WASM_GDI_BACKEND}")

set(SOUI_WASM_FUN_TEST_STACK "32MB" CACHE STRING "wasm fun_test: -sSTACK_SIZE")
# 找缺陷的产物默认带可读断言：abort 时给出具体原因，而不是 "Aborted()"。
option(SOUI_WASM_FUN_TEST_ASSERTIONS "wasm fun_test: link with -sASSERTIONS=2" ON)

# ---------------------------------------------------------------------------
# 源集：只取 swinx 的 GDI / 文本 / 区域 / 句柄这一层
# ---------------------------------------------------------------------------
# 四个 TU 全是纯 Win32 兼容面调用（CreateCompatibleDC、ExtTextOut、CombineRgn、
# GetGuiResources），不碰 SOUI 控件树，也不碰系统资源包，因此不需要 --preload-file。
# 其余 TU 依赖 wasm 构建图里没有的能力（log4z / ws / network、dlopen、pthread、
# zlib 资源包、真窗口与显示），列进来是链接错误而非运行期降级，故按能力剔除。
# 不注册 CTest：宿主没有 wasm 测试启动器，ctest 起不了 .js；执行由 wasm/run-node.js
# （node + 假 DOM/canvas 桩）与产物自带的 .html（浏览器真 canvas）两条腿承担。
set(FUN_TEST_WASM_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_dc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_region.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_swinx_handle_type_unit.cpp)

add_executable(fun_test ${FUN_TEST_WASM_SRC})
target_include_directories(fun_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${SOUI4_DIR}/utilities/include
    ${SOUI4_DIR}/third-part/gtest/include
    ${SOUI4_DIR}/swinx/include
    ${SOUI4_BUILD}/config)

# swinx 是这层唯一的实现方：GDI 后端的差异全在 swinx 内部按 SWINX_WASM_GDI_BACKEND
# 分派，本目标不带 soui4 —— 这几个用例只调 Win32 兼容面。
target_link_libraries(fun_test PRIVATE gtest swinx utilities4 m)
# cairo 后端：freetype 只 PRIVATE 链 brotlidec，而 brotli 的 CMakeLists 在 Emscripten
# 下删掉了 dec -> common 这条链接边，所以最终链接必须显式带上 brotlicommon，
# 否则 _kBrotliPrefixCodeRanges 等符号全 undefined。canvas2d 分支不构建 brotli，
# TARGET 不存在时自然跳过。
if(TARGET brotlicommon)
    target_link_libraries(fun_test PRIVATE brotlicommon)
endif()
target_compile_options(fun_test PRIVATE -Wall)

set_target_properties(fun_test PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "fun_test-wasm"
    # emcc 只有在输出名是 .html 时才生成浏览器外壳；.js 仍然单独可用，node 腿直接
    # require 它（先在 wasm/run-node.js 里装好假 DOM/canvas 桩）。
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY "${_fun_test_out}")
foreach(_cfg IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set_target_properties(fun_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${_cfg} "${_fun_test_out}")
endforeach()

target_link_options(fun_test PRIVATE
    # canvas2d 后端用 emscripten::val 持有离屏 canvas / 2D context，emval 运行时由
    # embind 提供；cairo 后端不引用这些符号，多带一份不改变产物。
    "-lembind"
    "-sALLOW_MEMORY_GROWTH=1"
    "-sSTACK_SIZE=${SOUI_WASM_FUN_TEST_STACK}"
    "-sEXPORTED_FUNCTIONS=_main,_malloc,_free"
    # gtest 跑完 main 返回即退出：node 腿要拿到退出码，浏览器腿只是停止泵帧。
    "-sEXIT_RUNTIME=1"
    # 数据段必须落在 0x10000 以上，否则 swinx 的 IS_INTRESOURCE 会把串字面量当成
    # 原子号（同一件事的完整说明见 demos/wasm-demo/CMakeLists.txt 末尾）。
    "-Wl,--stack-first")
if(SOUI_WASM_FUN_TEST_ASSERTIONS)
    target_link_options(fun_test PRIVATE "-sASSERTIONS=2")
endif()

message(STATUS "fun_test-wasm: gdi-backend=${SWINX_WASM_GDI_BACKEND}, "
               "out=${_fun_test_out}, stack=${SOUI_WASM_FUN_TEST_STACK}, "
               "assertions=${SOUI_WASM_FUN_TEST_ASSERTIONS}")

