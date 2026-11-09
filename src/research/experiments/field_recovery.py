"""Develop and validate physical-coordinate recovery on disjoint field records."""
import argparse
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT, current_results
from research.common.tabular import rows, write_rows
from research.data.mapped_shots import SODA, read_shots, soda_waveform
from research.experiments.field_layouts import constrained_layouts, spatial_cells
from research.experiments.mapped_field import PROTOCOL as BASELINE, hidden_mask
from research.recovery_geometry import combine, fit, predictors


PROTOCOL = Path(__file__).with_name('field_recovery_protocol.json')


def dump(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n', encoding='utf-8')


def input_hashes(plan, development_only=False):
    hashes = json.loads(BASELINE.read_text())['input_sha256']
    registration = SODA / 'additional/selection.json'
    split = json.loads(registration.read_text())
    if (split['development'] != plan['soda_development'] or split['holdout'] != plan['soda_holdout'] or
            any(set(plan[f'{s}_development']) & set(plan[f'{s}_holdout']) for s in ('poland', 'soda'))):
        raise ValueError('Development and holdout identities differ from the registered split')
    hashes[registration.relative_to(ROOT).as_posix()] = digest(registration)
    ids = plan['soda_development'] + ([] if development_only else plan['soda_holdout'])
    for fid in ids:
        if fid == 7733:
            continue
        path = soda_waveform(fid)
        receipt = path.with_name(path.name + '.json')
        hashes[path.relative_to(ROOT).as_posix()] = json.loads(receipt.read_text())['sha256']
        hashes[receipt.relative_to(ROOT).as_posix()] = digest(receipt)
    for name, expected in hashes.items():
        if digest(ROOT / name) != expected:
            raise ValueError(f'Input identity changed: {name}')
    return hashes


def train(plan):
    blocks = defaultdict(dict)
    for survey, fid, component, xy, source, dt, data in read_shots(plan, 'development'):
        for seed in plan['hidden_seeds']:
            hidden = hidden_mask(len(xy), seed)
            for fraction in plan['keep_fractions']:
                cells = spatial_cells(xy, np.flatnonzero(~hidden), round(fraction * len(xy)))
                for layout_seed in plan['layout_seeds']:
                    rng = np.random.default_rng(layout_seed)
                    known = np.zeros(len(xy), bool)
                    known[[rng.choice(cell) for cell in cells]] = True
                    for start, stop in plan['windows']:
                        features, _, strata = predictors(data[:, known], xy, source, known, hidden, start, stop, dt)
                        truth = data[start:stop, hidden]
                        normalization = max(float(np.sum(truth ** 2)), 1e-30)
                        for group in range(3):
                            x = features[:, :, strata == group].reshape(len(features), -1)
                            y = truth[:, strata == group].ravel()
                            if not len(y):
                                continue
                            key = f'{survey}_{component}_{start}_{group}'
                            a, b, energy = blocks[key].setdefault(fid, (np.zeros((len(x), len(x))), np.zeros(len(x)), np.zeros(())))
                            a += x @ x.T / normalization
                            b += x @ y / normalization
                            energy += float(y @ y) / normalization
        print('developed', survey, fid, component, flush=True)
    return {key: fit(shots.values()) for key, shots in sorted(blocks.items())}


def summarize(records, plan):
    grouped = defaultdict(list)
    for row in records:
        grouped[row['survey'], int(row['ffid'])].append(row)
    shots = []
    for (survey, fid), group in sorted(grouped.items()):
        shots.append(dict(survey=survey, ffid=fid, cases=len(group),
                          nmse=float(np.mean([float(r['nmse']) for r in group])),
                          idw_nmse=float(np.mean([float(r['idw_nmse']) for r in group]))))
    summaries = {}
    for survey in ('poland', 'soda'):
        subset = [r for r in shots if r['survey'] == survey]
        if {r['ffid'] for r in subset} != set(plan[f'{survey}_holdout']):
            raise ValueError('A fixed holdout shot is missing')
        mean = float(np.mean([r['nmse'] for r in subset]))
        idw = float(np.mean([r['idw_nmse'] for r in subset]))
        positive = sum(r['nmse'] < 1 for r in subset)
        summaries[survey] = dict(shots=len(subset), positive_shots=positive, nmse=mean, zero_nmse=1., idw_nmse=idw,
                                 passed=mean < min(1, idw) and positive >= plan['minimum_positive_shots'])
    return shots, summaries


def report(out, shots, summaries, records):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    pos = np.arange(len(shots))
    fig, ax = plt.subplots(figsize=(10, 4.5), layout='constrained')
    ax.bar(pos - .2, [r['nmse'] for r in shots], .4, label='Geometry-aware')
    ax.bar(pos + .2, [r['idw_nmse'] for r in shots], .4, label='Coordinate IDW')
    ax.axhline(1, color='black', linewidth=1, linestyle='--', label='Zero fill')
    ax.set(xticks=pos, xticklabels=[f"{r['survey']}\n{r['ffid']}" for r in shots], ylabel='Shot mean raw hidden-trace NMSE')
    ax.legend()
    fig.savefig(out / 'absolute_nmse.png', dpi=150)
    plt.close(fig)
    by_layout = defaultdict(list)
    for row in records:
        by_layout[row['survey'], row['ffid'], row['m'], row['method']].append(row['nmse'])
    write_rows(out / 'layout_summary.csv', [dict(survey=s, ffid=f, m=m, layout=method, nmse=float(np.mean(v)))
               for (s, f, m, method), v in sorted(by_layout.items())])
    table = ''.join(f"<tr><td>{s}</td><td>{r['positive_shots']}/{r['shots']}</td><td>{r['nmse']:.5f}</td>"
                    f"<td>{r['idw_nmse']:.5f}</td><td>{'pass' if r['passed'] else 'fail'}</td></tr>"
                    for s, r in summaries.items())
    (out / 'report.html').write_text('''<!doctype html><meta charset="utf-8"><title>Surveyed prestack recovery</title>
<h1>Surveyed prestack recovery on fixed holdout shots</h1>
<p>Raw hidden-trace NMSE, zero fill = 1. Models fitted on separate development shots.
All layouts enforce one retained receiver per physical cell. Shot averages are descriptive within each survey.</p>
<table><tr><th>Survey</th><th>Shots beating zero</th><th>NMSE</th><th>IDW NMSE</th><th>Performance gate</th></tr>'''
        + table + '</table><p><img src="absolute_nmse.png" alt="Absolute NMSE by shot" style="max-width:100%"></p>'
        '<p><a href="shot_summary.csv">Shot summary</a> · <a href="layout_summary.csv">Layout summary</a> · '
        '<a href="recovery.csv">All cases</a></p>', encoding='utf-8')


def evaluate(out, plan, model, workers):
    records = []
    layouts = {}
    (out / 'predictions').mkdir()
    for survey, fid, component, xy, source, dt, data in read_shots(plan, 'holdout'):
        shot = survey, fid
        if shot not in layouts:
            layouts[shot] = []
            for h in plan['hidden_seeds']:
                hidden = hidden_mask(len(xy), h)
                for fraction in plan['keep_fractions']:
                    count = round(fraction * len(xy))
                    for seed in plan['layout_seeds']:
                        for method, known, metrics in constrained_layouts(xy, hidden, count, seed, plan['layout_budget'], plan['hybrid_prefix']):
                            layouts[shot].append((h, count, seed, method, known, hidden, metrics))
        jobs = [(start, stop, layout) for start, stop in plan['windows'] for layout in layouts[shot]]

        def one(job):
            start, stop, (h, count, seed, method, known, hidden, metrics) = job
            # Extraction precedes prediction. The predictor receives only observed columns.
            features, idw, strata = predictors(data[:, known], xy, source, known, hidden, start, stop, dt)
            models = [model[f'{survey}_{component}_{start}_{g}'] for g in range(3)]
            result = combine(features, strata, models)
            truth = data[start:stop, hidden]
            energy = float(np.sum(truth ** 2))
            if energy <= 0 or not np.isfinite(result).all():
                raise ValueError('Invalid fixed score target or prediction')
            case = f'{survey}_{fid}_c{component}_w{start}_h{h}_m{count}_s{seed}_{method}'
            target = out / 'predictions' / f'{case}.npz'
            np.savez_compressed(target, prediction=result, idw=idw, truth=truth, known=known, hidden=hidden, xy=xy)
            nmse = float(np.sum((result - truth) ** 2) / energy)
            idw_nmse = float(np.sum((idw - truth) ** 2) / energy)
            return dict(case=case, survey=survey, ffid=fid, component=component, start=start, h=h, m=count, seed=seed,
                        method=method, **metrics, nmse=nmse, snr=-10 * np.log10(max(nmse, 1e-30)), idw_nmse=idw_nmse,
                        zero_nmse=1., prediction_sha256=digest(target))

        with ThreadPoolExecutor(max_workers=workers) as pool:
            records.extend(pool.map(one, jobs))
        print('evaluated', survey, fid, component, flush=True)
    return records


def verify(out):
    state = json.loads((out / 'manifest.json').read_text())
    plan = json.loads(PROTOCOL.read_text())
    if state['protocol_sha256'] != digest(PROTOCOL):
        raise ValueError('Recovery protocol changed')
    for name, expected in state['input_sha256'].items():
        if digest(ROOT / name) != expected:
            raise ValueError(f'Recovery input changed: {name}')
    for name, expected in state['source_sha256'].items():
        if digest(ROOT / name) != expected:
            raise ValueError(f'Recovery source changed: {name}')
    for name, expected in state['output_sha256'].items():
        if digest(out / name) != expected:
            raise ValueError(f'Recovery result changed: {name}')
    records = rows(out / 'recovery.csv')
    seen = set()
    acquisitions = {(s, fid, c): (xy, data) for s, fid, c, xy, _, _, data in read_shots(plan, 'holdout')}
    windows = dict(plan['windows'])
    expected = {(s, fid, component, a, h, round(f * n), seed, method)
                for s, n, components in [('poland', plan['poland_receivers'], (0,)), ('soda', plan['soda_receivers'], (1, 2, 3))]
                for fid in plan[f'{s}_holdout'] for component in components for a, _ in plan['windows']
                for h in plan['hidden_seeds'] for f in plan['keep_fractions'] for seed in plan['layout_seeds']
                for method in ('Jitter', 'RandomBudget', 'SA', 'Hybrid')}
    for row in records:
        key = (row['survey'], *(int(row[k]) for k in ('ffid', 'component', 'start', 'h', 'm', 'seed')), row['method'])
        if key in seen or key not in expected:
            raise ValueError('Duplicate or unexpected recovery case')
        seen.add(key)
        path = out / 'predictions' / (row['case'] + '.npz')
        if digest(path) != row['prediction_sha256']:
            raise ValueError('Recovery prediction changed')
        with np.load(path, allow_pickle=False) as saved:
            truth, known, hidden, xy = (saved[k] for k in ('truth', 'known', 'hidden', 'xy'))
            physical_xy, samples = acquisitions[key[:3]]
            if not np.array_equal(xy, physical_xy) or not np.array_equal(truth, samples[key[3]:windows[key[3]], hidden]):
                raise ValueError('Saved field coordinates or target do not match the registered shot')
            cells = spatial_cells(xy, np.flatnonzero(~hidden), key[5])
            if (known.sum() != key[5] or np.any(known & hidden) or not np.array_equal(hidden, hidden_mask(len(xy), key[4])) or
                    any(known[cell].sum() != 1 for cell in cells)):
                raise ValueError('Layout constraint or holdout changed')
            distances = np.linalg.norm(xy[:, None] - xy[known][None, :], axis=2)
            max_distance = float(distances.min(axis=1).max())
            if (abs(max_distance - float(row['max_nearest_distance'])) > 1e-9 or
                    max_distance > float(row['cell_cover_bound']) + 1e-9):
                raise ValueError('Reported spatial coverage differs')
            for column, values in [('nmse', saved['prediction']), ('idw_nmse', saved['idw'])]:
                score = float(np.sum((values - truth) ** 2) / np.sum(truth ** 2))
                if not np.isfinite(score) or abs(score - float(row[column])) > 1e-12:
                    raise ValueError('Independent prediction score differs')
    if seen != expected:
        raise ValueError('Incomplete holdout results')
    _, summary = summarize(records, plan)
    return dict(status='passed', performance_status='passed' if all(s['passed'] for s in summary.values()) else 'failed',
                cases=len(records), surveys=summary, manifest_sha256=digest(out / 'manifest.json'))


def run(out, workers, development_only=False):
    plan = json.loads(PROTOCOL.read_text())
    inputs = input_hashes(plan, development_only)
    out.mkdir(parents=True, exist_ok=False)
    dump(out / 'protocol.json', plan)
    model = train(plan)
    dump(out / 'frozen_model.json', model)
    if development_only:
        return
    records = evaluate(out, plan, model, workers)
    write_rows(out / 'recovery.csv', records)
    shots, summaries = summarize(records, plan)
    write_rows(out / 'shot_summary.csv', shots)
    dump(out / 'performance.json', summaries)
    report(out, shots, summaries, records)
    dump(out / 'manifest.json', dict(status='passed', protocol_sha256=digest(PROTOCOL), input_sha256=inputs,
         source_sha256={p: digest(ROOT / p) for p in ('src/research/experiments/field_recovery.py',
            'src/research/experiments/field_layouts.py', 'src/research/recovery_geometry.py', 'src/research/data/mapped_shots.py')},
         output_sha256={name: digest(out / name) for name in ('protocol.json', 'frozen_model.json', 'recovery.csv', 'shot_summary.csv',
            'performance.json', 'absolute_nmse.png', 'report.html', 'layout_summary.csv')}))
    result = verify(out)
    dump(out / 'verification.json', result)
    print(json.dumps(result, indent=2), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=current_results() / 'field_recovery_v2')
    parser.add_argument('--workers', type=int, default=4, choices=range(1, 17))
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--verify', action='store_true')
    group.add_argument('--development-only', action='store_true')
    args = parser.parse_args()
    if args.verify:
        print(json.dumps(verify(args.out), indent=2))
    else:
        run(args.out.resolve(), args.workers, args.development_only)
