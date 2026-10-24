@echo off
call "%~dp0build\native.bat" Debug
exit /b %ERRORLEVEL%
