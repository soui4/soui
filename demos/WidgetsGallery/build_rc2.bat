@echo off
python -B "%~dp0build_resources.py"
exit /b %ERRORLEVEL%
