@echo off
rem =============================================================================
rem build-fun_test.bat -- Windows wrapper for the wasm GDI unit-test leg.
rem
rem This file only declares the app metadata and the extra cmake configure
rem arguments. Argument parsing, tool checks, first configure / re-configure,
rem build and artifact report all live in __cmake\wasm-build-common.bat, shared
rem by every wasm app. Compile and link options are in CMakeLists.txt.
rem
rem Usage: build-fun_test.bat [build_dir] [-r|--release]
rem        --gdi=canvas2d|cairo / --cairo / --canvas2d select the GDI backend.
rem
rem Build dir: positional arg > %SOUI_WASM_FUN_TEST_BUILD% > <this dir>\build.
rem   release uses %SOUI_WASM_FUN_TEST_SHIP_BUILD% > <this dir>\build-release.
rem   Each app owns its own build tree; never share one CMakeCache.
rem
rem The artifact dir is owned by the app CMakeLists (..\out.<backend>), because
rem run-node.js and browser-runner.html resolve ..\out.<backend>\fun_test-wasm.js
rem relative to wasm\. There is therefore no SOUI_WASM_OUT_VAR: this wrapper
rem does not push a -D output variable, and the backend switch is passed to
rem cmake as -DSWINX_WASM_GDI_BACKEND=<value>.
rem
rem Tunables (CMake cache variables, same-named environment variables):
rem   SOUI_WASM_FUN_TEST_STACK       -sSTACK_SIZE, default 32MB
rem   SOUI_WASM_FUN_TEST_ASSERTIONS  -sASSERTIONS=2, default ON
rem   SOUI_WASM_JOBS                 parallelism, default 2
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files in the
rem active OEM codepage; UTF-8 Chinese comments split into phantom commands.
rem See build-fun_test.sh and demos/fun_test/wasm/run-node.js for the test flow.
rem =============================================================================
setlocal EnableDelayedExpansion
for %%I in ("%~dp0.") do set "SOUI_WASM_SRC=%%~fI"
for %%I in ("%~dp0..\..\..") do set "SOUI_WASM_REPO=%%~fI"
set "SOUI_WASM_PROJECT=fun_test-wasm"
set "SOUI_WASM_TARGET=fun_test"
set "SOUI_WASM_PAGE=fun_test-wasm.html"
set "SOUI_WASM_BUILD_TYPE=Debug"
set "SOUI_WASM_SHIP_TYPE=Release"
set "SOUI_WASM_BUILD_ENV=SOUI_WASM_FUN_TEST_BUILD"
set "SOUI_WASM_SHIP_BUILD_ENV=SOUI_WASM_FUN_TEST_SHIP_BUILD"
set "SOUI_WASM_BUILD_DIR=!SOUI_WASM_SRC!\build"
set "SOUI_WASM_SHIP_BUILD_DIR=!SOUI_WASM_SRC!\build-release"
set "SOUI_WASM_OUT_VAR="
set "SOUI_WASM_OUT_PATTERN=@SRC@\..\out.@GDI@"
set "SOUI_WASM_SHIP_OUT_PATTERN=@SRC@\..\out.@GDI@"
set "SOUI_WASM_ENV_VARS=SOUI_WASM_FUN_TEST_STACK SOUI_WASM_FUN_TEST_ASSERTIONS"
set "SOUI_WASM_CONFIG="
set "SOUI_WASM_JOBS_DEFAULT=2"
set "SOUI_WASM_USE_PTHREADS=0"
set "SOUI_WASM_USE_GDI=1"
call "!SOUI_WASM_REPO!\__cmake\wasm-build-common.bat" %*
set "_RC=!ERRORLEVEL!"
endlocal & exit /b %_RC%
