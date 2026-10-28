"""Prespecified v4 studies. Scientific input archives and v3 results are read only."""  # add by wushanzhou in 2026/10/24
from __future__ import annotations  # add by wushanzhou in 2026/10/24

from research.common.tabular import rows as read_rows, write_rows
from research.common.processes import native_environment
import argparse  # add by wushanzhou in 2026/10/24
import csv  # add by wushanzhou in 2026/10/24
from concurrent.futures import ThreadPoolExecutor  # add by wushanzhou in 2026/10/24
import hashlib  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
import os  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24
import platform  # add by wushanzhou in 2026/10/24
import subprocess  # add by wushanzhou in 2026/10/24
import threading  # add by wushanzhou in 2026/10/24
import time  # add by wushanzhou in 2026/10/24

import numpy as np  # add by wushanzhou in 2026/10/24

from research.data.convert_rraw_to_engine_csv import read_segy  # add by wushanzhou in 2026/10/24
from research.data.make_59x256 import convert as convert_measured  # add by wushanzhou in 2026/10/24

from research.common.provenance import add_real_data_arguments, real_source_provenance, require_inputs
from research.common.paths import ROOT, WORKSPACE, source_files
PROTOCOL = "adaptive_hybrid_v4"  # add by wushanzhou in 2026/10/24
REAL = WORKSPACE / "data/real_seismic/real_gather_59x256.csv"  # add by wushanzhou in 2026/10/24
METHODS = ("Regular", "Random", "Jitter", "SAOnly", "Local", "Hybrid", "Restart", "SA_Budget",  # add by wushanzhou in 2026/10/24
           "Adaptive", "FixedSwitch", "NoRecycle", "AdjacentOnly", "W_03_03", "HardGap")  # add by wushanzhou in 2026/10/24
CONFIGURATIONS = [  # add by wushanzhou in 2026/10/24
    dict(name="legacy", iterations=40, start=.0265, end=.0005, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="cold", iterations=40, start=.0265, end=.0005, scales=4, angles=16, warm=False),  # add by wushanzhou in 2026/10/24
    dict(name="iter20", iterations=20, start=.0265, end=.0005, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="iter80", iterations=80, start=.0265, end=.0005, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="weak", iterations=40, start=.008, end=.0002, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="strong", iterations=40, start=.08, end=.002, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="scale3_angle8", iterations=40, start=.0265, end=.0005, scales=3, angles=8, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="scale3_angle16", iterations=40, start=.0265, end=.0005, scales=3, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="angle8", iterations=40, start=.0265, end=.0005, scales=4, angles=8, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="constant", iterations=40, start=.005, end=.005, scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
    dict(name="no_shrink", iterations=1, start=0., end=0., scales=4, angles=16, warm=True),  # add by wushanzhou in 2026/10/24
]  # add by wushanzhou in 2026/10/24


from research.common.hashing import digest as sha






def read_gather(path):  # add by wushanzhou in 2026/10/24
    x = None  # add by wushanzhou in 2026/10/24
    for line in Path(path).read_text(encoding="utf-8-sig").splitlines():  # add by wushanzhou in 2026/10/24
        if line.startswith("# offsets_m="):  # add by wushanzhou in 2026/10/24
            if x is not None:  # add by wushanzhou in 2026/10/24
                raise ValueError("duplicate geometry")  # add by wushanzhou in 2026/10/24
            x = np.fromstring(line.split("=", 1)[1], sep=",")  # add by wushanzhou in 2026/10/24
    data = np.loadtxt(path, delimiter=",", comments="#")  # add by wushanzhou in 2026/10/24
    if data.ndim != 2 or x is None or len(x) != data.shape[1] or not np.isfinite(data).all() or not np.all(np.diff(x) > 0):  # add by wushanzhou in 2026/10/24
        raise ValueError("invalid research gather")  # add by wushanzhou in 2026/10/24
    return data, x  # add by wushanzhou in 2026/10/24


def write_gather(path, data, x, label):  # add by wushanzhou in 2026/10/24
    path = Path(path)  # add by wushanzhou in 2026/10/24
    path.parent.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/24
    with path.open("w", encoding="utf-8", newline="\n") as stream:  # add by wushanzhou in 2026/10/24
        stream.write(f"# {label}\n# offsets_m=" + ",".join(f"{v:.12g}" for v in x) + "\n")  # add by wushanzhou in 2026/10/24
        np.savetxt(stream, data, delimiter=",", fmt="%.9g")  # add by wushanzhou in 2026/10/24


def read_mask(path):  # add by wushanzhou in 2026/10/24
    values = np.loadtxt(path, comments="#")  # add by wushanzhou in 2026/10/24
    if values.ndim != 1 or not np.isin(values, (0, 1)).all():  # add by wushanzhou in 2026/10/24
        raise ValueError("invalid mask")  # add by wushanzhou in 2026/10/24
    return values.astype(bool)  # add by wushanzhou in 2026/10/24


def write_mask(path, mask):  # add by wushanzhou in 2026/10/24
    path = Path(path)  # add by wushanzhou in 2026/10/24
    path.parent.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/24
    np.savetxt(path, np.asarray(mask, dtype=int), fmt="%d", header=f"protocol={PROTOCOL}")  # add by wushanzhou in 2026/10/24


def mask_hash(mask):  # add by wushanzhou in 2026/10/24
    return hashlib.sha1("".join("1" if v else "0" for v in mask).encode("ascii")).hexdigest()  # add by wushanzhou in 2026/10/24


def snr(truth, prediction, columns):  # add by wushanzhou in 2026/10/24
    target = np.asarray(truth)[:, columns]  # add by wushanzhou in 2026/10/24
    residual = target - np.asarray(prediction)[:, columns]  # add by wushanzhou in 2026/10/24
    energy, error = float(np.sum(target * target)), float(np.sum(residual * residual))  # add by wushanzhou in 2026/10/24
    if energy <= 0:  # add by wushanzhou in 2026/10/24
        raise ValueError("SNR undefined for zero-energy test targets")  # add by wushanzhou in 2026/10/24
    return 10 * np.log10(energy / max(error, 1e-30))  # add by wushanzhou in 2026/10/24


def linear_known(data, x, mask):  # add by wushanzhou in 2026/10/24
    known = np.flatnonzero(mask)  # add by wushanzhou in 2026/10/24
    if len(known) < 2:  # add by wushanzhou in 2026/10/24
        raise ValueError("need at least two training traces")  # add by wushanzhou in 2026/10/24
    return np.stack([np.interp(x, x[known], row[known]) for row in data])  # add by wushanzhou in 2026/10/24


def validation_choice(validation_truth, predictions, max_gain=1.0):  # add by wushanzhou in 2026/10/24
    """Only inner validation values enter calibration and model selection; no outer-test argument."""  # add by wushanzhou in 2026/10/24
    if not np.isfinite(max_gain) or max_gain <= 0:
        raise ValueError("invalid max_gain")
    target = np.asarray(validation_truth, dtype=float).ravel()  # add by wushanzhou in 2026/10/24
    target_energy = float(target @ target)
    records = []  # add by wushanzhou in 2026/10/24
    for name, prediction in predictions:  # add by wushanzhou in 2026/10/24
        values = np.asarray(prediction, dtype=float).ravel()  # add by wushanzhou in 2026/10/24
        if values.shape != target.shape or not np.isfinite(values).all() or not np.isfinite(target).all():  # add by wushanzhou in 2026/10/24
            raise ValueError("invalid validation prediction")  # add by wushanzhou in 2026/10/24
        denominator = float(values @ values)  # add by wushanzhou in 2026/10/24
        dot = float(values @ target) if denominator > 1e-30 else 0.
        unclipped_gain = float(max(0., dot / denominator)) if denominator > 1e-30 else 0.
        phase_cosine = float(dot / np.sqrt(denominator * target_energy)) if denominator > 1e-30 and target_energy > 1e-30 else 0.
        gain = float(np.clip(dot / denominator, 0, max_gain)) if denominator > 1e-30 else 0.  # add by wushanzhou in 2026/10/24
        mse = float(np.mean((target - gain * values) ** 2))  # add by wushanzhou in 2026/10/24
        records.append(dict(candidate=name, gain=gain, validation_mse=mse,  # add by wushanzhou in 2026/10/24
                            raw_validation_mse=float(np.mean((target - values) ** 2)),
                            unclipped_gain=unclipped_gain, phase_cosine=phase_cosine))  # add by wushanzhou in 2026/10/24
    # Input order is the declared complexity tie breaker: zero, linear, then curvelet configurations.  # add by wushanzhou in 2026/10/24
    selected = min(range(len(records)), key=lambda k: (records[k]["validation_mse"], k))  # add by wushanzhou in 2026/10/24
    return records[selected], records  # add by wushanzhou in 2026/10/24


class Recorder:  # add by wushanzhou in 2026/10/24
    def __init__(self, output, executable, configuration, resume):  # add by wushanzhou in 2026/10/24
        self.lock = threading.RLock()  # add by wushanzhou in 2026/10/24
        self.output = Path(output).resolve()  # add by wushanzhou in 2026/10/24
        self.executable = Path(executable).resolve()  # add by wushanzhou in 2026/10/24
        stat=self.executable.stat();self.binary_signature=(stat.st_mtime_ns,stat.st_size)  # add by wushanzhou in 2026/10/24
        self.output.mkdir(parents=True, exist_ok=True)  # add by wushanzhou in 2026/10/24
        (self.output / "logs").mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
        self.path = self.output / "run_manifest.json"  # add by wushanzhou in 2026/10/24
        source_paths = source_files()
        tool_paths = sorted((ROOT/'src/research').rglob('*.py'))
        fingerprint = dict(protocol=PROTOCOL, executable_sha256=sha(self.executable),  # add by wushanzhou in 2026/10/24
                           source_sha256={str(p.relative_to(ROOT)): sha(p) for p in source_paths},  # add by wushanzhou in 2026/10/24
                           tool_sha256={str(p.relative_to(ROOT)): sha(p) for p in tool_paths},  # add by wushanzhou in 2026/10/24
                           real_input_sha256=None if configuration.get("synthetic_only") else (sha(REAL) if REAL.is_file() else None), configuration=configuration,  # add by wushanzhou in 2026/10/24
                           python=platform.python_version(), numpy=np.__version__)  # add by wushanzhou in 2026/10/24
        if self.path.exists():  # add by wushanzhou in 2026/10/24
            self.state = json.loads(self.path.read_text(encoding="utf-8"))  # add by wushanzhou in 2026/10/24
            if not resume or self.state["fingerprint"] != fingerprint:  # add by wushanzhou in 2026/10/24
                raise RuntimeError("Existing manifest differs or --resume is absent; use a new output directory")  # add by wushanzhou in 2026/10/24
        else:  # add by wushanzhou in 2026/10/24
            self.state = dict(fingerprint=fingerprint, started_at=time.strftime("%Y-%m-%dT%H:%M:%S"), steps={})  # add by wushanzhou in 2026/10/24
            self.save()  # add by wushanzhou in 2026/10/24
        self.journal = self.output / "step_journal.jsonl"  # add by wushanzhou in 2026/10/24
        if self.journal.exists():  # add by wushanzhou in 2026/10/24
            for line in self.journal.read_text(encoding="utf-8").splitlines():  # add by wushanzhou in 2026/10/24
                entry = json.loads(line)  # add by wushanzhou in 2026/10/24
                self.state["steps"][entry["key"]] = entry["step"]  # add by wushanzhou in 2026/10/24
        self.calls = 0  # add by wushanzhou in 2026/10/24

    def save(self):  # add by wushanzhou in 2026/10/24
        with self.lock:  # add by wushanzhou in 2026/10/24
            temp = self.path.with_suffix(".tmp")  # add by wushanzhou in 2026/10/24
            temp.write_text(json.dumps(self.state, indent=2, allow_nan=False), encoding="utf-8")  # add by wushanzhou in 2026/10/24
            temp.replace(self.path)  # add by wushanzhou in 2026/10/24

    def call(self, key, arguments, destination):  # add by wushanzhou in 2026/10/24
        stat=self.executable.stat()  # add by wushanzhou in 2026/10/24
        if (stat.st_mtime_ns,stat.st_size)!=self.binary_signature:  # add by wushanzhou in 2026/10/24
            raise RuntimeError("Executable changed during this batch; preserve the batch and rerun in a new directory")  # add by wushanzhou in 2026/10/24
        command = [str(self.executable), *map(str, arguments)]  # add by wushanzhou in 2026/10/24
        inputs = {str(Path(v).resolve()): sha(v) for v in command[1:] if Path(v).is_file()}  # add by wushanzhou in 2026/10/24
        prior = self.state["steps"].get(key)  # add by wushanzhou in 2026/10/24
        if prior and prior["command"] == command and prior["inputs"] == inputs and prior["exit_code"] == 0:  # add by wushanzhou in 2026/10/24
            if all((self.output / p).is_file() and sha(self.output / p) == value for p, value in prior["outputs"].items()):  # add by wushanzhou in 2026/10/24
                return  # add by wushanzhou in 2026/10/24
        if prior or Path(destination).exists():  # add by wushanzhou in 2026/10/24
            raise RuntimeError(f"Unverified existing step {key}; preserve it and use a fresh directory")  # add by wushanzhou in 2026/10/24
        start = time.perf_counter()  # add by wushanzhou in 2026/10/24
        log = self.output / "logs" / (key.replace("/", "_") + ".log")  # add by wushanzhou in 2026/10/24
        with log.open("wb") as stream:  # add by wushanzhou in 2026/10/24
            result = subprocess.run(command, cwd=WORKSPACE, env=native_environment(), stdout=stream, stderr=subprocess.STDOUT,  # add by wushanzhou in 2026/10/24
                                    creationflags=subprocess.CREATE_NO_WINDOW if os.name == "nt" else 0)  # add by wushanzhou in 2026/10/24
        outputs = {str(p.relative_to(self.output)): sha(p) for p in Path(destination).rglob("*") if p.is_file()}  # add by wushanzhou in 2026/10/24
        step = dict(command=command, inputs=inputs, exit_code=result.returncode,  # add by wushanzhou in 2026/10/24
                    seconds=time.perf_counter() - start, outputs=outputs)  # add by wushanzhou in 2026/10/24
        with self.lock:  # add by wushanzhou in 2026/10/24
            self.state["steps"][key] = step  # add by wushanzhou in 2026/10/24
            with self.journal.open("a", encoding="utf-8") as stream:  # add by wushanzhou in 2026/10/24
                stream.write(json.dumps(dict(key=key, step=step), allow_nan=False) + "\n")  # add by wushanzhou in 2026/10/24
            self.calls += 1  # add by wushanzhou in 2026/10/24
            if self.calls % 25 == 0 or result.returncode:  # add by wushanzhou in 2026/10/24
                self.save()  # add by wushanzhou in 2026/10/24
                print(f"completed {self.calls} native steps; latest={key}", flush=True)  # add by wushanzhou in 2026/10/24
        if result.returncode:  # add by wushanzhou in 2026/10/24
            raise RuntimeError(f"Native step failed: {key}; see {log}")  # add by wushanzhou in 2026/10/24


def reconstruct(recorder, key, gather, mask_path, configuration):  # add by wushanzhou in 2026/10/24
    target = recorder.output / "reconstruction" / key  # add by wushanzhou in 2026/10/24
    data, coordinates = read_gather(gather)
    mask = read_mask(mask_path)
    if len(mask) != len(coordinates):raise ValueError("mask/gather size mismatch")
    observed = np.zeros_like(data);observed[:,mask] = data[:,mask]
    observed_path = recorder.output / "observed" / (key + ".csv")
    write_gather(observed_path, observed, coordinates, "known traces only; hidden amplitudes absent")
    args = ["--review-reconstruct", observed_path, "--mask", mask_path, "--out", target,  # add by wushanzhou in 2026/10/24
            "--iter", configuration["iterations"], "--threshold-start", configuration["start"],  # add by wushanzhou in 2026/10/24
            "--threshold-end", configuration["end"], "--scales", configuration["scales"], "--angles", configuration["angles"]]  # add by wushanzhou in 2026/10/24
    if not configuration["warm"]:  # add by wushanzhou in 2026/10/24
        args.append("--cold")  # add by wushanzhou in 2026/10/24
    recorder.call("reconstruct/" + key, args, target)  # add by wushanzhou in 2026/10/24
    return np.loadtxt(target / "prediction.csv", delimiter=","), target  # add by wushanzhou in 2026/10/24


def generate_synthetic(output, count):  # add by wushanzhou in 2026/10/24
    cases = []  # add by wushanzhou in 2026/10/24
    x = np.arange(128) * 12.5  # add by wushanzhou in 2026/10/24
    t = np.arange(256)[:, None] * .004  # add by wushanzhou in 2026/10/24
    for case in range(count):  # add by wushanzhou in 2026/10/24
        seed = 731000 + 101 * case  # add by wushanzhou in 2026/10/24
        rng = np.random.default_rng(seed)  # add by wushanzhou in 2026/10/24
        data = np.zeros((256, 128))  # add by wushanzhou in 2026/10/24
        family = ("curved", "dipping", "faulted")[case % 3]  # add by wushanzhou in 2026/10/24
        events = []  # add by wushanzhou in 2026/10/24
        for layer in range(3):  # add by wushanzhou in 2026/10/24
            t0 = .17 + .22 * layer + rng.uniform(-.025, .025)  # add by wushanzhou in 2026/10/24
            velocity = rng.uniform(1800, 3400)  # add by wushanzhou in 2026/10/24
            dip = rng.uniform(-.00012, .00012)  # add by wushanzhou in 2026/10/24
            position = x - 800  # add by wushanzhou in 2026/10/24
            travel = np.sqrt(t0 ** 2 + (position / velocity) ** 2) if family == "curved" else t0 + dip * position  # add by wushanzhou in 2026/10/24
            throw = rng.uniform(.025, .065) if family == "faulted" else 0.  # add by wushanzhou in 2026/10/24
            fault_position = rng.uniform(-150, 150)  # add by wushanzhou in 2026/10/24
            travel = travel + throw * (position > fault_position)  # add by wushanzhou in 2026/10/24
            frequency = rng.uniform(18, 32)  # add by wushanzhou in 2026/10/24
            arg = (np.pi * frequency * (t - travel)) ** 2  # add by wushanzhou in 2026/10/24
            amplitude = (1 - .18 * layer) * (1 + .15 * np.sin(x / 400 + layer))  # add by wushanzhou in 2026/10/24
            data += amplitude * (1 - 2 * arg) * np.exp(-arg)  # add by wushanzhou in 2026/10/24
            events.append(dict(t0=t0, velocity=velocity, dip=dip, throw=throw, frequency=frequency, fault_position=fault_position))  # add by wushanzhou in 2026/10/24
        noise = (.01, .025)[case // 3 % 2]  # add by wushanzhou in 2026/10/24
        data += rng.normal(0, noise, data.shape)  # add by wushanzhou in 2026/10/24
        path = output / "inputs" / f"synthetic_{case:02d}.csv"  # add by wushanzhou in 2026/10/24
        write_gather(path, data, x, f"synthetic_model={family}; realization_seed={seed}; dt_s=.004")  # add by wushanzhou in 2026/10/24
        metadata = dict(name=path.stem, family=family, seed=seed, noise_sd=noise, events=events, dt_s=.004, dx_m=12.5, shape=[256,128], sha256=sha(path))  # add by wushanzhou in 2026/10/24
        path.with_suffix(".json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")  # add by wushanzhou in 2026/10/24
        cases.append((path, metadata))  # add by wushanzhou in 2026/10/24
    return cases  # add by wushanzhou in 2026/10/24


def nested_case(recorder, name, gather, data, x, outer_mask, seed, configurations):  # add by wushanzhou in 2026/10/24
    known = np.flatnonzero(outer_mask)  # add by wushanzhou in 2026/10/24
    if len(known) < 4:  # add by wushanzhou in 2026/10/24
        raise ValueError("at least four outer-known traces are needed for nested validation")  # add by wushanzhou in 2026/10/24
    rng = np.random.default_rng(seed)  # add by wushanzhou in 2026/10/24
    shuffled = rng.permutation(known)  # add by wushanzhou in 2026/10/24
    validation = shuffled[:max(2, len(known) // 3)]  # add by wushanzhou in 2026/10/24
    train_mask = outer_mask.copy()  # add by wushanzhou in 2026/10/24
    train_mask[validation] = False  # add by wushanzhou in 2026/10/24
    train_path = recorder.output / "masks" / (name + "_inner.txt")  # add by wushanzhou in 2026/10/24
    outer_path = recorder.output / "masks" / (name + "_outer.txt")  # add by wushanzhou in 2026/10/24
    write_mask(train_path, train_mask)  # add by wushanzhou in 2026/10/24
    write_mask(outer_path, outer_mask)  # add by wushanzhou in 2026/10/24
    candidates = [("zero", np.zeros((len(data), len(validation)))),  # add by wushanzhou in 2026/10/24
                  ("linear", linear_known(data, x, train_mask)[:, validation])]  # add by wushanzhou in 2026/10/24
    for configuration in configurations:  # add by wushanzhou in 2026/10/24
        prediction, _ = reconstruct(recorder, name + "_inner_" + configuration["name"], gather, train_path, configuration)  # add by wushanzhou in 2026/10/24
        candidates.append((configuration["name"], prediction[:, validation]))  # add by wushanzhou in 2026/10/24
    selected, candidate_rows = validation_choice(data[:, validation], candidates)  # add by wushanzhou in 2026/10/24
    # Freeze selection before requesting or evaluating any outer-test prediction.  # add by wushanzhou in 2026/10/24
    choice_path = recorder.output / "selection" / (name + ".json")  # add by wushanzhou in 2026/10/24
    choice_path.parent.mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
    choice_path.write_text(json.dumps(dict(selected=selected, candidates=candidate_rows,  # add by wushanzhou in 2026/10/24
                                           outer_known=known.tolist(), inner_train=np.flatnonzero(train_mask).tolist(),  # add by wushanzhou in 2026/10/24
                                           validation=validation.tolist(), seed=int(seed)), indent=2), encoding="utf-8")  # add by wushanzhou in 2026/10/24
    linear = linear_known(data, x, outer_mask)  # add by wushanzhou in 2026/10/24
    fixed, fixed_path = reconstruct(recorder, name + "_outer_fixed", gather, outer_path, CONFIGURATIONS[0])  # add by wushanzhou in 2026/10/24
    if selected["candidate"] == "zero":  # add by wushanzhou in 2026/10/24
        prediction = np.zeros_like(data)  # add by wushanzhou in 2026/10/24
    elif selected["candidate"] == "linear":  # add by wushanzhou in 2026/10/24
        prediction = linear.copy()  # add by wushanzhou in 2026/10/24
    else:  # add by wushanzhou in 2026/10/24
        configuration = next(c for c in configurations if c["name"] == selected["candidate"])  # add by wushanzhou in 2026/10/24
        prediction, _ = reconstruct(recorder, name + "_outer_selected", gather, outer_path, configuration)  # add by wushanzhou in 2026/10/24
    prediction = selected["gain"] * prediction  # add by wushanzhou in 2026/10/24
    prediction[:, outer_mask] = data[:, outer_mask]  # add by wushanzhou in 2026/10/24
    prediction_path = recorder.output / "predictions" / (name + ".csv")  # add by wushanzhou in 2026/10/24
    prediction_path.parent.mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
    np.savetxt(prediction_path, prediction, delimiter=",", fmt="%.12g")  # add by wushanzhou in 2026/10/24
    record = dict(case=name, input=str(gather.relative_to(ROOT)) if gather.is_relative_to(ROOT) else str(gather),  # add by wushanzhou in 2026/10/24
                  input_sha256=sha(gather), n=len(x), keep=int(outer_mask.sum()), seed=int(seed),  # add by wushanzhou in 2026/10/24
                  outer_mask=str(outer_path.relative_to(recorder.output)), outer_mask_sha1=mask_hash(outer_mask),  # add by wushanzhou in 2026/10/24
                  selected=selected["candidate"], gain=selected["gain"], validation_mse=selected["validation_mse"],  # add by wushanzhou in 2026/10/24
                  snr_zero=0., snr_linear=float(snr(data, linear, ~outer_mask)),  # add by wushanzhou in 2026/10/24
                  snr_fixed=float(snr(data, fixed, ~outer_mask)), snr_selected=float(snr(data, prediction, ~outer_mask)),  # add by wushanzhou in 2026/10/24
                  fixed_prediction=str((fixed_path / "prediction.csv").relative_to(recorder.output)),  # add by wushanzhou in 2026/10/24
                  selected_prediction=str(prediction_path.relative_to(recorder.output)))  # add by wushanzhou in 2026/10/24
    return record, selected, prediction  # add by wushanzhou in 2026/10/24


def diagnostics(output, real_input=None):  # add by wushanzhou in 2026/10/24
    raw, offsets, cdps, meta = read_segy(WORKSPACE / "data/real_seismic/Data/RRAW.SGY")  # add by wushanzhou in 2026/10/24
    archived, x = convert_measured(raw, offsets, normalize=False)
    real_input = real_input or output / "inputs/real_gather_59x256.csv"
    write_gather(real_input, archived, x, "raw amplitude; time resampling only; no full-field normalization")
    order = np.argsort(offsets, kind="stable")  # add by wushanzhou in 2026/10/24
    result = dict(source_sha256=meta["sha256"], independent_real_sources=1,  # add by wushanzhou in 2026/10/24
                  preprocessing="time interpolation only; raw amplitude; solver scaling uses known traces",
                  source_dt_ms=8., raw_nyquist_hz=62.5, window=[0., 1.020],  # add by wushanzhou in 2026/10/24
                  trace_headers_checked=meta["trace_headers_checked"], binary_dt_field=meta["dt_raw_field"],  # add by wushanzhou in 2026/10/24
                  trace_dt_us=meta["dt_trace_field"], payload_profile=meta["payload_profile"], groups=[])  # add by wushanzhou in 2026/10/24
    groups = [("mixed", np.ones(len(x), dtype=bool))] + [(f"CDP_{c}", cdps[order] == c) for c in np.unique(cdps)]  # add by wushanzhou in 2026/10/24
    for name, subset in groups:  # add by wushanzhou in 2026/10/24
        values = archived[:, subset]  # add by wushanzhou in 2026/10/24
        positions = x[subset]  # add by wushanzhou in 2026/10/24
        path = output / "inputs" / (name + ".csv")  # add by wushanzhou in 2026/10/24
        write_gather(path, values, positions, f"single_source_diagnostic={name}; dt_s=.004")  # add by wushanzhou in 2026/10/24
        # Only a fixed geometry-defined training subset enters event-continuity diagnostics.  # add by wushanzhou in 2026/10/24
        rng = np.random.default_rng(20261024)  # add by wushanzhou in 2026/10/24
        training = np.sort(rng.choice(values.shape[1], max(2, 2 * values.shape[1] // 3), replace=False))  # add by wushanzhou in 2026/10/24
        known = values[:, training]  # add by wushanzhou in 2026/10/24
        centered = known - known.mean(axis=0)  # add by wushanzhou in 2026/10/24
        correlations, lag_correlations, lags = [], [], []  # add by wushanzhou in 2026/10/24
        for j in range(known.shape[1] - 1):  # add by wushanzhou in 2026/10/24
            a, b = centered[:, j], centered[:, j+1]  # add by wushanzhou in 2026/10/24
            denom = np.linalg.norm(a) * np.linalg.norm(b)  # add by wushanzhou in 2026/10/24
            correlations.append(float(a @ b / denom) if denom else 0.)  # add by wushanzhou in 2026/10/24
            candidates = []  # add by wushanzhou in 2026/10/24
            for lag in range(-12, 13):  # add by wushanzhou in 2026/10/24
                aa, bb = (a[-lag:], b[:lag]) if lag < 0 else (a[:-lag], b[lag:]) if lag else (a, b)  # add by wushanzhou in 2026/10/24
                denominator = np.linalg.norm(aa) * np.linalg.norm(bb)  # add by wushanzhou in 2026/10/24
                candidates.append(float(aa @ bb / denominator) if denominator else 0.)  # add by wushanzhou in 2026/10/24
            best = int(np.argmax(candidates))  # add by wushanzhou in 2026/10/24
            lag_correlations.append(candidates[best]);lags.append((best - 12) * .004)  # add by wushanzhou in 2026/10/24
        result["groups"].append(dict(name=name, traces=len(positions), input=str(path.relative_to(output)),  # add by wushanzhou in 2026/10/24
                                     diagnostic_training=training.tolist(), zero_lag_mean=float(np.mean(correlations)),  # add by wushanzhou in 2026/10/24
                                     bounded_lag_mean=float(np.mean(lag_correlations)),  # add by wushanzhou in 2026/10/24
                                     median_absolute_lag_s=float(np.median(np.abs(lags)))))  # add by wushanzhou in 2026/10/24
    (output / "data_diagnostics.json").write_text(json.dumps(result, indent=2), encoding="utf-8")  # add by wushanzhou in 2026/10/24
    return result  # add by wushanzhou in 2026/10/24


def synthetic_case(recorder, gather, model, search, recon_seeds, weight_seeds):  # add by wushanzhou in 2026/10/24
    truth, x = read_gather(gather)  # add by wushanzhou in 2026/10/24
    cache, records = {}, []  # add by wushanzhou in 2026/10/24
    for row in search:  # add by wushanzhou in 2026/10/24
        seed = int(row["seed"])  # add by wushanzhou in 2026/10/24
        is_method = row["method"] in METHODS and seed in recon_seeds  # add by wushanzhou in 2026/10/24
        is_weight = row["method"].startswith("W_") and seed in weight_seeds  # add by wushanzhou in 2026/10/24
        if not is_method and not is_weight:  # add by wushanzhou in 2026/10/24
            continue  # add by wushanzhou in 2026/10/24
        mask_path = recorder.output / "search_uniform_200" / row["mask_path"]  # add by wushanzhou in 2026/10/24
        mask = read_mask(mask_path)  # add by wushanzhou in 2026/10/24
        identity = mask_hash(mask)  # add by wushanzhou in 2026/10/24
        key = f"{model['name']}_{row['method']}_r{row['rate_index']}_s{seed}"  # add by wushanzhou in 2026/10/24
        if identity not in cache:  # add by wushanzhou in 2026/10/24
            cache[identity] = reconstruct(recorder, key, gather, mask_path, CONFIGURATIONS[0])  # add by wushanzhou in 2026/10/24
        prediction, path = cache[identity]  # add by wushanzhou in 2026/10/24
        linear = linear_known(truth, x, mask)  # add by wushanzhou in 2026/10/24
        records.append(dict(model=model["name"], family=model["family"], realization_seed=model["seed"],  # add by wushanzhou in 2026/10/24
                            rate_index=int(row["rate_index"]), method=row["method"], seed=seed,  # add by wushanzhou in 2026/10/24
                            mask=str(mask_path.relative_to(recorder.output)), mask_sha1=identity,  # add by wushanzhou in 2026/10/24
                            input=str(gather.relative_to(recorder.output)), input_sha256=sha(gather),  # add by wushanzhou in 2026/10/24
                            prediction=str((path / "prediction.csv").relative_to(recorder.output)),  # add by wushanzhou in 2026/10/24
                            mu=float(row["mu"]), max_gap=int(row["max_gap"]), nn_cv=float(row["nn_cv"]),  # add by wushanzhou in 2026/10/24
                            lambda_gap=float(row["lambda_gap"]), lambda_nn=float(row["lambda_nn"]),  # add by wushanzhou in 2026/10/24
                            snr_missing=float(snr(truth, prediction, ~mask)), snr_linear=float(snr(truth, linear, ~mask))))  # add by wushanzhou in 2026/10/24
    print("Synthetic realization complete:", model["name"], flush=True)  # add by wushanzhou in 2026/10/24
    return records  # add by wushanzhou in 2026/10/24


def run(args):  # add by wushanzhou in 2026/10/24
    synthetic_only = getattr(args, "synthetic_only", False)
    require_inputs([args.exe])
    provenance = dict(status="not_used", field_claims_eligible=False, independent_real_sources=0) if synthetic_only else real_source_provenance(
        WORKSPACE / "data/real_seismic/Data/RRAW.SGY", getattr(args,"source_confirmation",None), getattr(args,"allow_unconfirmed_real",False))
    configuration = dict(quick=args.quick, search_seeds=3 if args.quick else 20,  # add by wushanzhou in 2026/10/24
                         workers=args.workers,  # add by wushanzhou in 2026/10/24
                         reconstruction_seeds=1 if args.quick else 5, weight_reconstruction_seeds=1 if args.quick else 3,  # add by wushanzhou in 2026/10/24
                         synthetic_realizations=1 if args.quick else 6, outer_real_seeds=1 if args.quick else 10,  # add by wushanzhou in 2026/10/24
                         budgets=[200] if args.quick else [200, 600], configurations=CONFIGURATIONS,  # add by wushanzhou in 2026/10/24
                         statistical_family="Adaptive vs Hybrid/SAOnly/SA_Budget/FixedSwitch/NoRecycle/AdjacentOnly across domains/rates/budgets",  # add by wushanzhou in 2026/10/24
                         real_sources=0 if synthetic_only else 1, real_source_provenance=provenance,
                         synthetic_only=synthetic_only, strict_preprocessing_isolation=True, stopping="fixed iteration counts; no hidden-test early stopping")  # add by wushanzhou in 2026/10/24
    recorder = Recorder(args.out, args.exe, configuration, args.resume)  # add by wushanzhou in 2026/10/24
    output = recorder.output  # add by wushanzhou in 2026/10/24
    (output / "inputs").mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
    real_input = output / "inputs/real_gather_59x256.csv"
    data_diagnostics = None if synthetic_only else diagnostics(output, real_input)
    for domain in (("uniform",) if synthetic_only else ("uniform", "physical")):  # add by wushanzhou in 2026/10/24
        for budget in configuration["budgets"]:  # add by wushanzhou in 2026/10/24
            target = output / f"search_{domain}_{budget}"  # add by wushanzhou in 2026/10/24
            command = ["--review-search", target, "--seeds", configuration["search_seeds"], "--budget", budget, "--seed", 20261024]  # add by wushanzhou in 2026/10/24
            if domain == "physical":  # add by wushanzhou in 2026/10/24
                command += ["--input", real_input]  # add by wushanzhou in 2026/10/24
            if budget == 200:  # add by wushanzhou in 2026/10/24
                command.append("--tradeoff")  # add by wushanzhou in 2026/10/24
            recorder.call(f"search/{domain}/{budget}", command, target)  # add by wushanzhou in 2026/10/24
    if args.stage == "search":  # add by wushanzhou in 2026/10/24
        recorder.save()  # add by wushanzhou in 2026/10/24
        print("Search stage complete", flush=True)  # add by wushanzhou in 2026/10/24
        return  # add by wushanzhou in 2026/10/24
    models = generate_synthetic(output, configuration["synthetic_realizations"])  # add by wushanzhou in 2026/10/24
    search = read_rows(output / "search_uniform_200/search.csv")  # add by wushanzhou in 2026/10/24
    all_seeds = sorted({int(row["seed"]) for row in search})  # add by wushanzhou in 2026/10/24
    recon_seeds = set(all_seeds[:configuration["reconstruction_seeds"]])  # add by wushanzhou in 2026/10/24
    weight_seeds = set(all_seeds[:configuration["weight_reconstruction_seeds"]])  # add by wushanzhou in 2026/10/24
    synthetic_rows = []  # add by wushanzhou in 2026/10/24
    with ThreadPoolExecutor(max_workers=args.workers) as pool:  # add by wushanzhou in 2026/10/24
        futures = [pool.submit(synthetic_case,recorder,gather,model,search,recon_seeds,weight_seeds) for gather,model in models]  # add by wushanzhou in 2026/10/24
        for future in futures:  # add by wushanzhou in 2026/10/24
            synthetic_rows.extend(future.result())  # add by wushanzhou in 2026/10/24
    write_rows(output / "synthetic_reconstruction.csv", synthetic_rows)  # add by wushanzhou in 2026/10/24
    if synthetic_only:
        recorder.state["completed"] = True;recorder.save()
        print("Synthetic v4 studies completed", flush=True);return
    real, x = read_gather(real_input)  # add by wushanzhou in 2026/10/24
    real_mask_rows = []  # add by wushanzhou in 2026/10/24
    real_search = read_rows(output / "search_physical_200/search.csv")  # add by wushanzhou in 2026/10/24
    real_cache = {}  # add by wushanzhou in 2026/10/24
    real_jobs={} # add by wushanzhou in 2026/10/24
    for row in real_search: # add by wushanzhou in 2026/10/24
        if row['method'] in METHODS and int(row['seed']) in recon_seeds:real_jobs.setdefault(row['mask_sha1'],row) # add by wushanzhou in 2026/10/24
    def run_real(item): # add by wushanzhou in 2026/10/24
        identity,row=item;mask_path=output/'search_physical_200'/row['mask_path'] # add by wushanzhou in 2026/10/24
        key=f"real_mask_{row['method']}_r{row['rate_index']}_s{row['seed']}" # add by wushanzhou in 2026/10/24
        return identity,reconstruct(recorder,key,real_input,mask_path,CONFIGURATIONS[0]) # add by wushanzhou in 2026/10/24
    with ThreadPoolExecutor(max_workers=args.workers) as pool: # add by wushanzhou in 2026/10/24
        real_cache=dict(pool.map(run_real,list(real_jobs.items()))) # add by wushanzhou in 2026/10/24
    for row in real_search:  # add by wushanzhou in 2026/10/24
        if row["method"] not in METHODS or int(row["seed"]) not in recon_seeds:  # add by wushanzhou in 2026/10/24
            continue  # add by wushanzhou in 2026/10/24
        mask_path = output / "search_physical_200" / row["mask_path"]  # add by wushanzhou in 2026/10/24
        mask = read_mask(mask_path); identity = mask_hash(mask)  # add by wushanzhou in 2026/10/24
        prediction, path = real_cache[identity]  # add by wushanzhou in 2026/10/24
        linear = linear_known(real, x, mask)  # add by wushanzhou in 2026/10/24
        real_mask_rows.append(dict(rate_index=int(row["rate_index"]), method=row["method"], seed=int(row["seed"]),  # add by wushanzhou in 2026/10/24
                                   mask=str(mask_path.relative_to(output)), mask_sha1=identity, keep=int(mask.sum()),  # add by wushanzhou in 2026/10/24
                                   prediction=str((path / "prediction.csv").relative_to(output)), mu=float(row["mu"]),  # add by wushanzhou in 2026/10/24
                                   max_gap_m=float(row["max_gap_m"]), nn_cv=float(row["nn_cv"]),  # add by wushanzhou in 2026/10/24
                                   snr_missing=float(snr(real, prediction, ~mask)), snr_linear=float(snr(real, linear, ~mask))))  # add by wushanzhou in 2026/10/24
    write_rows(output / "real_mask_reconstruction.csv", real_mask_rows)  # add by wushanzhou in 2026/10/24
    nested_rows = []  # add by wushanzhou in 2026/10/24
    configurations = CONFIGURATIONS[:2] if args.quick else CONFIGURATIONS  # add by wushanzhou in 2026/10/24
    def run_nested(job):  # add by wushanzhou in 2026/10/24
            keep,instance=job  # add by wushanzhou in 2026/10/24
            seed = 2026102400 + instance * 179  # add by wushanzhou in 2026/10/24
            rng = np.random.default_rng(seed)  # add by wushanzhou in 2026/10/24
            mask = np.zeros(59, dtype=bool)  # add by wushanzhou in 2026/10/24
            mask[rng.choice(59, keep, replace=False)] = True  # add by wushanzhou in 2026/10/24
            name = f"real_keep{keep}_seed{seed}"  # add by wushanzhou in 2026/10/24
            row, selected, prediction = nested_case(recorder, name, real_input, real, x, mask, seed+1, configurations)  # add by wushanzhou in 2026/10/24
            print("Nested case complete:", name, "choice=", selected["candidate"], flush=True)  # add by wushanzhou in 2026/10/24
            return row  # add by wushanzhou in 2026/10/24
    with ThreadPoolExecutor(max_workers=args.workers) as pool:  # add by wushanzhou in 2026/10/24
        nested_rows=list(pool.map(run_nested,[(keep,i) for keep in (20,30,40) for i in range(configuration["outer_real_seeds"])]))  # add by wushanzhou in 2026/10/24
    write_rows(output / "real_nested.csv", nested_rows)  # add by wushanzhou in 2026/10/24
    # Additional CDP subsets remain dependent, descriptive, fixed-split diagnostics.  # add by wushanzhou in 2026/10/24
    cdp_rows = []  # add by wushanzhou in 2026/10/24
    for group in data_diagnostics["groups"][1:]:  # add by wushanzhou in 2026/10/24
        gather = output / group["input"]  # add by wushanzhou in 2026/10/24
        truth, x_group = read_gather(gather)  # add by wushanzhou in 2026/10/24
        rng = np.random.default_rng(20261024)  # add by wushanzhou in 2026/10/24
        mask = np.zeros(len(x_group), dtype=bool)  # add by wushanzhou in 2026/10/24
        mask[rng.choice(len(mask), max(4, int(2*len(mask)/3+.5)), replace=False)] = True  # add by wushanzhou in 2026/10/24
        row, _, _ = nested_case(recorder, group["name"], gather, truth, x_group, mask, 20261025, configurations)  # add by wushanzhou in 2026/10/24
        cdp_rows.append(row)  # add by wushanzhou in 2026/10/24
    write_rows(output / "cdp_nested.csv", cdp_rows)  # add by wushanzhou in 2026/10/24
    # One actual end-to-end leakage challenge, in addition to native solver isolation regressions.  # add by wushanzhou in 2026/10/24
    first = nested_rows[0]  # add by wushanzhou in 2026/10/24
    mask = read_mask(output / first["outer_mask"])  # add by wushanzhou in 2026/10/24
    changed = real.copy();changed[:, ~mask] = changed[:, ~mask] * 13 + 7  # add by wushanzhou in 2026/10/24
    changed_path = output / "inputs/leakage_challenge.csv"  # add by wushanzhou in 2026/10/24
    write_gather(changed_path, changed, x, "hidden_test_values_changed_only")  # add by wushanzhou in 2026/10/24
    altered, selected, prediction = nested_case(recorder, "leakage_challenge", changed_path, changed, x,  # add by wushanzhou in 2026/10/24
                                                mask, int(first["seed"]), configurations)  # add by wushanzhou in 2026/10/24
    original_selection = json.loads((output / "selection" / (first["case"]+".json")).read_text())["selected"]  # add by wushanzhou in 2026/10/24
    baseline_prediction = np.loadtxt(output / first["selected_prediction"], delimiter=",")  # add by wushanzhou in 2026/10/24
    difference = float(np.max(np.abs(prediction-baseline_prediction)))  # add by wushanzhou in 2026/10/24
    if selected != original_selection or difference > 2e-6:  # add by wushanzhou in 2026/10/24
        raise AssertionError("Outer-test leakage challenge failed")  # add by wushanzhou in 2026/10/24
    (output / "leakage_check.json").write_text(json.dumps(dict(passed=True, max_prediction_difference=difference,  # add by wushanzhou in 2026/10/24
                                                              selected_unchanged=True), indent=2), encoding="utf-8")  # add by wushanzhou in 2026/10/24
    recorder.state["completed"] = True  # add by wushanzhou in 2026/10/24
    recorder.save()  # add by wushanzhou in 2026/10/24
    print("All v4 studies completed", flush=True)  # add by wushanzhou in 2026/10/24


if __name__ == "__main__":  # add by wushanzhou in 2026/10/24
    parser = argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/24
    parser.add_argument("--out", type=Path, default=WORKSPACE / "results/review_20261024_v4/final") # add by wushanzhou in 2026/10/24
    parser.add_argument("--exe", type=Path, default=WORKSPACE / "build/bin/Release/IrrGeometryDesign.exe")  # add by wushanzhou in 2026/10/24
    parser.add_argument("--synthetic-only", action="store_true", help="uniform search and synthetic recovery without external real inputs")
    add_real_data_arguments(parser)
    parser.add_argument("--quick", action="store_true")  # add by wushanzhou in 2026/10/24
    parser.add_argument("--resume", action="store_true")  # add by wushanzhou in 2026/10/24
    parser.add_argument("--stage", choices=("all", "search"), default="all")  # add by wushanzhou in 2026/10/24
    parser.add_argument("--workers",type=int,choices=range(1,9),default=4)  # add by wushanzhou in 2026/10/24
    run(parser.parse_args())  # add by wushanzhou in 2026/10/24
