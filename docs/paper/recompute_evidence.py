"""Independently recompute manuscript statistics from read-only archived CSVs.

Run from any directory; --out must name a new JSON file. No existing research
modules are imported and no archived inputs or historical results are written.
"""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import itertools
import json
from pathlib import Path
import platform
import subprocess
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
V5 = ROOT / "results/audit_20261024_v5"
V4 = ROOT / "results/review_20261024_v4/final"
BOOTSTRAP_SEED = 54127
BOOTSTRAP_REPLICATES = 10000


def sha256(path):
    hasher = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            hasher.update(block)
    return hasher.hexdigest()


def read_csv(path):
    with path.open(encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream))


def interval(values):
    values = np.asarray(values, dtype=float)
    rng = np.random.default_rng(BOOTSTRAP_SEED)
    means = values[rng.integers(len(values), size=(BOOTSTRAP_REPLICATES, len(values)))].mean(axis=1)
    return [float(value) for value in np.quantile(means, [0.025, 0.975])]


def signed_rank(values):
    values = np.asarray(values, dtype=float)
    values = values[np.abs(values) > 1e-12]
    if len(values) == 0:
        return 0.0, 1.0
    magnitudes = np.abs(values)
    ranks = np.array([
        1 + np.count_nonzero(magnitudes < magnitude - 1e-12)
        + (np.count_nonzero(np.abs(magnitudes - magnitude) < 1e-12) - 1) / 2
        for magnitude in magnitudes
    ])
    positive = float(ranks[values > 0].sum())
    negative = float(ranks[values < 0].sum())
    effect = (positive - negative) / (positive + negative)
    # Six-template pairs have only 64 conditional sign assignments.
    if len(values) <= 10:
        observed = min(positive, negative)
        extreme = 0
        for assignment in itertools.product([False, True], repeat=len(values)):
            weight = float(ranks[list(assignment)].sum())
            extreme += min(weight, ranks.sum() - weight) <= observed + 1e-12
        p_value = extreme / (2 ** len(values))
    else:
        p_value = None  # v4 p values are separately identified as archived values.
    return effect, p_value


def holm(values):
    order = sorted(range(len(values)), key=lambda index: values[index])
    adjusted = [0.0] * len(values)
    previous = 0.0
    for rank, index in enumerate(order):
        previous = max(previous, min(1.0, values[index] * (len(values) - rank)))
        adjusted[index] = previous
    return adjusted


def recompute():
    paths = [
        V5 / "joint/joint.csv", V5 / "analysis_thesis/joint_means.csv",
        V5 / "analysis_thesis/joint_pairs.csv", V5 / "real/real.csv",
        V4 / "search_uniform_600/search.csv", V4 / "search_statistics.csv",
        ROOT / "data/real_seismic/Data/RRAW.SGY",
    ]
    before = {path.relative_to(ROOT).as_posix(): sha256(path) for path in paths}
    raw = read_csv(paths[0])
    assert len(raw) == 1080
    pool = {}
    for row in raw:
        if row["design"] != "common_H":
            continue
        for recovery in ["fixed", "selected"]:
            key = (row["m"], recovery, row["method"], row["family"], row["structure"], row["noise"])
            pool.setdefault(key, []).append(float(row[recovery + "_snr"]))
    templates = {}
    for (m, recovery, method, family, structure, noise), values in pool.items():
        assert len(values) == 3
        key = (m, recovery, method, family, structure)
        templates.setdefault(key, []).append(float(np.mean(values)))
    vectors = {}
    for (m, recovery, method, family, structure), values in templates.items():
        assert len(values) == 2
        vectors.setdefault((m, recovery, method), {})[(family, structure)] = float(np.mean(values))
    for vector in vectors.values():
        assert len(vector) == 6

    means = []
    max_mean_error = max_interval_error = max_effect_error = max_p_error = 0.0
    for archived in read_csv(paths[1]):
        vector = vectors[(archived["m"], archived["recovery"], archived["method"])]
        values = [vector[key] for key in sorted(vector)]
        mean, bounds = float(np.mean(values)), interval(values)
        max_mean_error = max(max_mean_error, abs(mean - float(archived["mean_snr"])))
        max_interval_error = max(max_interval_error, max(abs(bounds[i] - float(archived[field])) for i, field in enumerate(["ci_low", "ci_high"])))
        means.append(dict(m=int(archived["m"]), recovery=archived["recovery"], method=archived["method"], n=6, mean=mean, ci95=bounds))

    pairs, pair_archives = [], read_csv(paths[2])
    for archived in pair_archives:
        a = vectors[(archived["m"], archived["recovery"], "Adaptive")]
        b = vectors[(archived["m"], archived["recovery"], archived["comparison"][len("Adaptive-"):])]
        differences = [a[key] - b[key] for key in sorted(a)]
        mean, bounds = float(np.mean(differences)), interval(differences)
        effect, p_value = signed_rank(differences)
        max_mean_error = max(max_mean_error, abs(mean - float(archived["difference"])))
        max_interval_error = max(max_interval_error, max(abs(bounds[i] - float(archived[field])) for i, field in enumerate(["ci_low", "ci_high"])))
        max_effect_error = max(max_effect_error, abs(effect - float(archived["rank_biserial"])))
        max_p_error = max(max_p_error, abs(p_value - float(archived["p_raw"])))
        pairs.append(dict(m=int(archived["m"]), recovery=archived["recovery"], comparison=archived["comparison"], n=6,
                          mean_difference=mean, ci95=bounds, rank_biserial=effect, p_raw=p_value))
    for record, adjusted, archived in zip(pairs, holm([record["p_raw"] for record in pairs]), pair_archives):
        record["p_holm"] = adjusted
        max_p_error = max(max_p_error, abs(adjusted - float(archived["p_holm"])))
    assert len(means) == 30 and len(pairs) == 24
    assert max(max_mean_error, max_interval_error, max_effect_error, max_p_error) < 1e-12

    v4_rows, v4_statistics = read_csv(paths[4]), read_csv(paths[5])
    v4 = []
    a = {row["seed"]: float(row["mu"]) for row in v4_rows if row["method"] == "Adaptive" and row["rate_index"] == "0"}
    for method in ["Hybrid", "SAOnly"]:
        b = {row["seed"]: float(row["mu"]) for row in v4_rows if row["method"] == method and row["rate_index"] == "0"}
        assert a.keys() == b.keys() and len(a) == 20
        differences = [a[key] - b[key] for key in sorted(a)]
        effect, _ = signed_rank(differences)
        archived = next(row for row in v4_statistics if row["domain"] == "uniform" and row["budget"] == "600" and row["rate"] == "0" and row["A"] == "Adaptive" and row["B"] == method)
        assert abs(np.mean(differences) - float(archived["mean_difference"])) < 1e-12
        assert abs(effect - float(archived["paired_rank_biserial"])) < 1e-12
        v4.append(dict(domain="uniform", budget=600, m=38, comparison="Adaptive-" + method, n=20,
                       mean_difference=float(np.mean(differences)), ci95=interval(differences), rank_biserial=effect,
                       archived_p_raw=float(archived["p_raw"]), archived_p_holm=float(archived["p_holm"])))

    real = read_csv(paths[3])
    real_counts = dict(records=len(real), fixed_negative=sum(float(row["fixed_snr"]) < 0 for row in real),
                       selected_negative=sum(float(row["selected_snr"]) < 0 for row in real))
    assert real_counts == dict(records=24, fixed_negative=17, selected_negative=1)
    assert before["data/real_seismic/Data/RRAW.SGY"] == "ddcb311b2406af17cf375b570b3eb8ea7ca2d81cdeb002f972e6d668cd7b2bc2"
    after = {path.relative_to(ROOT).as_posix(): sha256(path) for path in paths}
    assert after == before
    return dict(
        status="PASS", utc=dt.datetime.now(dt.timezone.utc).isoformat(),
        git_commit=subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        command=[sys.executable, str(Path(__file__).resolve()), *sys.argv[1:]],
        python=platform.python_version(), numpy=np.__version__, script_sha256=sha256(Path(__file__)),
        input_sha256=before, inputs_unchanged=True,
        bootstrap=dict(seed=BOOTSTRAP_SEED, replicates=BOOTSTRAP_REPLICATES, canonical_template_order="alphabetical family, structure", interpretation="Conditional percentile intervals; not simultaneous or population-generalization intervals."),
        v5_means=means, v5_pairs=pairs,
        maximum_archive_differences=dict(mean=max_mean_error, interval=max_interval_error, rank_biserial=max_effect_error, p_raw_and_holm=max_p_error),
        v4_recomputed_bootstrap=v4, real_counts=real_counts,
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True, help="New JSON evidence file; existing files are refused.")
    arguments = parser.parse_args()
    if arguments.out.exists():
        raise FileExistsError(arguments.out)
    result = recompute()
    arguments.out.parent.mkdir(parents=True, exist_ok=True)
    with arguments.out.open("x", encoding="utf-8") as stream:
        json.dump(result, stream, ensure_ascii=False, indent=2, allow_nan=False)
        stream.write("\n")
    print("PASS:", arguments.out.resolve())
