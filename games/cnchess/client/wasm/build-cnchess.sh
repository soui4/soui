#!/usr/bin/env bash
# =============================================================================
# build-cnchess.sh -- 构建中国象棋客户端的 wasm 版（Unix 主机 CMake 包装器）
#
# 编译与链接命令全部在 games/cnchess/client/wasm/CMakeLists.txt 里（目标
# cnchess-wasm）。本脚本只做三件事：查 em++、把构建目录按档位配好、构建目标。
# Windows 主机用同目录的 build-cnchess.bat（同样的档位、同样的环境变量）。
#
# 两个脚本各覆盖两档 x 两条 TaskLoop 后端，产物目录按 soui-wasm 的口径分开：
#   ./build-cnchess.sh [构建目录]              调试档 RelWithDebInfo -> wasm/out_pt
#   ./build-cnchess.sh --no-pthreads ...       同上，但退回泵后端（解压跑在 UI 线程）-> wasm/out
#   ./build-cnchess.sh --release [构建目录]    上线档 Release        -> wasm/release_pt
# 整模块 -O2 -DNDEBUG 只发生在上线档；调试档源码层 -O0 -g，出错时行号可读。
#
# TaskLoop 后端：默认打开 -DSOUI_WASM_PTHREADS=ON，主题包的解压跑在真正的 worker 线程上
# （与桌面版同口径）。这条能力的代价在浏览器侧 —— 产物要 crossOriginIsolated（COOP+COEP
# 响应头）才建得起 worker，起页面用 demos/wasm-demo/serve-isolated.py，python3 -m
# http.server 不行（缺头时不报错，只是静默退回泵后端，看漏了还以为改了）。node 烟测没有
# 这道前提，两条后端都能跑。环境变量 SOUI_WASM_PTHREADS=OFF/ON 与 --no-pthreads/--pthreads
# 等价，命令行优先。
#
# 构建目录：第一个位置参数 > 同名环境变量 > 每档各自的默认值。
#   调试档 $SOUI_WASM_BUILD          默认 /tmp/build-soui-wasm（与演示页共用也行）
#   上线档 $SOUI_CNCHESS_SHIP_BUILD  默认 /tmp/build-cnchess-ship
# 目录没配置过就现场 emcmake 配一个（-G Ninja）；已经配过则只把下面这些环境变量透传
# 进缓存重新配置，其余选项保持不变，所以增量构建不会被脚本毁掉。
# 换后端（SOUI_WASM_PTHREADS）等于整树重编，脚本每次都把它写进缓存：这是有意的，
# 让"这条命令要哪条后端"和"产物是哪条后端"永远对得上。
#
# 首次配置也可以手工完成，脚本认得已经配好的目录：
#   source $EMSDK/emsdk_env.sh
#   emcmake cmake -S <soui4> -B /tmp/build-soui-wasm \
#       -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DSOUI_BUILD_WASM_CNCHESS=ON \
#       -DSOUI_WASM_PTHREADS=ON
#   games/cnchess/client/wasm/build-cnchess.sh /tmp/build-soui-wasm
#
# 换档位（改 CMAKE_BUILD_TYPE）等于整树重编，脚本不替用户做这个决定：上线档指到一个非
# Release 的目录时报错退出，调试档沿用目录里已有的档位、只提示一句"产物名不副实"。
#
# 可调项都是 CMake 缓存变量，用同名环境变量覆盖：
#   SOUI_CNCHESS_STACK     -sSTACK_SIZE，默认 32MB（5MB 会在 SHostWnd::Create 踩爆）
#   SOUI_WASM_CNCHESS_OUT  产物目录，默认按后端取 out_pt / out；--release 取 release_pt / release
#   SOUI_WASM_PTHREADS     ON（默认）或 OFF，见上
#   SOUI_WASM_JOBS         并行度，透传给 cmake --build -j（不给就用 Ninja 的默认值）
# =============================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../../.." && pwd)"
TARGET="cnchess-wasm"

die() { echo "错误: $*" >&2; exit 1; }

RELEASE=0
# TaskLoop 后端：默认 worker（-pthread）。环境变量先给一次覆盖，命令行再盖过它。
PTHREADS=${SOUI_WASM_PTHREADS:-ON}
case "$PTHREADS" in ON|on|1|TRUE) PTHREADS=ON ;; OFF|off|0|FALSE) PTHREADS=OFF ;;
    *) echo "SOUI_WASM_PTHREADS 只认 ON/OFF，收到 '$PTHREADS'，按默认 ON 处理" >&2; PTHREADS=ON ;; esac
POSITIONAL=()
for _arg in "$@"; do
    case "$_arg" in
        -r|--release)     RELEASE=1 ;;
        -p|--pthreads)    PTHREADS=ON ;;
        -P|--no-pthreads) PTHREADS=OFF ;;
        -h|--help)    sed -n '/^set -euo pipefail/q;p' "${BASH_SOURCE[0]}"; exit 0 ;;
        *)            POSITIONAL+=("$_arg") ;;
    esac
done

command -v em++ >/dev/null 2>&1 || die "找不到 em++；请先 source <emsdk>/emsdk_env.sh"
[ -d "$ROOT/swinx/include" ] || die "找不到 $ROOT/swinx/include（子模块没检出？）"

# 产物目录：环境变量 > 按档位与后端取默认。两条后端的 .js/.wasm/.data 是三件套，
# 落在同一个目录里换构建会拿错胶水（LinkError），所以 worker 档另起 out_pt / release_pt。
_PT_SUFFIX=""
[ "$PTHREADS" = ON ] && _PT_SUFFIX="_pt"
if [ "$RELEASE" = 1 ]; then
    BUILD_TYPE="Release"
    OUT_DIR="${SOUI_WASM_CNCHESS_OUT:-$HERE/release$_PT_SUFFIX}"
    DEFAULT_BUILD="${SOUI_CNCHESS_SHIP_BUILD:-/tmp/build-cnchess-ship}"
else
    BUILD_TYPE="RelWithDebInfo"
    OUT_DIR="${SOUI_WASM_CNCHESS_OUT:-$HERE/out$_PT_SUFFIX}"
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
# 每次都写：脚本按它选产物目录，缓存里的值必须和这条命令要的后端一致。
RECONFIG+=("-DSOUI_WASM_PTHREADS=$PTHREADS")

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
OUT="${OUT:-$HERE/out$_PT_SUFFIX}"
echo
echo "产物（$OUT，TaskLoop 后端=$PTHREADS）："
ls -la "$OUT"
echo
if [ "$PTHREADS" = ON ]; then
    echo "运行： python3 \"$ROOT/demos/wasm-demo/serve-isolated.py\" \"$OUT\" --port 8080"
    echo "      然后打开 http://localhost:8080/$TARGET.html，控制台里先确认"
    echo "      window.crossOriginIsolated === true —— 缺 COOP/COEP 头时 worker 线程建不起来，"
    echo "      页面会静默退回泵后端（解压又回到 UI 线程，状态行的 worker 计数停在 0/0）。"
else
    echo "运行： python3 -m http.server 8080 -d \"$OUT\"   然后打开 http://localhost:8080/$TARGET.html"
fi
echo "      （页面里的服务器地址要填 ws://能访问到的 cnchess_server，例如 127.0.0.1:3311）"
echo "烟测： cd \"$HERE\" && node smoke.js \"$OUT/$TARGET.js\"   （需要 native 构建里的 cnchess_server）"
