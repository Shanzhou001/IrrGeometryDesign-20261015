# -*- coding: utf-8 -*-
import os

target = os.path.join("IrrGeometryDesign", "geometryexperimentengine.cpp")
with open(target, "rb") as f:
    raw = f.read()

if raw.startswith(b"\xef\xbb\xbf"):
    raw = raw[3:]
content = raw.decode("utf-8")

old_str = 'QString dirName = dir.dirName();\r\n    if (dirName.startsWith("01_")'
if old_str not in content:
    old_str = 'QString dirName = dir.dirName();\n    if (dirName.startsWith("01_")'

new_str = 'QString dirName = dir.dirName();\r\n    if (dirName == "all_result_figures") return dir.absolutePath();\r\n    if (dirName.startsWith("01_")'

assert old_str in content, "old_str not found"
content = content.replace(old_str, new_str, 1)

out_bytes = b"\xef\xbb\xbf" + content.encode("utf-8")
with open(target, "wb") as f:
    f.write(out_bytes)
print("Updated successfully. Bytes:", len(out_bytes))
