# -*- coding: utf-8 -*-
"""Paired ClassicPSF comparisons: exact signed-rank DP (including ties),
Student-t reference p values, paired rank-biserial effect and Holm correction.
Only like-objective comparisons enter the multiplicity family. Standard library only.
"""  # add by wushanzhou in 2026/10/23
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

    if n <= 60:  # add by wushanzhou in 2026/10/23
        p = exact_wilcoxon_p(stat, n, ranks)  # add by wushanzhou in 2026/10/23
    else:
        mean = n * (n + 1) / 4.0
        sd = math.sqrt(sum(r * r for r in ranks) / 4.0)  # add by wushanzhou in 2026/10/23
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


def exact_wilcoxon_p(stat, n, ranks=None):  # add by wushanzhou in 2026/10/23
    """Conditional sign permutation distribution, with doubled midranks for ties."""  # add by wushanzhou in 2026/10/23
    if ranks is None:  # add by wushanzhou in 2026/10/23
        ranks = list(range(1, n + 1))  # add by wushanzhou in 2026/10/23
    weights = [int(round(2 * rank)) for rank in ranks]  # add by wushanzhou in 2026/10/23
    total = sum(weights)  # add by wushanzhou in 2026/10/23
    distribution = [1] + [0] * total  # add by wushanzhou in 2026/10/23
    for weight in weights:  # add by wushanzhou in 2026/10/23
        for value in range(total, weight - 1, -1):  # add by wushanzhou in 2026/10/23
            distribution[value] += distribution[value - weight]  # add by wushanzhou in 2026/10/23
    limit = int(round(2 * stat))  # add by wushanzhou in 2026/10/23
    count = sum(c for value, c in enumerate(distribution)  # add by wushanzhou in 2026/10/23
                if min(value, total - value) <= limit)  # add by wushanzhou in 2026/10/23
    return count / float(2 ** n)  # add by wushanzhou in 2026/10/23


def beta_fraction(a, b, x):  # add by wushanzhou in 2026/10/23
    """Continued fraction for the regularized incomplete beta function."""  # add by wushanzhou in 2026/10/23
    tiny, eps = 1e-300, 3e-14  # add by wushanzhou in 2026/10/23
    c = 1.0  # add by wushanzhou in 2026/10/23
    d = 1.0 - (a + b) * x / (a + 1.0)  # add by wushanzhou in 2026/10/23
    if abs(d) < tiny:  # add by wushanzhou in 2026/10/23
        d = tiny  # add by wushanzhou in 2026/10/23
    d = 1.0 / d  # add by wushanzhou in 2026/10/23
    result = d  # add by wushanzhou in 2026/10/23
    for m in range(1, 301):  # add by wushanzhou in 2026/10/23
        for term in (m * (b - m) * x / ((a + 2*m - 1) * (a + 2*m)),  # add by wushanzhou in 2026/10/23
                     -(a + m) * (a + b + m) * x / ((a + 2*m) * (a + 2*m + 1))):  # add by wushanzhou in 2026/10/23
            d = 1.0 + term * d  # add by wushanzhou in 2026/10/23
            if abs(d) < tiny:  # add by wushanzhou in 2026/10/23
                d = tiny  # add by wushanzhou in 2026/10/23
            c = 1.0 + term / c  # add by wushanzhou in 2026/10/23
            if abs(c) < tiny:  # add by wushanzhou in 2026/10/23
                c = tiny  # add by wushanzhou in 2026/10/23
            d = 1.0 / d  # add by wushanzhou in 2026/10/23
            change = d * c  # add by wushanzhou in 2026/10/23
            result *= change  # add by wushanzhou in 2026/10/23
        if abs(change - 1.0) < eps:  # add by wushanzhou in 2026/10/23
            return result  # add by wushanzhou in 2026/10/23
    raise ArithmeticError("incomplete beta did not converge")  # add by wushanzhou in 2026/10/23


def student_t_two_sided(t, df):  # add by wushanzhou in 2026/10/23
    if df <= 0:  # add by wushanzhou in 2026/10/23
        raise ValueError("positive degrees of freedom required")  # add by wushanzhou in 2026/10/23
    if math.isinf(t):  # add by wushanzhou in 2026/10/23
        return 0.0  # add by wushanzhou in 2026/10/23
    x = df / (df + t*t)  # add by wushanzhou in 2026/10/23
    if x >= 1.0:  # add by wushanzhou in 2026/10/23
        return 1.0  # add by wushanzhou in 2026/10/23
    a, b = df / 2.0, 0.5  # add by wushanzhou in 2026/10/23
    factor = math.exp(math.lgamma(a+b)-math.lgamma(a)-math.lgamma(b)  # add by wushanzhou in 2026/10/23
                      + a*math.log(x) + b*math.log1p(-x))  # add by wushanzhou in 2026/10/23
    if x < (a+1)/(a+b+2):  # add by wushanzhou in 2026/10/23
        result = factor * beta_fraction(a,b,x) / a  # add by wushanzhou in 2026/10/23
    else:  # add by wushanzhou in 2026/10/23
        result = 1 - factor * beta_fraction(b,a,1-x) / b  # add by wushanzhou in 2026/10/23
    return min(1.0, max(0.0, result))  # add by wushanzhou in 2026/10/23


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
        return (0.0, 1.0) if mean == 0 else (math.copysign(float("inf"), mean), 0.0)  # add by wushanzhou in 2026/10/23
    t = mean / math.sqrt(var / n)
    p = student_t_two_sided(t, n - 1)  # add by wushanzhou in 2026/10/23
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
                    default=os.path.join("results", "revised_20261023", "ablation_multiseed",  # add by wushanzhou in 2026/10/23
                                         "ablation_multiseed_raw.csv"))
    ap.add_argument("--out", dest="out",
                    default=os.path.join("results", "revised_20261023", "ablation_multiseed",  # add by wushanzhou in 2026/10/23
                                         "ablation_stats.csv"))
    args = ap.parse_args()

    if not os.path.exists(args.inp):
        sys.exit("ERROR: input not found: %s (run --ablation-multiseed first)" % args.inp)

    rows = load(args.inp)
    if not rows or any(r.get("protocol") != "classic_psf_v2" for r in rows):  # add by wushanzhou in 2026/10/23
        sys.exit("ERROR: only classic_psf_v2 rows are comparable; legacy mixed-objective data must be rerun")  # add by wushanzhou in 2026/10/23
    if len({(r["rate"], r["seed"]) for r in rows}) != len(rows):  # add by wushanzhou in 2026/10/23
        sys.exit("ERROR: duplicate rate/seed pairs")  # add by wushanzhou in 2026/10/23
    if any(r["budget_cap"] != "200" or r["sa_prefix_equal"] != "1" for r in rows):  # add by wushanzhou in 2026/10/23
        sys.exit("ERROR: inconsistent budget or SA prefix")  # add by wushanzhou in 2026/10/23
    by_rate = defaultdict(list)
    for r in rows:
        by_rate[r["rate"]].append(r)

    comparisons = [
        ("SA+Local", "mu_sa_local", "Restart", "mu_restart_best"),
        ("SA+Local", "mu_sa_local", "SA-only", "mu_sa_only"),
        ("SA+Local", "mu_sa_local", "Local-only", "mu_local_only"),
        ("SA+Local", "mu_sa_local", "JitterInit", "mu_jitter_init"),
    ]  # add by wushanzhou in 2026/10/23

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
                "p_wilcoxon": "%.9g" % p,  # add by wushanzhou in 2026/10/23
                "p_ttest": "%.9g" % pt,  # add by wushanzhou in 2026/10/23
                "cliffs_delta": "%.4f" % delta,
                "n_effective": n,
                "paired_rank_biserial": "%.6f" % ((wp-wm)/(wp+wm) if wp+wm else 0.0),  # add by wushanzhou in 2026/10/23
            })

    adj = holm(pvals) if pvals else []
    for i, r in enumerate(out_rows):
        r["p_holm"] = "%.9g" % adj[i]  # add by wushanzhou in 2026/10/23
        r["significant_0.05"] = "YES" if adj[i] < 0.05 else "NO"

    if not out_rows:
        sys.exit("ERROR: no comparisons produced")

    keys = ["rate", "n", "A", "B", "mean_A", "mean_B", "mean_diff", "dB_A_over_B",
            "W_plus", "W_minus", "p_wilcoxon", "p_ttest", "p_holm",
            "significant_0.05", "cliffs_delta", "paired_rank_biserial", "n_effective"]  # add by wushanzhou in 2026/10/23
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
    print("\n[Method note] Wilcoxon signed-rank (exact conditional sign distribution for n<=60, ties included) + Holm-Bonferroni;")  # add by wushanzhou in 2026/10/23
    print("              paired t-test given as reference only (normality assumed).")


if __name__ == "__main__":
    main()
