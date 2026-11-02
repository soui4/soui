#!/usr/bin/env bash
# =============================================================================
# build.sh -- 链接 swinx wasm 裸窗口演示
#
# 阶段 0 的 wasm 构建只覆盖 swinx（SOUI 上层还没进 wasm 构建），所以这个演示
# 不走 soui4 根 CMakeLists，直接调 em++ 链 libswinx.a + vendor 静态库。
#
# 前置：先用 emcmake 配好并构建 swinx 的 wasm 产物，例如
#   source $EMSDK/emsdk_env.sh
#   emcmake cmake -S soui4/swinx -B /tmp/build-swinx-wasm-clean \
#       -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_PLATFORM=wasm \
#       -DSOUI_ENABLE_CORE_LIB=ON
#   cmake --build /tmp/build-swinx-wasm-clean --target swinx -j
# 构建目录可以用环境变量 SWINX_WASM_BUILD 或第一个位置参数覆盖。
#
# libswinx.a 必须是静态库：brotli 会强制 BUILD_SHARED_LIBS OFF，共享 cairo 在
# 链接 wasm 时报 "relocation R_WASM_MEMORY_ADDR_LEB cannot be used against
# symbol kBrotliBitMask"（详见 swinx/wasm.cmake 头部说明）。
#
# 帧缓冲尺寸（SWINX_FB_WIDTH/HEIGHT）是从构建目录的 CMakeCache.txt 里读出来的，
# 因为那个尺寸已经烘进 libswinx.a 的 .bss；演示再传一遍只为保证两侧宏值一致。
# =============================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"   # 本目录是 demos/wasm-demo，仓库根在上两级
SWINX="$ROOT/swinx"
BUILD="${1:-${SWINX_WASM_BUILD:-/tmp/build-swinx-wasm-clean}}"
OUT="$HERE/out"
TARGET="swinx-wasm-demo"

die() { echo "错误: $*" >&2; exit 1; }

command -v em++ >/dev/null 2>&1 || die "找不到 em++；请先 source <emsdk>/emsdk_env.sh"
[ -f "$BUILD/libswinx.a" ] || die "没有 $BUILD/libswinx.a —— 先在 $BUILD 里 --target swinx 构建"
[ -d "$SWINX/include" ] || die "找不到 $SWINX/include（子模块没检出？）"

# CMakeCache 里是 "SWINX_FB_WIDTH:STRING=800" 或 "...:UNINITIALIZED=800"
FB_W=$(sed -n 's/^SWINX_FB_WIDTH:[^=]*=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
FB_H=$(sed -n 's/^SWINX_FB_HEIGHT:[^=]*=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
if [ -z "${FB_W:-}" ] || [ -z "${FB_H:-}" ]; then
    echo "提示: $BUILD/CMakeCache.txt 里没有 SWINX_FB_WIDTH/HEIGHT，按 framebuffer.h 的默认 320x240 编译演示"
    FB_W=${FB_W:-320}
    FB_H=${FB_H:-240}
fi

# vendor 静态库：cairo/pixman/fontconfig/freetype/png/zlib/expat/brotli。
# 目录结构随 CMake 版本可能变，所以直接扫，不写死路径。
VENDOR=()
while IFS= read -r lib; do VENDOR+=("$lib"); done < <(find "$BUILD/thirdparty" -name '*.a' | sort)
[ ${#VENDOR[@]} -gt 0 ] || die "$BUILD/thirdparty 下没有静态库"

EXPORTS="_main,_SwinxDemoStat,_SwinxDemoPump,_SwinxDemoMouse,_SwinxDemoKey,_SwinxDemoRepaint,_SwinxFbBits,_SwinxFbWidth,_SwinxFbHeight,_SwinxFbStride"

mkdir -p "$OUT"
echo "== 配置: $BUILD"
echo "== 帧缓冲: ${FB_W}x${FB_H}"
echo "== vendor 库: ${#VENDOR[@]} 个"

set -x
em++ \
    -O0 -g \
    -std=c++11 \
    -Wall \
    -DSWINX_PLATFORM_WASM \
    -DSWINX_FB_WIDTH="$FB_W" -DSWINX_FB_HEIGHT="$FB_H" \
    -I "$SWINX/include" \
    -I "$SWINX/src/platform/wasm" \
    "$HERE/main.cpp" \
    -Wl,--start-group "$BUILD/libswinx.a" "${VENDOR[@]}" -Wl,--end-group \
    -lm \
    -sMODULARIZE=1 \
    -sEXPORT_NAME=createSwinxDemo \
    -sALLOW_MEMORY_GROWTH=1 \
    -sEXPORTED_RUNTIME_METHODS=HEAPU32 \
    -sEXPORTED_FUNCTIONS="$EXPORTS" \
    --shell-file "$HERE/index.html" \
    -o "$OUT/$TARGET.html"
set +x

echo
echo "产物："
ls -la "$OUT"
echo
echo "运行： python3 -m http.server 8080 -d \"$OUT\"   然后打开 http://localhost:8080/$TARGET.html"
