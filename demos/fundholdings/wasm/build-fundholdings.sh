#!/usr/bin/env bash
# =============================================================================
# build-fundholdings.sh -- 构建基金持仓工具 wasm 版（Unix 主机包装器）
#
# 本文件只声明本 app 的元数据与 cmake 配置参数；参数解析、工具检查、首次配置、
# 重配置、构建、产物回显等全部流程在 __cmake/wasm-build-common.sh，四个 wasm
# app 共用同一份实现。编译与链接命令本身在 wasm/CMakeLists.txt。
#
# 用法（--help 打印本注释块）：
#   ./build-fundholdings.sh [构建目录]           调试档 RelWithDebInfo -> wasm/out_pt
#   ./build-fundholdings.sh --no-pthreads ...    泵后端 -> wasm/out
#   ./build-fundholdings.sh --release [构建目录] 上线档 Release -> wasm/release_pt
#   --gdi=canvas2d|cairo 选择 swinx 的 GDI 后端，--cairo/--canvas2d 是简写。
#
# TaskLoop 后端默认 worker（-pthread）：基金数据抓取跑在真正的 worker 线程上
# （同步 XHR 只允许在 worker 里调用，这也是 httpclient/wasm 的前提）。产物需要
# crossOriginIsolated，用本目录的 serve-fundholdings.py 起页面，并且必须带
# --http-proxy（基金站点不给 CORS 头、XHR 也不能设 Cookie/Referer）。通用服务
# tools/wasm/serve-isolated.py 不含 /__soui_http 端点，别拿它直接跑基金持仓。
#
# 资源与布局：wasm 版用 demos/fundholdings/mobile/uires（竖屏 390x844），预载到
# MEMFS 的 /opt/soui/uires；因此 config 必须固定 SWINX_FB_WIDTH/HEIGHT，否则缓存
# 里残留别的尺寸会把竖屏布局裁掉。
#
# 构建目录：位置参数 > $SOUI_WASM_BUILD > <本目录>/build；上线档用
#   $SOUI_FUNDHOLDINGS_SHIP_BUILD > <本目录>/build-release。
#   每个 app 各自一棵构建树，不要共用 CMakeCache（不同 -S 会互相污染）。
#
# 可调项（CMake 缓存变量，用同名环境变量覆盖）：
#   SOUI_FUNDHOLDINGS_STACK      -sSTACK_SIZE，默认 32MB
#   SOUI_WASM_FUNDHOLDINGS_OUT   产物目录，默认按档位与后端取 out_pt / out
#   SOUI_WASM_PTHREADS           ON（默认）| OFF
#   SOUI_WASM_JOBS               并行度，默认 2
#
# Windows 主机用同目录的 build-fundholdings.bat（同样的档位与环境变量）。
# =============================================================================
SOUI_WASM_WRAPPER="${BASH_SOURCE[0]}"
SOUI_WASM_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOUI_WASM_REPO="$(cd "$SOUI_WASM_SRC/../../.." && pwd)"
SOUI_WASM_PROJECT="fundholdings-wasm"
SOUI_WASM_TARGET="fundholdings-wasm"
SOUI_WASM_PAGE="fundholdings.html"
SOUI_WASM_BUILD_TYPE="RelWithDebInfo"
SOUI_WASM_SHIP_TYPE="Release"
SOUI_WASM_BUILD_ENV="SOUI_WASM_BUILD"
SOUI_WASM_SHIP_BUILD_ENV="SOUI_FUNDHOLDINGS_SHIP_BUILD"
SOUI_WASM_BUILD_DIR="$SOUI_WASM_SRC/build"
SOUI_WASM_SHIP_BUILD_DIR="$SOUI_WASM_SRC/build-release"
SOUI_WASM_OUT_VAR="SOUI_WASM_FUNDHOLDINGS_OUT"
SOUI_WASM_OUT_PATTERN="@SRC@/out@PT@"
SOUI_WASM_SHIP_OUT_PATTERN="@SRC@/release@PT@"
SOUI_WASM_ENV_VARS="SOUI_FUNDHOLDINGS_STACK"
SOUI_WASM_CONFIG=(-DSOUI_BUILD_HTTPCLIENT=ON -DSWINX_FB_WIDTH=390 -DSWINX_FB_HEIGHT=844)
SOUI_WASM_JOBS_DEFAULT="2"
SOUI_WASM_USE_PTHREADS=1
SOUI_WASM_USE_GDI=1
# 东财接口走同源 /__soui_http，任何档位的服务都必须带 --http-proxy。
# 转发端点属于本 app，启动提示因此指向自己的入口而不是通用服务。
SOUI_WASM_NEEDS_HTTP_PROXY=1
SOUI_WASM_SERVER="demos/fundholdings/wasm/serve-fundholdings.py"
SOUI_WASM_SERVER_ARGS=" --http-proxy"

# shellcheck source=../../../__cmake/wasm-build-common.sh
source "$SOUI_WASM_REPO/__cmake/wasm-build-common.sh" "$@"
