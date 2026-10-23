"""Reproduce the corrected protocol, preserving legacy experiment directories.

Run from any directory after build_release.bat. --quick is a smoke run in a
separate output tree; --resume reuses successful steps only for the same binary.
"""  # add by wushanzhou in 2026/10/23
import argparse  # add by wushanzhou in 2026/10/23
import csv  # add by wushanzhou in 2026/10/23
import hashlib  # add by wushanzhou in 2026/10/23
import json  # add by wushanzhou in 2026/10/23
import os  # add by wushanzhou in 2026/10/23
from pathlib import Path  # add by wushanzhou in 2026/10/23
import statistics  # add by wushanzhou in 2026/10/23
import subprocess  # add by wushanzhou in 2026/10/23
import sys  # add by wushanzhou in 2026/10/23
import time  # add by wushanzhou in 2026/10/23

import numpy as np  # add by wushanzhou in 2026/10/23
import ablation_stats as stats  # add by wushanzhou in 2026/10/23

ROOT = Path(__file__).resolve().parents[1]  # add by wushanzhou in 2026/10/23
EXE = ROOT / "x64/Release/IrrGeometryDesign.exe"  # add by wushanzhou in 2026/10/23
PROTOCOL = "physical_psf_v3"  # add by wushanzhou in 2026/10/23


def cache_fingerprint(state):  # add by wushanzhou in 2026/10/23
    keys = ("protocol", "binary_sha256", "runtime_versions", "tools_sha256", "source_sha256", "input_sha256")  # add by wushanzhou in 2026/10/23
    return hashlib.sha256(json.dumps({k: state[k] for k in keys}, sort_keys=True).encode()).hexdigest()  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


def reusable_step(previous, command, output):  # add by wushanzhou in 2026/10/23
    artifacts = previous.get("artifacts", {})  # add by wushanzhou in 2026/10/23
    return (previous.get("returncode") == 0 and previous.get("command") == command and bool(artifacts)  # add by wushanzhou in 2026/10/23
            and all((output / p).is_file() and sha256(output / p) == digest for p, digest in artifacts.items()))  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


def artifact_hashes(output, paths):  # add by wushanzhou in 2026/10/23
    excluded = {"recon_original_gather.csv", "recon_subsampled_gather.csv", "recon_recovered_gather.csv", "recon_residual_slice.csv"}  # add by wushanzhou in 2026/10/23
    files = {p for path in paths for p in (path.rglob("*") if path.is_dir() else [path])  # add by wushanzhou in 2026/10/23
             if p.is_file() and p.suffix in (".csv", ".txt", ".json") and p.name not in excluded}  # add by wushanzhou in 2026/10/23
    return {str(p.relative_to(output)): sha256(p) for p in sorted(files)}  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


def sha256(path):  # add by wushanzhou in 2026/10/23
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()  # add by wushanzhou in 2026/10/23


def prepare_models(directory):  # add by wushanzhou in 2026/10/23
    directory.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/23
    t = np.arange(256)[:, None] * .004  # add by wushanzhou in 2026/10/23
    x = (np.arange(128)[None, :] - 64) * 12.5  # add by wushanzhou in 2026/10/23
    cases = {  # add by wushanzhou in 2026/10/23
        "dipping": {"seed": 3101, "noise_std": .015,  # add by wushanzhou in 2026/10/23
                    "events": [{"t0": .25, "dip": .00010, "amplitude": 1.0},  # add by wushanzhou in 2026/10/23
                               {"t0": .48, "dip": -.00013, "amplitude": .75},  # add by wushanzhou in 2026/10/23
                               {"t0": .72, "dip": .00006, "amplitude": .55}]},  # add by wushanzhou in 2026/10/23
        "faulted": {"seed": 3102, "noise_std": .015,  # add by wushanzhou in 2026/10/23
                    "events": [{"t0": .22, "velocity": 1900, "fault_shift": .035, "amplitude": 1.0},  # add by wushanzhou in 2026/10/23
                               {"t0": .45, "velocity": 2600, "fault_shift": .060, "amplitude": .8},  # add by wushanzhou in 2026/10/23
                               {"t0": .67, "velocity": 3300, "fault_shift": .045, "amplitude": .6}]},  # add by wushanzhou in 2026/10/23
    }  # add by wushanzhou in 2026/10/23
    paths = []  # add by wushanzhou in 2026/10/23
    for name, parameters in cases.items():  # add by wushanzhou in 2026/10/23
        gather = np.zeros((256, 128))  # add by wushanzhou in 2026/10/23
        for event in parameters["events"]:  # add by wushanzhou in 2026/10/23
            if "dip" in event:  # add by wushanzhou in 2026/10/23
                arrival = event["t0"] + event["dip"] * x  # add by wushanzhou in 2026/10/23
            else:  # add by wushanzhou in 2026/10/23
                arrival = np.sqrt(event["t0"] ** 2 + (x / event["velocity"]) ** 2)  # add by wushanzhou in 2026/10/23
                arrival += (x > 0) * event["fault_shift"]  # add by wushanzhou in 2026/10/23
            arg = (np.pi * 25 * (t - arrival)) ** 2  # add by wushanzhou in 2026/10/23
            gather += event["amplitude"] * (1 - 2 * arg) * np.exp(-arg)  # add by wushanzhou in 2026/10/23
        gather += np.random.default_rng(parameters["seed"]).normal(0, parameters["noise_std"], gather.shape)  # add by wushanzhou in 2026/10/23
        path = directory / (name + ".csv")  # add by wushanzhou in 2026/10/23
        with path.open("w", encoding="utf-8", newline="\n") as f:  # add by wushanzhou in 2026/10/23
            f.write(f"# synthetic_model={name}; Nt=256; Nx=128; dt_s=0.004; dx_m=12.5; wavelet_hz=25\n")  # add by wushanzhou in 2026/10/23
            np.savetxt(f, gather, delimiter=",", fmt="%.9g")  # add by wushanzhou in 2026/10/23
        parameters.update({"shape": [256, 128], "dt_s": .004, "dx_m": 12.5,  # add by wushanzhou in 2026/10/23
                           "wavelet_hz": 25, "csv_sha256": sha256(path)})  # add by wushanzhou in 2026/10/23
        (directory / (name + ".json")).write_text(json.dumps(parameters, indent=2) + "\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
        paths.append(path)  # add by wushanzhou in 2026/10/23
    return paths  # add by wushanzhou in 2026/10/23


def read_rows(path):  # add by wushanzhou in 2026/10/23
    with Path(path).open(encoding="utf-8-sig") as f:  # add by wushanzhou in 2026/10/23
        return list(csv.DictReader(line for line in f if not line.startswith("#")))  # add by wushanzhou in 2026/10/23


def average_ranks(values):  # add by wushanzhou in 2026/10/23
    _, inverse, counts = np.unique(values, return_inverse=True, return_counts=True)  # add by wushanzhou in 2026/10/23
    return (np.cumsum(counts) - (counts - 1) / 2)[inverse]  # add by wushanzhou in 2026/10/23


def summarize(output):  # add by wushanzhou in 2026/10/23
    summary = {"protocol": PROTOCOL, "independent_real_sources": 1,  # add by wushanzhou in 2026/10/23
               "synthetic": {}, "synthetic_correlations": {}, "real": []}  # add by wushanzhou in 2026/10/23
    for path in sorted(output.glob("mask_recon_*/mask_recon_study.csv")):  # add by wushanzhou in 2026/10/23
        groups = {}  # add by wushanzhou in 2026/10/23
        source_rows = read_rows(path)  # add by wushanzhou in 2026/10/23
        for row in source_rows:  # add by wushanzhou in 2026/10/23
            groups.setdefault((row["rate"], row["mask_type"]), []).append(row)  # add by wushanzhou in 2026/10/23
        summary["synthetic"][path.parent.name] = [  # add by wushanzhou in 2026/10/23
            {"rate": rate, "type": typ, "n": len(group), "unique_masks": len({r["mask_sha1"] for r in group}),  # add by wushanzhou in 2026/10/23
             "snr_missing_mean": statistics.mean(float(r["snr_pocs_missing"]) for r in group),  # add by wushanzhou in 2026/10/23
             "snr_missing_sd": statistics.stdev(float(r["snr_pocs_missing"]) for r in group) if len(group) > 1 else 0}  # add by wushanzhou in 2026/10/23
            for (rate, typ), group in sorted(groups.items())]  # add by wushanzhou in 2026/10/23
        correlations = []  # add by wushanzhou in 2026/10/23
        for rate in sorted({r["rate"] for r in source_rows}):  # add by wushanzhou in 2026/10/23
            unique = {r["mask_sha1"]: r for r in source_rows if r["rate"] == rate}  # add by wushanzhou in 2026/10/23
            mu = np.array([float(r["mu"]) for r in unique.values()])  # add by wushanzhou in 2026/10/23
            snr = np.array([float(r["snr_pocs_missing"]) for r in unique.values()])  # add by wushanzhou in 2026/10/23
            correlations.append({"rate": rate, "unique_masks": len(unique),  # add by wushanzhou in 2026/10/23
                                 "pearson": float(np.corrcoef(mu, snr)[0, 1]),  # add by wushanzhou in 2026/10/23
                                 "spearman": float(np.corrcoef(average_ranks(mu), average_ranks(snr))[0, 1])})  # add by wushanzhou in 2026/10/23
        summary["synthetic_correlations"][path.parent.name] = correlations  # add by wushanzhou in 2026/10/23
    real = output / "real_mask_study/real_mask_study.csv"  # add by wushanzhou in 2026/10/23
    if real.exists():  # add by wushanzhou in 2026/10/23
        groups = {}  # add by wushanzhou in 2026/10/23
        for row in read_rows(real):  # add by wushanzhou in 2026/10/23
            groups.setdefault((row["keep"], row["mask_type"]), []).append(row)  # add by wushanzhou in 2026/10/23
        for (keep, typ), rows in sorted(groups.items()):  # add by wushanzhou in 2026/10/23
            summary["real"].append({"keep": int(keep), "type": typ, "n": len(rows),  # add by wushanzhou in 2026/10/23
                "unique_masks": len({r["mask_sha1"] for r in rows}),  # add by wushanzhou in 2026/10/23
                "snr_pocs_mean": statistics.mean(float(r["snr_pocs_heldout"]) for r in rows),  # add by wushanzhou in 2026/10/23
                "gain_linear_mean": statistics.mean(float(r["gain_vs_linear"]) for r in rows),  # add by wushanzhou in 2026/10/23
                "gain_zero_mean": statistics.mean(float(r["gain_vs_zero"]) for r in rows),  # add by wushanzhou in 2026/10/23
                "consistency_all_passed": all(r["consistency_passed"] == "1" for r in rows)})  # add by wushanzhou in 2026/10/23
    comparisons = []  # add by wushanzhou in 2026/10/23
    for path in sorted(output.glob("mask_recon_*/mask_recon_study.csv")):  # add by wushanzhou in 2026/10/23
        data = read_rows(path)  # add by wushanzhou in 2026/10/23
        for rate in sorted({r["rate"] for r in data}):  # add by wushanzhou in 2026/10/23
            by_type = {}  # add by wushanzhou in 2026/10/23
            for r in data:  # add by wushanzhou in 2026/10/23
                if r["rate"] == rate:  # add by wushanzhou in 2026/10/23
                    by_type.setdefault(r["mask_type"], {})[r["seed"]] = float(r["snr_pocs_missing"])  # add by wushanzhou in 2026/10/23
            for first, second in (("SAOnly", "Hybrid"), ("Constrained", "Hybrid")):  # add by wushanzhou in 2026/10/23
                if first not in by_type or second not in by_type:  # add by wushanzhou in 2026/10/23
                    continue  # add by wushanzhou in 2026/10/23
                seeds = sorted(by_type[first].keys() & by_type[second].keys())  # add by wushanzhou in 2026/10/23
                a, b = ([by_type[name][seed] for seed in seeds] for name in (first, second))  # add by wushanzhou in 2026/10/23
                _, _, pvalue, _ = stats.wilcoxon_signed_rank(a, b)  # add by wushanzhou in 2026/10/23
                comparisons.append({"model": path.parent.name, "rate": rate, "A": first, "B": second,  # add by wushanzhou in 2026/10/23
                    "n": len(seeds), "mean_snr_difference_db": statistics.mean(x-y for x,y in zip(a,b)), "p_wilcoxon": pvalue})  # add by wushanzhou in 2026/10/23
    for comparison, adjusted in zip(comparisons, stats.holm([r["p_wilcoxon"] for r in comparisons])):  # add by wushanzhou in 2026/10/23
        comparison["p_holm"] = adjusted  # add by wushanzhou in 2026/10/23
    summary["exploratory_paired_snr_comparisons"] = comparisons  # add by wushanzhou in 2026/10/23
    summary["paired_snr_correction_family_size"] = len(comparisons)  # add by wushanzhou in 2026/10/23
    summary["two_dimensional"] = []  # add by wushanzhou in 2026/10/23
    two_d = output / "benchmark_2d/sampling_2d_benchmark.csv"  # add by wushanzhou in 2026/10/23
    if two_d.exists():  # add by wushanzhou in 2026/10/23
        groups = {}  # add by wushanzhou in 2026/10/23
        for r in read_rows(two_d):  # add by wushanzhou in 2026/10/23
            groups.setdefault((r["rate"], r["method"]), []).append(r)  # add by wushanzhou in 2026/10/23
        summary["two_dimensional"] = [{"rate": rate, "method": method, "n": len(group),  # add by wushanzhou in 2026/10/23
            "unique_masks": len({r["mask_sha1"] for r in group}),  # add by wushanzhou in 2026/10/23
            "mu_mean": statistics.mean(float(r["true_2d_mu"]) for r in group)}  # add by wushanzhou in 2026/10/23
            for (rate, method), group in sorted(groups.items())]  # add by wushanzhou in 2026/10/23
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23
    lines = ["# Experiment results", "", f"Protocol: {PROTOCOL}. One independent real waveform source.",  # add by wushanzhou in 2026/10/23
             "", "## Synthetic missing-trace SNR", "", "|Model|Rate|Mask|Runs|Unique masks|Mean dB|SD dB|", "|---|---:|---|---:|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for model, rows in summary["synthetic"].items():  # add by wushanzhou in 2026/10/23
        for row in rows:  # add by wushanzhou in 2026/10/23
            lines.append(f"|{model}|{row['rate']}|{row['type']}|{row['n']}|{row['unique_masks']}|{row['snr_missing_mean']:.4f}|{row['snr_missing_sd']:.4f}|")  # add by wushanzhou in 2026/10/23
    lines += ["", "## Within-rate exploratory correlation, duplicate masks removed", "",  # add by wushanzhou in 2026/10/23
              "|Model|Rate|Unique masks|Pearson r|Spearman rho|", "|---|---:|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for model, rows in summary["synthetic_correlations"].items():  # add by wushanzhou in 2026/10/23
        for row in rows:  # add by wushanzhou in 2026/10/23
            lines.append(f"|{model}|{row['rate']}|{row['unique_masks']}|{row['pearson']:.4f}|{row['spearman']:.4f}|")  # add by wushanzhou in 2026/10/23
    lines += ["", "## Measured-trace holdout with actual offsets", "", "|Keep|Mask|n|POCS dB|Gain vs linear dB|Gain vs zero dB|", "|---:|---|---:|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for row in summary["real"]:  # add by wushanzhou in 2026/10/23
        lines.append(f"|{row['keep']}|{row['type']}|{row['n']}|{row['snr_pocs_mean']:.4f}|{row['gain_linear_mean']:.4f}|{row['gain_zero_mean']:.4f}|")  # add by wushanzhou in 2026/10/23
    lines += ["", "## Exploratory paired synthetic SNR comparisons", "",  # add by wushanzhou in 2026/10/23
              "Holm correction covers all model/rate comparisons below; larger SNR is better. These are exploratory comparisons over mask seeds on fixed models.", "",  # add by wushanzhou in 2026/10/23
              "|Model|Rate|A|B|n|Mean A-B dB|Wilcoxon p|Holm p|", "|---|---:|---|---|---:|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for r in comparisons:  # add by wushanzhou in 2026/10/23
        lines.append(f"|{r['model']}|{r['rate']}|{r['A']}|{r['B']}|{r['n']}|{r['mean_snr_difference_db']:.4f}|{r['p_wilcoxon']:.6g}|{r['p_holm']:.6g}|")  # add by wushanzhou in 2026/10/23
    lines += ["", "## Two-dimensional mask benchmark", "", "|Rate|Method|Runs|Unique masks|Mean mu|", "|---:|---|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for r in summary["two_dimensional"]:  # add by wushanzhou in 2026/10/23
        lines.append(f"|{r['rate']}|{r['method']}|{r['n']}|{r['unique_masks']}|{r['mu_mean']:.6f}|")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23
    lines += ["", "## Interpretation limits", "", "- Selection and reconstruction masks are linked by canonical SHA1.",  # add by wushanzhou in 2026/10/23
              "- Methods share a 200-evaluation maximum; local search may stop early. Actual counts and times are reported.",  # add by wushanzhou in 2026/10/23
              "- The constrained method optimizes a different, explicitly reported penalized objective.",  # add by wushanzhou in 2026/10/23
              "- Real selection uses physical coordinates: dictionary frequencies k/L, k=0..N-1, L=N*(x_max-x_min)/(N-1); soft penalties also use physical distances.",  # add by wushanzhou in 2026/10/23
              "- SAOnly and Hybrid are distinct methods. Synthetic studies use the index Fourier objective; real studies use the physical-coordinate objective.",  # add by wushanzhou in 2026/10/23
              "- Regular masks are deterministic: repeated seeds are not independent observations for that baseline.",  # add by wushanzhou in 2026/10/23
              "- The raw real file contains five CDP values; it is not a single clean CMP gather.",  # add by wushanzhou in 2026/10/23
              "- Synthetic models and different random splits do not add independent real surveys.", ""]  # add by wushanzhou in 2026/10/23
    (output / "RESULTS.md").write_text("\n".join(lines), encoding="utf-8")  # add by wushanzhou in 2026/10/23


def main():  # add by wushanzhou in 2026/10/23
    parser = argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/23
    parser.add_argument("--quick", action="store_true")  # add by wushanzhou in 2026/10/23
    parser.add_argument("--resume", action="store_true")  # add by wushanzhou in 2026/10/23
    parser.add_argument("--output", type=Path, help="New experiment output directory")  # add by wushanzhou in 2026/10/23
    args = parser.parse_args()  # add by wushanzhou in 2026/10/23
    os.chdir(ROOT)  # add by wushanzhou in 2026/10/23
    output = args.output.resolve() if args.output else ROOT / "results" / ("enhanced_20261023_smoke" if args.quick else "enhanced_20261023")  # add by wushanzhou in 2026/10/23
    output.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/23
    models = prepare_models(ROOT / "data/synthetic_models")  # add by wushanzhou in 2026/10/23
    if not EXE.exists():  # add by wushanzhou in 2026/10/23
        parser.error("build the Release executable first")  # add by wushanzhou in 2026/10/23
    env = os.environ.copy()  # add by wushanzhou in 2026/10/23
    env["IRR_FIGURES_DIR"] = str(ROOT / "figures" / output.name)  # add by wushanzhou in 2026/10/23
    qmake = __import__("shutil").which("qmake.exe")  # add by wushanzhou in 2026/10/23
    if qmake:  # add by wushanzhou in 2026/10/23
        env["PATH"] = str(Path(qmake).parent) + os.pathsep + env.get("PATH", "")  # add by wushanzhou in 2026/10/23
    state_file = output / "run_manifest.json"  # add by wushanzhou in 2026/10/23
    previous = json.loads(state_file.read_text()) if state_file.exists() and args.resume else {}  # add by wushanzhou in 2026/10/23
    binary_hash = sha256(EXE)  # add by wushanzhou in 2026/10/23
    source_paths = [p for p in (ROOT / "IrrGeometryDesign").rglob("*")  # add by wushanzhou in 2026/10/23
                    if p.suffix in (".cpp", ".h", ".inc") and  # add by wushanzhou in 2026/10/23
                    not {"GeneratedFiles", "x64", "Debug", "Release"}.intersection(p.parts)]  # add by wushanzhou in 2026/10/23
    state = {"protocol": PROTOCOL, "binary_sha256": binary_hash,  # add by wushanzhou in 2026/10/23
             "runtime_versions": {"python": sys.version, "numpy": np.__version__},  # add by wushanzhou in 2026/10/23
             "tools_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in sorted((ROOT / "tools").glob("*.py"))},  # add by wushanzhou in 2026/10/23
             "source_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in sorted(source_paths)},  # add by wushanzhou in 2026/10/23
             "input_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in [  # add by wushanzhou in 2026/10/23
                 ROOT / "data/real_seismic/Data/RRAW.SGY", ROOT / "data/real_seismic/real_gather_59x256.csv",  # add by wushanzhou in 2026/10/23
                 ROOT / "data/real_seismic/real_gather_256x128.csv", *models,  # add by wushanzhou in 2026/10/23
                 *(ROOT / "data/sps").glob("5151-5171.*")]}, "steps": []}  # add by wushanzhou in 2026/10/23
    state["cache_fingerprint"] = cache_fingerprint(state)  # add by wushanzhou in 2026/10/23
    old_steps = {s["name"]: s for s in previous.get("steps", [])} if previous.get("cache_fingerprint") == state["cache_fingerprint"] else {}  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23
    seeds, masks, real_seeds = (3, 1, 1) if args.quick else (20, 10, 10)  # add by wushanzhou in 2026/10/23
    def native(*command):  # add by wushanzhou in 2026/10/23
        return [str(EXE), *map(str, command)]  # add by wushanzhou in 2026/10/23
    steps = [  # add by wushanzhou in 2026/10/23
        ("sampling_suite", native("--run-experiments", output / "synthetic", "--repeat", 3 if args.quick else 10, "--seed", 42)),  # add by wushanzhou in 2026/10/23
        ("ablation", native("--ablation-multiseed", output / "ablation_multiseed", "--seeds", seeds, "--seed", 20261023)),  # add by wushanzhou in 2026/10/23
        ("statistics", [sys.executable, "tools/ablation_stats.py", "--in", str(output / "ablation_multiseed/ablation_multiseed_raw.csv"), "--out", str(output / "ablation_multiseed/ablation_stats.csv")]),  # add by wushanzhou in 2026/10/23
        ("constraints", native("--constrained", output / "constrained", "--seeds", 2 if args.quick else 10)),  # add by wushanzhou in 2026/10/23
        ("two_dimensional", native("--sampling-2d", output / "benchmark_2d", "--grid", 16, "--seed", 42, "--seeds", 2 if args.quick else 10)),  # add by wushanzhou in 2026/10/23
        ("mask_builtin", native("--mask-recon", output / "mask_recon_builtin", "--masks", masks, "--seed", 20261023)),  # add by wushanzhou in 2026/10/23
    ]  # add by wushanzhou in 2026/10/23
    for model in models:  # add by wushanzhou in 2026/10/23
        steps.append(("mask_" + model.stem, native("--mask-recon", output / ("mask_recon_" + model.stem), "--masks", masks, "--study-input", model, "--seed", 20261023)))  # add by wushanzhou in 2026/10/23
    steps += [  # add by wushanzhou in 2026/10/23
        ("real_data_audit", [sys.executable, "tools/audit_real_gather.py", "--out", str(output / "real_data_audit"), "--exe", str(EXE)]),  # add by wushanzhou in 2026/10/23
        ("real_reference", native("--run-real-reconstruction", "data/real_seismic/real_gather_256x128.csv", "--out", output / "real_field", "--ratio", .5)),  # add by wushanzhou in 2026/10/23
        ("real_holdout_batch", native("--holdout-batch", "data/real_seismic/real_gather_59x256.csv", "--traces", 59, "--iter", 40, "--out", output / "real_field_holdout")),  # add by wushanzhou in 2026/10/23
        ("real_masks", native("--holdout-study", "data/real_seismic/real_gather_59x256.csv", "--out", output / "real_mask_study", "--seeds", real_seeds, "--seed", 20261023)),  # add by wushanzhou in 2026/10/23
    ]  # add by wushanzhou in 2026/10/23
    step_outputs = {name: [output / directory] for name, directory in {  # add by wushanzhou in 2026/10/23
        "sampling_suite": "synthetic", "ablation": "ablation_multiseed", "statistics": "ablation_multiseed/ablation_stats.csv",  # add by wushanzhou in 2026/10/23
        "constraints": "constrained", "two_dimensional": "benchmark_2d", "mask_builtin": "mask_recon_builtin",  # add by wushanzhou in 2026/10/23
        "mask_dipping": "mask_recon_dipping", "mask_faulted": "mask_recon_faulted", "real_reference": "real_field",  # add by wushanzhou in 2026/10/23
        "real_holdout_batch": "real_field_holdout", "real_masks": "real_mask_study", "real_data_audit": "real_data_audit"}.items()}  # add by wushanzhou in 2026/10/23
    step_outputs["sampling_suite"].append(output / "real_sps")  # add by wushanzhou in 2026/10/23
    for name, command in steps:  # add by wushanzhou in 2026/10/23
        if name in old_steps and reusable_step(old_steps[name], command, output):  # add by wushanzhou in 2026/10/23
            state["steps"].append(old_steps[name]); print("[cached]", name, flush=True); continue  # add by wushanzhou in 2026/10/23
        print("[run]", name, flush=True)  # add by wushanzhou in 2026/10/23
        started = time.time()  # add by wushanzhou in 2026/10/23
        step_env = env.copy()  # add by wushanzhou in 2026/10/23
        if name != "sampling_suite":  # add by wushanzhou in 2026/10/23
            step_env["IRR_FIGURES_DIR"] = str(ROOT / "figures" / output.name / name)  # add by wushanzhou in 2026/10/23
        with (output / (name + ".log")).open("wb") as log:  # add by wushanzhou in 2026/10/23
            result = subprocess.run(command, cwd=ROOT, env=step_env, stdout=log, stderr=subprocess.STDOUT,  # add by wushanzhou in 2026/10/23
                                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)  # add by wushanzhou in 2026/10/23
        state["steps"].append({"name": name, "command": command, "returncode": result.returncode,  # add by wushanzhou in 2026/10/23
                               "elapsed_seconds": round(time.time() - started, 3),  # add by wushanzhou in 2026/10/23
                               "artifacts": artifact_hashes(output, step_outputs[name])})  # add by wushanzhou in 2026/10/23
        # add by wushanzhou in 2026/10/23
        state_file.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
        if result.returncode:  # add by wushanzhou in 2026/10/23
            raise SystemExit(f"failed: {name}; inspect {output / (name + '.log')}")  # add by wushanzhou in 2026/10/23
    summarize(output)  # add by wushanzhou in 2026/10/23
    print("[complete]", output, flush=True)  # add by wushanzhou in 2026/10/23


if __name__ == "__main__":  # add by wushanzhou in 2026/10/23
    main()  # add by wushanzhou in 2026/10/23
