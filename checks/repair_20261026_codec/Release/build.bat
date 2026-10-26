@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
if errorlevel 1 exit /b 1
cl /nologo /EHsc /Od /MD /DQT_DLL /DWIN32 /ID:\Qt\4.7.4\include /ID:\Qt\4.7.4\include\QtCore /ID:\Desktop\sci\IrrGeometryDesign\src\native /ID:\Desktop\sci\IrrGeometryDesign\src\native\io\sps /ID:\Desktop\sci\IrrGeometryDesign\src\native\sampling D:\Desktop\sci\IrrGeometryDesign\src\tests\native\legacy_codec_regression.cpp D:\Desktop\sci\IrrGeometryDesign\src\native\io\sps\segyio.cpp D:\Desktop\sci\IrrGeometryDesign\src\native\sampling\Diagmul.cpp /Felegacy_codec_tests.exe /link /LIBPATH:D:\Qt\4.7.4\lib QtCore4.lib
