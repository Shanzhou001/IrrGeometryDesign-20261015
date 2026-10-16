@echo off
set QTDIR=D:\Qt\4.7.4
set PATH=D:\Qt\4.7.4\bin;%PATH%
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
cl /EHsc /MD /I D:\Qt\4.7.4\include /I D:\Qt\4.7.4\include\QtCore /I D:\Qt\4.7.4\include\QtGui /I IrrGeometryDesign scratch\test_plotter_render.cpp /link /LIBPATH:D:\Qt\4.7.4\lib QtCore4.lib QtGui4.lib
test_plotter_render.exe
