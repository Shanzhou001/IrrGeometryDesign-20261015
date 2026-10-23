@echo off
setlocal EnableExtensions EnableDelayedExpansion
rem =============================================================================
rem  IrrGeometryDesign - Release x64 build script
rem  2026.10.20: path-parameterised (no more hard-coded D:\Qt\4.7.4).
rem
rem  Optional environment variables:
rem    QTDIR    Qt 4.x root   (default: qmake.exe on PATH, then common folders)
rem    VCVAUTO  full path to vcvarsall.bat (default: probes VS2008 folders)
rem    VCARCH   architecture passed to vcvarsall (default: x86_amd64)
rem =============================================================================

rem ---- 1. locate Qt -----------------------------------------------------------
if defined QTDIR if not exist "!QTDIR!\bin\qmake.exe" (
    echo [build] QTDIR is unavailable; checking qmake.exe on PATH.
    set "QTDIR="
)
if not defined QTDIR (
    for /f "delims=" %%Q in ('where qmake.exe 2^>nul') do (
        if not defined QTDIR set "QTDIR=%%~dpQ.."
    )
)
set "QROOT1=D:\Qt\4.7.4"
set "QROOT2=C:\Qt\4.7.4"
if not defined QTDIR if exist "!QROOT1!\bin\qmake.exe" set "QTDIR=!QROOT1!"
if not defined QTDIR if exist "!QROOT2!\bin\qmake.exe" set "QTDIR=!QROOT2!"
if not defined QTDIR (
    echo [ERROR] QTDIR is not set and qmake.exe was not found.
    echo         Set it first, e.g.  set QTDIR=D:\Qt\4.7.4
    exit /b 2
)
if not exist "!QTDIR!\bin\qmake.exe" (
    echo [ERROR] qmake.exe not found under QTDIR=!QTDIR!
    exit /b 2
)

rem ---- 2. locate Visual Studio 9.0 (VS2008) ----------------------------------
set "VCROOT1=!ProgramFiles(x86)!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set "VCROOT2=!ProgramFiles!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if not defined VCVAUTO if exist "!VCROOT1!" set "VCVAUTO=!VCROOT1!"
if not defined VCVAUTO if exist "!VCROOT2!" set "VCVAUTO=!VCROOT2!"
if not defined VCVAUTO (
    echo [ERROR] vcvarsall.bat for Visual Studio 9.0 not found.
    echo         Set it first via the VCVAUTO environment variable.
    exit /b 2
)
if not defined VCARCH set "VCARCH=x86_amd64"

rem ---- 3. build --------------------------------------------------------------
set "PATH=!QTDIR!\bin;!PATH!"
echo [build] QTDIR = !QTDIR!
echo [build] VC    = !VCVAUTO!  (!VCARCH!)
call "!VCVAUTO!" !VCARCH!
if errorlevel 1 (
    echo [ERROR] vcvarsall failed.
    exit /b 3
)

cd /d "%~dp0"
vcbuild IrrGeometryDesign.sln "Release|x64"
set "BUILD_RC=!ERRORLEVEL!"
if "!BUILD_RC!"=="0" if not exist "x64\Release\fftw.dll" (
    copy /Y "IrrGeometryDesign\fftw.dll" "x64\Release\fftw.dll" >nul
    if errorlevel 1 set "BUILD_RC=4"
)
endlocal & exit /b %BUILD_RC%
