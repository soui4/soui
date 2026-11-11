#!/usr/bin/env bash
# =============================================================================
# build-soui.sh -- 构建 SOUI on WebAssembly 演示（Unix 主机包装器）
#
# 本文件只声明本 app 的元数据与 cmake 配置参数；参数解析、工具检查、首次配置、
# 重配置、构建、产物回显等全部流程在 __cmake/wasm-build-common.sh，四个 wasm
# app 共用同一份实现。编译与链接命令本身在 demos/wasm-demo/CMakeLists.txt。
#
# 用法（--help 打印本注释块）：
#   bash demos/wasm-demo/build-soui.sh [构建目录] [--release]
#   bash demos/wasm-demo/build-soui.sh --gdi=cairo [构建目录]
#   --cairo / --canvas2d 是 --gdi=<值> 的简写；--gdi <值> 亦可。
#
# 构建目录：位置参数 > $SOUI_WASM_BUILD > <本目录>/build；
#   --release 上线档用 $SOUI_WASM_SHIP_BUILD > <本目录>/build-release，
#   产物目录由 wasm/out 换成 wasm/release。两档都是 Release（与历史行为一致）。
#
# 可调项（CMake 缓存变量，用同名环境变量覆盖）：
#   SOUI_DEMO_FONT             字体文件（覆盖自动探测）
#   SOUI_DEMO_STACK            -sSTACK_SIZE，默认 32MB
#   SOUI_WASM_DEMO_OUT         产物目录，默认 wasm/out（--release 为 wasm/release）
#   SOUI_BUILD_SCRIPT_LUA      lua-54 + ScriptModule-LUA + uires/lua 预载
#   SOUI_BUILD_WASM_SCINTILLA  Scintilla + controls.extend 的 <scintilla> 控件
#   SWINX_WASM_GDI_BACKEND     canvas2d（默认）| cairo
#   SOUI_WASM_JOBS             并行度，默认 8
#
# 本 app 相对公共配置只多两条 config 参数：-DSOUI_BUILD_WS=OFF 与
# -DSOUI_BUILD_RES_ZIP=OFF（演示不链接 ws / resprovider-zip，保持最小构建图）。
# Windows 主机用同目录的 build.bat（同样的流程与环境变量）。
# =============================================================================
SOUI_WASM_WRAPPER="${BASH_SOURCE[0]}"
SOUI_WASM_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOUI_WASM_REPO="$(cd "$SOUI_WASM_SRC/../.." && pwd)"
SOUI_WASM_PROJECT="wasm-demo"
SOUI_WASM_TARGET="soui-wasm-demo"
SOUI_WASM_PAGE="soui-wasm-demo.html"
SOUI_WASM_BUILD_TYPE="Release"
SOUI_WASM_SHIP_TYPE="Release"
SOUI_WASM_BUILD_ENV="SOUI_WASM_BUILD"
SOUI_WASM_SHIP_BUILD_ENV="SOUI_WASM_SHIP_BUILD"
SOUI_WASM_BUILD_DIR="$SOUI_WASM_SRC/build"
SOUI_WASM_SHIP_BUILD_DIR="$SOUI_WASM_SRC/build-release"
SOUI_WASM_OUT_VAR="SOUI_WASM_DEMO_OUT"
SOUI_WASM_OUT_PATTERN="@SRC@/out"
SOUI_WASM_SHIP_OUT_PATTERN="@SRC@/release"
SOUI_WASM_ENV_VARS="SOUI_DEMO_FONT SOUI_DEMO_STACK SOUI_BUILD_SCRIPT_LUA SOUI_BUILD_WASM_SCINTILLA"
SOUI_WASM_CONFIG=(-DSOUI_BUILD_WS=OFF -DSOUI_BUILD_RES_ZIP=OFF)
SOUI_WASM_JOBS_DEFAULT="8"
SOUI_WASM_USE_PTHREADS=0
SOUI_WASM_USE_GDI=1

# shellcheck source=../../__cmake/wasm-build-common.sh
source "$SOUI_WASM_REPO/__cmake/wasm-build-common.sh" "$@"
