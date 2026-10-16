# -*- coding: utf-8 -*-
with open("x64/Release/IrrGeometryDesign.exe", "rb") as f:
    exe_bytes = f.read()

with open("IrrGeometryDesign/geometryexperimentengine.cpp", "r", encoding="utf-8") as f:
    lines = f.readlines()

print("Searching for outReportSummary lines in geometryexperimentengine.cpp:")
for i, line in enumerate(lines):
    if "outReportSummary" in line and "QString::fromLocal8Bit" in line:
        print(f"Line {i+1}: {line.strip()}")
        # Check what bytes are in exe
        # Extract string inside quotes
        start = line.find('"')
        end = line.rfind('"')
        if start != -1 and end > start:
            s = line[start+1:end]
            gbk_bytes = s.encode("gbk", "ignore")
            utf8_bytes = s.encode("utf-8", "ignore")
            print(f"   as GBK in exe: {gbk_bytes in exe_bytes if gbk_bytes else False}")
            print(f"   as UTF8 in exe: {utf8_bytes in exe_bytes if utf8_bytes else False}")
