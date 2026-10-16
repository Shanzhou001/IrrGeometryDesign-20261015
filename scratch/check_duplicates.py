# -*- coding: utf-8 -*-
with open("IrrGeometryDesign/spsshow/tspsform.cpp", "r", encoding="utf-8") as f:
    lines = f.readlines()

print(f"Total lines in tspsform.cpp: {len(lines)}")
for i, l in enumerate(lines):
    if "on_actionRunSamplingBenchmark_triggered" in l:
        print(f"Found on_actionRunSamplingBenchmark_triggered at line {i+1}")
    if "on_actionRunFullExperimentSuite_triggered" in l:
        print(f"Found on_actionRunFullExperimentSuite_triggered at line {i+1}")
