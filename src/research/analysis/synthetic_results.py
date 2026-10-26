"""Analyze and plot a fresh synthetic-only search / common-H recovery batch."""
import argparse
from collections import defaultdict
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.analysis.summarize_audit_v5 import confidence, paired, correction
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.tabular import rows, write_rows

METHODS = ('Jitter', 'SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive')
COUNTS = (38, 64, 90)


def groups(records, keys):
    result = defaultdict(list)
    for record in records:
        result[tuple(record[key] for key in keys)].append(record)
    return result


def finite(record, field):
    value = float(record[field])
    if not np.isfinite(value):raise ValueError(field + ' must be finite')
    return value


def recorded(path, base, manifest):
    """Check preserved raw output hashes before using the measurements."""
    key = path.relative_to(base).as_posix()
    hashes = {name.replace('\\', '/'): value for name, value in manifest.get('output_tables_sha256', {}).items()}
    for step in manifest.get('steps', {}).values():
        if step.get('exit_code') != 0:raise ValueError('Batch contains a failed native step')
        hashes.update({name.replace('\\', '/'): value for name, value in step.get('outputs', {}).items()})
    if key not in hashes or digest(path) != hashes[key]:
        raise ValueError('Unverified or changed raw artifact: ' + str(path))


def inputs(v4, joint, budgets):
    files = [v4/'run_manifest.json', joint/'manifest.json', joint/'joint.csv']
    files += [v4/f'search_uniform_{budget}/search.csv' for budget in budgets]
    missing = [str(path) for path in files if not path.is_file()]
    if missing:raise FileNotFoundError('Synthetic analysis inputs are missing:\n'+'\n'.join(missing))
    v4_manifest = json.loads(files[0].read_text(encoding='utf-8-sig'))
    joint_manifest = json.loads(files[1].read_text(encoding='utf-8-sig'))
    configurations = [v4_manifest['fingerprint']['configuration'], joint_manifest['configuration']]
    if any(not c.get('synthetic_only') or c.get('real_source_provenance', {}).get('status') != 'not_used'
           for c in configurations):
        raise ValueError('Synthetic-only analysis requires both batches to declare that real data was not used')
    if not joint_manifest.get('completed'):raise ValueError('Common-H batch has not completed')
    if v4_manifest['fingerprint']['executable_sha256'] != joint_manifest['executable_sha256']:
        raise ValueError('Search and recovery batches must use the same frozen native executable')
    search = []
    for budget in budgets:
        path = v4/f'search_uniform_{budget}/search.csv'; recorded(path, v4, v4_manifest)
        data = [r for r in rows(path) if r['method'] in (*METHODS, 'NoRecycle')]
        expected = {(str(rate), method) for rate in range(3) for method in (*METHODS, 'NoRecycle')}
        if set(groups(data, ['rate_index', 'method'])) != expected:
            raise ValueError('Incomplete uniform search groups: ' + str(path))
        reference_seeds = None
        for (rate, method), same in groups(data, ['rate_index', 'method']).items():
            seeds = [r['seed'] for r in same]
            if len(seeds) < 2 or len(seeds) != len(set(seeds)):
                raise ValueError('Search requires distinct repeated seeds')
            if reference_seeds is not None and set(seeds) != reference_seeds:
                raise ValueError('Every search method and rate must share seeds')
            reference_seeds = set(seeds)
            for record in same:
                if (record['domain'] != 'uniform' or int(record['n']) != 128 or
                        int(record['m']) != COUNTS[int(rate)] or int(record['budget_cap']) != budget or
                        finite(record, 'lambda_gap') != 0 or finite(record, 'lambda_nn') != 0):
                    raise ValueError('Unexpected search geometry, weights or evaluation budget')
                if not 0 <= finite(record, 'mu') <= 1:raise ValueError('Invalid Fourier coherence')
                trace = path.parent/(record['mask_path']+'.trace.csv'); files.append(trace)
                if not trace.is_file():raise FileNotFoundError(trace)
                recorded(trace, v4, v4_manifest)
                record = dict(record, budget=budget, trace=rows(trace))
                if not record['trace'] or len(record['trace']) != int(record['evals']):raise ValueError('Trace/evaluation count mismatch')
                search.append(record)
    recorded(joint/'joint.csv', joint, joint_manifest)
    common = [r for r in rows(joint/'joint.csv') if r['design'] == 'common_H']
    expected = {(str(count), method) for count in COUNTS for method in METHODS}
    if set(groups(common, ['m', 'method'])) != expected:raise ValueError('Incomplete common-H method/count groups')
    expected_hidden = ''.join('1' if j % 8 == 4 else '0' for j in range(128))
    for record in common:
        if (record['evaluation_bits'] != expected_hidden or len(record['mask']) != 128 or
                any(bit not in '01' for bit in record['mask']) or record['mask'].count('1') != int(record['m']) or
                any(a == b == '1' for a, b in zip(record['mask'], expected_hidden))):
            raise ValueError('Common-H eligibility or evaluation mask mismatch')
        for field in ('fixed_snr', 'selected_snr'):finite(record, field)
    models = {r['model'] for r in common}
    if len(models) != 12:raise ValueError('Expected all 12 predefined model/noise conditions')
    for model, same in groups(common, ['model']).items():
        if len({(r['family'],r['structure'],r['noise']) for r in same}) != 1:
            raise ValueError('Model metadata changes across methods or seeds')
    reference_seeds = None
    for key, same in groups(common, ['m', 'method', 'model']).items():
        seeds = [r['seed'] for r in same]
        if len(seeds) != 3 or len(set(seeds)) != 3:raise ValueError('Each common-H model needs all three mask seeds')
        if reference_seeds is not None and set(seeds) != reference_seeds:
            raise ValueError('Common-H methods and models must share mask seeds')
        reference_seeds = set(seeds)
    for key, same in groups(common, ['m', 'method']).items():
        if {r['model'] for r in same} != models:raise ValueError('Methods must share identical model conditions')
    return search, common, list(dict.fromkeys(files))


def statistics(search, common):
    means = []; pairs = []; matched = []
    for (budget, rate), same in sorted(groups(search, ['budget', 'rate_index']).items()):
        by = {method: sorted(values, key=lambda r: int(r['seed']))
              for (method,), values in groups(same, ['method']).items()}
        for method, values in by.items():
            values = [finite(r, 'mu') for r in values]; low, high = confidence(values)
            means.append(dict(budget=budget, m=COUNTS[int(rate)], method=method, n=len(values),
                              mu=float(np.mean(values)), ci_low=low, ci_high=high))
        a = by['Adaptive']
        for method in ('Hybrid', 'SAOnly', 'SA_Budget', 'NoRecycle'):
            b = by[method]
            pairs.append(dict(budget=budget, m=COUNTS[int(rate)], comparison='Adaptive-'+method,
                              **paired([finite(r, 'mu') for r in a], [finite(r, 'mu') for r in b])))
            counts = [min(len(ra['trace']), len(rb['trace'])) for ra, rb in zip(a, b)]
            av = [finite(ra['trace'][k-1], 'best_objective') for ra, k in zip(a, counts)]
            bv = [finite(rb['trace'][k-1], 'best_objective') for rb, k in zip(b, counts)]
            matched.append(dict(budget=budget, m=COUNTS[int(rate)], comparison='Adaptive-'+method,
                                mean_common_evaluations=float(np.mean(counts)), **paired(av, bv)))
    model_means = []
    for (m, method, model), same in groups(common, ['m', 'method', 'model']).items():
        for recovery in ('fixed', 'selected'):
            model_means.append(dict(m=m, method=method, model=model, family=same[0]['family'], structure=same[0]['structure'],noise=same[0]['noise'],
                                    recovery=recovery, snr=float(np.mean([finite(r, recovery+'_snr') for r in same]))))
    recovery_means = []; recovery_pairs = []; templates = []
    for (m, recovery), same in sorted(groups(model_means, ['m', 'recovery']).items()):
        by = {}
        for (method,), values in groups(same, ['method']).items():
            structures = groups(values, ['family', 'structure'])
            if len(structures) != 6 or any(len(v) != 2 or len({r['noise'] for r in v}) != 2 for v in structures.values()):
                raise ValueError('Expected six clean templates with two noise conditions each')
            by[method] = []
            for (family, structure), values in sorted(structures.items()):
                value = float(np.mean([r['snr'] for r in values])); by[method].append(value)
                templates.append(dict(m=int(m), recovery=recovery, method=method, family=family, structure=structure, snr=value))
            low, high = confidence(by[method])
            recovery_means.append(dict(m=int(m), recovery=recovery, method=method, mean_snr=float(np.mean(by[method])),
                                       ci_low=low, ci_high=high, analysis_units=6, unit='clean structural template'))
        for method in METHODS[:-1]:
            recovery_pairs.append(dict(m=int(m), recovery=recovery, comparison='Adaptive-'+method,
                                       **paired(by['Adaptive'], by[method])))
    return dict(search_means=means, search_pairs=correction(pairs), common_evaluation_pairs=correction(matched),
                joint_model_means=model_means, joint_template_means=templates,
                joint_means=recovery_means, joint_pairs=correction(recovery_pairs))


def plot(tables, group, manifest):
    writer = FigureWriter(group); colors = ['#4c78a8', '#f58518', '#54a24b', '#b279a2', '#e45756']
    if any(writer.directory.iterdir()):raise FileExistsError('Use a new synthetic figure group: '+str(writer.directory))
    def save(figure, name, records):
        figure.suptitle('Synthetic-only: no real-data claims; predefined seeds and common H', fontsize=9)
        writer.save(figure, name, dict(records=records, input_sha256=manifest['input_sha256'],
                                       source_status='not_used', independent_real_sources=0), dpi=250)
    budgets = sorted({r['budget'] for r in tables['search_means']})
    figure, axes = plt.subplots(1, len(budgets), figsize=(5*len(budgets), 3.5), layout='constrained', squeeze=False)
    for ax, budget in zip(axes[0], budgets):
        for method, color in zip(METHODS, colors):
            data = sorted([r for r in tables['search_means'] if r['method']==method and r['budget']==budget], key=lambda r:r['m'])
            y = np.array([r['mu'] for r in data]); low=np.array([r['ci_low'] for r in data]); high=np.array([r['ci_high'] for r in data])
            ax.errorbar(COUNTS,y,yerr=[y-low,high-y],fmt='o-',ms=3,capsize=2,label=method,color=color)
        ax.set(title=f'Uniform Fourier search; B={budget}',xlabel='Retained traces (N=128)',ylabel='Fourier coherence',xticks=COUNTS)
        ax.legend(fontsize=7);ax.grid(alpha=.2)
    save(figure,'search',tables['search_means'])
    figure, axes = plt.subplots(1,2,figsize=(10,3.5),layout='constrained')
    for ax, comparison in zip(axes, ['Adaptive-Hybrid','Adaptive-NoRecycle']):
        for name,label,shift in [('search_pairs','Final evaluation',-.08),('common_evaluation_pairs','Common evaluation count',.08)]:
            data=sorted([r for r in tables[name] if r['comparison']==comparison],key=lambda r:(r['budget'],r['m']))
            y=np.array([r['difference'] for r in data]);low=np.array([r['ci_low'] for r in data]);high=np.array([r['ci_high'] for r in data])
            ax.errorbar(np.arange(len(data))+shift,y,yerr=[y-low,high-y],fmt='o',ms=3,capsize=2,label=label)
            ax.set_xticks(range(len(data)),[f"{r['budget']}/{r['m']}" for r in data],rotation=30)
        ax.axhline(0,color='.4',lw=.7);ax.set(title=comparison,xlabel='Evaluation cap / retained traces',ylabel='Paired coherence difference');ax.legend(fontsize=7)
    save(figure,'budget',dict(final=tables['search_pairs'],common=tables['common_evaluation_pairs']))
    figure, axes = plt.subplots(1,2,figsize=(10,3.5),layout='constrained')
    for ax,recovery in zip(axes,['fixed','selected']):
        for method,color in zip(METHODS,colors):
            data=sorted([r for r in tables['joint_means'] if r['method']==method and r['recovery']==recovery],key=lambda r:r['m'])
            y=np.array([r['mean_snr'] for r in data]);low=np.array([r['ci_low'] for r in data]);high=np.array([r['ci_high'] for r in data])
            ax.errorbar(COUNTS,y,yerr=[y-low,high-y],fmt='o-',ms=3,capsize=2,label=method,color=color)
        ax.axhline(0,color='.4',lw=.7);ax.grid(alpha=.2)
        ax.set(title='Fixed curvelet' if recovery=='fixed' else 'Inner-selected: zero / linear / curvelet',
               xlabel='Retained traces (N=128)',ylabel='SNR on common H (dB)',xticks=COUNTS)
        ax.legend(fontsize=7)
    save(figure,'recovery',tables['joint_means']);writer.gallery('Synthetic-only experiments; six independent clean templates; no field-data claims')
    return writer.directory


def run(v4, joint, out, group='research/synthetic_current', budgets=(200,600)):
    v4,joint,out=(Path(path).resolve() for path in (v4,joint,out))
    if not budgets or len(set(budgets)) != len(budgets) or any(budget < 1 for budget in budgets):
        raise ValueError('Evaluation budgets must be distinct positive integers')
    if out.exists():raise FileExistsError(out)
    search, common, files = inputs(v4,joint,budgets)
    tables = statistics(search,common)
    manifest = dict(protocol='synthetic_results_v1',source_status='not_used',independent_real_sources=0,
                    structural_templates=6,model_noise_conditions=12,common_H=list(range(4,128,8)),
                    input_sha256={str(path):digest(path) for path in files},script_sha256=digest(__file__),
                    note='Noise conditions and mask seeds are pooled before inference over six clean templates. Negative outcomes are retained.')
    out.mkdir(parents=True)
    for name, records in tables.items():write_rows(out/(name+'.csv'),records)
    manifest['table_sha256']={path.name:digest(path) for path in out.glob('*.csv')}
    manifest['figure_directory']=str(plot(tables,group,manifest))
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2,allow_nan=False),encoding='utf-8')
    print(json.dumps(dict(output=str(out),figure_directory=manifest['figure_directory'],structural_templates=6)))
    return manifest


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--v4-root',type=Path,required=True)
    parser.add_argument('--joint-root',type=Path,required=True,help='v5 joint stage directory containing joint.csv')
    parser.add_argument('--out',type=Path,required=True,help='new analysis directory; existing results are never overwritten')
    parser.add_argument('--group',default='research/synthetic_current',help='new output group under figures/')
    parser.add_argument('--budgets',type=int,nargs='+',default=[200,600])
    args=parser.parse_args();run(args.v4_root,args.joint_root,args.out,args.group,tuple(args.budgets))
