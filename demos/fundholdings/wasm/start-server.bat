@echo off
rem =============================================================================
rem start-server.bat -- start the fundholdings wasm static + /__soui_http server.
rem
rem This folder is self-contained (html/js/wasm/data + serve-fundholdings.py),
rem so the whole folder can be copied to the server and started from here.
rem
rem Defaults target the Apache reverse-proxy deployment documented in
rem apache\fundholdings.conf: listen 127.0.0.1:18080, upstream limited to
rem eastmoney.com, Apache forwards /__soui_http to this port.
rem
rem Extra arguments are appended, e.g.:
rem   start-server.bat --port 18081
rem
rem Without Apache (open http://localhost:8080/fundholdings.html instead):
rem   start-server.bat --bind 127.0.0.1 --port 8080 --allow-host eastmoney.com
rem
rem NOTE: keep this file ASCII-only and CRLF. cmd.exe decodes batch files in the
rem active OEM codepage; UTF-8 Chinese comments split into phantom commands.
rem =============================================================================
setlocal
rem %~dp0 ends with a backslash. "%DIR%" would expand to "C:\...\wasm\" and the
rem trailing backslash escapes the closing quote for MSVCRT argument parsing, so
rem the whole rest of the line (--bind/--port/--http-proxy) would be swallowed
rem into the directory argument. Strip it and always join with "\".
set "DIR=%~dp0"
if "%DIR:~-1%"=="\" set "DIR=%DIR:~0,-1%"
set "PY=python"
where python >nul 2>nul
if errorlevel 1 (
    where py >nul 2>nul
    if errorlevel 1 (
        echo [ERROR] Python 3 not found in PATH. Install Python 3 or edit PY= in this file.
        pause
        exit /b 1
    )
    set "PY=py -3"
)

echo Serving %DIR%
echo Press Ctrl+C to stop.
%PY% "%DIR%\serve-fundholdings.py" "%DIR%" --bind 127.0.0.1 --port 18080 --http-proxy --allow-host eastmoney.com %*
set "_RC=%ERRORLEVEL%"
if not "%_RC%"=="0" pause
endlocal & exit /b %_RC%
