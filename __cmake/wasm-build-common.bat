@echo off
rem =============================================================================
rem __cmake/wasm-build-common.bat -- shared build driver for every wasm app.
rem
rem A per-app wrapper (e.g. demos\wasm-demo\build.bat) declares the metadata
rem below and calls this file with %*. Everything else -- argument parsing, tool
rem checks, first configure / re-configure, build and artifact report -- lives
rem here, so all wasm apps share exactly the same flow. The only per-app code is
rem SOUI_WASM_CONFIG (extra -D... arguments for the cmake configure step) plus
rem the declarative variables.
rem
rem Variables supplied by the wrapper:
rem   SOUI_WASM_SRC           app wasm dir = the CMake top-level project dir
rem   SOUI_WASM_REPO          soui4 repo root
rem   SOUI_WASM_PROJECT       CMAKE_PROJECT_NAME, used as the reconfigure probe
rem   SOUI_WASM_TARGET        cmake --build target
rem   SOUI_WASM_CONFIG        extra -D... configure args (this is the app part)
rem   SOUI_WASM_ENV_VARS      space separated env var names passed through as -D
rem   SOUI_WASM_BUILD_TYPE / SOUI_WASM_SHIP_TYPE        debug / ship build type
rem   SOUI_WASM_BUILD_DIR / SOUI_WASM_SHIP_BUILD_DIR    fallback build dirs
rem                           (default: app wasm dir\build and \build-release)
rem   SOUI_WASM_BUILD_ENV / SOUI_WASM_SHIP_BUILD_ENV    env var names for them
rem   SOUI_WASM_OUT_VAR       CMake cache var holding the artifact dir; empty when
rem                           the app owns its output dir (no -D, no readback)
rem   SOUI_WASM_OUT_PATTERN / SOUI_WASM_SHIP_OUT_PATTERN
rem                           @SRC@ / @PT@ / @GDI@ placeholders are substituted
rem   SOUI_WASM_PAGE          html file name printed in the run hint
rem   SOUI_WASM_USE_PTHREADS  1: expose --pthreads/--no-pthreads, default ON, and
rem                           append @PT@ to the output dir (default 0)
rem   SOUI_WASM_USE_GDI       1: expose --gdi=.../--cairo/--canvas2d (default 1)
rem   SOUI_WASM_JOBS_DEFAULT  default -j for cmake --build (default 2)
rem
rem WINDOWS QUOTING: -S / -B values must NEVER end with a backslash. emcmake is
rem a Python script; cmd passes the raw command line to it and the MSVCRT argv
rem parser reads \ followed by " as a literal quote, swallowing every following
rem argument into -S (including -G, so emcmake then adds MinGW Makefiles). Both
rem paths are normalized with %%~fI below. Keep it that way.
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files line by
rem line in the active OEM codepage; UTF-8 Chinese comments split into phantom
rem commands. Chinese documentation lives in the .sh wrappers and readme.md.
rem =============================================================================
setlocal EnableDelayedExpansion

rem ---- wrapper contract ------------------------------------------------------
if not defined SOUI_WASM_SRC (echo ERROR: wrapper did not set SOUI_WASM_SRC 1>&2 & exit /b 2)
if not defined SOUI_WASM_REPO (echo ERROR: wrapper did not set SOUI_WASM_REPO 1>&2 & exit /b 2)
if not defined SOUI_WASM_PROJECT (echo ERROR: wrapper did not set SOUI_WASM_PROJECT 1>&2 & exit /b 2)
if not defined SOUI_WASM_TARGET (echo ERROR: wrapper did not set SOUI_WASM_TARGET 1>&2 & exit /b 2)
if not exist "!SOUI_WASM_REPO!\__cmake\wasm-lib-config.cmake" (
    echo ERROR: no __cmake\wasm-lib-config.cmake under !SOUI_WASM_REPO! 1>&2
    exit /b 2
)

if not defined SOUI_WASM_BUILD_TYPE set "SOUI_WASM_BUILD_TYPE=RelWithDebInfo"
if not defined SOUI_WASM_SHIP_TYPE set "SOUI_WASM_SHIP_TYPE=Release"
if not defined SOUI_WASM_BUILD_ENV set "SOUI_WASM_BUILD_ENV=SOUI_WASM_BUILD"
if not defined SOUI_WASM_SHIP_BUILD_ENV set "SOUI_WASM_SHIP_BUILD_ENV=SOUI_WASM_SHIP_BUILD"
if not defined SOUI_WASM_OUT_VAR set "SOUI_WASM_OUT_VAR="
if not defined SOUI_WASM_OUT_PATTERN set "SOUI_WASM_OUT_PATTERN=@SRC@\out"
if not defined SOUI_WASM_SHIP_OUT_PATTERN set "SOUI_WASM_SHIP_OUT_PATTERN=@SRC@\release"
if not defined SOUI_WASM_ENV_VARS set "SOUI_WASM_ENV_VARS="
if not defined SOUI_WASM_PAGE set "SOUI_WASM_PAGE=%SOUI_WASM_TARGET%.html"
if not defined SOUI_WASM_USE_PTHREADS set "SOUI_WASM_USE_PTHREADS=0"
if not defined SOUI_WASM_USE_GDI set "SOUI_WASM_USE_GDI=1"
if not defined SOUI_WASM_JOBS_DEFAULT set "SOUI_WASM_JOBS_DEFAULT=2"
if not defined SOUI_WASM_CONFIG set "SOUI_WASM_CONFIG="

rem Normalize the source path: %%~fI drops the trailing backslash from %~dp0.
for %%I in ("!SOUI_WASM_SRC!\.") do set "SOUI_WASM_SRC=%%~fI"

rem Default build trees live under each app's own wasm dir. A single shared dir
rem (%TEMP%\build-soui-wasm) let different apps reuse one CMakeCache and then
rem fail with "source does not match the source used to generate cache".
if not defined SOUI_WASM_BUILD_DIR set "SOUI_WASM_BUILD_DIR=!SOUI_WASM_SRC!\build"
if not defined SOUI_WASM_SHIP_BUILD_DIR set "SOUI_WASM_SHIP_BUILD_DIR=!SOUI_WASM_SRC!\build-release"

rem ---- args: [build_dir] [--release] [--gdi=v] [--pthreads|--no-pthreads] ---
set "BUILD="
set "SOUI_WASM_BUILD_ARG="
set "SECOND_DIR="
set "TIER=0"
set "NEXT_GDI=0"
set "GDI_FLAG=0"
set "PTHREAD_FLAG=0"
set "GDI_BACKEND=%SWINX_WASM_GDI_BACKEND%"
set "PTHREADS=OFF"
if "%SOUI_WASM_USE_PTHREADS%"=="1" set "PTHREADS=%SOUI_WASM_PTHREADS%"
if "%SOUI_WASM_USE_PTHREADS%"=="1" if not defined PTHREADS set "PTHREADS=ON"

:parse_args
if "%~1"=="" goto :args_done
set "_A=%~1"
if "%NEXT_GDI%"=="1" ( set "GDI_BACKEND=%~1" & set "NEXT_GDI=0" & shift & goto :parse_args )
if /i "%~1"=="-h" goto :usage
if /i "%~1"=="--help" goto :usage
if /i "%~1"=="-r" ( set "TIER=1" & shift & goto :parse_args )
if /i "%~1"=="--release" ( set "TIER=1" & shift & goto :parse_args )
if /i "%~1"=="release" ( set "TIER=1" & shift & goto :parse_args )
if /i "%~1"=="/release" ( set "TIER=1" & shift & goto :parse_args )
if /i "!_A:~0,6!"=="--gdi=" ( set "GDI_BACKEND=!_A:~6!" & set "GDI_FLAG=1" & shift & goto :parse_args )
if /i "%~1"=="--gdi" ( set "NEXT_GDI=1" & set "GDI_FLAG=1" & shift & goto :parse_args )
if /i "%~1"=="--cairo" ( set "GDI_BACKEND=cairo" & set "GDI_FLAG=1" & shift & goto :parse_args )
if /i "%~1"=="--canvas2d" ( set "GDI_BACKEND=canvas2d" & set "GDI_FLAG=1" & shift & goto :parse_args )
if "%~1"=="-p" ( set "PTHREADS=ON" & set "PTHREAD_FLAG=1" & shift & goto :parse_args )
if "%~1"=="-P" ( set "PTHREADS=OFF" & set "PTHREAD_FLAG=1" & shift & goto :parse_args )
if /i "%~1"=="--pthreads" ( set "PTHREADS=ON" & set "PTHREAD_FLAG=1" & shift & goto :parse_args )
if /i "%~1"=="--no-pthreads" ( set "PTHREADS=OFF" & set "PTHREAD_FLAG=1" & shift & goto :parse_args )
if "!_A:~0,1!"=="-" goto :bad_flag
if not defined SOUI_WASM_BUILD_ARG ( set "SOUI_WASM_BUILD_ARG=%~1" & shift & goto :parse_args )
set "SECOND_DIR=%~1"
shift
goto :parse_args
:args_done

if "%NEXT_GDI%"=="1" ( echo ERROR: --gdi needs a value: canvas2d or cairo 1>&2 & exit /b 2 )
if defined SECOND_DIR goto :two_dirs
if "%GDI_FLAG%"=="1" if not "%SOUI_WASM_USE_GDI%"=="1" ( echo ERROR: --gdi is not supported by this app 1>&2 & exit /b 2 )
if "%PTHREAD_FLAG%"=="1" if not "%SOUI_WASM_USE_PTHREADS%"=="1" ( echo ERROR: pthreads tier is not supported by this app 1>&2 & exit /b 2 )
if defined GDI_BACKEND if /i not "%GDI_BACKEND%"=="canvas2d" if /i not "%GDI_BACKEND%"=="cairo" (
    echo ERROR: --gdi only accepts canvas2d or cairo, got "%GDI_BACKEND%" 1>&2
    exit /b 2
)
set "GDI_FOR_PATH=%GDI_BACKEND%"
if not defined GDI_FOR_PATH set "GDI_FOR_PATH=canvas2d"
if not "%SOUI_WASM_USE_PTHREADS%"=="1" goto :pthreads_done
if /i "%PTHREADS%"=="OFF" goto :pthreads_off
if /i "%PTHREADS%"=="0" goto :pthreads_off
if /i "%PTHREADS%"=="FALSE" goto :pthreads_off
set "PTHREADS=ON"
goto :pthreads_done
:pthreads_off
set "PTHREADS=OFF"
:pthreads_done
set "PT_SUFFIX="
if "%SOUI_WASM_USE_PTHREADS%"=="1" if "%PTHREADS%"=="ON" set "PT_SUFFIX=_pt"

rem ---- tier: build type, default build dir, output pattern -------------------
if "%TIER%"=="1" (
    set "BUILD_TYPE=%SOUI_WASM_SHIP_TYPE%"
    set "DEFAULT_BUILD=%SOUI_WASM_SHIP_BUILD_DIR%"
    set "OUT_PATTERN=%SOUI_WASM_SHIP_OUT_PATTERN%"
    set "BUILD_ENV=%SOUI_WASM_SHIP_BUILD_ENV%"
) else (
    set "BUILD_TYPE=%SOUI_WASM_BUILD_TYPE%"
    set "DEFAULT_BUILD=%SOUI_WASM_BUILD_DIR%"
    set "OUT_PATTERN=%SOUI_WASM_OUT_PATTERN%"
    set "BUILD_ENV=%SOUI_WASM_BUILD_ENV%"
)
set "_ENV_VALUE="
if defined BUILD_ENV call :get_env_value "!BUILD_ENV!"
if defined _ENV_VALUE set "DEFAULT_BUILD=!_ENV_VALUE!"
if not "!SOUI_WASM_BUILD_ARG!"=="" set "BUILD=!SOUI_WASM_BUILD_ARG!"
if not defined BUILD set "BUILD=!DEFAULT_BUILD!"
rem Normalize the build dir too: a user-supplied "D:\build\" would break -B.
for %%I in ("!BUILD!\.") do set "BUILD=%%~fI"

rem ---- output dir: substitute @SRC@ / @PT@ / @GDI@ --------------------------
set "_SRC_REP=!SOUI_WASM_SRC!"
set "_PT_REP=!PT_SUFFIX!"
set "_GDI_REP=!GDI_FOR_PATH!"
set "OUT_DIR=!OUT_PATTERN!"
set "OUT_DIR=!OUT_DIR:@SRC@=%_SRC_REP%!"
set "OUT_DIR=!OUT_DIR:@PT@=%_PT_REP%!"
set "OUT_DIR=!OUT_DIR:@GDI@=%_GDI_REP%!"
set "_ENV_VALUE="
if defined SOUI_WASM_OUT_VAR call :get_env_value "!SOUI_WASM_OUT_VAR!"
if defined _ENV_VALUE set "OUT_DIR=!_ENV_VALUE!"

rem ---- reconfigure args: generic env passthrough + app-specific config -------
set "RECONFIG="
for %%E in (%SOUI_WASM_ENV_VARS%) do call :add_env_arg %%E
if defined SOUI_WASM_OUT_VAR set RECONFIG=!RECONFIG! -D!SOUI_WASM_OUT_VAR!="!OUT_DIR!"
rem Apps without a pthread tier must still pin the cached value to OFF. Leaving
rem the -D out entirely means a build tree configured for another app (or with
rem -p) keeps SOUI_WASM_PTHREADS=ON, so "pump" output is compiled with -pthread.
if "%SOUI_WASM_USE_PTHREADS%"=="1" (
    set "RECONFIG=!RECONFIG! -DSOUI_WASM_PTHREADS=!PTHREADS!"
) else (
    set "RECONFIG=!RECONFIG! -DSOUI_WASM_PTHREADS=OFF"
)
if defined GDI_BACKEND set "RECONFIG=!RECONFIG! -DSWINX_WASM_GDI_BACKEND=%GDI_BACKEND%"
set "RECONFIG=!RECONFIG! %SOUI_WASM_CONFIG%"

rem ---- tool / tree checks ----------------------------------------------------
where em++ >nul 2>nul
if errorlevel 1 if defined EMSDK call "%EMSDK%\emsdk_env.bat" >nul 2>nul
where em++ >nul 2>nul
if errorlevel 1 ( echo ERROR: em++ not found; run emsdk_env.bat or add emsdk to PATH 1>&2 & exit /b 1 )
where ninja >nul 2>nul
if errorlevel 1 ( echo ERROR: ninja not found; the wasm builds use -G Ninja 1>&2 & exit /b 1 )
if not exist "!SOUI_WASM_REPO!\swinx\include" (
    echo ERROR: !SOUI_WASM_REPO!\swinx\include not found - is swinx checked out? 1>&2
    exit /b 1
)

rem ---- configure (fresh) or reconfigure (existing cache) --------------------
if exist "!BUILD!\CMakeCache.txt" goto :have_cache
where emcmake >nul 2>nul
if errorlevel 1 ( echo ERROR: emcmake not found - check the emscripten dir in PATH 1>&2 & exit /b 1 )
echo == configure: !BUILD! [!BUILD_TYPE!, Ninja]
call emcmake cmake -S "!SOUI_WASM_SRC!" -B "!BUILD!" -G Ninja -DCMAKE_BUILD_TYPE=!BUILD_TYPE! !RECONFIG!
if errorlevel 1 (
    echo ERROR: emcmake configure failed 1>&2
    if exist "!BUILD!\CMakeCache.txt" (
        echo HINT: a cache created for another tree still points at old paths.
        echo       delete the build dir and configure again: rmdir /s /q "!BUILD!"
    )
    exit /b 1
)
goto :build

:have_cache
if "%TIER%"=="1" (
    findstr /b /c:"CMAKE_BUILD_TYPE:STRING=%BUILD_TYPE%" "!BUILD!\CMakeCache.txt" >nul 2>nul
    if errorlevel 1 ( echo ERROR: !BUILD! is not a %BUILD_TYPE% build dir; the ship tier needs its own 1>&2 & exit /b 1 )
)
if "%TIER%"=="0" (
    findstr /b /c:"CMAKE_BUILD_TYPE:STRING=%BUILD_TYPE%" "!BUILD!\CMakeCache.txt" >nul 2>nul
    if errorlevel 1 echo note: CMAKE_BUILD_TYPE in !BUILD! is not %BUILD_TYPE%; keeping the cached type
)
findstr /b /c:"CMAKE_PROJECT_NAME:STRING=%SOUI_WASM_PROJECT%" "!BUILD!\CMakeCache.txt" >nul 2>nul
if errorlevel 1 echo == !BUILD! has no %SOUI_WASM_PROJECT% project marker
echo == reconfigure: !BUILD!
call cmake -S "!SOUI_WASM_SRC!" -B "!BUILD!" !RECONFIG!
if errorlevel 1 (
    echo ERROR: re-configure failed 1>&2
    echo HINT: a cache created for another tree still points at old paths.
    echo       delete the build dir and configure again: rmdir /s /q "!BUILD!"
    exit /b 1
)

rem ---- build -----------------------------------------------------------------
:build
if not defined SOUI_WASM_JOBS set "SOUI_WASM_JOBS=%SOUI_WASM_JOBS_DEFAULT%"
echo == build: cmake --build !BUILD! --target %SOUI_WASM_TARGET% -j%SOUI_WASM_JOBS%
call cmake --build "!BUILD!" --target "%SOUI_WASM_TARGET%" -j%SOUI_WASM_JOBS%
if errorlevel 1 ( echo ERROR: build failed. If a TU hung, kill the stuck clang/emcc and rerun. 1>&2 & exit /b 1 )

rem ---- artifacts -------------------------------------------------------------
set "OUT=!OUT_DIR!"
if "!SOUI_WASM_OUT_VAR!"=="" goto :out_done
for /f "tokens=1,* delims==" %%A in ('findstr /b /c:"!SOUI_WASM_OUT_VAR!:" "!BUILD!\CMakeCache.txt" 2^>nul') do set "OUT=%%B"
:out_done
echo.
echo artifacts: !OUT!
dir /b "!OUT!" 2>nul
echo.
echo open : http://localhost:8080/%SOUI_WASM_PAGE%
rem Apps setting SOUI_WASM_NEEDS_HTTP_PROXY (fundholdings) need the same-origin
rem /__soui_http endpoint even for pump builds, so never suggest a bare
rem "python -m http.server" for them. SOUI_WASM_NEEDS_SERVER does the same for
rem apps that always need a local WebSocket proxy (cnchess), pthread tier or not.
rem SOUI_WASM_SERVER / SOUI_WASM_SERVER_ARGS let an app point the hint at its
rem own server entry and its own flags; defaults are the shared tool with no
rem extra flags. The shared script knows no app-specific option names.
if not defined SOUI_WASM_SERVER set "SOUI_WASM_SERVER=tools\wasm\serve-isolated.py"
if not defined SOUI_WASM_SERVER_ARGS set "SOUI_WASM_SERVER_ARGS="
if "%SOUI_WASM_USE_PTHREADS%"=="1" if "%PTHREADS%"=="ON" goto :pt_hint
if "%SOUI_WASM_NEEDS_HTTP_PROXY%"=="1" goto :pt_hint
if "%SOUI_WASM_NEEDS_SERVER%"=="1" goto :pt_hint
echo serve: python -m http.server 8080 -d "!OUT!"
goto :hint_done
:pt_hint
echo serve: python "!SOUI_WASM_REPO!\!SOUI_WASM_SERVER!" "!OUT!" --port 8080!SOUI_WASM_SERVER_ARGS!
:hint_done
endlocal
exit /b 0

rem ---- messages / subroutines ------------------------------------------------
:usage
echo Usage: build.bat [build_dir] [flags]
echo.
echo   build_dir              cmake binary dir (default: app wasm dir\build)
echo   -r, --release          ship tier (Release, its own build/out dir)
echo   --gdi=VALUE            swinx wasm GDI backend, VALUE is canvas2d or cairo
echo   -p, --pthreads         worker backend (env SOUI_WASM_PTHREADS)
echo   -P, --no-pthreads      pump backend
echo   -h, --help             show the app wrapper header (see build-soui.sh)
echo.
echo App-specific cmake configure args live in SOUI_WASM_CONFIG in the wrapper.
echo Env tunables are passed through by name via SOUI_WASM_ENV_VARS.
exit /b 0

:bad_flag
echo ERROR: unknown option: !_A! 1>&2
exit /b 2

:two_dirs
echo ERROR: two build dirs given: "!SOUI_WASM_BUILD_ARG!" and "!SECOND_DIR!" 1>&2
exit /b 2

rem Read the value of the env var whose NAME is %~1 into _ENV_VALUE.
:get_env_value
set "_ENV_VALUE="
if "%~1"=="" goto :eof
set "_ENV_VALUE=!%~1!"
goto :eof

rem Append "-D<name>=<value>" for one of SOUI_WASM_ENV_VARS.
:add_env_arg
set "_N=%~1"
set "_V=!%_N%!"
if not defined _V goto :eof
set RECONFIG=!RECONFIG! -D%_N%="!_V!"
goto :eof
