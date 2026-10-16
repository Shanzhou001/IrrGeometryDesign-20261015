# -*- coding: utf-8 -*-
with open("IrrGeometryDesign/scientificplotter.h", "r", encoding="utf-8") as f:
    lines = f.readlines()

line176 = lines[175] # 0-indexed
print("Line 176 text:", repr(line176))
for ch in line176:
    print(f"{repr(ch)}: U+{ord(ch):04X}")
