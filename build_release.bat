@echo off
call "%~dp0build\native.bat" Release
exit /b %ERRORLEVEL%
