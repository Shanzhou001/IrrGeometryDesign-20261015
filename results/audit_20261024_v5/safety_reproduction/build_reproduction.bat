@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
if errorlevel 1 exit /b 1
cl /nologo /EHsc /Od /Fe"D:\Desktop\work\IrrGeometryDesign-20261015\results\audit_20261024_v5\safety_reproduction\ibm_zero.exe" /Fo"D:\Desktop\work\IrrGeometryDesign-20261015\results\audit_20261024_v5\safety_reproduction\ibm_zero.obj" "D:\Desktop\work\IrrGeometryDesign-20261015\results\audit_20261024_v5\safety_reproduction\ibm_zero.cpp"
