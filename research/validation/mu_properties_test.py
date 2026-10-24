# -*- coding: utf-8 -*-
"""
mu_properties_test.py
=====================
独立复现 CurveletMiu 的数值行为，用于"问题 1：目标函数数学性质"取证。

不依赖 C++ 源码的自我陈述，而是：
  1) 用 Python 重新实现 FFT1D_fftshift 与 fdct_wrapping 的**可复现近似**不可行，
     因此改为直接调用已编译的 C++ 可执行文件，通过新增的 --mu-probe 入口
     读取真实数值（见 geometryexperimentengine.cpp 的 runMiuProbe）。
  2) 本脚本负责：设计探针用例、抓取输出、做数学性质判定。

用法：
    python -m research.validation.mu_properties_test --exe x64\\Release\\IrrGeometryDesign.exe ^
        --out results/audit/mu_properties.csv
"""
import argparse
import csv
import os
import subprocess
import sys
from research.common.paths import ROOT


def run_probe(exe, cases, out_csv):
    """调用 C++ 探针入口，返回解析后的行。"""
    cmd = [exe, "--mu-probe", out_csv]
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if r.returncode != 0:
        sys.stderr.write(r.stdout.decode("utf-8", "replace"))
        raise SystemExit("mu-probe failed with code %d" % r.returncode)
    rows = []
    with open(out_csv, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            rows.append(s.split(","))
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exe", default=str(ROOT / "build/bin/Release/IrrGeometryDesign.exe"))
    ap.add_argument("--out", default=os.path.join("results", "audit", "mu_properties.csv"))
    args = ap.parse_args()

    if not os.path.exists(args.exe):
        sys.exit("ERROR: exe not found: %s" % args.exe)
    os.makedirs(os.path.dirname(args.out), exist_ok=True)

    rows = run_probe(args.exe, None, args.out)
    hdr = rows[0]
    data = rows[1:]
    print("cases:", len(data))
    print(",".join(hdr))
    for d in data:
        print(",".join(d))


if __name__ == "__main__":
    main()
