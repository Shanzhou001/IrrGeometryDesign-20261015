# -*- coding: utf-8 -*-
"""
ablation_stats.py
=================
对 `--ablation-multiseed` 的输出做统计检验。

输入：results/ablation_multiseed/ablation_multiseed_raw.csv
输出：控制台表格 + results/ablation_multiseed/ablation_stats.csv

检验方法及适用条件（必须写入论文）：
  - 配对比较用 **Wilcoxon 符号秩检验**（非参数，不假设正态；n=20 时双侧 alpha=0.05
    的最小可检出效应约为 6 个非零差值同号，见临界值表）。
  - 同时给出 **配对 t 检验** 作参照，并注明其正态性假设。
  - 效应量用 **Cliff's delta**（非参数），避免 Cohen's d 的正态假设。
  - 多重比较（3 档采样率 × 4 组对照 = 12 次）用 **Holm-Bonferroni** 校正。

依赖：仅标准库（不引入 scipy，保证可复现）。
"""
import argparse
import csv
import math
import os
import sys
from collections import defaultdict


def load(path):
    rows = []
    with open(path, "r", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            rows.append(r)
    return rows


def wilcoxon_signed_rank(a, b):
    """返回 (W+, W-, p_two_sided, n_effective)。

    精确分布（n<=25 时枚举），否则正态近似。
    """
    diffs = [x - y for x, y in zip(a, b)]
    diffs = [d for d in diffs if abs(d) > 1e-12]
    n = len(diffs)
    if n == 0:
        return 0.0, 0.0, 1.0, 0
    ranks = rank_abs(diffs)
    w_plus = sum(r for r, d in zip(ranks, diffs) if d > 0)
    w_minus = sum(r for r, d in zip(ranks, diffs) if d < 0)
    stat = min(w_plus, w_minus)

    if n <= 25:
        p = exact_wilcoxon_p(stat, n)
    else:
        mean = n * (n + 1) / 4.0
        sd = math.sqrt(n * (n + 1) * (2 * n + 1) / 24.0)
        z = (stat - mean + 0.5) / sd if sd > 0 else 0.0
        p = 2.0 * norm_cdf(-abs(z))
    return w_plus, w_minus, min(1.0, p), n


def rank_abs(diffs):
    pairs = sorted(((abs(d), i) for i, d in enumerate(diffs)), key=lambda t: t[0])
    ranks = [0.0] * len(diffs)
    i = 0
    while i < len(pairs):
        j = i
        while j + 1 < len(pairs) and abs(pairs[j + 1][0] - pairs[i][0]) < 1e-12:
            j += 1
        avg = (i + j + 2) / 2.0
        for k in range(i, j + 1):
            ranks[pairs[k][1]] = avg
        i = j + 1
    return ranks


def exact_wilcoxon_p(stat, n):
    """枚举所有符号组合，计算 P(min(W+,W-) <= stat)。"""
    total = 1 << n
    count = 0
    for mask in range(total):
        wp = 0
        for i in range(n):
            if mask & (1 << i):
                wp += (i + 1)
        wm = n * (n + 1) // 2 - wp
        if min(wp, wm) <= stat:
            count += 1
    return count / float(total)


def norm_cdf(x):
    return 0.5 * (1.0 + math.erf(x / math.sqrt(2.0)))


def paired_t(a, b):
    n = len(a)
    if n < 2:
        return 0.0, 1.0
    d = [x - y for x, y in zip(a, b)]
    mean = sum(d) / n
    var = sum((x - mean) ** 2 for x in d) / (n - 1)
    if var <= 0:
        return 0.0, 1.0
    t = mean / math.sqrt(var / n)
    # 双尾 p 用正态近似（n>=20 时可接受；已在报告中注明）
    p = 2.0 * norm_cdf(-abs(t))
    return t, p


def cliffs_delta(a, b):
    gt = lt = 0
    for x in a:
        for y in b:
            if x > y:
                gt += 1
            elif x < y:
                lt += 1
    tot = len(a) * len(b)
    return (gt - lt) / float(tot) if tot else 0.0


def holm(pvals):
    """Holm-Bonferroni 校正，返回校正后 p 值（与输入同序）。"""
    m = len(pvals)
    order = sorted(range(m), key=lambda i: pvals[i])
    adj = [1.0] * m
    running = 0.0
    for rank, idx in enumerate(order):
        val = (m - rank) * pvals[idx]
        running = max(running, val)
        adj[idx] = min(1.0, running)
    return adj


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in", dest="inp",
                    default=os.path.join("results", "ablation_multiseed",
                                         "ablation_multiseed_raw.csv"))
    ap.add_argument("--out", dest="out",
                    default=os.path.join("results", "ablation_multiseed",
                                         "ablation_stats.csv"))
    args = ap.parse_args()

    if not os.path.exists(args.inp):
        sys.exit("ERROR: input not found: %s (run --ablation-multiseed first)" % args.inp)

    rows = load(args.inp)
    by_rate = defaultdict(list)
    for r in rows:
        by_rate[r["rate"]].append(r)

    comparisons = [
        ("SA+Local", "mu_sa_local", "Restart", "mu_restart_best"),
        ("SA+Local", "mu_sa_local", "SA-only", "mu_sa_only"),
        ("SA+Local", "mu_sa_local", "Local-only", "mu_local_only"),
        ("SA+Local", "mu_sa_local", "JitterInit", "mu_jitter_init"),
        ("Constrained", "mu_constrained", "SA+Local", "mu_sa_local"),
    ]

    out_rows = []
    pvals = []
    for rate in sorted(by_rate.keys()):
        rs = by_rate[rate]
        for name_a, col_a, name_b, col_b in comparisons:
            a = [float(r[col_a]) for r in rs]
            b = [float(r[col_b]) for r in rs]
            if len(a) < 3:
                continue
            wp, wm, p, n = wilcoxon_signed_rank(a, b)
            t, pt = paired_t(a, b)
            delta = cliffs_delta(a, b)
            mean_a = sum(a) / len(a)
            mean_b = sum(b) / len(b)
            pvals.append(p)
            out_rows.append({
                "rate": rate, "n": len(a),
                "A": name_a, "B": name_b,
                "mean_A": "%.6f" % mean_a,
                "mean_B": "%.6f" % mean_b,
                "mean_diff": "%.6f" % (mean_a - mean_b),
                "dB_A_over_B": "%.3f" % (20 * math.log10(mean_b / mean_a) if mean_a > 0 and mean_b > 0 else 0.0),
                "W_plus": "%.1f" % wp, "W_minus": "%.1f" % wm,
                "p_wilcoxon": "%.6f" % p,
                "p_ttest": "%.6f" % pt,
                "cliffs_delta": "%.4f" % delta,
                "n_effective": n,
            })

    adj = holm(pvals) if pvals else []
    for i, r in enumerate(out_rows):
        r["p_holm"] = "%.6f" % adj[i]
        r["significant_0.05"] = "YES" if adj[i] < 0.05 else "NO"

    if not out_rows:
        sys.exit("ERROR: no comparisons produced")

    keys = ["rate", "n", "A", "B", "mean_A", "mean_B", "mean_diff", "dB_A_over_B",
            "W_plus", "W_minus", "p_wilcoxon", "p_ttest", "p_holm",
            "significant_0.05", "cliffs_delta", "n_effective"]
    with open(args.out, "w", encoding="utf-8", newline="\n") as f:
        w = csv.DictWriter(f, fieldnames=keys)
        w.writeheader()
        for r in out_rows:
            w.writerow(r)

    print("%-6s %-12s vs %-12s %8s %8s %9s %8s %6s" % (
        "rate", "A", "B", "meanA", "meanB", "p_holm", "delta", "sig"))
    for r in out_rows:
        print("%-6s %-12s vs %-12s %8s %8s %9s %8s %6s" % (
            r["rate"], r["A"], r["B"], r["mean_A"], r["mean_B"],
            r["p_holm"], r["cliffs_delta"], r["significant_0.05"]))
    print("\nwritten: %s" % args.out)
    print("\n[Method note] Wilcoxon signed-rank (exact for n<=25) + Holm-Bonferroni;")
    print("              paired t-test given as reference only (normality assumed).")


if __name__ == "__main__":
    main()
