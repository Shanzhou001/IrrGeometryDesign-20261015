"""Fit and validate the existing geometry-aware bank on the dense GVDA grid."""
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import argparse
from itertools import product
import json

import numpy as np

from research.common.paths import ROOT, current_results
from research.data.gvda import DATA, DT, read as gather

from research.common.hashing import digest
from research.common.tabular import rows, write_rows
from research.experiments.field_layouts import constrained_layouts, spatial_cells
from research.experiments.field_recovery import dump
from research.experiments.mapped_field import hidden_mask
from research.recovery_geometry import combine, fit, predictors
from research.validation.field_artifacts import audit, check_manifest, seal, source_snapshot, write_verification

PROTOCOL = Path(__file__).with_name('gvda_protocol.json')
CONFIRMATION = PROTOCOL.with_name('gvda_confirmation_protocol.json')


def make_layouts(xy, plan):
    layouts = []
    for h in plan['hidden_seeds']:
        hidden = hidden_mask(len(xy), h)
        for fraction in plan['keep_fractions']:
            count = round(fraction * len(xy))
            for seed in plan['layout_seeds']:
                for method, known, metrics in constrained_layouts(
                        xy, hidden, count, seed, plan['layout_budget'], plan['hybrid_prefix']):
                    layouts.append((h, count, seed, method, known, hidden, metrics))
    return layouts


def training(plan):
    equations, inputs = defaultdict(dict), {}
    for item in plan['development']:
        fid=item['source']
        xy,source,data,windows,onset,verified = gather(item,plan)
        inputs.update(verified)
        for h in plan['hidden_seeds']:
            hidden=hidden_mask(len(xy),h)
            for fraction in plan['keep_fractions']:
                cells=spatial_cells(xy,np.flatnonzero(~hidden),round(fraction*len(xy)))
                for seed in plan['layout_seeds']:
                    rng=np.random.default_rng(seed)
                    known=np.zeros(len(xy),bool)
                    known[[rng.choice(cell) for cell in cells]]=True
                    for w,(start,stop) in enumerate(windows):
                        features,_,strata=predictors(data[:,known],xy,source,known,hidden,start,stop,DT)
                        y=data[start:stop,hidden]
                        energy=float(np.sum(y**2))
                        if energy<=0:
                            raise ValueError('Zero energy in a fixed development window')
                        for g in range(3):
                            selected=strata==g
                            if not selected.any():
                                continue
                            x=features[:,:,selected].reshape(len(features),-1)
                            target=y[:,selected].ravel()
                            a,b,n=equations[f'{w}_{g}'].setdefault(fid, (np.zeros((len(x),len(x))),np.zeros(len(x)),np.zeros(())))
                            a+=x@x.T/energy
                            b+=x@target/energy
                            n+=target@target/energy
        print('development',fid,'source_time',onset,flush=True)
    return {key:fit(group.values()) for key,group in sorted(equations.items())},inputs


def score(item, plan, model, out, workers=6, layouts=None):
    fid=item['source']
    xy,source,data,windows,onset,inputs=gather(item,plan)
    layouts = layouts if layouts is not None else make_layouts(xy, plan)

    def one(job):
        w,(start,stop),(h,count,seed,method,known,hidden,metrics)=job
        measured=data[:,known]
        x,idw,strata=predictors(measured,xy,source,known,hidden,start,stop,DT)
        prediction=combine(x,strata,[model[f'{w}_{g}'] for g in range(3)])
        truth=data[start:stop,hidden]
        energy=float(np.sum(truth**2))
        if energy<=0 or not np.isfinite(prediction).all():
            raise ValueError('Invalid recorded target or prediction')
        path=out/'predictions'/f's{fid}_w{w}_h{h}_m{count}_s{seed}_{method}.npz'
        arrays = dict(prediction=prediction, idw=idw, truth=truth, known=known, hidden=hidden, xy=xy)
        extra = {}
        if plan.get('secondary'):
            missing = ~known
            features, baseline, groups = predictors(measured, xy, source, known, missing, start, stop, DT)
            full_prediction = combine(features, groups, [model[f'{w}_{g}'] for g in range(3)])
            full_truth = data[start:stop, missing]
            full_energy = float(np.sum(full_truth ** 2))
            arrays.update(prediction_all=full_prediction, idw_all=baseline, truth_all=full_truth)
            extra.update(all_nmse=float(np.sum((full_prediction-full_truth)**2)/full_energy),
                         all_idw_nmse=float(np.sum((baseline-full_truth)**2)/full_energy))
        np.savez_compressed(path, **arrays)
        return dict(source=fid,window=w,first_sample=start,last_sample_exclusive=stop,source_time=onset,
                    hidden_seed=h,receivers=count,layout_seed=seed,method=method,**metrics,
                    nmse=float(np.sum((prediction-truth)**2)/energy),idw_nmse=float(np.sum((idw-truth)**2)/energy),
                    zero_nmse=1.,prediction=path.relative_to(out).as_posix(),sha256=digest(path),**extra)

    with ThreadPoolExecutor(max_workers=workers) as pool:
        cases=list(pool.map(one,((w,window,layout) for w,window in enumerate(windows) for layout in layouts)))
    if len(cases) != 2 * len(layouts):
        raise ValueError('Incomplete source scoring cases')
    shot=dict(source=fid,cases=len(cases),nmse=float(np.mean([r['nmse'] for r in cases])),
              idw_nmse=float(np.mean([r['idw_nmse'] for r in cases])))
    print('holdout',shot,flush=True)
    return cases,shot,inputs


def run(out, workers=6, protocol=PROTOCOL, model_study=None):
    initial_sources=source_snapshot()
    out=Path(out)
    plan=json.loads(protocol.read_text())
    if protocol == CONFIRMATION and model_study is None:
        raise ValueError('Prospective confirmation requires the previously frozen model')
    if (plan['status'] != 'frozen' or
            {r['source'] for r in plan['development']} & {r['source'] for r in plan['holdout']}):
        raise ValueError('Development and registered confirmation overlap')
    for name,h in plan['geometry_sha256'].items():
        if digest(DATA/name) != h:
            raise ValueError('Registered geometry changed')
    out.mkdir(parents=True)
    (out/'predictions').mkdir()
    dump(out/'protocol.json',plan)
    if model_study is None:
        model,inputs=training(plan)
    else:
        model_path=Path(model_study)/'frozen_model.json'
        if digest(model_path)!=plan['model_sha256']:
            raise ValueError('Prospective model differs from frozen registration')
        model=json.loads(model_path.read_text())
        inputs={model_path.relative_to(ROOT).as_posix():digest(model_path)}
    dump(out/'frozen_model.json',model)
    model_sha=digest(out/'frozen_model.json')
    cases,shots=[],[]
    layouts=None
    for item in plan['holdout']:
        if layouts is None:
            xy,*_=gather(item,plan)
            layouts=make_layouts(xy,plan)
        records,shot,used=score(item,plan,model,out,workers,layouts)
        cases.extend(records)
        shots.append(shot)
        inputs.update(used)
    if digest(out/'frozen_model.json')!=model_sha:
        raise ValueError('Frozen model changed during evaluation')
    write_rows(out/'recovery.csv',cases)
    write_rows(out/'shot_summary.csv',shots)
    mean=float(np.mean([r['nmse'] for r in shots]))
    idw=float(np.mean([r['idw_nmse'] for r in shots]))
    positive=sum(r['nmse']<1 for r in shots)
    passed=mean<min(1,idw) and positive>=plan['minimum_positive_shots']
    summary=dict(status='passed',performance_status='passed' if passed else 'failed',cases=len(cases),
                 positive_shots=positive,shots=len(shots),mean_nmse=mean,mean_idw_nmse=idw,zero_nmse=1.)
    dump(out/'performance.json',summary)
    seal(out, protocol, inputs, ['recovery.csv', 'shot_summary.csv', 'performance.json'], initial_sources)
    write_verification(out, verify(out))
    print(json.dumps(summary,indent=2))
    return summary


def verify(out):
    out = Path(out)
    plan = check_manifest(out)
    acquisitions, expected, cases, registered_windows = {}, set(), [], {}
    for item in plan['holdout']:
        xy, _, data, windows, _, _ = gather(item, plan)
        fid = item['source']
        acquisitions[fid, 0] = xy, data
        registered_windows.update({(fid, w): window for w, window in enumerate(windows)})
        for w, h, fraction, seed, method in product(
                range(2), plan['hidden_seeds'], plan['keep_fractions'], plan['layout_seeds'],
                ('Jitter', 'RandomBudget', 'SA', 'Hybrid')):
            expected.add((fid, 0, w, h, round(fraction*len(xy)), seed, method))
    for row in rows(out / 'recovery.csv'):
        if plan.get('secondary') and any(k not in row for k in ('all_nmse', 'all_idw_nmse')):
            raise ValueError('Prospective confirmation lacks full missing-station scores')
        fid, w, h, count, seed = (int(row[k]) for k in
                                  ('source', 'window', 'hidden_seed', 'receivers', 'layout_seed'))
        key = fid, 0, w, h, count, seed, row['method']
        start, stop = int(row['first_sample']), int(row['last_sample_exclusive'])
        if (start, stop) != registered_windows[fid, w]:
            raise ValueError('Source-force-aligned scoring window changed')
        cases.append((key, row['prediction'], row['sha256'], h, count,
                      start, stop, row['nmse'], row['idw_nmse'],
                      {k: row[k] for k in ('all_nmse', 'all_idw_nmse') if k in row}))
    return audit(out, acquisitions, cases, expected)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=current_results() / 'gvda_spatial_recovery')
    parser.add_argument('--workers', type=int, default=6, choices=range(1, 17))
    parser.add_argument('--confirmation', action='store_true')
    parser.add_argument('--model-study', type=Path)
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    if args.verify:
        print(json.dumps(verify(args.out), indent=2))
    else:
        run(args.out.resolve(), args.workers, CONFIRMATION if args.confirmation else PROTOCOL, args.model_study)
