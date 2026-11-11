@echo off
rem =============================================================================
rem build-cnchess.bat -- Build the cnchess (Chinese chess) wasm client.
rem
rem This file only declares the app metadata and the extra cmake configure
rem arguments. Argument parsing, tool checks, first configure / re-configure,
rem build and artifact report all live in __cmake\wasm-build-common.bat, shared
rem by every wasm app. Compile and link options are in wasm\CMakeLists.txt.
rem
rem Usage: build-cnchess.bat [build_dir] [release|-r|--release|/release]
rem        -p|--pthreads / -P|--no-pthreads select the TaskLoop backend.
rem        --gdi=canvas2d|cairo / --cairo / --canvas2d select the GDI backend.
rem
rem Build dir: positional arg > %SOUI_WASM_BUILD% > <this dir>\build.
rem   release uses %SOUI_CNCHESS_SHIP_BUILD% > <this dir>\build-release.
rem   Each app owns its own build tree; never share one CMakeCache.
rem   A build dir whose CMAKE_BUILD_TYPE is not Release is rejected for the
rem   ship tier (changing the type means a full rebuild).
rem
rem TaskLoop backend: worker (-pthread) by default; artifacts only start a
rem worker when the page is crossOriginIsolated, so serve them with
rem tools\wasm\serve-isolated.py. Worker artifacts go to out_pt /
rem release_pt, pump artifacts to out / release.
rem
rem Tunables (CMake cache variables, same-named environment variables):
rem   SOUI_CNCHESS_STACK       -sSTACK_SIZE, default 32MB
rem   SOUI_WASM_CNCHESS_OUT    output dir, default as described above
rem   SOUI_WASM_PTHREADS       ON (default) | OFF
rem   SOUI_WASM_JOBS           parallelism, default 2
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files in the
rem active OEM codepage; UTF-8 Chinese comments split into phantom commands.
rem Chinese documentation lives in build-cnchess.sh and readme.md.
rem =============================================================================
setlocal EnableDelayedExpansion
for %%I in ("%~dp0.") do set "SOUI_WASM_SRC=%%~fI"
for %%I in ("%~dp0..\..\..\..") do set "SOUI_WASM_REPO=%%~fI"
set "SOUI_WASM_PROJECT=cnchess-wasm"
set "SOUI_WASM_TARGET=cnchess-wasm"
set "SOUI_WASM_PAGE=cnchess-wasm.html"
set "SOUI_WASM_BUILD_TYPE=RelWithDebInfo"
set "SOUI_WASM_SHIP_TYPE=Release"
set "SOUI_WASM_BUILD_ENV=SOUI_WASM_BUILD"
set "SOUI_WASM_SHIP_BUILD_ENV=SOUI_CNCHESS_SHIP_BUILD"
set "SOUI_WASM_BUILD_DIR=!SOUI_WASM_SRC!\build"
set "SOUI_WASM_SHIP_BUILD_DIR=!SOUI_WASM_SRC!\build-release"
set "SOUI_WASM_OUT_VAR=SOUI_WASM_CNCHESS_OUT"
set "SOUI_WASM_OUT_PATTERN=@SRC@\out@PT@"
set "SOUI_WASM_SHIP_OUT_PATTERN=@SRC@\release@PT@"
set "SOUI_WASM_ENV_VARS=SOUI_CNCHESS_STACK"
set "SOUI_WASM_CONFIG=-DSOUI_BUILD_WS=ON -DSOUI_BUILD_RES_ZIP=ON"
set "SOUI_WASM_JOBS_DEFAULT=2"
set "SOUI_WASM_USE_PTHREADS=0"
set "SOUI_WASM_NEEDS_SERVER=1"
set "SOUI_WASM_SERVER_ARGS= --ws-proxy 127.0.0.1:3311"
set "SOUI_WASM_USE_GDI=1"
call "!SOUI_WASM_REPO!\__cmake\wasm-build-common.bat" %*
set "_RC=!ERRORLEVEL!"
endlocal & exit /b %_RC%
