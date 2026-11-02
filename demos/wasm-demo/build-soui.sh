#!/usr/bin/env bash
# =============================================================================
# build-soui.sh -- 构建 SOUI on WebAssembly 演示（CMake 包装器）
#
# 编译与链接命令全部在 demos/wasm-demo/CMakeLists.txt 里（目标 soui-wasm-demo）。
# 本脚本只做三件事：查 em++、确保构建目录打开了 SOUI_BUILD_WASM_DEMO、构建目标。
# 保留它是因为现有调用方式（脚本 + 构建目录参数）不变；Windows 主机用同目录的
# build.bat（同样的流程与环境变量，见 readme.md「SOUI XML 演示（阶段 2）」）。
#
# 前置（首次配置），构建目录用第一个位置参数或 SOUI_WASM_BUILD 指定：
#   source $EMSDK/emsdk_env.sh
#   emcmake cmake -S <soui4> -B /tmp/build-soui-wasm \
#       -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_DEMO=ON
#   demos/wasm-demo/build-soui.sh /tmp/build-soui-wasm
#
# 演示的可调项都是 CMake 缓存变量，脚本只把环境变量的值透传过去重新配置：
#   SOUI_DEMO_FONT   字体文件（覆盖自动探测；自动探测按宿主平台给候选）
#   SOUI_DEMO_STACK  -sSTACK_SIZE，默认 32MB
#   SOUI_WASM_DEMO_OUT  产物目录，默认 demos/wasm-demo/out
# 可选能力开关（0/1，只在 wasm 构建图里有意义）：
#   SOUI_BUILD_WASM_LUA         lua-54 + ScriptModule-LUA + translator + uires/lua 预载
#   SOUI_BUILD_WASM_SCINTILLA   Scintilla + controls.extend 的 <scintilla> 控件
# 关掉的那一项同时从 .wasm（静态库）和 .data（预载资源）里消失。
# =============================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"   # 本目录是 demos/wasm-demo，仓库根在上两级
BUILD="${1:-${SOUI_WASM_BUILD:-/tmp/build-soui-wasm}}"
TARGET="soui-wasm-demo"

die() { echo "错误: $*" >&2; exit 1; }

command -v em++ >/dev/null 2>&1 || die "找不到 em++；请先 source <emsdk>/emsdk_env.sh"
[ -f "$BUILD/CMakeCache.txt" ] || die "没有 $BUILD/CMakeCache.txt —— 先用 emcmake cmake 配置 soui4 的 wasm 构建"

# 构建目录被整体搬过（soui-wasm -> demos/wasm-demo 这类）之后，CMakeCache 里记的
# "我是哪个目录的缓存" 就对不上了，CMake 会拒用并报一句很难懂的话。这里提前判出来。
cache_dir="$(sed -n 's/^CMAKE_CACHEFILE_DIR:INTERNAL=//p' "$BUILD/CMakeCache.txt" | tr '\\' '/')"
if command -v cygpath >/dev/null 2>&1; then
    # Windows 路径大小写不敏感：缓存里可能写 c:/ 而 cygpath 给 C:/，统一小写再比。
    now_dir="$(cygpath -m "$BUILD" | tr 'A-Z' 'a-z')"
    cache_dir="$(printf %s "$cache_dir" | tr 'A-Z' 'a-z')"
else
    now_dir="$(cd "$BUILD" && pwd)"
fi
if [ -n "$cache_dir" ] && [ "$cache_dir" != "$now_dir" ]; then
    die "构建目录被移动过：$BUILD/CMakeCache.txt 记的是 $cache_dir。
     缓存里的路径全是旧目录的，CMake 会用不了。删掉重来：
         rm -rf \"$BUILD\"
     再按 readme 的「第一次配置」跑一遍 emcmake cmake -S $ROOT -B $BUILD -G Ninja -DSOUI_BUILD_WASM_DEMO=ON"
fi

# 缓存变量在配置期生效，所以环境变量有值时每次都带上去重新配置：
# 只用 SOUI_DEMO_FONT=... build-soui.sh 这种一次性用法也能奏效。
RECONFIG=()
[ -n "${SOUI_DEMO_FONT:-}" ] && RECONFIG+=("-DSOUI_DEMO_FONT=$SOUI_DEMO_FONT")
[ -n "${SOUI_DEMO_STACK:-}" ] && RECONFIG+=("-DSOUI_DEMO_STACK=$SOUI_DEMO_STACK")
[ -n "${SOUI_WASM_DEMO_OUT:-}" ] && RECONFIG+=("-DSOUI_WASM_DEMO_OUT=$SOUI_WASM_DEMO_OUT")
[ -n "${SOUI_BUILD_WASM_LUA:-}" ] && RECONFIG+=("-DSOUI_BUILD_WASM_LUA=$SOUI_BUILD_WASM_LUA")
[ -n "${SOUI_BUILD_WASM_SCINTILLA:-}" ] && RECONFIG+=("-DSOUI_BUILD_WASM_SCINTILLA=$SOUI_BUILD_WASM_SCINTILLA")

# 老构建目录（本脚本还是手写 em++ 的时代留下的）没有这个目标，补一次配置。
# 空数组在 set -u 下要写成 ${arr[@]+"${arr[@]}"}，macOS 自带的 bash 3.2 不认裸的 "${arr[@]}"。
if ! grep -q '^SOUI_BUILD_WASM_DEMO:BOOL=ON' "$BUILD/CMakeCache.txt"; then
    echo "== $BUILD 没有打开 SOUI_BUILD_WASM_DEMO，重新配置（缓存里的其他选项保持不变）"
    cmake -S "$ROOT" -B "$BUILD" -DSOUI_BUILD_WASM_DEMO=ON ${RECONFIG[@]+"${RECONFIG[@]}"}
elif [ ${#RECONFIG[@]} -gt 0 ]; then
    cmake -S "$ROOT" -B "$BUILD" ${RECONFIG[@]+"${RECONFIG[@]}"}
fi

cmake --build "$BUILD" --target "$TARGET"

OUT=$(sed -n 's/^SOUI_WASM_DEMO_OUT:[^=]*=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")
OUT="${OUT:-$HERE/out}"
echo
echo "产物（$OUT）："
ls -la "$OUT"
echo
echo "运行： python3 -m http.server 8080 -d \"$OUT\"   然后打开 http://localhost:8080/$TARGET.html"
