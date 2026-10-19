# -*- coding: utf-8 -*-
with open("IrrGeometryDesign/scientificplotter.h", "r", encoding="utf-8") as f:
    lines = f.readlines()

print("Strings with non-ascii or replacement characters in scientificplotter.h:")
for i, line in enumerate(lines):
    if "drawText" in line:
        print(f"Line {i+1}: {line.strip()}")
