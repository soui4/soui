#!/usr/bin/env bash
# =============================================================================
# __cmake/wasm-build-common.sh -- shared build driver for every wasm app.
#
# A per-app wrapper (e.g. demos/wasm-demo/build-soui.sh) declares the metadata
# below and `source`s this file with "$@". Everything else -- argument parsing,
# tool checks, first configure / re-configure, build and artifact report -- lives
# here, so all wasm apps share exactly the same flow. The only per-app code is
# SOUI_WASM_CONFIG (extra -D... arguments for the cmake configure step) plus the
# declarative variables.
#
# Variables supplied by the wrapper (paths absolute, no trailing slash):
#   SOUI_WASM_SRC           app wasm dir = the CMake top-level project dir
#   SOUI_WASM_REPO          soui4 repo root
#   SOUI_WASM_WRAPPER       wrapper path (its leading comment block is --help)
#   SOUI_WASM_PROJECT       CMAKE_PROJECT_NAME, used as the reconfigure probe
#   SOUI_WASM_TARGET        cmake --build target
#   SOUI_WASM_CONFIG        bash array of app-specific -D... configure args
#   SOUI_WASM_ENV_VARS      space separated env var names passed through as -D
#   SOUI_WASM_BUILD_TYPE / SOUI_WASM_SHIP_TYPE        debug / ship build type
#   SOUI_WASM_BUILD_DIR / SOUI_WASM_SHIP_BUILD_DIR    fallback build dirs
#                           (default: app wasm dir/build and /build-release)
#   SOUI_WASM_BUILD_ENV / SOUI_WASM_SHIP_BUILD_ENV    env var names for them
#   SOUI_WASM_OUT_VAR       CMake cache var holding the artifact dir; empty when
#                           the app owns its output dir (no -D, no readback)
#   SOUI_WASM_OUT_PATTERN / SOUI_WASM_SHIP_OUT_PATTERN
#                           @SRC@ / @PT@ / @GDI@ placeholders are substituted
#   SOUI_WASM_PAGE          html file name printed in the run hint
#   SOUI_WASM_USE_PTHREADS  1: expose --pthreads/--no-pthreads, default ON, and
#                           append @PT@ to the output dir (default 0)
#   SOUI_WASM_USE_GDI       1: expose --gdi=.../--cairo/--canvas2d (default 1)
#   SOUI_WASM_JOBS_DEFAULT  default -j for cmake --build (default 2)
#
# NOTE: keep this file ASCII-only; it is shared by every host.
# =============================================================================
set -euo pipefail

die() { echo "ERROR: $*" >&2; exit 1; }

# ---- wrapper contract -------------------------------------------------------
[ -n "${SOUI_WASM_SRC:-}" ]    || die "wrapper did not set SOUI_WASM_SRC"
[ -n "${SOUI_WASM_REPO:-}" ]   || die "wrapper did not set SOUI_WASM_REPO"
[ -n "${SOUI_WASM_PROJECT:-}" ] || die "wrapper did not set SOUI_WASM_PROJECT"
[ -n "${SOUI_WASM_TARGET:-}" ] || die "wrapper did not set SOUI_WASM_TARGET"
[ -f "$SOUI_WASM_REPO/__cmake/wasm-lib-config.cmake" ] || \
    die "no __cmake/wasm-lib-config.cmake under $SOUI_WASM_REPO"

SOUI_WASM_WRAPPER="${SOUI_WASM_WRAPPER:-$0}"
SOUI_WASM_BUILD_TYPE="${SOUI_WASM_BUILD_TYPE:-RelWithDebInfo}"
SOUI_WASM_SHIP_TYPE="${SOUI_WASM_SHIP_TYPE:-Release}"
SOUI_WASM_BUILD_DIR="${SOUI_WASM_BUILD_DIR:-$SOUI_WASM_SRC/build}"
SOUI_WASM_SHIP_BUILD_DIR="${SOUI_WASM_SHIP_BUILD_DIR:-$SOUI_WASM_SRC/build-release}"
SOUI_WASM_BUILD_ENV="${SOUI_WASM_BUILD_ENV:-SOUI_WASM_BUILD}"
SOUI_WASM_SHIP_BUILD_ENV="${SOUI_WASM_SHIP_BUILD_ENV:-SOUI_WASM_SHIP_BUILD}"
SOUI_WASM_OUT_VAR="${SOUI_WASM_OUT_VAR:-}"
SOUI_WASM_OUT_PATTERN="${SOUI_WASM_OUT_PATTERN:-@SRC@/out}"
SOUI_WASM_SHIP_OUT_PATTERN="${SOUI_WASM_SHIP_OUT_PATTERN:-@SRC@/release}"
SOUI_WASM_ENV_VARS="${SOUI_WASM_ENV_VARS:-}"
SOUI_WASM_PAGE="${SOUI_WASM_PAGE:-$SOUI_WASM_TARGET.html}"
SOUI_WASM_USE_PTHREADS="${SOUI_WASM_USE_PTHREADS:-0}"
SOUI_WASM_USE_GDI="${SOUI_WASM_USE_GDI:-1}"
SOUI_WASM_JOBS_DEFAULT="${SOUI_WASM_JOBS_DEFAULT:-2}"
if ! declare -p SOUI_WASM_CONFIG >/dev/null 2>&1; then SOUI_WASM_CONFIG=(); fi

# ---- args: [build_dir] [--release] [--gdi=v] [--pthreads|--no-pthreads] -----
RELEASE=0
GDI_BACKEND="${SWINX_WASM_GDI_BACKEND:-}"
PTHREADS=OFF
if [ "$SOUI_WASM_USE_PTHREADS" = 1 ]; then PTHREADS="${SOUI_WASM_PTHREADS:-ON}"; fi
POSITIONAL=()
while [ $# -gt 0 ]; do
    case "$1" in
        -h|--help)
            awk 'NR==1 {next} /^#/ || /^[[:space:]]*$/ {print; next} {exit}' \
                "$SOUI_WASM_WRAPPER"
            exit 0 ;;
        -r|--release) RELEASE=1 ;;
        --gdi=*)
            [ "$SOUI_WASM_USE_GDI" = 1 ] || die "--gdi is not supported by this app"
            GDI_BACKEND="${1#*=}" ;;
        --gdi)
            [ "$SOUI_WASM_USE_GDI" = 1 ] || die "--gdi is not supported by this app"
            shift
            [ $# -gt 0 ] || die "--gdi needs a value (canvas2d|cairo)"
            GDI_BACKEND="$1" ;;
        --cairo|--canvas2d)
            [ "$SOUI_WASM_USE_GDI" = 1 ] || die "--gdi is not supported by this app"
            GDI_BACKEND="${1#--}" ;;
        -p|--pthreads)
            [ "$SOUI_WASM_USE_PTHREADS" = 1 ] || die "pthreads tier is not supported by this app"
            PTHREADS=ON ;;
        -P|--no-pthreads)
            [ "$SOUI_WASM_USE_PTHREADS" = 1 ] || die "pthreads tier is not supported by this app"
            PTHREADS=OFF ;;
        -*) die "unknown option: $1" ;;
        *)  POSITIONAL+=("$1") ;;
    esac
    shift
done
[ ${#POSITIONAL[@]} -le 1 ] || die "at most one build dir may be given"

if [ -n "$GDI_BACKEND" ]; then
    case "$GDI_BACKEND" in
        canvas2d|cairo) ;;
        *) die "--gdi only accepts canvas2d or cairo (got: $GDI_BACKEND)" ;;
    esac
fi
GDI_FOR_PATH="${GDI_BACKEND:-canvas2d}"
GDI_ARGS=()
if [ -n "$GDI_BACKEND" ]; then GDI_ARGS+=("-DSWINX_WASM_GDI_BACKEND=$GDI_BACKEND"); fi

if [ "$SOUI_WASM_USE_PTHREADS" = 1 ]; then
    case "$PTHREADS" in
        ON|on|1|TRUE)  PTHREADS=ON ;;
        OFF|off|0|FALSE) PTHREADS=OFF ;;
        *) echo "WARN: SOUI_WASM_PTHREADS='$PTHREADS', using ON" >&2; PTHREADS=ON ;;
    esac
fi
PT_SUFFIX=""
if [ "$SOUI_WASM_USE_PTHREADS" = 1 ] && [ "$PTHREADS" = ON ]; then PT_SUFFIX="_pt"; fi

# ---- tier: build type, default build dir, output pattern --------------------
if [ "$RELEASE" = 1 ]; then
    BUILD_TYPE="$SOUI_WASM_SHIP_TYPE"
    DEFAULT_BUILD="$SOUI_WASM_SHIP_BUILD_DIR"
    OUT_PATTERN="$SOUI_WASM_SHIP_OUT_PATTERN"
    BUILD_ENV="$SOUI_WASM_SHIP_BUILD_ENV"
else
    BUILD_TYPE="$SOUI_WASM_BUILD_TYPE"
    DEFAULT_BUILD="$SOUI_WASM_BUILD_DIR"
    OUT_PATTERN="$SOUI_WASM_OUT_PATTERN"
    BUILD_ENV="$SOUI_WASM_BUILD_ENV"
fi
if [ -n "$BUILD_ENV" ]; then
    eval "_env_default=\${${BUILD_ENV}:-}"
    if [ -n "$_env_default" ]; then DEFAULT_BUILD="$_env_default"; fi
fi
BUILD="${POSITIONAL[0]:-$DEFAULT_BUILD}"

OUT_DIR="${OUT_PATTERN//@SRC@/$SOUI_WASM_SRC}"
OUT_DIR="${OUT_DIR//@PT@/$PT_SUFFIX}"
OUT_DIR="${OUT_DIR//@GDI@/$GDI_FOR_PATH}"
if [ -n "$SOUI_WASM_OUT_VAR" ]; then
    eval "_out_env=\${${SOUI_WASM_OUT_VAR}:-}"
    if [ -n "$_out_env" ]; then OUT_DIR="$_out_env"; fi
fi

# ---- tool / tree checks -----------------------------------------------------
command -v em++ >/dev/null 2>&1 || die "em++ not found; source <emsdk>/emsdk_env.sh first"
command -v ninja >/dev/null 2>&1 || die "ninja not found; the wasm builds use -G Ninja"
[ -d "$SOUI_WASM_REPO/swinx/include" ] || \
    die "$SOUI_WASM_REPO/swinx/include not found (swinx submodule missing?)"

# ---- reconfigure args: generic env passthrough + app-specific config --------
RECONFIG=()
for _name in $SOUI_WASM_ENV_VARS; do
    eval "_value=\${${_name}:-}"
    if [ -n "$_value" ]; then RECONFIG+=("-D${_name}=${_value}"); fi
done
if [ -n "$SOUI_WASM_OUT_VAR" ]; then
    RECONFIG+=("-D${SOUI_WASM_OUT_VAR}=${OUT_DIR}")
fi
if [ "$SOUI_WASM_USE_PTHREADS" = 1 ]; then
    RECONFIG+=("-DSOUI_WASM_PTHREADS=$PTHREADS")
else
    # Apps without a pthread tier must still pin the cached value to OFF, or a
    # build tree configured for another app keeps -pthread in its compile flags.
    RECONFIG+=("-DSOUI_WASM_PTHREADS=OFF")
fi
if [ ${#GDI_ARGS[@]} -gt 0 ]; then RECONFIG+=("${GDI_ARGS[@]}"); fi
if [ ${#SOUI_WASM_CONFIG[@]} -gt 0 ]; then RECONFIG+=("${SOUI_WASM_CONFIG[@]}"); fi

# ---- a build dir that was moved still carries the old absolute paths --------
if [ -f "$BUILD/CMakeCache.txt" ]; then
    cache_dir="$(sed -n 's/^CMAKE_CACHEFILE_DIR:INTERNAL=//p' "$BUILD/CMakeCache.txt" | tr '\\' '/')"
    now_dir="$(cd "$BUILD" && pwd)"
    if command -v cygpath >/dev/null 2>&1; then
        now_dir="$(cygpath -m "$now_dir")"
    fi
    cache_dir="$(printf %s "$cache_dir" | tr 'A-Z' 'a-z')"
    now_dir="$(printf %s "$now_dir" | tr 'A-Z' 'a-z')"
    if [ -n "$cache_dir" ] && [ "$cache_dir" != "$now_dir" ]; then
        die "$BUILD/CMakeCache.txt was created for $cache_dir.
     The build dir was moved; delete it and configure again:
         rm -rf \"$BUILD\""
    fi
fi

# ---- configure (fresh) or reconfigure (existing cache) ---------------------
FRESH=0
if [ ! -f "$BUILD/CMakeCache.txt" ]; then
    echo "== configure: $BUILD ($BUILD_TYPE, Ninja)"
    command -v emcmake >/dev/null 2>&1 || die "emcmake not found; check the emscripten dir in PATH"
    emcmake cmake -S "$SOUI_WASM_SRC" -B "$BUILD" -G Ninja \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        ${RECONFIG[@]+"${RECONFIG[@]}"}
    FRESH=1
else
    _cached="$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=\(.*\)/\1/p' "$BUILD/CMakeCache.txt")"
    if [ "$RELEASE" = 1 ] && [ "$_cached" != "$BUILD_TYPE" ]; then
        die "$BUILD is ${_cached:-unset}, not $BUILD_TYPE; the ship tier needs its own build dir"
    fi
    if [ "$RELEASE" = 0 ] && [ "$_cached" != "$BUILD_TYPE" ]; then
        echo "note: $BUILD is ${_cached:-unset}, not $BUILD_TYPE; keeping the cached type"
    fi
    # The project marker only explains a reconfigure; the plain cmake call below
    # is the reconfigure probe itself (it fails loudly on a foreign cache).
    if ! grep -q "^CMAKE_PROJECT_NAME:STRING=$SOUI_WASM_PROJECT" "$BUILD/CMakeCache.txt"; then
        echo "== $BUILD has no $SOUI_WASM_PROJECT project marker"
    fi
    echo "== reconfigure: $BUILD"
    cmake -S "$SOUI_WASM_SRC" -B "$BUILD" ${RECONFIG[@]+"${RECONFIG[@]}"}
fi

# ---- build ------------------------------------------------------------------
JOBS=()
_jobs="${SOUI_WASM_JOBS:-$SOUI_WASM_JOBS_DEFAULT}"
if [ -n "$_jobs" ]; then JOBS+=("-j${_jobs}"); fi
echo "== build: cmake --build $BUILD --target $SOUI_WASM_TARGET ${JOBS[*]:-}"
cmake --build "$BUILD" --target "$SOUI_WASM_TARGET" ${JOBS[@]+"${JOBS[@]}"}

# ---- artifacts --------------------------------------------------------------
OUT="$OUT_DIR"
if [ -n "$SOUI_WASM_OUT_VAR" ]; then
    _cached_out="$(sed -n "s/^${SOUI_WASM_OUT_VAR}:[^=]*=//p" "$BUILD/CMakeCache.txt" | tail -n 1)"
    if [ -n "$_cached_out" ]; then OUT="$_cached_out"; fi
fi
echo
echo "artifacts ($OUT):"
ls -la "$OUT" 2>/dev/null || echo "(not found)"
echo
echo "open : http://localhost:8080/$SOUI_WASM_PAGE"
# Apps setting SOUI_WASM_NEEDS_HTTP_PROXY (fundholdings) need the same-origin
# /__soui_http endpoint even for pump builds; SOUI_WASM_NEEDS_SERVER does the
# same for apps that always need a local WebSocket proxy (cnchess).
# SOUI_WASM_SERVER / _ARGS let an app point the hint at its own server entry and
# flags; defaults are the shared tool with no extra flags. This shared script
# knows no app-specific option names.
SOUI_WASM_SERVER="${SOUI_WASM_SERVER:-tools/wasm/serve-isolated.py}"
SOUI_WASM_SERVER_ARGS="${SOUI_WASM_SERVER_ARGS:-}"
if { [ "$SOUI_WASM_USE_PTHREADS" = 1 ] && [ "$PTHREADS" = ON ]; } || [ "${SOUI_WASM_NEEDS_HTTP_PROXY:-}" = 1 ] || [ "${SOUI_WASM_NEEDS_SERVER:-}" = 1 ]; then
    echo "serve: python3 \"$SOUI_WASM_REPO/$SOUI_WASM_SERVER\" \"$OUT\" --port 8080$SOUI_WASM_SERVER_ARGS"
else
    echo "serve: python3 -m http.server 8080 -d \"$OUT\""
fi
if [ "$FRESH" = 1 ]; then
    echo "(first configure done; re-run this script any time to rebuild)"
fi
