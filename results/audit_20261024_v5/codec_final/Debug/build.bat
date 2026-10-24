@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
if errorlevel 1 exit /b 1
cl /nologo /EHsc /Od /MDd /DQT_DLL /DWIN32 /ID:\Qt\4.7.4\include /ID:\Qt\4.7.4\include\QtCore /ID:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign /D_DEBUG D:\Desktop\work\IrrGeometryDesign-20261015\tools\legacy_codec_regression.cpp D:\Desktop\work\IrrGeometryDesign-20261015\IrrGeometryDesign\spssegy\segyio.cpp /Felegacy_codec_tests.exe /link /LIBPATH:D:\Qt\4.7.4\lib QtCored4.lib
