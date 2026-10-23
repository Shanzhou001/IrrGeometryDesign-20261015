"""Reproduce the corrected protocol, preserving legacy experiment directories.

Run from any directory after build_release.bat. --quick is a smoke run in a
separate output tree; --resume reuses successful steps only for the same binary.
"""
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import statistics
import subprocess
import sys
import time

import numpy as np

ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "x64/Release/IrrGeometryDesign.exe"


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def prepare_models(directory):
    directory.mkdir(parents=True, exist_ok=True)
    t = np.arange(256)[:, None] * .004
    x = (np.arange(128)[None, :] - 64) * 12.5
    cases = {
        "dipping": {"seed": 3101, "noise_std": .015,
                    "events": [{"t0": .25, "dip": .00010, "amplitude": 1.0},
                               {"t0": .48, "dip": -.00013, "amplitude": .75},
                               {"t0": .72, "dip": .00006, "amplitude": .55}]},
        "faulted": {"seed": 3102, "noise_std": .015,
                    "events": [{"t0": .22, "velocity": 1900, "fault_shift": .035, "amplitude": 1.0},
                               {"t0": .45, "velocity": 2600, "fault_shift": .060, "amplitude": .8},
                               {"t0": .67, "velocity": 3300, "fault_shift": .045, "amplitude": .6}]},
    }
    paths = []
    for name, parameters in cases.items():
        gather = np.zeros((256, 128))
        for event in parameters["events"]:
            if "dip" in event:
                arrival = event["t0"] + event["dip"] * x
            else:
                arrival = np.sqrt(event["t0"] ** 2 + (x / event["velocity"]) ** 2)
                arrival += (x > 0) * event["fault_shift"]
            arg = (np.pi * 25 * (t - arrival)) ** 2
            gather += event["amplitude"] * (1 - 2 * arg) * np.exp(-arg)
        gather += np.random.default_rng(parameters["seed"]).normal(0, parameters["noise_std"], gather.shape)
        path = directory / (name + ".csv")
        with path.open("w", encoding="utf-8", newline="\n") as f:
            f.write(f"# synthetic_model={name}; Nt=256; Nx=128; dt_s=0.004; dx_m=12.5; wavelet_hz=25\n")
            np.savetxt(f, gather, delimiter=",", fmt="%.9g")
        parameters.update({"shape": [256, 128], "dt_s": .004, "dx_m": 12.5,
                           "wavelet_hz": 25, "csv_sha256": sha256(path)})
        (directory / (name + ".json")).write_text(json.dumps(parameters, indent=2) + "\n", encoding="utf-8")
        paths.append(path)
    return paths


def read_rows(path):
    with Path(path).open(encoding="utf-8-sig") as f:
        return list(csv.DictReader(line for line in f if not line.startswith("#")))


def average_ranks(values):
    _, inverse, counts = np.unique(values, return_inverse=True, return_counts=True)
    return (np.cumsum(counts) - (counts - 1) / 2)[inverse]


def summarize(output):
    summary = {"protocol": "classic_psf_v2", "independent_real_sources": 1,
               "synthetic": {}, "synthetic_correlations": {}, "real": []}
    for path in sorted(output.glob("mask_recon_*/mask_recon_study.csv")):
        groups = {}
        source_rows = read_rows(path)
        for row in source_rows:
            groups.setdefault((row["rate"], row["mask_type"]), []).append(row)
        summary["synthetic"][path.parent.name] = [
            {"rate": rate, "type": typ, "n": len(group), "unique_masks": len({r["mask_sha1"] for r in group}),
             "snr_missing_mean": statistics.mean(float(r["snr_pocs_missing"]) for r in group),
             "snr_missing_sd": statistics.stdev(float(r["snr_pocs_missing"]) for r in group) if len(group) > 1 else 0}
            for (rate, typ), group in sorted(groups.items())]
        correlations = []
        for rate in sorted({r["rate"] for r in source_rows}):
            unique = {r["mask_sha1"]: r for r in source_rows if r["rate"] == rate}
            mu = np.array([float(r["mu"]) for r in unique.values()])
            snr = np.array([float(r["snr_pocs_missing"]) for r in unique.values()])
            correlations.append({"rate": rate, "unique_masks": len(unique),
                                 "pearson": float(np.corrcoef(mu, snr)[0, 1]),
                                 "spearman": float(np.corrcoef(average_ranks(mu), average_ranks(snr))[0, 1])})
        summary["synthetic_correlations"][path.parent.name] = correlations
    real = output / "real_mask_study/real_mask_study.csv"
    if real.exists():
        groups = {}
        for row in read_rows(real):
            groups.setdefault((row["keep"], row["mask_type"]), []).append(row)
        for (keep, typ), rows in sorted(groups.items()):
            summary["real"].append({"keep": int(keep), "type": typ, "n": len(rows),
                "unique_masks": len({r["mask_sha1"] for r in rows}),
                "snr_pocs_mean": statistics.mean(float(r["snr_pocs_heldout"]) for r in rows),
                "gain_linear_mean": statistics.mean(float(r["gain_vs_linear"]) for r in rows),
                "gain_zero_mean": statistics.mean(float(r["gain_vs_zero"]) for r in rows),
                "consistency_all_passed": all(r["consistency_passed"] == "1" for r in rows)})
    (output / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    lines = ["# Corrected experiment results", "", "Protocol: classic_psf_v2. One independent real waveform source.",
             "", "## Synthetic missing-trace SNR", "", "|Model|Rate|Mask|Runs|Unique masks|Mean dB|SD dB|", "|---|---:|---|---:|---:|---:|---:|"]
    for model, rows in summary["synthetic"].items():
        for row in rows:
            lines.append(f"|{model}|{row['rate']}|{row['type']}|{row['n']}|{row['unique_masks']}|{row['snr_missing_mean']:.4f}|{row['snr_missing_sd']:.4f}|")
    lines += ["", "## Within-rate exploratory correlation, duplicate masks removed", "",
              "|Model|Rate|Unique masks|Pearson r|Spearman rho|", "|---|---:|---:|---:|---:|"]
    for model, rows in summary["synthetic_correlations"].items():
        for row in rows:
            lines.append(f"|{model}|{row['rate']}|{row['unique_masks']}|{row['pearson']:.4f}|{row['spearman']:.4f}|")
    lines += ["", "## Measured-trace holdout with actual offsets", "", "|Keep|Mask|n|POCS dB|Gain vs linear dB|Gain vs zero dB|", "|---:|---|---:|---:|---:|---:|"]
    for row in summary["real"]:
        lines.append(f"|{row['keep']}|{row['type']}|{row['n']}|{row['snr_pocs_mean']:.4f}|{row['gain_linear_mean']:.4f}|{row['gain_zero_mean']:.4f}|")
    lines += ["", "## Interpretation limits", "", "- Selection and reconstruction masks are linked by canonical SHA1.",
              "- Methods share a 200-evaluation maximum; local search may stop early. Actual counts and times are reported.",
              "- The constrained method optimizes a different, explicitly reported penalized objective.",
              "- Real mask selection uses sorted trace indices. Reconstruction and the reported maximum gap use actual offsets.",
              "- Regular masks are deterministic: repeated seeds are not independent observations for that baseline.",
              "- The raw real file contains five CDP values; it is not a single clean CMP gather.",
              "- Synthetic models and different random splits do not add independent real surveys.", ""]
    (output / "RESULTS.md").write_text("\n".join(lines), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--quick", action="store_true")
    parser.add_argument("--resume", action="store_true")
    args = parser.parse_args()
    os.chdir(ROOT)
    output = ROOT / "results" / ("revised_20261023_smoke" if args.quick else "revised_20261023")
    output.mkdir(parents=True, exist_ok=True)
    models = prepare_models(ROOT / "data/synthetic_models")
    if not EXE.exists():
        parser.error("build the Release executable first")
    env = os.environ.copy()
    env["IRR_FIGURES_DIR"] = str(ROOT / "figures" / output.name)
    qmake = __import__("shutil").which("qmake.exe")
    if qmake:
        env["PATH"] = str(Path(qmake).parent) + os.pathsep + env.get("PATH", "")
    state_file = output / "run_manifest.json"
    previous = json.loads(state_file.read_text()) if state_file.exists() and args.resume else {}
    binary_hash = sha256(EXE)
    source_paths = [p for p in (ROOT / "IrrGeometryDesign").rglob("*")
                    if p.suffix in (".cpp", ".h", ".inc") and
                    not {"GeneratedFiles", "x64", "Debug", "Release"}.intersection(p.parts)]
    state = {"protocol": "classic_psf_v2", "binary_sha256": binary_hash,
             "runtime_versions": {"python": sys.version, "numpy": np.__version__},
             "tools_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in sorted((ROOT / "tools").glob("*.py"))},
             "source_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in sorted(source_paths)},
             "input_sha256": {str(p.relative_to(ROOT)): sha256(p) for p in [
                 ROOT / "data/real_seismic/Data/RRAW.SGY", ROOT / "data/real_seismic/real_gather_59x256.csv",
                 ROOT / "data/real_seismic/real_gather_256x128.csv", *models,
                 *(ROOT / "data/sps").glob("5151-5171.*")]}, "steps": []}
    old_steps = {s["name"]: s for s in previous.get("steps", [])} if previous.get("binary_sha256") == binary_hash and previous.get("input_sha256") == state["input_sha256"] else {}
    seeds, masks, real_seeds = (3, 1, 1) if args.quick else (20, 10, 10)
    def native(*command):
        return [str(EXE), *map(str, command)]
    steps = [
        ("sampling_suite", native("--run-experiments", output / "synthetic", "--repeat", 3 if args.quick else 10, "--seed", 42)),
        ("ablation", native("--ablation-multiseed", output / "ablation_multiseed", "--seeds", seeds, "--seed", 20261023)),
        ("statistics", [sys.executable, "tools/ablation_stats.py", "--in", str(output / "ablation_multiseed/ablation_multiseed_raw.csv"), "--out", str(output / "ablation_multiseed/ablation_stats.csv")]),
        ("constraints", native("--constrained", output / "constrained", "--seeds", 2 if args.quick else 10)),
        ("two_dimensional", native("--sampling-2d", output / "benchmark_2d", "--grid", 16, "--seed", 42)),
        ("mask_builtin", native("--mask-recon", output / "mask_recon_builtin", "--masks", masks, "--seed", 20261023)),
    ]
    for model in models:
        steps.append(("mask_" + model.stem, native("--mask-recon", output / ("mask_recon_" + model.stem), "--masks", masks, "--study-input", model, "--seed", 20261023)))
    steps += [
        ("real_reference", native("--run-real-reconstruction", "data/real_seismic/real_gather_256x128.csv", "--out", output / "real_field", "--ratio", .5)),
        ("real_holdout_batch", native("--holdout-batch", "data/real_seismic/real_gather_59x256.csv", "--traces", 59, "--iter", 40, "--out", output / "real_field_holdout")),
        ("real_masks", native("--holdout-study", "data/real_seismic/real_gather_59x256.csv", "--out", output / "real_mask_study", "--seeds", real_seeds, "--seed", 20261023)),
    ]
    for name, command in steps:
        if name in old_steps and old_steps[name]["returncode"] == 0:
            state["steps"].append(old_steps[name]); print("[cached]", name, flush=True); continue
        print("[run]", name, flush=True)
        started = time.time()
        step_env = env.copy()
        if name != "sampling_suite":
            step_env["IRR_FIGURES_DIR"] = str(ROOT / "figures" / output.name / name)
        with (output / (name + ".log")).open("wb") as log:
            result = subprocess.run(command, cwd=ROOT, env=step_env, stdout=log, stderr=subprocess.STDOUT,
                                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)
        state["steps"].append({"name": name, "command": command, "returncode": result.returncode,
                               "elapsed_seconds": round(time.time() - started, 3)})
        state_file.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        if result.returncode:
            raise SystemExit(f"failed: {name}; inspect {output / (name + '.log')}")
    summarize(output)
    print("[complete]", output, flush=True)


if __name__ == "__main__":
    main()
