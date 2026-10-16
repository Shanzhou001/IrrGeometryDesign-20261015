# -*- coding: utf-8 -*-
import os

files_to_check = [
    "IrrGeometryDesign/scientificplotter.h",
    "IrrGeometryDesign/geometryexperimentengine.cpp",
    "IrrGeometryDesign/geometryexperimentengine.h",
    "IrrGeometryDesign/spsshow/tspsform.cpp",
    "IrrGeometryDesign/spsshow/tspsform.h",
    "IrrGeometryDesign/main.cpp"
]

for p in files_to_check:
    with open(p, "rb") as f:
        data = f.read()
    has_efbfbd = b"\xef\xbf\xbd" in data
    text = data.decode("utf-8", errors="replace")
    count_replace = text.count("\ufffd")
    print(f"{p}: has_efbfbd={has_efbfbd}, count_replace={count_replace}")
