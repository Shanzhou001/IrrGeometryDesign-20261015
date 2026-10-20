#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
compare_masks.py
================
在同一道集、同一采样率、同一 POCS 迭代次数下，对比多份 50% 掩码的重构质量。

背景
----
2026-10-20 之前，`--run-experiments` 计算出的 Jitter-SA 最优掩码从不落盘，
后端重构用的是另一次自动生成的普通 Jitter 掩码，因此"采样优化 -> 重构"
的闭环无法验证。现在 `--run-experiments` 会导出：

    results/synthetic/01_sampling_benchmark/mask_50pct_sa_best.txt
    results/synthetic/01_sampling_benchmark/mask_50pct_jitter_init.txt

本脚本把 SA 最优、其 Jitter 初始、自动 Jitter、纯随机、规则减抽五组掩码
依次送入重构，并汇总**缺失道口径** SNR（`SNR_POCS_MissingOnly`），
因为全幅 SNR 含被精确置回的道，恒比缺失道高约 3 dB，掩盖掩码差异。

用法
----
    python tools/compare_masks.py ^
      --gather results\\synthetic\\03_wavefield_reconstruction\\recon_original_gather.csv ^
      --sa     results\\synthetic\\01_sampling_benchmark\\mask_50pct_sa_best.txt ^
      --jitter results\\synthetic\\01_sampling_benchmark\\mask_50pct_jitter_init.txt ^
      --auto   results\\synthetic\\03_wavefield_reconstruction\\cmp_mask.txt ^
      --out    results\\mask_compare

输出：<out>/mask_comparison.csv 与控制台表格。
"""
import argparse
import csv
import os
import random
import subprocess
import sys

EXE = os.path.join("x64", "Release", "IrrGeometryDesign.exe")


def load_mask(path):
    bits = []
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            s = line.strip()
            if not s or s.startswith("#"):
                continue
            for tok in s.replace(",", " ").split():
                if tok in ("0", "1"):
                    bits.append(int(tok))
    return bits


def save_mask(path, bits):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        for b in bits:
            f.write("%d\n" % b)


def make_random(n, m, seed=543):
    arr = [0] * (n - m) + [1] * m
    rng = random.Random(seed)
    for i in range(n - 1, 0, -1):
        j = rng.randrange(i + 1)
        arr[i], arr[j] = arr[j], arr[i]
    return arr


def make_regular(n, m):
    bits = [0] * n
    for k in range(m):
        bits[(k * n) // m] = 1
    return bits


def read_metrics(path):
    d = {}
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            parts = line.strip().split(",")
            if len(parts) >= 2:
                d[parts[0]] = parts[1]
    return d


def run_one(gather, mask_path, out_dir, figures_dir):
    env = os.environ.copy()
    env["IRR_FIGURES_DIR"] = figures_dir
    # 2026.10.20: 本脚本只做掩码对照，不应改写 figures/real/ 下的正式图件
    env["IRR_SKIP_FIGURES"] = "1"
    cmd = [EXE, "--run-real-reconstruction", gather,
           "--out", out_dir, "--mask", mask_path, "--ratio", "0.50"]
    r = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, env=env)
    if r.returncode != 0:
        print("  [FAIL] returncode=%d : %s" % (r.returncode, mask_path))
        return None
    met = os.path.join(out_dir, "recon_metrics_summary.csv")
    if not os.path.exists(met):
        print("  [FAIL] no metrics: %s" % met)
        return None
    return read_metrics(met)


def fnum(d, key):
    try:
        return float(d.get(key, "nan"))
    except ValueError:
        return float("nan")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--gather", required=True, help="256x128 gather CSV")
    ap.add_argument("--sa", required=True, help="SA-optimized mask (50%)")
    ap.add_argument("--jitter", required=True, help="Jitter init mask of the best SA run")
    ap.add_argument("--auto", default="", help="auto-generated jitter mask (optional)")
    ap.add_argument("--out", default=os.path.join("results", "mask_compare"))
    args = ap.parse_args()

    if not os.path.exists(EXE):
        sys.exit("ERROR: exe not found: %s (run build_release.bat first)" % EXE)
    if not os.path.exists(args.gather):
        sys.exit("ERROR: gather not found: %s" % args.gather)

    os.makedirs(args.out, exist_ok=True)
    figs = os.path.join(args.out, "figures")
    os.makedirs(figs, exist_ok=True)
    mdir = os.path.join(args.out, "masks")
    os.makedirs(mdir, exist_ok=True)

    masks = []
    masks.append(("SA_best", args.sa))
    masks.append(("Jitter_init", args.jitter))
    if args.auto and os.path.exists(args.auto):
        masks.append(("Auto_Jitter", args.auto))

    probe = load_mask(args.sa)
    n = len(probe)
    m = sum(probe)
    if n <= 0 or m <= 0:
        sys.exit("ERROR: invalid SA mask")

    rnd = os.path.join(mdir, "mask_random.txt")
    reg = os.path.join(mdir, "mask_regular.txt")
    save_mask(rnd, make_random(n, m))
    save_mask(reg, make_regular(n, m))
    masks.append(("Random", rnd))
    masks.append(("Regular", reg))

    rows = []
    for name, path in masks:
        print("[run] %-12s %s" % (name, path))
        out_dir = os.path.join(args.out, name)
        met = run_one(args.gather, path, out_dir, figs)
        if met is None:
            continue
        rows.append({
            "Mask": name,
            "Sampled": int(fnum(met, "Active_Traces")),
            "SNR_Zero_Full": fnum(met, "SNR_Zero_Fill"),
            "SNR_Linear_Full": fnum(met, "SNR_Linear_Interp"),
            "SNR_POCS_Full": fnum(met, "SNR_POCS_CS"),
            "SNR_Linear_Missing": fnum(met, "SNR_Linear_MissingOnly"),
            "SNR_POCS_Missing": fnum(met, "SNR_POCS_MissingOnly"),
        })

    if not rows:
        sys.exit("ERROR: no successful runs")

    for r in rows:
        r["Gain_vs_Linear_Missing"] = r["SNR_POCS_Missing"] - r["SNR_Linear_Missing"]

    out_csv = os.path.join(args.out, "mask_comparison.csv")
    keys = ["Mask", "Sampled", "SNR_Zero_Full", "SNR_Linear_Full", "SNR_POCS_Full",
            "SNR_Linear_Missing", "SNR_POCS_Missing", "Gain_vs_Linear_Missing"]
    with open(out_csv, "w", encoding="utf-8", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=keys)
        w.writeheader()
        for r in rows:
            w.writerow({k: ("%.4f" % r[k]) if isinstance(r[k], float) else r[k] for k in keys})

    print("\n%-13s %7s %9s %9s %9s %9s %9s %9s" % (
        "Mask", "m", "ZeroFull", "LinFull", "POCSFull", "LinMiss", "POCSMiss", "GainMiss"))
    for r in rows:
        print("%-13s %7d %9.2f %9.2f %9.2f %9.2f %9.2f %+9.2f" % (
            r["Mask"], r["Sampled"], r["SNR_Zero_Full"], r["SNR_Linear_Full"],
            r["SNR_POCS_Full"], r["SNR_Linear_Missing"], r["SNR_POCS_Missing"],
            r["Gain_vs_Linear_Missing"]))
    print("\nwritten: %s" % out_csv)


if __name__ == "__main__":
    main()
