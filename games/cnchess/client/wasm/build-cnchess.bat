@echo off
rem =============================================================================
rem build-cnchess.bat -- Build the cnchess (Chinese chess) wasm client
rem                      (Windows host wrapper).
rem
rem Windows counterpart of build-cnchess.sh: same tiers, same env vars, same
rem flow. Compile and link flags live in games/cnchess/client/wasm/CMakeLists.txt
rem (target cnchess-wasm); this script only locates em++, configures the build
rem dir when needed, and builds the target.
rem
rem Usage (from the soui4 repo root or anywhere else):
rem
rem   call <emsdk>\emsdk_env.bat            (or put emsdk on PATH)
rem   games\cnchess\client\wasm\build-cnchess.bat                    rem debug tier
rem   games\cnchess\client\wasm\build-cnchess.bat release            rem ship tier
rem   games\cnchess\client\wasm\build-cnchess.bat D:\build-soui-wasm rem debug, given dir
rem   games\cnchess\client\wasm\build-cnchess.bat D:\ship release    rem ship, given dir
rem   games\cnchess\client\wasm\build-cnchess.bat D:\build --gdi=cairo  rem cairo backend
rem   games\cnchess\client\wasm\build-cnchess.bat --cairo release       rem cairo, ship
rem   -r, --release and /release work too; --gdi=canvas2d|cairo (or the --cairo /
rem   --canvas2d shortcuts) selects the GDI backend. The order of the args does not
rem   matter, but at most one build dir and one release flag are accepted. The
rem   ship flag is spelled "--release" in build-cnchess.sh and "release" here,
rem   because cmd.exe would hand a leading "-" to cmake as an option.
rem
rem Build dir = 1st arg, else %SOUI_WASM_BUILD% (debug) / %SOUI_CNCHESS_SHIP_BUILD%
rem (ship), else %TEMP%\build-soui-wasm / %TEMP%\build-cnchess-ship. A missing
rem dir is configured on the fly with -G Ninja; an existing one keeps its cached
rem options (including its CMAKE_BUILD_TYPE) and only gets the env vars below
rem pushed through, so incremental builds survive the script.
rem
rem Tunables are CMake cache variables, passed through from same-named env vars:
rem   SOUI_CNCHESS_STACK       -sSTACK_SIZE, default 32MB (5MB overruns
rem                            SHostWnd::Create)
rem   SOUI_WASM_CNCHESS_OUT    output dir, default wasm\out; the ship tier always
rem                            forces it to wasm\release
rem   SOUI_WASM_JOBS           parallelism, default 2 - emcc wedges at high -j on
rem                            Windows (seen 2026-10-28); raise it on a host that
rem                            behaves
rem
rem GDI backend (also a script arg --gdi=canvas2d|cairo, or --cairo / --canvas2d):
rem   SWINX_WASM_GDI_BACKEND   canvas2d (default) | cairo. canvas2d uses the
rem                            browser Canvas 2D + system fonts; cairo uses a
rem                            software rasterizer + freetype/fontconfig and needs
rem                            wasm\fonts\ populated so the font is packed.
rem
rem NOTE: keep this file ASCII-only. cmd.exe decodes batch files line by line in
rem the active OEM codepage; UTF-8 Chinese comments get split into phantom
rem commands (chcp 65001 does not fully fix it, chunked reads still corrupt).
rem Chinese documentation lives in build-cnchess.sh and readme.md.
rem
rem NOTE: this file keeps CRLF line endings - it jumps with goto, and cmd.exe
rem scans batch files for labels in chunks, which can miss a label that straddles
rem a chunk boundary in an LF-only file. soui-wasm\build.bat is linear, so it
rem does not have that exposure.
rem =============================================================================
setlocal EnableDelayedExpansion

set "HERE=%~dp0"
for %%I in ("%HERE%..\..\..\..") do set "ROOT=%%~fI"
set "TARGET=cnchess-wasm"
set "BUILD="
set "RELEASE=0"

rem ---- args: [build_dir] [release] [--gdi <v>], any order ---------------------
rem cmd.exe splits batch arguments on '=', so "--gdi=cairo" arrives as two tokens
rem ("--gdi" "cairo"); the --gdi <value> form below accepts both the space and
rem the '=' spelling. A shift-based loop consumes the value token; leftover args
rem are classified as release flags or the (single) build dir.
set "GDI_BACKEND=%SWINX_WASM_GDI_BACKEND%"
set "BUILD="
set "NEXT_GDI=0"
:parse_args
if "%~1"=="" goto :args_done
if "%NEXT_GDI%"=="1" ( set "GDI_BACKEND=%~1" & set "NEXT_GDI=0" & shift & goto :parse_args )
if /i "%~1"=="--gdi"      ( set "NEXT_GDI=1" & shift & goto :parse_args )
if /i "%~1"=="--cairo"    ( set "GDI_BACKEND=cairo"    & shift & goto :parse_args )
if /i "%~1"=="--canvas2d" ( set "GDI_BACKEND=canvas2d" & shift & goto :parse_args )
if /i "%~1"=="release"    ( set "RELEASE=1" & shift & goto :parse_args )
if /i "%~1"=="-r"         ( set "RELEASE=1" & shift & goto :parse_args )
if /i "%~1"=="--release"  ( set "RELEASE=1" & shift & goto :parse_args )
if /i "%~1"=="/release"   ( set "RELEASE=1" & shift & goto :parse_args )
if not defined BUILD goto :first_dir
set "SECOND_DIR=%~1"
goto :two_dirs
:first_dir
set "BUILD=%~1"
shift
goto :parse_args
:args_done
if not "%GDI_BACKEND%"=="" if /i not "%GDI_BACKEND%"=="canvas2d" if /i not "%GDI_BACKEND%"=="cairo" (
    echo ERROR: GDI backend only accepts canvas2d or cairo, got "%GDI_BACKEND%" 1>&2
    exit /b 2
)

rem ---- tier selects the build type, the default dir and the output dir -------
rem Flat on purpose: nested ( ... ) blocks are parsed before they run, so a value
rem with a stray bracket breaks them - one condition per line. Each tunable is
rem built as its own -D argument with unquoted "set VAR=value", which keeps the
rem inner quotes literal; set "VAR=..." would strip the outer pair and leave a
rem dangling quote behind. The path values are quoted because cmake reads them
rem raw, and %HERE% / %TEMP% / a user font path can all contain spaces.
set "BUILD_TYPE=RelWithDebInfo"
if "%RELEASE%"=="1" set "BUILD_TYPE=Release"

set "D_OUT="
set "D_STACK="
set "D_GDI="
if "%RELEASE%"=="1" set D_OUT=-DSOUI_WASM_CNCHESS_OUT="!HERE!release"
if "%RELEASE%"=="0" if defined SOUI_WASM_CNCHESS_OUT set D_OUT=-DSOUI_WASM_CNCHESS_OUT="%SOUI_WASM_CNCHESS_OUT%"
if defined SOUI_CNCHESS_STACK set D_STACK=-DSOUI_CNCHESS_STACK="%SOUI_CNCHESS_STACK%"
if defined GDI_BACKEND set D_GDI=-DSWINX_WASM_GDI_BACKEND="!GDI_BACKEND!"

set "RECONFIG="
if defined D_OUT   set RECONFIG=!D_OUT!
if defined D_STACK set RECONFIG=!RECONFIG! !D_STACK!
if defined D_GDI   set RECONFIG=!RECONFIG! !D_GDI!

if "%BUILD%"=="" if "%RELEASE%"=="0" set "BUILD=%SOUI_WASM_BUILD%"
if "%BUILD%"=="" if "%RELEASE%"=="1" set "BUILD=%SOUI_CNCHESS_SHIP_BUILD%"
if "%BUILD%"=="" if "%RELEASE%"=="0" set "BUILD=%TEMP%\build-soui-wasm"
if "%BUILD%"=="" if "%RELEASE%"=="1" set "BUILD=%TEMP%\build-cnchess-ship"

if not exist "%ROOT%\swinx\include" (
    echo ERROR: %ROOT%\swinx\include not found - is the swinx submodule checked out? 1>&2
    exit /b 1
)

rem ---- locate em++ / emcmake / ninja (run emsdk_env.bat when EMSDK is set) ---
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

rem ---- configure only when the dir has no cache; otherwise keep its options --
if exist "%BUILD%\CMakeCache.txt" goto :have_cache
echo == configure: %BUILD%  ^(!BUILD_TYPE!, Ninja^)
call emcmake cmake -S "%ROOT%" -B "%BUILD%" -G Ninja ^
        -DCMAKE_BUILD_TYPE=!BUILD_TYPE! -DSOUI_BUILD_WASM_CNCHESS=ON !RECONFIG!
if errorlevel 1 (
    echo ERROR: emcmake configure failed 1>&2
    exit /b 1
)
goto :build

:have_cache
rem Tier vs. cached CMAKE_BUILD_TYPE. Changing the type means a full rebuild, so
rem the script never does it: the ship tier stops (a RelWithDebInfo artifact
rem landing in release/ would look like the ship build but carry DWARF), the debug
rem tier only warns that the artifact follows the cache.
if "%RELEASE%"=="1" goto :ship_needs_release
findstr /b /c:"CMAKE_BUILD_TYPE:STRING=RelWithDebInfo" "%BUILD%\CMakeCache.txt" >nul 2>nul
if not errorlevel 1 goto :ensure_flag
echo note: CMAKE_BUILD_TYPE in %BUILD% is not RelWithDebInfo - kept as is,
echo       the artifact in out/ follows the cache, use "release" for -O2 -DNDEBUG.
goto :ensure_flag

:ship_needs_release
findstr /b /c:"CMAKE_BUILD_TYPE:STRING=Release" "%BUILD%\CMakeCache.txt" >nul 2>nul
if not errorlevel 1 goto :ensure_flag
echo ERROR: %BUILD% is not a Release build dir - the ship tier needs its own 1>&2
echo        default %TEMP%\build-cnchess-ship, override with SOUI_CNCHESS_SHIP_BUILD 1>&2
exit /b 1

:ensure_flag
rem Old build dirs never had this switch, so the cnchess target is not in them.
rem Plain cmake (not emcmake) is enough to re-configure a cache that already
rem carries the emscripten toolchain; CMAKE_BUILD_TYPE is deliberately absent.
findstr /b /c:"SOUI_BUILD_WASM_CNCHESS:BOOL=ON" "%BUILD%\CMakeCache.txt" >nul 2>nul
if not errorlevel 1 goto :push_env
echo == %BUILD% has no SOUI_BUILD_WASM_CNCHESS, re-configuring with it ON
call cmake -S "%ROOT%" -B "%BUILD%" -DSOUI_BUILD_WASM_CNCHESS=ON !RECONFIG!
if errorlevel 1 (
    echo ERROR: re-configure failed 1>&2
    exit /b 1
)
goto :build

:push_env
if not defined RECONFIG goto :build
echo == push env tunables into %BUILD%
call cmake -S "%ROOT%" -B "%BUILD%" !RECONFIG!
if errorlevel 1 (
    echo ERROR: re-configure failed 1>&2
    exit /b 1
)

:build
if not defined SOUI_WASM_JOBS set "SOUI_WASM_JOBS=2"
echo == build: cmake --build %BUILD% --target %TARGET% -j%SOUI_WASM_JOBS%
call cmake --build "%BUILD%" --target %TARGET% -j%SOUI_WASM_JOBS%
if errorlevel 1 (
    echo ERROR: build failed. If a TU hung, kill the stuck clang/emcc and rerun to resume. 1>&2
    exit /b 1
)

rem ---- read the output dir back from CMakeCache ------------------------------
set "OUT="
for /f "tokens=1,* delims==" %%A in ('findstr /b /c:"SOUI_WASM_CNCHESS_OUT:" "%BUILD%\CMakeCache.txt" 2^>nul') do set "OUT=%%B"
if "%OUT%"=="" set "OUT=%HERE%out"

echo.
echo artifacts ^(!OUT!^):
dir /b "!OUT!"
echo.
echo run : python -m http.server 8080 -d "!OUT!"   then open http://localhost:8080/%TARGET%.html
echo       ^(the page must point at a reachable cnchess_server, e.g. 127.0.0.1:3311^)
echo headless smoke test: see readme.md ^(needs a native cnchess_server binary^)
endlocal
exit /b 0

rem ---- argument errors: kept in a label so the message is echoed at top level,
rem outside any ( ... ) block, where a path with brackets would end the block.
:two_dirs
echo ERROR: two build dirs given: "%BUILD%" and "%SECOND_DIR%" 1>&2
echo usage: build-cnchess.bat [build_dir] [release] 1>&2
exit /b 2
