@echo off
rem =============================================================================
rem build-fundholdings.bat -- Build the fund-holdings demo for wasm.
rem
rem This file only declares the app metadata and the extra cmake configure
rem arguments. Argument parsing, tool checks, first configure / re-configure,
rem build and artifact report all live in __cmake\wasm-build-common.bat, shared
rem by every wasm app. Compile and link options are in wasm\CMakeLists.txt.
rem
rem Usage: build-fundholdings.bat [build_dir] [release|-r|--release|/release]
rem        -p|--pthreads / -P|--no-pthreads select the TaskLoop backend.
rem        --gdi=canvas2d|cairo / --cairo / --canvas2d select the GDI backend.
rem
rem Build dir: positional arg > %SOUI_WASM_BUILD% > <this dir>\build.
rem   release uses %SOUI_FUNDHOLDINGS_SHIP_BUILD% > <this dir>\build-release.
rem   Each app owns its own build tree; never share one CMakeCache.
rem
rem TaskLoop backend: worker (-pthread) by default. The HTTP client uses a
rem synchronous XHR inside that worker, so the page must be crossOriginIsolated
rem and served by this dir's serve-fundholdings.py --http-proxy (the app-specific
rem entry that adds the /__soui_http endpoint). Worker artifacts go to
rem out_pt / release_pt, pump artifacts to out / release.
rem
rem The wasm layout is the desktop one (1440x860), so the config always pins
rem SWINX_FB_WIDTH=1440 / SWINX_FB_HEIGHT=860; a cache with other dimensions
rem would crop the desktop layout.
rem
rem Tunables (CMake cache variables, same-named environment variables):
rem   SOUI_FUNDHOLDINGS_STACK     -sSTACK_SIZE, default 32MB
rem   SOUI_WASM_FUNDHOLDINGS_OUT  output dir, default as described above
rem   SOUI_WASM_PTHREADS          ON (default) | OFF
rem   SOUI_WASM_JOBS              parallelism, default 2
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files in the
rem active OEM codepage; UTF-8 Chinese comments split into phantom commands.
rem Chinese documentation lives in build-fundholdings.sh and readme.md.
rem =============================================================================
setlocal EnableDelayedExpansion
for %%I in ("%~dp0.") do set "SOUI_WASM_SRC=%%~fI"
for %%I in ("%~dp0..\..\..") do set "SOUI_WASM_REPO=%%~fI"
set "SOUI_WASM_PROJECT=fundholdings-wasm"
set "SOUI_WASM_TARGET=fundholdings-wasm"
set "SOUI_WASM_PAGE=fundholdings.html"
set "SOUI_WASM_BUILD_TYPE=Release"
set "SOUI_WASM_SHIP_TYPE=Release"
set "SOUI_WASM_BUILD_ENV=SOUI_WASM_BUILD"
set "SOUI_WASM_SHIP_BUILD_ENV=SOUI_FUNDHOLDINGS_SHIP_BUILD"
set "SOUI_WASM_BUILD_DIR=!SOUI_WASM_SRC!\build"
set "SOUI_WASM_SHIP_BUILD_DIR=!SOUI_WASM_SRC!\build-release"
set "SOUI_WASM_OUT_VAR=SOUI_WASM_FUNDHOLDINGS_OUT"
set "SOUI_WASM_OUT_PATTERN=@SRC@\out@PT@"
set "SOUI_WASM_SHIP_OUT_PATTERN=@SRC@\release@PT@"
set "SOUI_WASM_ENV_VARS=SOUI_FUNDHOLDINGS_STACK"
set "SOUI_WASM_CONFIG=-DSOUI_BUILD_HTTPCLIENT=ON -DSWINX_FB_WIDTH=1440 -DSWINX_FB_HEIGHT=860"
set "SOUI_WASM_JOBS_DEFAULT=2"
set "SOUI_WASM_USE_PTHREADS=1"
set "SOUI_WASM_USE_GDI=1"
rem fundholdings fetches data through the same-origin /__soui_http endpoint,
rem which lives in its own server entry, not in the shared tools/wasm server.
set "SOUI_WASM_NEEDS_HTTP_PROXY=1"
set "SOUI_WASM_SERVER=demos\fundholdings\wasm\serve-fundholdings.py"
set "SOUI_WASM_SERVER_ARGS= --http-proxy"
call "!SOUI_WASM_REPO!\__cmake\wasm-build-common.bat" %*
set "_RC=!ERRORLEVEL!"
endlocal & exit /b %_RC%
