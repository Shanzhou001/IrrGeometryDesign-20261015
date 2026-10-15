# -*- coding: utf-8 -*-
import os

target = os.path.join("IrrGeometryDesign", "geometryexperimentengine.cpp")
with open(target, "rb") as f:
    raw = f.read()

if raw.startswith(b"\xef\xbb\xbf"):
    raw = raw[3:]
content = raw.decode("utf-8")

old_call = "QDir(centralFiguresDir).mkpath(centralFiguresDir);"
new_call = "QDir().mkpath(centralFiguresDir);"

count = content.count(old_call)
assert count == 4, f"Expected 4 occurrences, found {count}"
content = content.replace(old_call, new_call)

out_bytes = b"\xef\xbb\xbf" + content.encode("utf-8")
with open(target, "wb") as f:
    f.write(out_bytes)

print(f"Replaced {count} occurrences of QDir(centralFiguresDir).mkpath(centralFiguresDir);")
