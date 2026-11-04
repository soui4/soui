#!/usr/bin/env bash
# =============================================================================
# build-cnchess.sh -- 构建中国象棋客户端的 wasm 版（Unix 主机 CMake 包装器）
#
# 编译与链接命令全部在 games/cnchess/client/wasm/CMakeLists.txt 里（目标
# cnchess-wasm）。本脚本只做三件事：查 em++、把构建目录按档位配好、构建目标。
# Windows 主机用同目录的 build-cnchess.bat（同样的档位、同样的环境变量）。
#
# 两个脚本各覆盖两档，产物目录按 soui-wasm 的口径分开：
#   ./build-cnchess.sh [构建目录]              调试档 RelWithDebInfo -> wasm/out
#   ./build-cnchess.sh --release [构建目录]    上线档 Release        -> wasm/release
# 整模块 -O2 -DNDEBUG 只发生在上线档；调试档源码层 -O0 -g，出错时行号可读。
#
# 构建目录：第一个位置参数 > 同名环境变量 > 每档各自的默认值。
#   调试档 $SOUI_WASM_BUILD          默认 /tmp/build-soui-wasm（与演示页共用也行）
#   上线档 $SOUI_CNCHESS_SHIP_BUILD  默认 /tmp/build-cnchess-ship
# 目录没配置过就现场 emcmake 配一个（-G Ninja）；已经配过则只把下面这些环境变量透传
# 进缓存重新配置，其余选项保持不变，所以增量构建不会被脚本毁掉。
#
# 首次配置也可以手工完成，脚本认得已经配好的目录：
#   source $EMSDK/emsdk_env.sh
#   emcmake cmake -S <soui4> -B /tmp/build-soui-wasm \
#       -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_CNCHESS=ON
#   games/cnchess/client/wasm/build-cnchess.sh /tmp/build-soui-wasm
#
# 换档位（改 CMAKE_BUILD_TYPE）等于整树重编，脚本不替用户做这个决定：上线档指到一个非
# Release 的目录时报错退出，调试档沿用目录里已有的档位、只提示一句"产物名不副实"。
#
# 可调项都是 CMake 缓存变量，用同名环境变量覆盖：
#   SOUI_CNCHESS_STACK     -sSTACK_SIZE，默认 32MB（5MB 会在 SHostWnd::Create 踩爆）
#   SOUI_WASM_CNCHESS_OUT  产物目录，默认 wasm/out；--release 时固定成 wasm/release
#   SOUI_WASM_JOBS         并行度，透传给 cmake --build -j（不给就用 Ninja 的默认值）
#
# GDI 后端（也可用同名环境变量 SWINX_WASM_GDI_BACKEND）：
#   --gdi=canvas2d|cairo   选择 swinx 的 wasm GDI 后端。canvas2d（默认）走浏览器
#                          Canvas 2D + 系统字体，不打包字体；cairo 走软件光栅 +
#                          freetype/fontconfig，必须把字体放进 wasm/fonts/ 一起打包。
#                          --cairo / --canvas2d 是简写；不指定则沿用构建目录缓存值。
# =============================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
TARGET="cnchess-wasm"

die() { echo "错误: $*" >&2; exit 1; }

RELEASE=0
GDI_BACKEND="${SWINX_WASM_GDI_BACKEND:-}"
POSITIONAL=()
for _arg in "$@"; do
    case "$_arg" in
        -r|--release) RELEASE=1 ;;
        --gdi=*)      GDI_BACKEND="${_arg#*=}" ;;
        --cairo)      GDI_BACKEND="cairo" ;;
        --canvas2d)   GDI_BACKEND="canvas2d" ;;
        -h|--help)    sed -n '/^set -euo pipefail/q;p' "${BASH_SOURCE[0]}"; exit 0 ;;
        *)            POSITIONAL+=("$_arg") ;;
    esac
done
case "$GDI_BACKEND" in
    ""|canvas2d|cairo) ;;
    *) die "--gdi 只支持 canvas2d 或 cairo（收到: $GDI_BACKEND）" ;;
esac

command -v em++ >/dev/null 2>&1 || die "找不到 em++；请先 source <emsdk>/emsdk_env.sh"
[ -d "$ROOT/swinx/include" ] || die "找不到 $ROOT/swinx/include（子模块没检出？）"

if [ "$RELEASE" = 1 ]; then
    BUILD_TYPE="Release"
    OUT_DIR="$HERE/release"
    DEFAULT_BUILD="${SOUI_CNCHESS_SHIP_BUILD:-/tmp/build-cnchess-ship}"
else
    BUILD_TYPE="RelWithDebInfo"
    OUT_DIR="${SOUI_WASM_CNCHESS_OUT:-}"
    DEFAULT_BUILD="${SOUI_WASM_BUILD:-/tmp/build-soui-wasm}"
fi
BUILD=""
[ ${#POSITIONAL[@]} -gt 0 ] && BUILD="${POSITIONAL[0]}"
BUILD="${BUILD:-$DEFAULT_BUILD}"

# 缓存变量在配置期生效，所以环境变量有值时每次都带上去重新配置：
# 空数组在 set -u 下要写成 ${arr[@]+"${arr[@]}"}，macOS 自带的 bash 3.2 不认裸的 "${arr[@]}"。
RECONFIG=()
[ -n "${SOUI_CNCHESS_STACK:-}" ] && RECONFIG+=("-DSOUI_CNCHESS_STACK=$SOUI_CNCHESS_STACK")
[ -n "$OUT_DIR" ] && RECONFIG+=("-DSOUI_WASM_CNCHESS_OUT=$OUT_DIR")
# GDI 后端：--gdi 或同名环境变量给出时才透传，否则沿用缓存里的值。
[ -n "$GDI_BACKEND" ] && RECONFIG+=("-DSWINX_WASM_GDI_BACKEND=$GDI_BACKEND")

FRESH_CONFIGURE=0
if [ ! -f "$BUILD/CMakeCache.txt" ]; then
    echo "== 配置 $BUILD（$BUILD_TYPE，Ninja）"
    emcmake cmake -S "$ROOT" -B "$BUILD" -G Ninja \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DSOUI_BUILD_WASM_CNCHESS=ON ${RECONFIG[@]+"${RECONFIG[@]}"}
    FRESH_CONFIGURE=1
elif [ "$RELEASE" = 1 ] && ! grep -q "^CMAKE_BUILD_TYPE:STRING=Release" "$BUILD/CMakeCache.txt"; then
    # 上线档必须 own 一个 Release 的目录：指到调试档那个共享目录上会把 RelWithDebInfo
    # 的产物落到 release/ 里，看起来像上线档其实带符号。改构建类型又是整树重编，
    # 不替用户做这个决定，直接停下来给出口。
    _cached=$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
    die "$BUILD 的构建类型是 ${_cached:-空}，不是 Release；上线档请用自己的目录（默认 /tmp/build-cnchess-ship，可用 SOUI_CNCHESS_SHIP_BUILD 覆盖）"
elif ! grep -q "^CMAKE_BUILD_TYPE:STRING=$BUILD_TYPE" "$BUILD/CMakeCache.txt"; then
    # 调试档沿用目录里已有的档位（改类型等于整树重编），只提醒产物名不副实。
    _cached=$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
    echo "提示: $BUILD 的构建类型是 ${_cached:-空}，脚本不改它，优化档位跟着缓存走；要 -O2 -DNDEBUG 的上线档请用 --release 和它自己的目录"
fi

# 老构建目录（还没开过这个开关，或者只缺下面这几个缓存变量）补一次配置；刚由
# emcmake 配好的目录已经带上 -DSOUI_BUILD_WASM_CNCHESS 与全部透传项，不用再跑一遍。
if [ "$FRESH_CONFIGURE" = 0 ]; then
    if ! grep -q '^SOUI_BUILD_WASM_CNCHESS:BOOL=ON' "$BUILD/CMakeCache.txt"; then
        echo "== $BUILD 没有打开 SOUI_BUILD_WASM_CNCHESS，重新配置（缓存里的其他选项保持不变）"
        cmake -S "$ROOT" -B "$BUILD" -DSOUI_BUILD_WASM_CNCHESS=ON ${RECONFIG[@]+"${RECONFIG[@]}"}
    elif [ ${#RECONFIG[@]} -gt 0 ]; then
        cmake -S "$ROOT" -B "$BUILD" ${RECONFIG[@]+"${RECONFIG[@]}"}
    fi
fi

JOBS=()
[ -n "${SOUI_WASM_JOBS:-}" ] && JOBS+=("-j${SOUI_WASM_JOBS}")
cmake --build "$BUILD" --target "$TARGET" ${JOBS[@]+"${JOBS[@]}"}

OUT=$(sed -n 's/^SOUI_WASM_CNCHESS_OUT:[^=]*=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
OUT="${OUT:-$HERE/out}"
echo
echo "产物（$OUT）："
ls -la "$OUT"
echo
echo "运行： python3 -m http.server 8080 -d \"$OUT\"   然后打开 http://localhost:8080/$TARGET.html"
echo "      （页面里的服务器地址要填 ws://能访问到的 cnchess_server，例如 127.0.0.1:3311）"
echo "烟测： cd \"$HERE\" && node smoke.js \"$OUT/$TARGET.js\"   （需要 native 构建里的 cnchess_server）"
