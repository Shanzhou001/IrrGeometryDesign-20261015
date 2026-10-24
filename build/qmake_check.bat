@echo off
setlocal EnableExtensions EnableDelayedExpansion
call "%~dp0environment.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
if not exist "%~dp0qt-check" mkdir "%~dp0qt-check"
cd /d "%~dp0qt-check"
qmake.exe -spec win32-msvc2008 -o Makefile "..\qt\IrrGeometryDesign.pro" "CONFIG+=release" "CONFIG-=debug"
if errorlevel 1 exit /b %ERRORLEVEL%
nmake /nologo
if errorlevel 1 exit /b %ERRORLEVEL%
copy /Y "..\..\third_party\legacy-libs\*.dll" "bin\" >nul
if errorlevel 1 exit /b %ERRORLEVEL%
exit /b 0
