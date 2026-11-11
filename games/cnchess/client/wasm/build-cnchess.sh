#!/usr/bin/env bash
# =============================================================================
# build-cnchess.sh -- 构建中国象棋客户端的 wasm 版（Unix 主机包装器）
#
# 本文件只声明本 app 的元数据与 cmake 配置参数；参数解析、工具检查、首次配置、
# 重配置、构建、产物回显等全部流程在 __cmake/wasm-build-common.sh，四个 wasm
# app 共用同一份实现。编译与链接命令本身在 wasm/CMakeLists.txt。
#
# 用法（--help 打印本注释块）：
#   ./build-cnchess.sh [构建目录]              调试档 RelWithDebInfo -> wasm/out_pt
#   ./build-cnchess.sh --no-pthreads ...       泵后端（解压跑在 UI 线程）-> wasm/out
#   ./build-cnchess.sh --release [构建目录]    上线档 Release -> wasm/release_pt
#   --gdi=canvas2d|cairo 选择 swinx 的 GDI 后端，--cairo/--canvas2d 是简写。
#
# TaskLoop 后端默认 worker（-pthread），产物需要 crossOriginIsolated；起页面用
# tools/wasm/serve-isolated.py，python3 -m http.server 缺 COOP/COEP 头时
# 会静默退回泵后端。两条后端的 .js/.wasm/.data 是三件套，落在同一目录里换构建
# 会拿错胶水（LinkError），所以 worker 档另起 out_pt / release_pt。
#
# 构建目录：位置参数 > $SOUI_WASM_BUILD > <本目录>/build；上线档用
#   $SOUI_CNCHESS_SHIP_BUILD > <本目录>/build-release（脚本不替用户改缓存里的
#   CMAKE_BUILD_TYPE，上线档指到非 Release 的目录会直接报错）。
#   每个 app 各自一棵构建树，不要共用 CMakeCache（不同 -S 会互相污染）。
#
# 可调项（CMake 缓存变量，用同名环境变量覆盖）：
#   SOUI_CNCHESS_STACK      -sSTACK_SIZE，默认 32MB
#   SOUI_WASM_CNCHESS_OUT   产物目录，默认按档位与后端取 out_pt / out / release_pt
#   SOUI_WASM_PTHREADS      ON（默认）| OFF，也可 --pthreads / --no-pthreads
#   SOUI_WASM_JOBS          并行度，默认 2
#
# 本 app 相对公共配置多两条 config 参数：-DSOUI_BUILD_WS=ON（浏览器 WebSocket
# 后端）与 -DSOUI_BUILD_RES_ZIP=ON（解压服务器下发的主题包）。
# Windows 主机用同目录的 build-cnchess.bat（同样的档位与环境变量）。
# =============================================================================
SOUI_WASM_WRAPPER="${BASH_SOURCE[0]}"
SOUI_WASM_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOUI_WASM_REPO="$(cd "$SOUI_WASM_SRC/../../../.." && pwd)"
SOUI_WASM_PROJECT="cnchess-wasm"
SOUI_WASM_TARGET="cnchess-wasm"
SOUI_WASM_PAGE="cnchess-wasm.html"
SOUI_WASM_BUILD_TYPE="RelWithDebInfo"
SOUI_WASM_SHIP_TYPE="Release"
SOUI_WASM_BUILD_ENV="SOUI_WASM_BUILD"
SOUI_WASM_SHIP_BUILD_ENV="SOUI_CNCHESS_SHIP_BUILD"
SOUI_WASM_BUILD_DIR="$SOUI_WASM_SRC/build"
SOUI_WASM_SHIP_BUILD_DIR="$SOUI_WASM_SRC/build-release"
SOUI_WASM_OUT_VAR="SOUI_WASM_CNCHESS_OUT"
SOUI_WASM_OUT_PATTERN="@SRC@/out@PT@"
SOUI_WASM_SHIP_OUT_PATTERN="@SRC@/release@PT@"
SOUI_WASM_ENV_VARS="SOUI_CNCHESS_STACK"
SOUI_WASM_CONFIG=(-DSOUI_BUILD_WS=ON -DSOUI_BUILD_RES_ZIP=ON)
SOUI_WASM_JOBS_DEFAULT="2"
# Keep the default tier in sync with build-cnchess.bat: pump build, so the
# deployed artifacts do not need COOP/COEP. -p/--pthreads is unavailable while
# SOUI_WASM_USE_PTHREADS=0; flip it back to 1 to build the worker tier.
SOUI_WASM_USE_PTHREADS=0
SOUI_WASM_NEEDS_SERVER=1
SOUI_WASM_SERVER_ARGS=" --ws-proxy 127.0.0.1:3311"
SOUI_WASM_USE_GDI=1

# shellcheck source=../../../../__cmake/wasm-build-common.sh
source "$SOUI_WASM_REPO/__cmake/wasm-build-common.sh" "$@"
