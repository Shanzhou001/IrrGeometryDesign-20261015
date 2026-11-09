"""Apply the frozen Poland recovery to previously registered Viking shots."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import argparse
from itertools import product
import csv
import json

import numpy as np

from research.common.paths import ROOT, current_results
from research.common.hashing import digest
from research.common.tabular import rows, write_rows
from research.data.field_segy import FieldSegy
from research.experiments.field_layouts import constrained_layouts
from research.experiments.field_recovery import dump
from research.experiments.mapped_field import hidden_mask
from research.recovery_geometry import combine, predictors
from research.validation.field_artifacts import audit, check_manifest, seal, source_snapshot, write_verification

PLAN = Path(__file__).with_name('viking_transfer_protocol.json')
REGISTRY = ROOT/'src/research/data/viking_line12_subset.json'
BASE = ROOT/'data/public/viking_line12_small'



def gather(fid, shot, plan):
    stem = f'shot_ffid_{fid:04}'
    waveform = BASE/'raw'/f'{stem}.sgy'
    geometry = BASE/'geometry'/f'{stem}.csv'
    for path in (waveform, geometry):
        if digest(path) != shot['files'][path.relative_to(BASE).as_posix()]['sha256']:
            raise ValueError(f'Viking input changed: {path}')
    with geometry.open(newline='', encoding='utf-8-sig') as stream:
        rows = list(csv.DictReader(stream))
    n = plan['receivers']
    r = np.array([float(row['receiver_x_m']) for row in rows])
    source = float(shot['source_x_m'])
    with FieldSegy(waveform) as segy:
        if (len(r) != segy.count or len(r) != n or np.any(segy.field(8,'i4') != fid)
                or not np.array_equal(segy.field(12,'i4'), [int(row['original_channel']) for row in rows])
                or np.any(segy.field(72,'i4') != source) or not np.array_equal(segy.field(80,'i4'), r)
                or np.any(np.abs(np.diff(r)) != 25) or np.any(segy.field(70,'i2') != 1)
                or np.any(segy.field(108,'i2')) or segy.dt != 4000):
            raise ValueError(f'Viking header geometry or timing differs in {fid}')
        data = segy.decode(np.arange(n), 0, segy.ns)
    return np.column_stack((r, np.zeros(n))), np.array([source, 0.]), data


def run(out, model_study, workers=6):
    initial_sources=source_snapshot()
    out=Path(out)
    model_path=Path(model_study)/'frozen_model.json'
    plan = json.loads(PLAN.read_text())
    registered = json.loads(REGISTRY.read_text())
    model_sha = digest(model_path)
    if model_sha != json.loads(model_path.with_name('manifest.json').read_text())['output_sha256'][model_path.name]:
        raise ValueError('Frozen Poland model changed')
    model = json.loads(model_path.read_text())
    if [s['ffid'] for s in registered['shots']] != plan['ffids']:
        raise ValueError('Registered Viking transfer shots differ')
    out.mkdir(parents=True)
    (out/'predictions').mkdir()
    dump(out/'protocol.json',plan)
    inputs = {p.relative_to(ROOT).as_posix(): digest(p) for p in (REGISTRY, model_path, PLAN, Path(__file__))}
    cases, shots = [], []
    for shot in registered['shots']:
        fid = shot['ffid']
        xy, source, data = gather(fid, shot, plan)
        inputs.update({f'{BASE.relative_to(ROOT).as_posix()}/{name}': info['sha256'] for name,info in shot['files'].items()
                       if name.startswith(('raw/','geometry/'))})
        layouts = []
        for h in plan['hidden_seeds']:
            hidden = hidden_mask(len(xy),h)
            for fraction in plan['keep_fractions']:
                m = round(fraction*len(xy))
                for seed in plan['layout_seeds']:
                    for method, known, _ in constrained_layouts(xy,hidden,m,seed,plan['layout_budget'],plan['hybrid_prefix']):
                        layouts.append((h,m,seed,method,known,hidden))

        def one(job):
            (label,start,stop),(h,m,seed,method,known,hidden) = job
            features,idw,strata = predictors(data[:,known],xy,source,known,hidden,start,stop,.004)
            prediction = combine(features,strata,[model[f'poland_0_{label}_{g}'] for g in range(3)])
            truth = data[start:stop,hidden]
            energy = np.sum(truth**2)
            if energy<=0 or not np.isfinite(prediction).all():
                raise ValueError('Invalid Viking target or prediction')
            path = out/'predictions'/f'f{fid}_w{start}_h{h}_m{m}_s{seed}_{method}.npz'
            np.savez_compressed(path,prediction=prediction,truth=truth,idw=idw,xy=xy,known=known,hidden=hidden)
            return dict(ffid=fid,start=start,hidden_seed=h,receivers=m,layout_seed=seed,method=method,
                        nmse=float(np.sum((prediction-truth)**2)/energy),idw_nmse=float(np.sum((idw-truth)**2)/energy),
                        zero_nmse=1.,artifact=path.relative_to(out).as_posix(),sha256=digest(path))

        with ThreadPoolExecutor(max_workers=workers) as pool:
            result = list(pool.map(one, ((window,layout) for window in plan['windows'] for layout in layouts)))
        cases.extend(result)
        shots.append(dict(ffid=fid,cases=len(result),nmse=float(np.mean([r['nmse'] for r in result])),
                          idw_nmse=float(np.mean([r['idw_nmse'] for r in result]))))
        print('Viking',shots[-1],flush=True)
    if len(cases) != 960 or digest(model_path) != model_sha:
        raise ValueError('Incomplete cases or changed frozen transfer model')
    write_rows(out/'recovery.csv',cases)
    write_rows(out/'shot_summary.csv',shots)
    mean = float(np.mean([r['nmse'] for r in shots]))
    idw = float(np.mean([r['idw_nmse'] for r in shots]))
    positive = sum(r['nmse']<1 for r in shots)
    passed = mean<min(1,idw) and positive>=plan['minimum_positive_shots']
    summary = dict(status='passed',performance_status='passed' if passed else 'failed',shots=len(shots),
                   cases=len(cases),positive_shots=positive,mean_nmse=mean,mean_idw_nmse=idw,zero_nmse=1.)
    dump(out/'performance.json',summary)
    seal(out, PLAN, inputs, ['recovery.csv', 'shot_summary.csv', 'performance.json'], initial_sources)
    write_verification(out, verify(out))
    print(json.dumps(summary,indent=2))


def verify(out):
    out = Path(out)
    plan = check_manifest(out)
    registered = json.loads(REGISTRY.read_text())
    acquisitions, cases = {}, []
    for shot in registered['shots']:
        xy, _, data = gather(shot['ffid'], shot, plan)
        acquisitions[shot['ffid'], 0] = xy, data
    expected = {(fid, 0, start, h, round(120*fraction), seed, method)
                for fid, (_, start, _), h, fraction, seed, method in product(
                    plan['ffids'], plan['windows'], plan['hidden_seeds'], plan['keep_fractions'],
                    plan['layout_seeds'], ('Jitter', 'RandomBudget', 'SA', 'Hybrid'))}
    windows = {start: stop for _, start, stop in plan['windows']}
    for row in rows(out/'recovery.csv'):
        fid, start, h, count, seed = (int(row[k]) for k in
                                      ('ffid', 'start', 'hidden_seed', 'receivers', 'layout_seed'))
        key = fid, 0, start, h, count, seed, row['method']
        cases.append((key, row['artifact'], row['sha256'], h, count, start, windows[start],
                      row['nmse'], row['idw_nmse'], {}))
    return audit(out, acquisitions, cases, expected)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=current_results()/'viking_field_transfer')
    parser.add_argument('--model-study', type=Path, default=current_results()/'field_recovery_v2')
    parser.add_argument('--workers', type=int, default=6, choices=range(1, 17))
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    if args.verify:
        print(json.dumps(verify(args.out), indent=2))
    else:
        run(args.out.resolve(), args.model_study.resolve(), args.workers)
