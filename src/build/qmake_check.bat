@echo off
setlocal EnableExtensions EnableDelayedExpansion
for %%I in ("%~dp0..\..") do set "PROJECT_ROOT=%%~fI"
set "IRR_WORKSPACE=!PROJECT_ROOT!"
call "%~dp0environment.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
if not exist "!IRR_WORKSPACE!\build\qt" mkdir "!IRR_WORKSPACE!\build\qt"
cd /d "!IRR_WORKSPACE!\build\qt"
qmake.exe -spec win32-msvc2008 -o Makefile "!PROJECT_ROOT!\src\build\qt\IrrGeometryDesign.pro" "CONFIG+=release" "CONFIG-=debug"
if errorlevel 1 exit /b %ERRORLEVEL%
nmake /nologo
if errorlevel 1 exit /b %ERRORLEVEL%
copy /Y "!PROJECT_ROOT!\src\third_party\legacy-libs\*.dll" "bin\" >nul
exit /b %ERRORLEVEL%
