@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
if errorlevel 1 exit /b 1
cl /nologo /EHsc /Od /MDd /ID:\Desktop\sci\IrrGeometryDesign\src\third_party\curvelet\Curvelet2d D:\Desktop\sci\IrrGeometryDesign\src\tests\native\fdct_roundtrip_probe.cpp D:\Desktop\sci\IrrGeometryDesign\src\third_party\curvelet\Curvelet2d\fdct_wrapping.cpp D:\Desktop\sci\IrrGeometryDesign\src\third_party\curvelet\Curvelet2d\ifdct_wrapping.cpp /Fefdct_probe.exe /link D:\Desktop\sci\IrrGeometryDesign\src\third_party\legacy-libs\fftw.lib
