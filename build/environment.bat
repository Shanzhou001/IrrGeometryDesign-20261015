@echo off
if defined QTDIR if not exist "!QTDIR!\bin\qmake.exe" set "QTDIR="
if not defined QTDIR for /f "delims=" %%Q in ('where qmake.exe 2^>nul') do if not defined QTDIR set "QTDIR=%%~dpQ.."
if not defined QTDIR if exist "D:\Qt\4.7.4\bin\qmake.exe" set "QTDIR=D:\Qt\4.7.4"
if not defined QTDIR if exist "C:\Qt\4.7.4\bin\qmake.exe" set "QTDIR=C:\Qt\4.7.4"
if not defined QTDIR (
    echo [ERROR] Qt4 was not found. Set QTDIR to the existing Qt4 installation.
    exit /b 2
)
if not defined VCVAUTO if exist "!ProgramFiles(x86)!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" set "VCVAUTO=!ProgramFiles(x86)!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if not defined VCVAUTO if exist "!ProgramFiles!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" set "VCVAUTO=!ProgramFiles!\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if not defined VCVAUTO (
    echo [ERROR] VS2008 was not found. Set VCVAUTO to vcvarsall.bat.
    exit /b 2
)
if not defined VCARCH set "VCARCH=x86_amd64"
set "PATH=!QTDIR!\bin;!PATH!"
call "!VCVAUTO!" !VCARCH!
if errorlevel 1 exit /b 3
exit /b 0
