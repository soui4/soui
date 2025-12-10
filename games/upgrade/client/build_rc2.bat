@echo off
set SOUI_ROOT=%SOUI4_INSTALL_64%
if "%SOUI_ROOT%"=="" (
    echo Error: SOUI4_INSTALL_64 environment variable not set
    pause
    exit /b 1
)

set UIRESBUILDER=%SOUI_ROOT%\bin\Release\uiresbuilder.exe
if not exist "%UIRESBUILDER%" (
    echo Error: uiresbuilder.exe not found at %UIRESBUILDER%
    pause
    exit /b 1
)

echo Building UI resources...
"%UIRESBUILDER%" -i uires\uires.idx -p uires -r res\soui_res.rc2 -h res\resource.h idtable

if %errorlevel% neq 0 (
    echo Error: Failed to build UI resources
    pause
    exit /b 1
)

echo UI resources built successfully!
pause
