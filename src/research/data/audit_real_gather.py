"""Audit the single real source and run fixed, descriptive CDP holdout checks."""  # add by wushanzhou in 2026/10/23
import argparse  # add by wushanzhou in 2026/10/23
import csv  # add by wushanzhou in 2026/10/23
import json  # add by wushanzhou in 2026/10/23
import os  # add by wushanzhou in 2026/10/23
from pathlib import Path  # add by wushanzhou in 2026/10/23
import subprocess  # add by wushanzhou in 2026/10/23

import numpy as np  # add by wushanzhou in 2026/10/23
from research.data.convert_rraw_to_engine_csv import read_segy  # add by wushanzhou in 2026/10/23
from research.data.make_59x256 import convert, write_csv  # add by wushanzhou in 2026/10/23

from research.common.paths import ROOT, WORKSPACE  # add by wushanzhou in 2026/10/23


def describe(data, offsets, cdps):  # add by wushanzhou in 2026/10/23
    order = np.argsort(offsets)  # add by wushanzhou in 2026/10/23
    wave = data[:, order]  # add by wushanzhou in 2026/10/23
    centered = wave-wave.mean(axis=0)  # add by wushanzhou in 2026/10/23
    norm = np.sqrt(np.sum(centered*centered, axis=0))  # add by wushanzhou in 2026/10/23
    correlations = np.sum(centered[:, :-1]*centered[:, 1:], axis=0)/np.maximum(norm[:-1]*norm[1:], 1e-30)  # add by wushanzhou in 2026/10/23
    return {"traces": int(len(offsets)), "samples": int(data.shape[0]),  # add by wushanzhou in 2026/10/23
            "finite": bool(np.isfinite(data).all()), "zero_traces": int(np.sum(norm == 0)),  # add by wushanzhou in 2026/10/23
            "offset_min_m": int(offsets.min()), "offset_max_m": int(offsets.max()),  # add by wushanzhou in 2026/10/23
            "unique_offsets": int(len(np.unique(offsets))),  # add by wushanzhou in 2026/10/23
            "gap_min_m": int(np.diff(np.sort(offsets)).min()), "gap_max_m": int(np.diff(np.sort(offsets)).max()),  # add by wushanzhou in 2026/10/23
            "trace_rms_min": float(np.sqrt(np.mean(data*data, axis=0)).min()),  # add by wushanzhou in 2026/10/23
            "trace_rms_max": float(np.sqrt(np.mean(data*data, axis=0)).max()),  # add by wushanzhou in 2026/10/23
            "adjacent_zero_lag_correlation_mean": float(correlations.mean()),  # add by wushanzhou in 2026/10/23
            "adjacent_zero_lag_correlation_min": float(correlations.min()),  # add by wushanzhou in 2026/10/23
            "cdp_counts": {str(int(k)): int(np.sum(cdps == k)) for k in np.unique(cdps)}}  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


def audit(source, output, executable=None):  # add by wushanzhou in 2026/10/23
    data, offsets, cdps, meta = read_segy(source)  # add by wushanzhou in 2026/10/23
    output.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/23
    result = {"source_sha256": meta["sha256"], "independent_real_sources": 1,  # add by wushanzhou in 2026/10/23
              "declared_format": meta["declared_format"], "decoded_payload": "IEEE little endian float32",  # add by wushanzhou in 2026/10/23
              "source_dt_ms": 8., "holdout_window_seconds": [0, 1.020],  # add by wushanzhou in 2026/10/23
              "all_traces": describe(data, offsets, cdps), "groups": []}  # add by wushanzhou in 2026/10/23
    groups = [("MixedCDP", np.ones(len(offsets), dtype=bool))]  # add by wushanzhou in 2026/10/23
    groups += [(f"CDP_{int(cdp)}", cdps == cdp) for cdp in np.unique(cdps)]  # add by wushanzhou in 2026/10/23
    for name, selection in groups:  # add by wushanzhou in 2026/10/23
        info = {"name": name, **describe(data[:, selection], offsets[selection], cdps[selection])}  # add by wushanzhou in 2026/10/23
        if executable is not None and selection.sum() >= 4:  # add by wushanzhou in 2026/10/23
            mat, x = convert(data[:, selection], offsets[selection])  # add by wushanzhou in 2026/10/23
            path = output / (name+".csv")  # add by wushanzhou in 2026/10/23
            write_csv(str(path), mat, meta, x)  # add by wushanzhou in 2026/10/23
            keep = max(2, int(np.floor(2*selection.sum()/3+.5)))  # add by wushanzhou in 2026/10/23
            target = output / name  # add by wushanzhou in 2026/10/23
            command = [str(executable), "--holdout-validate", str(path), "--traces", str(int(selection.sum())),  # add by wushanzhou in 2026/10/23
                       "--keep", str(keep), "--seed", "20261023", "--iter", "40", "--out", str(target)]  # add by wushanzhou in 2026/10/23
            with (output / (name+".log")).open("wb") as log:  # add by wushanzhou in 2026/10/23
                run = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT,  # add by wushanzhou in 2026/10/23
                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)  # add by wushanzhou in 2026/10/23
            if run.returncode:  # add by wushanzhou in 2026/10/23
                raise RuntimeError(f"CDP diagnostic failed: {name}; inspect {output / (name+'.log')}")  # add by wushanzhou in 2026/10/23
            with (target / "holdout_metrics.csv").open(encoding="utf-8-sig") as f:  # add by wushanzhou in 2026/10/23
                metrics = {r["Metric_Name"]: r["Value"] for r in csv.DictReader(line for line in f if not line.startswith("#"))}  # add by wushanzhou in 2026/10/23
            info["holdout_metrics"] = metrics  # add by wushanzhou in 2026/10/23
            info["command"] = command  # add by wushanzhou in 2026/10/23
        result["groups"].append(info)  # add by wushanzhou in 2026/10/23
    (output / "audit.json").write_text(json.dumps(result, indent=2)+"\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
    lines = ["# Single-source real-data diagnostic", "",  # add by wushanzhou in 2026/10/23
             "One real source. CDP subsets are dependent descriptive checks; they do not add independent surveys or establish a cause of reconstruction error.", "",  # add by wushanzhou in 2026/10/23
             "All checks use the fixed seed 20261023, 40 iterations, about two thirds kept, and the same 0-1.020 s window. No parameters were selected using these SNR values.", "",  # add by wushanzhou in 2026/10/23
             "|Group|Traces|CDP counts|Mean adjacent zero-lag correlation|", "|---|---:|---|---:|"]  # add by wushanzhou in 2026/10/23
    for g in result["groups"]:  # add by wushanzhou in 2026/10/23
        lines.append(f"|{g['name']}|{g['traces']}|{g['cdp_counts']}|{g['adjacent_zero_lag_correlation_mean']:.4f}|")  # add by wushanzhou in 2026/10/23
    lines += ["", "|Group|Keep|Linear SNR dB|POCS SNR dB|Gain vs linear dB|", "|---|---:|---:|---:|---:|"]  # add by wushanzhou in 2026/10/23
    for g in result["groups"]:  # add by wushanzhou in 2026/10/23
        if "holdout_metrics" in g:  # add by wushanzhou in 2026/10/23
            met = g["holdout_metrics"]  # add by wushanzhou in 2026/10/23
            lines.append(f"|{g['name']}|{met['Traces_Kept']}|{met['SNR_Linear_HeldOut']}|{met['SNR_POCS_HeldOut']}|{met['Gain_POCS_vs_Linear']}|")  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23
    (output / "AUDIT.md").write_text("\n".join(lines)+"\n", encoding="utf-8")  # add by wushanzhou in 2026/10/23
    return result  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23


if __name__ == "__main__":  # add by wushanzhou in 2026/10/23
    parser = argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/23
    parser.add_argument("--input", type=Path, default=WORKSPACE / "data/real_seismic/Data/RRAW.SGY")  # add by wushanzhou in 2026/10/23
    parser.add_argument("--out", type=Path, required=True)  # add by wushanzhou in 2026/10/23
    parser.add_argument("--exe", type=Path)  # add by wushanzhou in 2026/10/23
    args = parser.parse_args()  # add by wushanzhou in 2026/10/23
    audit(args.input.resolve(), args.out.resolve(), args.exe.resolve() if args.exe else None)  # add by wushanzhou in 2026/10/23
    print("Real-data diagnostic complete:", args.out)  # add by wushanzhou in 2026/10/23
# add by wushanzhou in 2026/10/23
