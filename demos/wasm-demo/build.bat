@echo off
rem =============================================================================
rem build.bat -- Build the SOUI-on-WebAssembly demo (Windows host wrapper).
rem
rem Windows counterpart of build-soui.sh (same env vars, same flow): compile and
rem link commands live in demos/wasm-demo/CMakeLists.txt (target soui-wasm-demo), this
rem script only locates em++, reconfigures when needed, and builds the target.
rem
rem Usage (build dir = 1st arg, or %SOUI_WASM_BUILD%, default %TEMP%\build-soui-wasm):
rem
rem   call <emsdk>\emsdk_env.bat              (or put emsdk on PATH)
rem   demos\wasm-demo\build.bat D:\build-soui-wasm
rem
rem Tunables are CMake cache variables, passed through from same-named env vars:
rem   SOUI_DEMO_FONT             font file (overrides auto-detection)
rem   SOUI_DEMO_STACK            -sSTACK_SIZE, default 32MB
rem   SOUI_WASM_DEMO_OUT         output dir, default %HERE%/out
rem   SOUI_BUILD_WASM_LUA        0/1, lua-54 + ScriptModule-LUA + uires/lua preload
rem   SOUI_BUILD_WASM_SCINTILLA  0/1, Scintilla + the <scintilla> control
rem   SOUI_WASM_JOBS             parallelism (default 2: emcc hangs at high -j on
rem                              Windows, seen 2026-10-28; set e.g. 8 to override)
rem
rem NOTE: keep this file ASCII-only. cmd.exe decodes batch files line by line in
rem the active OEM codepage; UTF-8 Chinese comments get split into phantom
rem commands (chcp 65001 does not fully fix it, chunked reads still corrupt).
rem Chinese documentation lives in build-soui.sh and readme.md.
rem =============================================================================
setlocal EnableDelayedExpansion

set "HERE=%~dp0"
rem this file lives in demos\wasm-demo, so the repo root is two levels up
for %%I in ("%HERE%..\..") do set "ROOT=%%~fI"
set "TARGET=soui-wasm-demo"

set "BUILD=%~1"
if "%BUILD%"=="" set "BUILD=%SOUI_WASM_BUILD%"
if "%BUILD%"=="" set "BUILD=%HERE%/build"

if not exist "%ROOT%\swinx\include" (
    echo ERROR: %ROOT%\swinx\include not found - is the swinx submodule checked out? 1>&2
    exit /b 1
)

rem ---- locate em++ / emcmake (run emsdk_env.bat when EMSDK is set) -----------
where em++ >nul 2>nul
if errorlevel 1 if defined EMSDK call "%EMSDK%\emsdk_env.bat" >nul 2>nul
where em++ >nul 2>nul
if errorlevel 1 (
    echo ERROR: em++ not found. Run: call ^<emsdk^>\emsdk_env.bat  or add emsdk to PATH 1>&2
    exit /b 1
)
where emcmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: em++ found but emcmake not found - check the emscripten dir in PATH 1>&2
    exit /b 1
)
where ninja >nul 2>nul
if errorlevel 1 (
    echo ERROR: ninja not found - the build uses -G Ninja, same as the Linux flow 1>&2
    exit /b 1
)

rem ---- pass cache variables through (same names as build-soui.sh) ------------
set "RECONFIG="
if defined SOUI_DEMO_FONT          set "RECONFIG=!RECONFIG! -DSOUI_DEMO_FONT=%SOUI_DEMO_FONT%"
if defined SOUI_DEMO_STACK         set "RECONFIG=!RECONFIG! -DSOUI_DEMO_STACK=%SOUI_DEMO_STACK%"
if defined SOUI_WASM_DEMO_OUT      set "RECONFIG=!RECONFIG! -DSOUI_WASM_DEMO_OUT=%SOUI_WASM_DEMO_OUT%"
if defined SOUI_BUILD_WASM_LUA     set "RECONFIG=!RECONFIG! -DSOUI_BUILD_WASM_LUA=%SOUI_BUILD_WASM_LUA%"
if defined SOUI_BUILD_WASM_SCINTILLA set "RECONFIG=!RECONFIG! -DSOUI_BUILD_WASM_SCINTILLA=%SOUI_BUILD_WASM_SCINTILLA%"

rem ---- configure when missing, or when the old cache lacks the demo flag -----
echo == configure: %BUILD%
call emcmake cmake -S "%ROOT%" -B "%BUILD%" -G Ninja ^
        -DCMAKE_BUILD_TYPE=Release -DSOUI_BUILD_WASM_DEMO=ON !RECONFIG!
if errorlevel 1 (
        echo ERROR: emcmake configure failed 1>&2
        if exist "%BUILD%\CMakeCache.txt" (
                echo HINT: a cache created before the tree move still points at the old paths.
                echo       delete the build dir and configure again: rmdir /s /q "%BUILD%"
        )
        exit /b 1
)

rem ---- build ------------------------------------------------------------------
if not defined SOUI_WASM_JOBS set "SOUI_WASM_JOBS=8"
echo == build: cmake --build %BUILD% --target %TARGET% -j%SOUI_WASM_JOBS%
call cmake --build "%BUILD%" --target %TARGET% -j%SOUI_WASM_JOBS%
if errorlevel 1 (
    echo ERROR: build failed. If a TU hung, kill the stuck clang/emcc and rerun to resume. 1>&2
    exit /b 1
)

rem ---- read the output dir back from CMakeCache -------------------------------
set "OUT="
for /f "tokens=1,* delims==" %%A in ('findstr /b /c:"SOUI_WASM_DEMO_OUT:" "%BUILD%\CMakeCache.txt" 2^>nul') do set "OUT=%%B"
if "%OUT%"=="" set "OUT=%HERE%out"

echo.
echo artifacts ^(%OUT%^):
dir /b "%OUT%"
echo.
echo run : python -m http.server 8080 -d "%OUT%"   then open http://localhost:8080/%TARGET%.html
endlocal
