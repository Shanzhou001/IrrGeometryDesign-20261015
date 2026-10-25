@echo off
set "IRR_BUILD_MODE=%~1"
if not defined IRR_BUILD_MODE set "IRR_BUILD_MODE=Release"
call "%~dp0src\build\native.bat" %IRR_BUILD_MODE%
exit /b %ERRORLEVEL%
