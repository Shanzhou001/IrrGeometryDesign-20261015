@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "BUILD_MODE=%~1"
if not "%BUILD_MODE%"=="Release" if not "%BUILD_MODE%"=="Debug" exit /b 2
call "%~dp0environment.bat"
if errorlevel 1 exit /b %ERRORLEVEL%
cd /d "%~dp0.."
if not exist "build\generated\!BUILD_MODE!" mkdir "build\generated\!BUILD_MODE!"
vcbuild IrrGeometryDesign.sln "!BUILD_MODE!|x64"
set "BUILD_RC=!ERRORLEVEL!"
if "!BUILD_RC!"=="0" (
    copy /Y "third_party\legacy-libs\*.dll" "build\bin\!BUILD_MODE!\" >nul
    if errorlevel 1 set "BUILD_RC=4"
    copy /Y "resources\qt\IrrGeometryDesign_zh.qm" "build\bin\!BUILD_MODE!\IrrGeometryDesign_zh.qm" >nul
    if errorlevel 1 set "BUILD_RC=4"
)
endlocal & exit /b %BUILD_RC%
