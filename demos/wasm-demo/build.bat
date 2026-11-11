@echo off
rem =============================================================================
rem build.bat -- Build the SOUI-on-WebAssembly demo (Windows host wrapper).
rem
rem This file only declares the app metadata and the extra cmake configure
rem arguments. Argument parsing, tool checks, first configure / re-configure,
rem build and artifact report all live in __cmake\wasm-build-common.bat, shared
rem by every wasm app. Compile and link options are in CMakeLists.txt.
rem
rem Usage: build.bat [build_dir] [-r|--release] [--gdi=canvas2d|cairo]
rem        --cairo / --canvas2d are shortcuts; --gdi <value> also works.
rem
rem Build dir: positional arg > %SOUI_WASM_BUILD% > <this dir>\build.
rem   -r / --release uses %SOUI_WASM_SHIP_BUILD% > <this dir>\build-release,
rem   artifacts go to <this dir>\release instead of <this dir>\out.
rem
rem Tunables (CMake cache variables, same-named environment variables):
rem   SOUI_DEMO_FONT             font file (overrides auto-detection)
rem   SOUI_DEMO_STACK            -sSTACK_SIZE, default 32MB
rem   SOUI_WASM_DEMO_OUT         output dir, default <this dir>\out
rem   SOUI_BUILD_SCRIPT_LUA      lua-54 + ScriptModule-LUA + uires/lua preload
rem   SOUI_BUILD_WASM_SCINTILLA  Scintilla + the <scintilla> control
rem   SWINX_WASM_GDI_BACKEND     canvas2d (default) | cairo
rem   SOUI_WASM_JOBS             parallelism, default 8
rem
rem WINDOWS QUOTING: %~dp0 ends with a backslash. Never pass it through to
rem emcmake directly -- the MSVCRT argv parser reads \" as a literal quote and
rem swallows the rest of the command line. %%~fI below normalizes both paths.
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files in the
rem active OEM codepage; UTF-8 Chinese comments split into phantom commands.
rem Chinese documentation lives in build-soui.sh and readme.md.
rem =============================================================================
setlocal EnableDelayedExpansion
chcp 65001
for %%I in ("%~dp0.") do set "SOUI_WASM_SRC=%%~fI"
for %%I in ("%~dp0..\..") do set "SOUI_WASM_REPO=%%~fI"
set "SOUI_WASM_PROJECT=wasm-demo"
set "SOUI_WASM_TARGET=soui-wasm-demo"
set "SOUI_WASM_PAGE=soui-wasm-demo.html"
set "SOUI_WASM_BUILD_TYPE=Release"
set "SOUI_WASM_SHIP_TYPE=Release"
set "SOUI_WASM_BUILD_ENV=SOUI_WASM_BUILD"
set "SOUI_WASM_SHIP_BUILD_ENV=SOUI_WASM_SHIP_BUILD"
set "SOUI_WASM_BUILD_DIR=!SOUI_WASM_SRC!\build"
set "SOUI_WASM_SHIP_BUILD_DIR=!SOUI_WASM_SRC!\build-release"
set "SOUI_WASM_OUT_VAR=SOUI_WASM_DEMO_OUT"
set "SOUI_WASM_OUT_PATTERN=@SRC@\out"
set "SOUI_WASM_SHIP_OUT_PATTERN=@SRC@\release"
set "SOUI_WASM_ENV_VARS=SOUI_DEMO_FONT SOUI_DEMO_STACK SOUI_BUILD_SCRIPT_LUA SOUI_BUILD_WASM_SCINTILLA"
set "SOUI_WASM_CONFIG=-DSOUI_BUILD_WS=OFF -DSOUI_BUILD_RES_ZIP=OFF"
set "SOUI_WASM_JOBS_DEFAULT=8"
set "SOUI_WASM_USE_PTHREADS=0"
set "SOUI_WASM_USE_GDI=1"
call "!SOUI_WASM_REPO!\__cmake\wasm-build-common.bat" %*
set "_RC=!ERRORLEVEL!"
endlocal & exit /b %_RC%
