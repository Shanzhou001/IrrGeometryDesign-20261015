@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "BUILD_MODE=%~1"
if not "%BUILD_MODE%"=="Release" if not "%BUILD_MODE%"=="Debug" exit /b 2
for %%I in ("%~dp0..\..") do set "PROJECT_ROOT=%%~fI"
set "IRR_WORKSPACE=!PROJECT_ROOT!"
call "%~dp0environment.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
cd /d "!PROJECT_ROOT!"
if not exist "!IRR_WORKSPACE!\build\generated\!BUILD_MODE!" mkdir "!IRR_WORKSPACE!\build\generated\!BUILD_MODE!"
vcbuild IrrGeometryDesign.sln "!BUILD_MODE!|x64"
set "BUILD_RC=!ERRORLEVEL!"
if "!BUILD_RC!"=="0" (
    copy /Y "src\third_party\legacy-libs\*.dll" "!IRR_WORKSPACE!\build\bin\!BUILD_MODE!\" >nul
    if errorlevel 1 set "BUILD_RC=4"
    copy /Y "src\resources\qt\IrrGeometryDesign_zh.qm" "!IRR_WORKSPACE!\build\bin\!BUILD_MODE!\IrrGeometryDesign_zh.qm" >nul
    if errorlevel 1 set "BUILD_RC=4"
)
endlocal & exit /b %BUILD_RC%
