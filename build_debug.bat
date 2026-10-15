@echo off
set QTDIR=D:\Qt\4.7.4
set PATH=D:\Qt\4.7.4\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
cd /d "%~dp0"
vcbuild IrrGeometryDesign.sln "Debug|x64"
