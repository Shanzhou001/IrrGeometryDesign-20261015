@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
if errorlevel 1 exit /b 1
cl /nologo /EHsc /O2 /MD /ID:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign\Curvelet2d D:\Desktop\work\IrrGeometryDesign-20261015\tools\fdct_roundtrip_probe.cpp D:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign\Curvelet2d\fdct_wrapping.cpp D:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign\Curvelet2d\ifdct_wrapping.cpp /Fefdct_probe.exe /link D:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign\fftw.lib
