"""Pilot-assisted, coverage-controlled Viking study with a frozen multi-H/window plan."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import FIGURES, executable
from research.common.tabular import write_rows
from research.data.viking import DEFAULT_DATASET, REGISTRY, load_windows
from research.experiments.audit_joint_studies import Recorder, initial_mask, native_queries
from research.experiments.review_studies import snr
from research.recovery import CANDIDATES, select_and_refit
from research.validation.audit_numerics import query

PROTOCOL_FILE = Path(__file__).with_name('viking_v2_protocol.json')
PROTOCOL = 'viking_pilot_common_H_v2'
METHODS = ('Jitter', 'SA_Mu', 'Adaptive_Mu', 'Adaptive_Gap', 'Adaptive_Signal')


def bits(mask):
    return ''.join(str(int(v)) for v in mask)


def pilot_mask(eligible, count):
    candidates = np.flatnonzero(eligible)
    if count < 2 or count > len(candidates):
        raise ValueError('Invalid pilot count')
    selected = []
    for target in np.linspace(candidates[0], candidates[-1], count):
        choices = [j for j in candidates if j not in selected]
        selected.append(min(choices, key=lambda j: (abs(j-target), j)))
    result = np.zeros(len(eligible), bool); result[selected] = True
    return result


def feasible_initial(eligible, pilot, m, seed, cap):
    if m < sum(pilot) or m > sum(eligible) or np.any(pilot & ~eligible):
        raise ValueError('Pilot/count/eligibility mismatch')
    remaining = eligible & ~pilot
    for attempt in range(1000):
        result = initial_mask(len(eligible), m-int(sum(pilot)), seed+1009*attempt, remaining) | pilot
        kept = np.flatnonzero(result)
        if np.max(np.diff(np.r_[0, kept, len(result)-1]))*25 <= cap:
            return result, attempt+1
    raise ValueError('No coverage-feasible stratified initial mask found; change the declared plan, not outer scores')


def signal_weights(pilot_values, pilot_indices, n, band=(5., 60.), floor=.25):
    """Pilot-only dirty spatial power; weights describe a PSF proxy, not a recovery guarantee."""
    values = np.asarray(pilot_values, float); indices = np.asarray(pilot_indices, int)
    if (values.ndim != 2 or len(values) < 32 or values.shape[1] != len(indices) or len(indices) < 2 or n < 2 or
            len(set(indices)) != len(indices) or np.any(indices < 0) or np.any(indices >= n) or
            not np.isfinite(values).all() or not 0 < floor <= 1 or not 0 <= band[0] < band[1] <= 125):
        raise ValueError('Invalid pilot spectrum input')
    values = values-values.mean(axis=0, keepdims=True)
    transformed = np.fft.rfft(values*np.hanning(len(values))[:, None], axis=0)
    frequencies = np.fft.rfftfreq(len(values), .004)
    keep = (frequencies >= band[0]) & (frequencies <= band[1])
    phase = np.exp(-2j*np.pi*np.outer(indices, np.arange(n))/n)
    power = np.sum(abs(transformed[keep]@phase)**2, axis=0)
    normalized = power/max(float(power.max()), 1e-30) if power.max() > 1e-30 else np.ones(n)
    weights = np.array([floor+(1-floor)*np.max(np.sqrt(normalized*np.roll(normalized, d))) for d in range(1, n)])
    weights /= weights.max()
    weights = np.clip(weights, floor, 1.)
    return weights, dict(pilot_indices=indices.tolist(), temporal_band_hz=list(band), floor=floor,
                         power=power.tolist(), weights=weights.tolist(),
                         definition='max_d w[d]*abs(DFT(mask)[d])/m; raw_w[d]=floor+(1-floor)*max_q sqrt(p[q]*p[q+d]); w=raw_w/max(raw_w)')


def make_plan(args):
    plan = json.loads(PROTOCOL_FILE.read_text(encoding='utf8'))
    for key, value in [('ffids', args.shots), ('window_starts', args.windows), ('counts', args.counts),
                       ('seeds', getattr(args, 'seeds', None))]:
        if value is not None:
            plan[key] = list(value)
    if args.hidden_sets is not None:
        wanted = set(args.hidden_sets)
        if wanted-{r['id'] for r in plan['hidden_sets']}:
            raise ValueError('Unknown registered hidden set')
        plan['hidden_sets'] = [r for r in plan['hidden_sets'] if r['id'] in wanted]
    if args.budget is not None:
        plan['budget'] = args.budget
    if args.inner_folds is not None:
        plan['inner_folds'] = args.inner_folds
    for field in ['ffids', 'window_starts', 'counts', 'seeds']:
        if not plan[field] or len(set(plan[field])) != len(plan[field]):
            raise ValueError('Plan lists must be nonempty and distinct: '+field)
    if plan['budget'] < 1 or plan['inner_folds'] not in (1, 2) or args.workers < 1:
        raise ValueError('Invalid evaluation budget, folds or workers')
    if any(not isinstance(seed, int) or seed < 0 or seed >= 2**32-1 for seed in plan['seeds']):
        raise ValueError('Seeds must be unsigned 32-bit integers leaving room for seed+1')
    if any(str(m) not in plan['hard_gap_m'] or m <= plan['pilot_count']+2 for m in plan['counts']):
        raise ValueError('Counts must use registered gap limits and leave inner validation observations')
    plan['recovery_candidates'] = list(CANDIDATES)
    plan['protocol_file_sha256'] = digest(PROTOCOL_FILE)
    return plan


def prepare_search(recorder, windows, plan):
    metadata, lines, bindings = {}, [], []
    folder = recorder.out/'features'; folder.mkdir()
    for hset in plan['hidden_sets']:
        hidden = np.zeros(plan['n'], bool); hidden[hset['indices']] = True
        pilot = pilot_mask(~hidden, plan['pilot_count'])
        features = {}
        for start, shots in windows.items():
            for ffid, shot in shots.items():
                weights, info = signal_weights(shot.data[:, pilot], np.flatnonzero(pilot), plan['n'],
                                               plan['signal_temporal_band_hz'], plan['weight_floor'])
                key = f"f{ffid}_w{start}_h{hset['id']}"
                np.savez_compressed(folder/(key+'.npz'), observations=shot.data[:, pilot], pilot=pilot)
                info['pilot_observations_sha256'] = digest(folder/(key+'.npz'))
                (folder/(key+'.json')).write_text(json.dumps(info, indent=2)+'\n', encoding='utf8')
                features[start, ffid] = (weights, 'features/'+key+'.json')
        for m in plan['counts']:
            cap = plan['hard_gap_m'][str(m)]
            for seed in plan['seeds']:
                initial, attempts = feasible_initial(~hidden, pilot, m, seed, cap)
                for start, shots in windows.items():
                    for ffid in shots:
                        for method in METHODS:
                            prefix = f"h{hset['id']}_m{m}_s{seed}_{method}"
                            identifier = f'f{ffid}_w{start}_'+prefix if method == 'Adaptive_Signal' else prefix
                            if identifier not in metadata:
                                weighted = method == 'Adaptive_Signal'; bounded = method in ['Adaptive_Gap', 'Adaptive_Signal']
                                weights, feature_path = features[start, ffid] if weighted else (None, '')
                                native_method = 'measure' if method == 'Jitter' else 'SA_Budget' if method == 'SA_Mu' else 'Adaptive'
                                lines.append(query(identifier, initial, np.arange(plan['n'])*25., native_method,
                                    seed+1, plan['budget'], hard_m=cap if bounded else 0., eligible=~hidden,
                                    frequency_weights=weights, required=pilot))
                                metadata[identifier] = dict(method=method, hidden_set=hset['id'], m=m, seed=seed,
                                    initial_mask=bits(initial), initial_attempts=attempts, pilot_bits=bits(pilot), H_bits=bits(hidden),
                                    hard_gap_m=cap if bounded else 0., feature_path=feature_path,
                                    objective_kind='pilot_weighted_PSF' if weighted else 'full_Fourier_mu')
                            case = f"f{ffid}_w{start}_h{hset['id']}_m{m}_s{seed}_{method}"
                            bindings.append(dict(case=case, ffid=ffid, window_start=start, hidden_set=hset['id'],
                                                 m=m, seed=seed, method=method, search_id=identifier))
    (recorder.out/'search_metadata.json').write_text(json.dumps(metadata, indent=2)+'\n', encoding='utf8')
    write_rows(recorder.out/'bindings.csv', bindings)
    measured = native_queries(recorder, 'search', lines)
    if len(measured) != len(metadata) or {r['id'] for r in measured} != set(metadata):
        raise ValueError('Incomplete or duplicate native search results')
    search = []
    for row in measured:
        info = metadata[row['id']]; mask = np.array(list(row['mask']), int).astype(bool)
        pilot = np.array(list(info['pilot_bits']), int).astype(bool); hidden = np.array(list(info['H_bits']), int).astype(bool)
        if (sum(mask) != info['m'] or np.any(mask & hidden) or not np.all(mask[pilot]) or
                int(row['evals']) > plan['budget'] or info['hard_gap_m'] and float(row['max_gap_m']) > info['hard_gap_m']+1e-6):
            raise ValueError('Search violated count, pilot, hidden set, gap or budget')
        spectrum = abs(np.fft.fft(mask)[1:])/sum(mask)
        weights = np.ones(len(mask)-1) if not info['feature_path'] else np.array(json.loads((recorder.out/info['feature_path']).read_text())['weights'])
        if abs(float(row['mu'])-spectrum.max()) > 3e-6 or abs(float(row['objective'])-np.max(spectrum*weights)) > 3e-6:
            raise ValueError('Native full/weighted Fourier scores differ from independent DFT')
        search.append(dict(row, **info))
    write_rows(recorder.out/'search.csv', search)
    return {r['id']: r for r in search}, bindings


def evaluate(recorder, shot, binding, search, plan):
    known = np.array(list(search['mask']), int).astype(bool)
    pilot = np.array(list(search['pilot_bits']), int).astype(bool)
    hidden = np.array(list(search['H_bits']), int).astype(bool)
    predictions, choice = select_and_refit(recorder, binding['case'], shot.data, shot.x, known, pilot,
                                           binding['seed']+909, plan['inner_folds'])
    folder = recorder.out/'predictions'; folder.mkdir(exist_ok=True)
    np.savez_compressed(folder/(binding['case']+'.npz'), known=known, hidden=hidden, **predictions)
    result = dict(binding, n=120, mask=search['mask'], pilot_bits=search['pilot_bits'], H_bits=search['H_bits'],
                  mu=float(search['mu']), objective=float(search['objective']), max_gap_m=float(search['max_gap_m']),
                  sample_interval_s=shot.sample_interval_s, time_origin_s=shot.time_origin_s,
                  selected=choice['selected']['candidate'], gain=choice['selected']['gain'])
    target = shot.data[:, hidden]; energy = float(np.sum(target**2))
    for name, values in dict(zero=np.zeros_like(shot.data), **predictions).items():
        if not np.isfinite(values).all() or values.shape != shot.data.shape:
            raise ValueError('Invalid recovery prediction')
        result[name+'_snr'] = float(snr(shot.data, values, hidden))
        result[name+'_nmse'] = float(np.sum((values[:, hidden]-target)**2)/energy)
    return result


def isolation_checks(recorder, windows, search, bindings, plan):
    binding = next(r for r in bindings if r['method'] == 'Adaptive_Signal')
    row = search[binding['search_id']]; shot = windows[binding['window_start']][binding['ffid']]
    pilot = np.array(list(row['pilot_bits']), int).astype(bool)
    changed = shot.data.copy(); changed[:, ~pilot] = 13*changed[:, ~pilot]+7
    before = signal_weights(shot.data[:, pilot], np.flatnonzero(pilot), plan['n'], plan['signal_temporal_band_hz'], plan['weight_floor'])[0]
    after = signal_weights(changed[:, pilot], np.flatnonzero(pilot), plan['n'], plan['signal_temporal_band_hz'], plan['weight_floor'])[0]
    hidden = np.array(list(row['H_bits']), int).astype(bool)
    replay = query('pilot_isolation', np.array(list(row['initial_mask']), int), np.arange(120)*25., 'Adaptive',
                   binding['seed']+1, plan['budget'], hard_m=row['hard_gap_m'], eligible=~hidden,
                   frequency_weights=after, required=pilot)
    replayed = native_queries(recorder, 'search_isolation', [replay])[0]
    changed = shot.data.copy(); changed[:, hidden] = 13*changed[:, hidden]+7
    known = np.array(list(row['mask']), int).astype(bool)
    models, choice = select_and_refit(recorder, 'isolation_'+binding['case'], changed, shot.x, known, pilot,
                                      binding['seed']+909, plan['inner_folds'])
    original_choice = json.loads((recorder.out/'selection'/(binding['case']+'.json')).read_text())
    with np.load(recorder.out/'predictions'/(binding['case']+'.npz')) as saved:
        error = max(float(np.max(abs(saved[name]-values))) for name, values in models.items())
    report = dict(nonpilot_feature_max_difference=float(np.max(abs(before-after))), search_mask_unchanged=replayed['mask'] == row['mask'],
                  selection_unchanged=choice == original_choice, prediction_max_difference=error)
    report['passed'] = report['nonpilot_feature_max_difference'] == 0 and report['search_mask_unchanged'] and report['selection_unchanged'] and error == 0
    (recorder.out/'isolation.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf8')
    if not report['passed']:
        raise AssertionError('Pilot-assisted search or recovery read forbidden amplitudes')
    return report


def run(args):
    output = args.out.resolve()
    if output.exists():
        raise FileExistsError(output)
    plan = make_plan(args)
    group = args.group or 'research/'+output.name
    figures = (FIGURES/group).resolve()
    if not figures.is_relative_to(FIGURES.resolve()) or (figures.exists() and any(figures.iterdir())):
        raise ValueError('Use a fresh figure group inside figures/')
    windows, provenance = load_windows(plan['window_starts'], args.dataset, plan['ffids'])
    config = dict(plan=plan, dataset_provenance=provenance, independent_surveys=1,
                  pilot_cost_included_in_retained_count=True, pilot_in_all_inner_train_sets=True,
                  worker_count=args.workers,
                  source_status=provenance['status'], statistical_scope='same-survey descriptive development experiment')
    recorder = Recorder(output, args.exe, config, protocol=PROTOCOL)
    try:
        (output/'frozen_plan.json').write_text(json.dumps(plan, indent=2)+'\n', encoding='utf8')
        recorder.state['plan_sha256'] = digest(output/'frozen_plan.json'); recorder.save()
        search, bindings = prepare_search(recorder, windows, plan)
        with ThreadPoolExecutor(max_workers=args.workers) as pool:
            records = list(pool.map(lambda b: evaluate(recorder, windows[b['window_start']][b['ffid']], b, search[b['search_id']], plan), bindings))
        write_rows(output/'recovery.csv', records)
        isolation = isolation_checks(recorder, windows, search, bindings, plan)
        for name, expected in provenance['input_sha256'].items():
            if digest(name) != expected:
                raise RuntimeError('Data changed during the batch')
        if digest(PROTOCOL_FILE) != plan['protocol_file_sha256'] or digest(REGISTRY) != provenance['registry_sha256']:
            raise RuntimeError('Protocol or dataset registry changed')
        artifacts = {p.relative_to(output).as_posix(): digest(p) for name in ['features', 'selection', 'python', 'predictions']
                     for p in (output/name).rglob('*') if p.is_file()}
        recorder.state.update(status='passed', recovery_cases=len(records), search_cases=len(search), native_calls=recorder.count,
                              isolation=isolation, derived_artifacts_sha256=artifacts,
                              search_metadata_sha256=digest(output/'search_metadata.json'),
                              isolation_sha256=digest(output/'isolation.json'))
        recorder.finish()
    except Exception as error:
        recorder.state.update(status='failed', completed=False, error=str(error)); recorder.save()
        raise
    from research.analysis.viking_v2_results import run as analyze
    analysis = analyze(output, output/'analysis', group)
    print(json.dumps(dict(status='passed', output=str(output), cases=len(records), native_calls=recorder.count,
                          figure_directory=analysis['figure_directory'])))
    return recorder.state


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--dataset', type=Path, default=DEFAULT_DATASET)
    parser.add_argument('--exe', type=Path, default=executable())
    parser.add_argument('--shots', type=int, nargs='+')
    parser.add_argument('--windows', type=int, nargs='+')
    parser.add_argument('--counts', type=int, nargs='+')
    parser.add_argument('--seeds', type=int, nargs='+', help='distinct paired mask seeds, frozen before scoring')
    parser.add_argument('--hidden-sets', nargs='+')
    parser.add_argument('--budget', type=int)
    parser.add_argument('--inner-folds', type=int, choices=[1, 2])
    parser.add_argument('--workers', type=int, default=4)
    parser.add_argument('--group')
    run(parser.parse_args())


if __name__ == '__main__':
    main()
