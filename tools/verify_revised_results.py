"""Independently recompute mask hashes, Fourier PSFs and result linkage with NumPy."""
import argparse
import csv
import hashlib
from pathlib import Path

import numpy as np


def rows(path):
    with Path(path).open(encoding="utf-8-sig") as f:
        return list(csv.DictReader(s for s in f if not s.startswith("#")))


def mask(path):
    with Path(path).open(encoding="utf-8-sig") as f:
        values = [int(s) for s in f if s.strip() and not s.startswith("#")]
    assert values and set(values) <= {0, 1}, path
    bits = np.array(values)
    digest = hashlib.sha1("".join(map(str, values)).encode("ascii")).hexdigest()
    return bits, digest


def metrics(path):
    return {r["Metric_Name"]: r["Value"] for r in rows(path)}


def coherence(bits, shape=None):
    spectrum = np.abs(np.fft.fftn(bits.reshape(shape)) if shape else np.fft.fft(bits))
    spectrum.flat[0] = 0
    return float(spectrum.max() / bits.sum())


def verify(output):
    checked = 0
    for study in sorted(output.glob("mask_recon_*/mask_recon_study.csv")):
        for r in rows(study):
            tag = f"{r['mask_type']}_r{round(float(r['rate'])*100)}_seed{r['seed']}"
            bits, digest = mask(study.parent / "masks" / (tag + ".txt"))
            assert bits.sum() == int(r["m"])
            assert digest == r["mask_sha1"]
            assert abs(coherence(bits) - float(r["mu"])) < 2e-6
            met = metrics(study.parent / ("run_" + tag) / "recon_metrics_summary.csv")
            assert met["Mask_SHA1"] == digest
            assert abs(float(met["SNR_POCS_MissingOnly"]) - float(r["snr_pocs_missing"])) < .011
            assert int(r["evals"]) <= int(r["budget_cap"])
            checked += 1
    study = output / "real_mask_study/real_mask_study.csv"
    if study.exists():
        for r in rows(study):
            tag = f"{r['mask_type']}_keep{r['keep']}_seed{r['seed']}"
            bits, digest = mask(study.parent / "masks" / (tag + ".txt"))
            assert digest == r["mask_sha1"] and bits.sum() == int(r["keep"])
            met = metrics(study.parent / ("run_" + tag) / "holdout_metrics.csv")
            assert met["Mask_SHA1"] == digest
            assert float(met["Data_Consistency_Max_Error"]) < 1e-5
            assert r["consistency_passed"] == "1"
            assert abs(coherence(bits) - float(r["mu_index_psf"])) < 2e-6
            checked += 1
    study = output / "benchmark_2d/sampling_2d_benchmark.csv"
    for r in rows(study):
        path = study.parent / "masks" / f"{r['method']}_r{round(float(r['rate'])*100)}.txt"
        bits, digest = mask(path)
        grid = int(r["grid"])
        assert digest == r["mask_sha1"] and bits.sum() == int(r["points"])
        assert abs(coherence(bits, (grid, grid)) - float(r["true_2d_mu"])) < 2e-6
        checked += 1
    study = output / "ablation_multiseed/ablation_multiseed_raw.csv"
    columns = {"SA": "mu_sa_only", "Local": "mu_local_only", "Hybrid": "mu_sa_local",
               "Restart": "mu_restart_best", "Constrained": "mu_constrained", "SAPrefix": "sa_prefix_mu"}
    for r in rows(study):
        assert r["protocol"] == "classic_psf_v2" and r["sa_prefix_equal"] == "1"
        for typ, column in columns.items():
            path = study.parent / "masks" / f"{typ}_r{round(float(r['rate'])*100)}_seed{r['seed']}.txt"
            bits, digest = mask(path)
            assert abs(coherence(bits) - float(r[column])) < 2e-6
            if typ == "SAPrefix":
                assert digest == r["sa_prefix_sha1"]
            checked += 1
    constrained = output / "constrained/constrained_comparison.csv"
    for r in rows(constrained):
        tag = f"{r['scheme']}_r{round(float(r['rate'])*100)}_seed{r['seed']}"
        bits, digest = mask(constrained.parent / "masks" / (tag + ".txt"))
        idx = np.flatnonzero(bits)
        gaps = np.diff(idx)
        gap = max(idx[0], len(bits) - 1 - idx[-1], gaps.max())
        nearest = np.minimum(np.r_[gaps[0], gaps], np.r_[gaps, gaps[-1]])
        cv = float(nearest.std() / nearest.mean())
        weight = .3 if r["scheme"] == "C_soft_l0.3" else 3 if r["scheme"] == "E_soft_l3.0" else 0
        value = coherence(bits)
        objective = value + weight * (max(0, gap / (len(bits) / len(idx)) - 1) + cv)
        assert bits.sum() == int(r["m"]) and digest == r["mask_sha1"]
        assert abs(value - float(r["mu"])) < 2e-6 and gap == int(r["max_gap"])
        assert abs(cv - float(r["nn_cv"])) < 2e-6
        assert abs(objective - float(r["objective"])) < 1e-5
        assert r["hard_ok"] == "1"
        if int(r["hard_max_gap"]):
            assert gap <= int(r["hard_max_gap"])
        assert int(r["evals"]) <= int(r["budget_cap"])
        checked += 1
    for r in rows(output / "constrained/lambda_sensitivity.csv"):
        tag = f"lambda{float(r['lambda']):g}_r{round(float(r['rate'])*100)}_seed{r['seed']}"
        bits, digest = mask(output / "constrained/masks" / (tag + ".txt"))
        assert digest == r["mask_sha1"] and abs(coherence(bits) - float(r["mu"])) < 2e-6
        checked += 1
    for trace in output.rglob("masks/*.trace.csv"):
        values = [float(r["best_objective"]) for r in rows(trace)]
        assert values and all(b <= a + 1e-7 for a, b in zip(values, values[1:])), trace
    selected, _ = mask(output / "synthetic/01_sampling_benchmark/mask_50pct_sa_best.txt")
    used, _ = mask(output / "synthetic/03_wavefield_reconstruction/cmp_mask.txt")
    np.testing.assert_array_equal(selected, used)
    print(f"PASS: {checked} archived rows independently checked; optimized mask linkage verified")
    return checked


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, nargs="?", default=Path("results/revised_20261023"))
    args = parser.parse_args()
    verify(args.output)
