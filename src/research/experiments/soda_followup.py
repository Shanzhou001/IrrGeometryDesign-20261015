"""Score the predeclared second Soda Lake holdout using observed pair warps."""
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import argparse
from itertools import product
import json

import numpy as np

from research.common.paths import ROOT, current_results
from research.recovery_local import warps
from research.common.hashing import digest
from research.common.tabular import rows, write_rows
from research.data.mapped_shots import read_shots, soda_waveform
from research.experiments.field_layouts import constrained_layouts, spatial_cells
from research.experiments.field_recovery import dump, input_hashes, PROTOCOL
from research.experiments.mapped_field import hidden_mask
from research.recovery_geometry import fit
from research.validation.field_artifacts import audit, check_manifest, seal, source_snapshot, write_verification


PLAN = Path(__file__).with_name('soda_followup_protocol.json')


def coordinate_idw(observed, xy, known, hidden, start, stop):
    distances = np.linalg.norm(xy[hidden, None] - xy[None, known], axis=2)
    nearest = np.argsort(distances, axis=1, kind='stable')[:, :6]
    weights = np.maximum(np.take_along_axis(distances, nearest, axis=1), 1e-6) ** -1.5
    weights /= weights.sum(axis=1, keepdims=True)
    return np.sum(observed[start:stop, nearest] * weights, axis=2)


def training(plan):
    blocks = defaultdict(dict)
    h, seed = plan['hidden_seeds'][0], plan['layout_seeds'][0]
    for _, fid, component, xy, _, dt, data in read_shots(plan, 'development'):
        hidden = hidden_mask(len(xy), h)
        for fraction in plan['keep_fractions']:
            cells = spatial_cells(xy, np.flatnonzero(~hidden), round(fraction * len(xy)))
            rng = np.random.default_rng(seed)
            known = np.zeros(len(xy), bool)
            known[[rng.choice(cell) for cell in cells]] = True
            observed = data[:, known]
            for start, stop in plan['windows']:
                x = warps(observed, xy, known, hidden, start, stop, dt).reshape(4, -1)
                y = data[start:stop, hidden].ravel()
                energy = float(y @ y)
                if energy <= 0:
                    raise ValueError('Invalid training score target')
                a, b, n = blocks[f'{component}_{start}'].setdefault(
                    fid, (np.zeros((4, 4)), np.zeros(4), np.zeros(())))
                a += x @ x.T / energy
                b += x @ y / energy
                n += 1
        print('development', fid, component, flush=True)
    return {name: fit(group.values()) for name, group in sorted(blocks.items())}


def scoring(plan, model, workers, out):
    layouts = {}
    results = []
    for _, fid, component, xy, _, dt, data in read_shots(plan, 'holdout'):
        if fid not in layouts:
            layouts[fid] = []
            for h in plan['hidden_seeds']:
                hidden = hidden_mask(len(xy), h)
                for fraction in plan['keep_fractions']:
                    count = round(fraction * len(xy))
                    for seed in plan['layout_seeds']:
                        choices = constrained_layouts(xy, hidden, count, seed, plan['layout_budget'], plan['hybrid_prefix'])
                        for method, known, _ in choices:
                            layouts[fid].append((h, count, seed, method, known, hidden))
        jobs = [(start, stop, layout) for start, stop in plan['windows'] for layout in layouts[fid]]

        def one(job):
            start, stop, (h, count, seed, method, known, hidden) = job
            observed = data[:, known]
            w = np.asarray(model[f'{component}_{start}']['weights'])
            shape = (stop - start, hidden.sum())
            pred = np.einsum('f,ftx->tx', w, warps(observed, xy, known, hidden, start, stop, dt)) if w.any() else np.zeros(shape)
            base = coordinate_idw(observed, xy, known, hidden, start, stop)
            truth = data[start:stop, hidden]
            energy = np.sum(truth ** 2)
            if energy <= 0 or not np.isfinite(pred).all():
                raise ValueError('Invalid confirmation score target or prediction')
            case = f'{fid}_c{component}_w{start}_h{h}_m{count}_s{seed}_{method}'
            path = out / 'predictions' / f'{case}.npz'
            np.savez_compressed(path, prediction=pred, idw=base, truth=truth, xy=xy, known=known, hidden=hidden)
            return dict(case=case, ffid=fid, component=component, start=start, h=h, m=count, seed=seed,
                        method=method, nmse=float(np.sum((pred - truth)**2)/energy),
                        idw_nmse=float(np.sum((base - truth)**2)/energy),
                        zero_nmse=1., prediction_sha256=digest(path))

        with ThreadPoolExecutor(max_workers=workers) as pool:
            results.extend(pool.map(one, jobs))
        print('holdout', fid, component, flush=True)
    return results


def run(out, workers=6):
    initial_sources=source_snapshot()
    out=Path(out)
    plan = json.loads(PLAN.read_text())
    if set(plan['soda_development']) & set(plan['soda_holdout']):
        raise ValueError('Development and holdout shots overlap')
    hashes = input_hashes(json.loads(PROTOCOL.read_text()))
    for fid in plan['soda_holdout']:
        path = soda_waveform(fid)
        receipt = path.with_name(path.name + '.json')
        expected = json.loads(receipt.read_text())['sha256']
        if digest(path) != expected:
            raise ValueError(f'Unregistered shot bytes: {fid}')
        hashes[path.relative_to(ROOT).as_posix()] = expected
        hashes[receipt.relative_to(ROOT).as_posix()] = digest(receipt)
    out.mkdir(parents=True)
    (out/'predictions').mkdir()
    dump(out/'protocol.json', plan)
    model = training(plan)
    dump(out/'frozen_model.json', model)
    model_sha = digest(out/'frozen_model.json')
    cases = scoring(plan, model, workers, out)
    ids = set(plan['soda_holdout'])
    if {r['ffid'] for r in cases} != ids or len({r['case'] for r in cases}) != len(ids)*288:
        raise ValueError('Missing confirmation cases')
    shots = []
    for fid in sorted(ids):
        group = [r for r in cases if r['ffid'] == fid]
        if len(group) != 288:
            raise ValueError(f'Missing cases for {fid}')
        shots.append(dict(ffid=fid, cases=len(group), nmse=float(np.mean([r['nmse'] for r in group])),
                          idw_nmse=float(np.mean([r['idw_nmse'] for r in group]))))
    nmse = float(np.mean([r['nmse'] for r in shots]))
    idw = float(np.mean([r['idw_nmse'] for r in shots]))
    count = sum(r['nmse'] < 1 for r in shots)
    summary = dict(status='passed', performance_status='passed' if nmse < min(1, idw)
                   and count >= plan['minimum_positive_shots'] else 'failed',
                   shots=shots, cases=len(cases), positive_shots=count,
                   mean_nmse=nmse, zero_nmse=1., mean_idw_nmse=idw)
    write_rows(out/'recovery.csv', cases)
    write_rows(out/'shot_summary.csv', shots)
    dump(out/'performance.json', summary)
    if digest(out/'frozen_model.json')!=model_sha:
        raise ValueError('Local-delay model changed during confirmation')
    seal(out, PLAN, hashes, ['recovery.csv', 'shot_summary.csv', 'performance.json'], initial_sources)
    write_verification(out, verify(out))
    print(json.dumps(summary, indent=2))


def verify(out):
    out = Path(out)
    plan = check_manifest(out)
    acquisitions = {(fid, c): (xy, data) for _, fid, c, xy, _, _, data in read_shots(plan, 'holdout')}
    windows = dict(plan['windows'])
    expected = {(fid, c, start, h, round(128*fraction), seed, method)
                for fid, c, (start, _), h, fraction, seed, method in product(
                    plan['soda_holdout'], (1, 2, 3), plan['windows'], plan['hidden_seeds'],
                    plan['keep_fractions'], plan['layout_seeds'], ('Jitter', 'RandomBudget', 'SA', 'Hybrid'))}
    cases = []
    for row in rows(out/'recovery.csv'):
        fid, c, start, h, count, seed = (int(row[k]) for k in ('ffid', 'component', 'start', 'h', 'm', 'seed'))
        key = fid, c, start, h, count, seed, row['method']
        cases.append((key, 'predictions/'+row['case']+'.npz', row['prediction_sha256'], h, count,
                      start, windows[start], row['nmse'], row['idw_nmse'], {}))
    return audit(out, acquisitions, cases, expected)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=current_results()/'soda_local_warp_confirmation')
    parser.add_argument('--workers', type=int, default=6, choices=range(1, 17))
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    if args.verify:
        print(json.dumps(verify(args.out), indent=2))
    else:
        run(args.out.resolve(), args.workers)
