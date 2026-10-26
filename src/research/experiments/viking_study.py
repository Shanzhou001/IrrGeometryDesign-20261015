"""Measured Viking trace selection with paired budgets and a common hidden set."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import FIGURES, executable
from research.common.tabular import write_rows
from research.data.viking import DEFAULT_DATASET, REGISTRY, load_dataset
from research.experiments.audit_joint_studies import (
    Recorder, initial_mask, native_queries, nested,
)
from research.experiments.review_studies import snr
from research.validation.audit_numerics import metrics, query

PROTOCOL = 'viking_line12_common_H_v1'
METHODS = ('Jitter', 'SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive')


def hidden_mask(n):
    hidden = np.zeros(n, dtype=bool)
    hidden[4::8] = True
    return hidden


def bits(mask):
    return ''.join('1' if v else '0' for v in mask)


def search(recorder, coordinates, counts, seeds, budget):
    hidden = hidden_mask(len(coordinates))
    lines, metadata = [], {}
    for m in counts:
        for seed in seeds:
            initial = initial_mask(len(coordinates), m, seed, ~hidden)
            for method in METHODS:
                key = f'm{m}_seed{seed}_{method}'
                lines.append(query(key, initial, coordinates, 'measure' if method == 'Jitter' else method,
                                   seed+1, budget, eligible=~hidden))
                metadata[key] = dict(method=method, seed=seed, m=m, initial_mask=bits(initial),
                                     evaluation_bits=bits(hidden), budget_cap=budget)
    measured = native_queries(recorder, 'search', lines)
    if len(measured) != len(metadata) or {r['id'] for r in measured} != set(metadata):
        raise ValueError('Native search returned missing or duplicate cases')
    result = []
    for row in measured:
        info = metadata[row['id']]
        mask = np.array(list(row['mask']), dtype=int).astype(bool)
        if (set(row['mask'])-{'0', '1'} or len(mask) != len(coordinates) or sum(mask) != info['m'] or np.any(mask & hidden) or
                not 1 <= int(row['evals']) <= budget):
            raise ValueError('Native search violated cardinality, hidden eligibility or evaluation cap')
        expected = metrics(mask, coordinates)
        if abs(float(row['mu'])-expected['mu']) > 3e-6:
            raise ValueError('Native Fourier score differs from explicit NumPy Gram construction')
        result.append(dict(row, **info))
    write_rows(recorder.out/'search.csv', result)
    return result


def evaluate_case(recorder, shot, row, iterations):
    mask = np.array(list(row['mask']), dtype=int).astype(bool)
    hidden = hidden_mask(len(mask))
    key = f"ffid{shot.ffid}_{row['id']}"
    fixed, selected, linear, choice = nested(recorder, key, shot.data, shot.x, mask,
                                            int(row['seed'])+909, iterations=iterations)
    result = dict(case=key, ffid=shot.ffid, search_id=row['id'], method=row['method'], seed=row['seed'],
                  n=len(mask), m=int(row['m']), mask=row['mask'], evaluation_bits=bits(hidden),
                  mu=float(row['mu']), evals=int(row['evals']), sample_interval_s=shot.sample_interval_s,
                  time_origin_s=shot.time_origin_s, independent_surveys=1,
                  selected=choice['selected']['candidate'], gain=choice['selected']['gain'])
    reference = shot.data[:, hidden]
    energy = float(np.sum(reference**2))
    for name, prediction in [('zero', np.zeros_like(fixed)), ('linear', linear),
                             ('fixed', fixed), ('selected', selected)]:
        if prediction.shape != shot.data.shape or not np.isfinite(prediction).all():
            raise ValueError('Invalid recovery prediction')
        error = float(np.sum((prediction[:, hidden]-reference)**2))
        result[name+'_snr'] = float(snr(shot.data, prediction, hidden))
        result[name+'_nmse'] = error/energy
    return result


def verify_isolation(recorder, shot, row, iterations):
    """Intervene on H after a completed baseline; both native inputs and selection must agree."""
    mask = np.array(list(row['mask']), dtype=int).astype(bool)
    hidden = hidden_mask(len(mask))
    baseline = f"ffid{shot.ffid}_{row['id']}"
    changed = shot.data.copy()
    changed[:, hidden] = 13*changed[:, hidden]+7
    key = 'isolation_'+baseline
    fixed, _, _, choice = nested(recorder, key, changed, shot.x, mask,
                                 int(row['seed'])+909, iterations=iterations)
    original_choice = json.loads((recorder.out/'selection'/(baseline+'.json')).read_text())
    original_fixed = np.loadtxt(recorder.out/'native'/(baseline+'_fixed')/'prediction.csv', delimiter=',')
    original_selected = np.loadtxt(recorder.out/'selected'/(baseline+'.csv'), delimiter=',')
    changed_selected = np.loadtxt(recorder.out/'selected'/(key+'.csv'), delimiter=',')
    report = dict(case=baseline, transformation='H <- 13*H+7; K unchanged',
                  selection_unchanged=original_choice == choice,
                  fixed_max_difference=float(np.max(abs(original_fixed-fixed))),
                  selected_max_difference=float(np.max(abs(original_selected-changed_selected))))
    report['passed'] = report['selection_unchanged'] and report['fixed_max_difference'] == 0 and report['selected_max_difference'] == 0
    (recorder.out/'isolation.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    if not report['passed']:
        raise AssertionError('Viking outer hidden amplitudes affected recovery or model selection')
    return report


def run(args):
    output = args.out.resolve()
    if output.exists():
        raise FileExistsError('Use a new Viking output directory: '+str(output))
    counts = tuple(args.counts)
    if (not counts or len(set(counts)) != len(counts) or args.seed_count < 1 or args.seed < 0 or
            args.iterations < 1 or args.budget < 1 or args.workers < 1):
        raise ValueError('Counts must be distinct; seeds, budget, iterations and workers must be valid')
    if args.shots is not None and len(args.shots) != len(set(args.shots)):
        raise ValueError('FFIDs must be distinct')
    shots, provenance = load_dataset(args.dataset, args.shots)
    n = len(shots[0].x)
    if any(m < 4 or m > n-sum(hidden_mask(n)) for m in counts):
        raise ValueError('Retained counts require at least four training candidates and must exclude H')
    coordinates = shots[0].x-shots[0].x[0]
    if any(not np.array_equal(shot.x-shot.x[0], coordinates) for shot in shots):
        raise ValueError('Shared geometric search requires identical relative receiver coordinates')
    if args.validate_only:
        output.mkdir(parents=True)
        (output/'dataset_audit.json').write_text(json.dumps(provenance, indent=2)+'\n', encoding='utf-8')
        print(json.dumps(dict(status='passed', scope='dataset validation only', selected_shots=len(shots))))
        return provenance
    program = args.exe.resolve()
    if not program.is_file():
        raise FileNotFoundError('Build the native application first: '+str(program))
    group = args.group or 'research/viking_'+output.name
    figure_path = (FIGURES/group).resolve()
    if not figure_path.is_relative_to(FIGURES.resolve()) or (figure_path.exists() and any(figure_path.iterdir())):
        raise ValueError('Use a new figure group inside figures/')
    seeds = [args.seed+179*j for j in range(args.seed_count)]
    configuration = dict(dataset_provenance=provenance, counts=list(counts), seeds=seeds, methods=list(METHODS),
                         budget=args.budget, iterations=args.iterations, common_H=np.flatnonzero(hidden_mask(n)).tolist(),
                         n=n, samples=256, relative_receiver_coordinates_m=coordinates.tolist(),
                         geometry_masks_shared_across_shots=True, model_selection_candidates=['zero', 'linear', 'default', 'cold'],
                         strict_preprocessing_isolation=True, independent_surveys=1,
                         analysis_scope='Descriptive within-survey comparisons; mask seeds are computational repeats')
    recorder = Recorder(output, program, configuration, protocol=PROTOCOL)
    try:
        (output/'dataset_audit.json').write_text(json.dumps(provenance, indent=2)+'\n', encoding='utf-8')
        measured = search(recorder, coordinates, counts, seeds, args.budget)
        tasks = [(shot, row) for shot in shots for row in measured]
        with ThreadPoolExecutor(max_workers=args.workers) as pool:
            records = list(pool.map(lambda task: evaluate_case(recorder, *task, args.iterations), tasks))
        write_rows(output/'recovery.csv', records)
        isolation = verify_isolation(recorder, shots[0], measured[0], args.iterations)
        for name, expected in provenance['input_sha256'].items():
            if digest(name) != expected:
                raise RuntimeError('Dataset changed during the experiment: '+name)
        if digest(REGISTRY) != provenance['registry_sha256']:
            raise RuntimeError('Dataset registry changed during the experiment')
        recorder.state.update(status='passed', recovery_cases=len(records), search_cases=len(measured),
                              native_calls=recorder.count, isolation=isolation,
                              dataset_audit_sha256=digest(output/'dataset_audit.json'),
                              isolation_sha256=digest(output/'isolation.json'))
        recorder.finish()
    except Exception as error:
        recorder.state.update(status='failed', completed=False, error=str(error))
        recorder.save()
        raise
    from research.analysis.viking_results import run as analyze
    analysis = analyze(output, output/'analysis', group)
    print(json.dumps(dict(output=str(output), status='passed', recovery_cases=len(records),
                          native_calls=recorder.count, independent_surveys=1, figure_directory=analysis['figure_directory'])))
    return recorder.state


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dataset', type=Path, default=DEFAULT_DATASET)
    parser.add_argument('--out', type=Path, required=True, help='new output directory; preserved runs are never overwritten')
    parser.add_argument('--exe', type=Path, default=executable())
    parser.add_argument('--shots', type=int, nargs='+', help='FFIDs; default all ten registered shots')
    parser.add_argument('--counts', type=int, nargs='+', default=[36, 60, 84])
    parser.add_argument('--seed-count', type=int, default=3)
    parser.add_argument('--seed', type=int, default=20261024)
    parser.add_argument('--budget', type=int, default=300)
    parser.add_argument('--iterations', type=int, default=40)
    parser.add_argument('--workers', type=int, default=4)
    parser.add_argument('--group', help='new figure group under figures/')
    parser.add_argument('--validate-only', action='store_true', help='check all registered files, SEG-Y fields and raw/CSV equivalence without native calls')
    run(parser.parse_args())


if __name__ == '__main__':
    main()
