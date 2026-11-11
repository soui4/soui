#!/usr/bin/env bash
# =============================================================================
# build-fun_test.sh -- Unix wrapper for the wasm GDI unit-test leg.
#
# This file only declares app metadata and the extra cmake configure arguments.
# Argument parsing, tool checks, first configure / re-configure, build and the
# artifact report all live in __cmake/wasm-build-common.sh, shared by every wasm
# app. Compile and link options are in demos/fun_test/wasm/CMakeLists.txt.
#
# Usage (--help prints this header):
#   ./build-fun_test.sh [build_dir]                    Debug -> ../out.canvas2d
#   ./build-fun_test.sh --gdi=cairo [build_dir]        -> ../out.cairo
#   ./build-fun_test.sh --release [build_dir]          Release tier
#   --cairo / --canvas2d are shortcuts for --gdi=<value>.
#
# The artifact dir is owned by the app CMakeLists (../out.<backend>), because
# run-node.js and browser-runner.html resolve ../out.<backend>/fun_test-wasm.js
# relative to wasm/. There is therefore no SOUI_WASM_OUT_VAR: this wrapper does
# not push a -D output variable, and the backend switch is passed to cmake as
# -DSWINX_WASM_GDI_BACKEND=<value>.
#
# Build dir: positional arg > $SOUI_WASM_FUN_TEST_BUILD > <this dir>/build;
#   --release uses $SOUI_WASM_FUN_TEST_SHIP_BUILD > <this dir>/build-release.
#   Each app owns its own build tree; never share one CMakeCache.
#
# Tunables (CMake cache variables, same-named environment variables):
#   SOUI_WASM_FUN_TEST_STACK       -sSTACK_SIZE, default 32MB
#   SOUI_WASM_FUN_TEST_ASSERTIONS  -sASSERTIONS=2, default ON
#   SOUI_WASM_JOBS                 parallelism, default 2
#
# Windows host: build-fun_test.bat in this directory.
# =============================================================================
SOUI_WASM_WRAPPER="${BASH_SOURCE[0]}"
SOUI_WASM_SRC="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOUI_WASM_REPO="$(cd "$SOUI_WASM_SRC/../../.." && pwd)"
SOUI_WASM_PROJECT="fun_test-wasm"
SOUI_WASM_TARGET="fun_test"
SOUI_WASM_PAGE="fun_test-wasm.html"
SOUI_WASM_BUILD_TYPE="Debug"
SOUI_WASM_SHIP_TYPE="Release"
SOUI_WASM_BUILD_ENV="SOUI_WASM_FUN_TEST_BUILD"
SOUI_WASM_SHIP_BUILD_ENV="SOUI_WASM_FUN_TEST_SHIP_BUILD"
SOUI_WASM_BUILD_DIR="$SOUI_WASM_SRC/build"
SOUI_WASM_SHIP_BUILD_DIR="$SOUI_WASM_SRC/build-release"
SOUI_WASM_OUT_VAR=""
SOUI_WASM_OUT_PATTERN="@SRC@/../out.@GDI@"
SOUI_WASM_SHIP_OUT_PATTERN="@SRC@/../out.@GDI@"
SOUI_WASM_ENV_VARS="SOUI_WASM_FUN_TEST_STACK SOUI_WASM_FUN_TEST_ASSERTIONS"
SOUI_WASM_CONFIG=()
SOUI_WASM_JOBS_DEFAULT="2"
SOUI_WASM_USE_PTHREADS=0
SOUI_WASM_USE_GDI=1

# shellcheck source=../../../__cmake/wasm-build-common.sh
source "$SOUI_WASM_REPO/__cmake/wasm-build-common.sh" "$@"
