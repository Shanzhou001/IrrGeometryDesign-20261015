"""Independently recompute mask hashes, Fourier PSFs and result linkage with NumPy."""  # add by wushanzhou in 2026/10/23
import argparse  # add by wushanzhou in 2026/10/23
import csv  # add by wushanzhou in 2026/10/23
import hashlib  # add by wushanzhou in 2026/10/23
from pathlib import Path  # add by wushanzhou in 2026/10/23

import numpy as np  # add by wushanzhou in 2026/10/23


def rows(path):  # add by wushanzhou in 2026/10/23
    with Path(path).open(encoding="utf-8-sig") as f:  # add by wushanzhou in 2026/10/23
        return list(csv.DictReader(s for s in f if not s.startswith("#")))  # add by wushanzhou in 2026/10/23


def mask(path):  # add by wushanzhou in 2026/10/23
    with Path(path).open(encoding="utf-8-sig") as f:  # add by wushanzhou in 2026/10/23
        values = [int(s) for s in f if s.strip() and not s.startswith("#")]  # add by wushanzhou in 2026/10/23
    assert values and set(values) <= {0, 1}, path  # add by wushanzhou in 2026/10/23
    bits = np.array(values)  # add by wushanzhou in 2026/10/23
    digest = hashlib.sha1("".join(map(str, values)).encode("ascii")).hexdigest()  # add by wushanzhou in 2026/10/23
    return bits, digest  # add by wushanzhou in 2026/10/23


def metrics(path):  # add by wushanzhou in 2026/10/23
    return {r["Metric_Name"]: r["Value"] for r in rows(path)}  # add by wushanzhou in 2026/10/23


def coherence(bits, shape=None):  # add by wushanzhou in 2026/10/23
    spectrum = np.abs(np.fft.fftn(bits.reshape(shape)) if shape else np.fft.fft(bits))  # add by wushanzhou in 2026/10/23
    spectrum.flat[0] = 0  # add by wushanzhou in 2026/10/23
    return float(spectrum.max() / bits.sum())  # add by wushanzhou in 2026/10/23


def matches_float32_export(value, exported):  # add by wushanzhou in 2026/10/24
    """QTextStream exports these float fields with six significant digits."""  # add by wushanzhou in 2026/10/24
    return float(format(np.float32(value), ".6g")) == float(exported)  # add by wushanzhou in 2026/10/24


def physical_metrics(bits, coordinates):  # add by wushanzhou in 2026/10/23
    x = np.asarray(coordinates, dtype=float)  # add by wushanzhou in 2026/10/23
    assert len(x) == len(bits) and np.isfinite(x).all() and np.all(np.diff(x) > 0)  # add by wushanzhou in 2026/10/23
    kept = x[bits.astype(bool)]  # add by wushanzhou in 2026/10/23
    period = len(x) * (x[-1]-x[0]) / (len(x)-1)  # add by wushanzhou in 2026/10/23
    spectrum = np.exp(-2j*np.pi*np.outer(np.arange(1, len(x)), (kept-x[0])/period)).sum(axis=1)  # add by wushanzhou in 2026/10/23
    gaps = np.diff(kept)  # add by wushanzhou in 2026/10/23
    gap = float(max(kept[0]-x[0], x[-1]-kept[-1], gaps.max(initial=0)))  # add by wushanzhou in 2026/10/23
    nearest = np.minimum(np.r_[gaps[0], gaps], np.r_[gaps, gaps[-1]]) if len(gaps) else np.array([0.])  # add by wushanzhou in 2026/10/23
    cv = float(nearest.std()/nearest.mean()) if nearest.mean() else 0.  # add by wushanzhou in 2026/10/23
    return float(np.abs(spectrum).max()/bits.sum()), gap, cv, period  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


def verify(output):  # add by wushanzhou in 2026/10/23
    checked = 0  # add by wushanzhou in 2026/10/23
    for study in sorted(output.glob("mask_recon_*/mask_recon_study.csv")):  # add by wushanzhou in 2026/10/23
        for r in rows(study):  # add by wushanzhou in 2026/10/23
            tag = f"{r['mask_type']}_r{round(float(r['rate'])*100)}_seed{r['seed']}"  # add by wushanzhou in 2026/10/23
            bits, digest = mask(study.parent / "masks" / (tag + ".txt"))  # add by wushanzhou in 2026/10/23
            assert bits.sum() == int(r["m"])  # add by wushanzhou in 2026/10/23
            assert digest == r["mask_sha1"]  # add by wushanzhou in 2026/10/23
            assert abs(coherence(bits) - float(r["mu"])) < 2e-6  # add by wushanzhou in 2026/10/23
            met = metrics(study.parent / ("run_" + tag) / "recon_metrics_summary.csv")  # add by wushanzhou in 2026/10/23
            assert met["Mask_SHA1"] == digest  # add by wushanzhou in 2026/10/23
            assert abs(float(met["SNR_POCS_MissingOnly"]) - float(r["snr_pocs_missing"])) < .011  # add by wushanzhou in 2026/10/23
            assert int(r["evals"]) <= int(r["budget_cap"])  # add by wushanzhou in 2026/10/23
            if r["mask_type"] in ("SAOnly", "Hybrid"):  # add by wushanzhou in 2026/10/23
                typ = "SA" if r["mask_type"] == "SAOnly" else "Hybrid"  # add by wushanzhou in 2026/10/23
                reference = output / "ablation_multiseed/masks" / f"{typ}_r{round(float(r['rate'])*100)}_seed{r['seed']}.txt"  # add by wushanzhou in 2026/10/23
                if reference.exists():  # add by wushanzhou in 2026/10/23
                    np.testing.assert_array_equal(bits, mask(reference)[0])  # add by wushanzhou in 2026/10/23
                assert int(r["budget_cap"]) == 200  # add by wushanzhou in 2026/10/23
            # add by wushanzhou in 2026/10/23
            checked += 1  # add by wushanzhou in 2026/10/23
    study = output / "real_mask_study/real_mask_study.csv"  # add by wushanzhou in 2026/10/23
    if study.exists():  # add by wushanzhou in 2026/10/23
        source = Path(__file__).resolve().parents[1] / "data/real_seismic/real_gather_59x256.csv"  # add by wushanzhou in 2026/10/23
        coordinates = next(np.fromstring(line.split("=", 1)[1], sep=",") for line in source.read_text().splitlines() if line.startswith("# offsets_m="))  # add by wushanzhou in 2026/10/23
        source_hash = hashlib.sha1(source.read_bytes()).hexdigest()  # add by wushanzhou in 2026/10/23
        # add by wushanzhou in 2026/10/23
        for r in rows(study):  # add by wushanzhou in 2026/10/23
            tag = f"{r['mask_type']}_keep{r['keep']}_seed{r['seed']}"  # add by wushanzhou in 2026/10/23
            bits, digest = mask(study.parent / "masks" / (tag + ".txt"))  # add by wushanzhou in 2026/10/23
            assert digest == r["mask_sha1"] and bits.sum() == int(r["keep"])  # add by wushanzhou in 2026/10/23
            met = metrics(study.parent / ("run_" + tag) / "holdout_metrics.csv")  # add by wushanzhou in 2026/10/23
            assert met["Mask_SHA1"] == digest  # add by wushanzhou in 2026/10/23
            assert float(met["Data_Consistency_Max_Error"]) < 1e-5  # add by wushanzhou in 2026/10/23
            assert r["consistency_passed"] == "1"  # add by wushanzhou in 2026/10/23
            if "mu_physical_psf" in r:  # add by wushanzhou in 2026/10/23
                mu, gap, cv, period = physical_metrics(bits, coordinates)  # add by wushanzhou in 2026/10/23
                weight = .3 if r["mask_type"] == "Constrained" else 0  # add by wushanzhou in 2026/10/23
                objective = mu + weight*(max(0, gap/(period/bits.sum())-1)+cv)  # add by wushanzhou in 2026/10/23
                assert abs(mu-float(r["mu_physical_psf"])) < 2e-6  # add by wushanzhou in 2026/10/23
                assert abs(gap-float(r["max_gap_m"])) < 1e-6  # add by wushanzhou in 2026/10/23
                assert matches_float32_export(cv, r["nn_cv_physical"])  # add by wushanzhou in 2026/10/24
                assert abs(period-float(r["period_m"])) < 1e-8  # add by wushanzhou in 2026/10/23
                assert abs(1/period-float(r["frequency_step_per_m"])) < 1e-12  # add by wushanzhou in 2026/10/23
                assert abs(objective-float(r["objective"])) < 1e-5  # add by wushanzhou in 2026/10/23
                assert r["input_sha1"] == source_hash and met["Input_SHA1"] == source_hash  # add by wushanzhou in 2026/10/23
            else:  # add by wushanzhou in 2026/10/23
                assert abs(coherence(bits) - float(r["mu_index_psf"])) < 2e-6  # add by wushanzhou in 2026/10/23
            # add by wushanzhou in 2026/10/23
            checked += 1  # add by wushanzhou in 2026/10/23
    study = output / "benchmark_2d/sampling_2d_benchmark.csv"  # add by wushanzhou in 2026/10/23
    for r in rows(study):  # add by wushanzhou in 2026/10/23
        suffix = f"_seed{r['seed']}" if "seed" in r else ""  # add by wushanzhou in 2026/10/23
        path = study.parent / "masks" / f"{r['method']}_r{round(float(r['rate'])*100)}{suffix}.txt"  # add by wushanzhou in 2026/10/23
        # add by wushanzhou in 2026/10/23
        bits, digest = mask(path)  # add by wushanzhou in 2026/10/23
        grid = int(r["grid"])  # add by wushanzhou in 2026/10/23
        assert digest == r["mask_sha1"] and bits.sum() == int(r["points"])  # add by wushanzhou in 2026/10/23
        assert abs(coherence(bits, (grid, grid)) - float(r["true_2d_mu"])) < 2e-6  # add by wushanzhou in 2026/10/23
        checked += 1  # add by wushanzhou in 2026/10/23
    study = output / "ablation_multiseed/ablation_multiseed_raw.csv"  # add by wushanzhou in 2026/10/23
    columns = {"SA": "mu_sa_only", "Local": "mu_local_only", "Hybrid": "mu_sa_local",  # add by wushanzhou in 2026/10/23
               "Restart": "mu_restart_best", "Constrained": "mu_constrained", "SAPrefix": "sa_prefix_mu"}  # add by wushanzhou in 2026/10/23
    for r in rows(study):  # add by wushanzhou in 2026/10/23
        assert r["protocol"] == "classic_psf_v2" and r["sa_prefix_equal"] == "1"  # add by wushanzhou in 2026/10/23
        for typ, column in columns.items():  # add by wushanzhou in 2026/10/23
            path = study.parent / "masks" / f"{typ}_r{round(float(r['rate'])*100)}_seed{r['seed']}.txt"  # add by wushanzhou in 2026/10/23
            bits, digest = mask(path)  # add by wushanzhou in 2026/10/23
            assert abs(coherence(bits) - float(r[column])) < 2e-6  # add by wushanzhou in 2026/10/23
            if typ == "SAPrefix":  # add by wushanzhou in 2026/10/23
                assert digest == r["sa_prefix_sha1"]  # add by wushanzhou in 2026/10/23
            checked += 1  # add by wushanzhou in 2026/10/23
    constrained = output / "constrained/constrained_comparison.csv"  # add by wushanzhou in 2026/10/23
    for r in rows(constrained):  # add by wushanzhou in 2026/10/23
        tag = f"{r['scheme']}_r{round(float(r['rate'])*100)}_seed{r['seed']}"  # add by wushanzhou in 2026/10/23
        bits, digest = mask(constrained.parent / "masks" / (tag + ".txt"))  # add by wushanzhou in 2026/10/23
        idx = np.flatnonzero(bits)  # add by wushanzhou in 2026/10/23
        gaps = np.diff(idx)  # add by wushanzhou in 2026/10/23
        gap = max(idx[0], len(bits) - 1 - idx[-1], gaps.max())  # add by wushanzhou in 2026/10/23
        nearest = np.minimum(np.r_[gaps[0], gaps], np.r_[gaps, gaps[-1]])  # add by wushanzhou in 2026/10/23
        cv = float(nearest.std() / nearest.mean())  # add by wushanzhou in 2026/10/23
        weight = .3 if r["scheme"] == "C_soft_l0.3" else 3 if r["scheme"] == "E_soft_l3.0" else 0  # add by wushanzhou in 2026/10/23
        value = coherence(bits)  # add by wushanzhou in 2026/10/23
        objective = value + weight * (max(0, gap / (len(bits) / len(idx)) - 1) + cv)  # add by wushanzhou in 2026/10/23
        assert bits.sum() == int(r["m"]) and digest == r["mask_sha1"]  # add by wushanzhou in 2026/10/23
        assert abs(value - float(r["mu"])) < 2e-6 and gap == int(r["max_gap"])  # add by wushanzhou in 2026/10/23
        assert abs(cv - float(r["nn_cv"])) < 2e-6  # add by wushanzhou in 2026/10/23
        assert abs(objective - float(r["objective"])) < 1e-5  # add by wushanzhou in 2026/10/23
        assert r["hard_ok"] == "1"  # add by wushanzhou in 2026/10/23
        if int(r["hard_max_gap"]):  # add by wushanzhou in 2026/10/23
            assert gap <= int(r["hard_max_gap"])  # add by wushanzhou in 2026/10/23
        assert int(r["evals"]) <= int(r["budget_cap"])  # add by wushanzhou in 2026/10/23
        checked += 1  # add by wushanzhou in 2026/10/23
    for r in rows(output / "constrained/lambda_sensitivity.csv"):  # add by wushanzhou in 2026/10/23
        tag = f"lambda{float(r['lambda']):g}_r{round(float(r['rate'])*100)}_seed{r['seed']}"  # add by wushanzhou in 2026/10/23
        bits, digest = mask(output / "constrained/masks" / (tag + ".txt"))  # add by wushanzhou in 2026/10/23
        assert digest == r["mask_sha1"] and abs(coherence(bits) - float(r["mu"])) < 2e-6  # add by wushanzhou in 2026/10/23
        checked += 1  # add by wushanzhou in 2026/10/23
    for trace in output.rglob("masks/*.trace.csv"):  # add by wushanzhou in 2026/10/23
        values = [float(r["best_objective"]) for r in rows(trace)]  # add by wushanzhou in 2026/10/23
        assert values and all(b <= a + 1e-7 for a, b in zip(values, values[1:])), trace  # add by wushanzhou in 2026/10/23
    selected, _ = mask(output / "synthetic/01_sampling_benchmark/mask_50pct_sa_best.txt")  # add by wushanzhou in 2026/10/23
    used, _ = mask(output / "synthetic/03_wavefield_reconstruction/cmp_mask.txt")  # add by wushanzhou in 2026/10/23
    np.testing.assert_array_equal(selected, used)  # add by wushanzhou in 2026/10/23
    print(f"PASS: {checked} archived rows independently checked; optimized mask linkage verified")  # add by wushanzhou in 2026/10/23
    return checked  # add by wushanzhou in 2026/10/23


if __name__ == "__main__":  # add by wushanzhou in 2026/10/23
    parser = argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/23
    parser.add_argument("output", type=Path, nargs="?", default=Path("results/enhanced_20261023"))  # add by wushanzhou in 2026/10/23
    args = parser.parse_args()  # add by wushanzhou in 2026/10/23
    verify(args.output)  # add by wushanzhou in 2026/10/23
