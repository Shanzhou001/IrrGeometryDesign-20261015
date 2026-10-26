"""Hash-checked, descriptive analysis of common-H Viking experiments from one survey."""
import argparse
from collections import defaultdict
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.analysis.synthetic_results import recorded
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import FIGURES
from research.common.tabular import rows, write_rows
from research.experiments.viking_study import METHODS, PROTOCOL, bits, hidden_mask


def groups(records, fields):
    result = defaultdict(list)
    for record in records:
        result[tuple(record[field] for field in fields)].append(record)
    return result


def inputs(root):
    manifest_path = root/'manifest.json'
    manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
    config = manifest['configuration']
    provenance = config['dataset_provenance']
    if (manifest.get('protocol') != PROTOCOL or not manifest.get('completed') or manifest.get('status') != 'passed' or
            provenance.get('independent_surveys') != 1 or not config.get('strict_preprocessing_isolation') or
            provenance.get('status') != 'documented_public_subset_with_header_caveat'):
        raise ValueError('Expected a completed, isolated Viking batch with its source caveat')
    for name, field in [('dataset_audit.json', 'dataset_audit_sha256'), ('isolation.json', 'isolation_sha256')]:
        if digest(root/name) != manifest.get(field):
            raise ValueError('Changed Viking source/isolation record: '+name)
    if not json.loads((root/'isolation.json').read_text())['passed']:
        raise ValueError('Outer hidden-data isolation has not passed')
    files = [manifest_path, root/'dataset_audit.json', root/'isolation.json', root/'search.csv', root/'recovery.csv']
    for path in files[-2:]:
        recorded(path, root, manifest)
    search, recovery = rows(root/'search.csv'), rows(root/'recovery.csv')
    expected = {(m, seed, method) for m in config['counts'] for seed in config['seeds'] for method in METHODS}
    keys = [(int(r['m']), int(r['seed']), r['method']) for r in search]
    if len(keys) != len(set(keys)) or set(keys) != expected or len({r['id'] for r in search}) != len(search):
        raise ValueError('Search cases must contain every paired count, seed and method exactly once')
    n = config['n']
    hidden = bits(hidden_mask(n))
    initial = {}
    for r in search:
        if (r['evaluation_bits'] != hidden or len(r['mask']) != n or set(r['mask'])-{'0', '1'} or
                r['mask'].count('1') != int(r['m']) or any(a == b == '1' for a, b in zip(r['mask'], hidden)) or
                int(r['budget_cap']) != config['budget'] or not 0 <= float(r['mu']) <= 1):
            raise ValueError('Search masks, hidden set, coherence or budgets disagree with the protocol')
        key = (r['m'], r['seed'])
        initial.setdefault(key, r['initial_mask'])
        if initial[key] != r['initial_mask']:
            raise ValueError('Methods did not share the same initial mask')
        trace_path = root/'search'/(r['id']+'.trace.csv')
        recorded(trace_path, root, manifest)
        trace = rows(trace_path)
        if not trace or len(trace) != int(r['evals']) or len(trace) > config['budget']:
            raise ValueError('Search trace/evaluation count mismatch')
        r['trace'] = trace
        files.append(trace_path)
    search_by_id = {r['id']: r for r in search}
    ffids = provenance['selected_ffids']
    expected_cases = {(ffid, r['id']) for ffid in ffids for r in search}
    actual = [(int(r['ffid']), r['search_id']) for r in recovery]
    if len(actual) != len(set(actual)) or set(actual) != expected_cases:
        raise ValueError('Recovery cases must cover each shot and paired geometric mask exactly once')
    for r in recovery:
        original = search_by_id[r['search_id']]
        if (any(r[field] != original[field] for field in ('mask', 'evaluation_bits', 'method', 'm', 'seed')) or
                int(r['n']) != n or int(r['independent_surveys']) != 1 or float(r['sample_interval_s']) != .004 or
                float(r['time_origin_s']) != 2.048 or r['selected'] not in config['model_selection_candidates'] or
                not 0 <= float(r['gain']) <= 1):
            raise ValueError('Recovery geometry, split, selection or acquisition metadata mismatch')
        for name in ('zero', 'linear', 'fixed', 'selected'):
            if not np.isfinite(float(r[name+'_snr'])) or not np.isfinite(float(r[name+'_nmse'])) or float(r[name+'_nmse']) < 0:
                raise ValueError('Recovery scores must be finite and NMSE nonnegative')
        if float(r['zero_snr']) != 0 or float(r['zero_nmse']) != 1:
            raise ValueError('Zero prediction must remain the 0 dB reference')
    return manifest, search, recovery, files


def statistics(search, recovery):
    search_means, search_pairs = [], []
    for (m, method), same in sorted(groups(search, ['m', 'method']).items()):
        mu = [float(r['mu']) for r in same]
        search_means.append(dict(m=int(m), method=method, seed_count=len(same), mean_mu=float(np.mean(mu)),
                                 min_mu=min(mu), max_mu=max(mu), mean_evaluations=float(np.mean([int(r['evals']) for r in same])),
                                 geometry_count=1))
    for (m,), same in sorted(groups(search, ['m']).items()):
        by = {method: sorted(values, key=lambda r: int(r['seed'])) for (method,), values in groups(same, ['method']).items()}
        for method in METHODS[:-1]:
            a, b = by['Adaptive'], by[method]
            final, common, counts = [], [], []
            for ra, rb in zip(a, b):
                k = min(len(ra['trace']), len(rb['trace']))
                final.append(float(ra['mu'])-float(rb['mu']))
                common.append(float(ra['trace'][k-1]['best'])-float(rb['trace'][k-1]['best']))
                counts.append(k)
            search_pairs.append(dict(m=int(m), comparison='Adaptive-'+method, seed_count=len(final),
                                     mean_final_delta_mu=float(np.mean(final)), mean_common_delta_mu=float(np.mean(common)),
                                     mean_common_evaluations=float(np.mean(counts)), scope='paired computational repeats on one geometry'))
    shot_means = []
    for (ffid, m, method), same in sorted(groups(recovery, ['ffid', 'm', 'method']).items()):
        for name in ('zero', 'linear', 'fixed', 'selected'):
            shot_means.append(dict(ffid=int(ffid), m=int(m), method=method, recovery=name, mask_seed_count=len(same),
                                   mean_snr=float(np.mean([float(r[name+'_snr']) for r in same])),
                                   mean_nmse=float(np.mean([float(r[name+'_nmse']) for r in same]))))
    means, pairs = [], []
    for (m, name), same in sorted(groups(shot_means, ['m', 'recovery']).items()):
        by = {method: sorted(values, key=lambda r: r['ffid']) for (method,), values in groups(same, ['method']).items()}
        for method, values in by.items():
            snrs = [r['mean_snr'] for r in values]
            means.append(dict(m=m, recovery=name, method=method, mean_snr=float(np.mean(snrs)),
                              min_shot_snr=min(snrs), max_shot_snr=max(snrs),
                              mean_nmse=float(np.mean([r['mean_nmse'] for r in values])),
                              shot_count=len(values), independent_surveys=1, scope='descriptive shot means within one survey'))
        for method in METHODS[:-1]:
            deltas = np.array([a['mean_snr']-b['mean_snr'] for a, b in zip(by['Adaptive'], by[method])])
            pairs.append(dict(m=m, recovery=name, comparison='Adaptive-'+method, mean_delta_snr=float(deltas.mean()),
                              min_shot_delta_snr=float(deltas.min()), max_shot_delta_snr=float(deltas.max()),
                              shots_better=int(sum(deltas > 1e-12)), shots_worse=int(sum(deltas < -1e-12)),
                              shot_count=len(deltas), independent_surveys=1,
                              inference='descriptive only; correlated shot gathers; no independent-survey p-value'))
    return dict(search_means=search_means, search_pairs=search_pairs,
                shot_means=shot_means, recovery_means=means, recovery_pairs=pairs)


def plot(tables, group, provenance):
    writer = FigureWriter(group)
    colors = ('#4c78a8', '#f58518', '#54a24b', '#b279a2', '#e45756')
    counts = sorted({r['m'] for r in tables['search_means']})
    def save(figure, name, data):
        figure.suptitle('Viking Line 12: one survey; descriptive shot comparisons; documented header caveat', fontsize=9)
        writer.save(figure, name, dict(records=data, source_status=provenance['status'], independent_surveys=1,
                                     coordinate_units_header_note=provenance['coordinate_units_header_note']), dpi=220)
    figure, axes = plt.subplots(1, 2, figsize=(10, 3.7), layout='constrained')
    for method, color in zip(METHODS, colors):
        same = sorted([r for r in tables['search_means'] if r['method'] == method], key=lambda r: r['m'])
        axes[0].plot(counts, [r['mean_mu'] for r in same], 'o-', label=method, color=color, ms=4)
        axes[1].plot(counts, [r['mean_evaluations'] for r in same], 'o-', label=method, color=color, ms=4)
    for ax in axes:
        ax.set(xlabel='Retained measured traces (N=120)', xticks=counts)
        ax.grid(alpha=.2); ax.legend(fontsize=7)
    axes[0].set(title='One shared receiver geometry', ylabel='Mean Fourier coherence over mask seeds')
    axes[1].set(title='Actual evaluation cost', ylabel='Mean scored candidates')
    save(figure, 'search', tables['search_means'])
    figure, axes = plt.subplots(1, 2, figsize=(10, 3.7), layout='constrained')
    for ax, recovery in zip(axes, ('fixed', 'selected')):
        for method, color in zip(METHODS, colors):
            same = sorted([r for r in tables['recovery_means'] if r['method'] == method and r['recovery'] == recovery], key=lambda r: r['m'])
            ax.plot(counts, [r['mean_snr'] for r in same], 'o-', label=method, color=color, ms=4)
        linear = sorted([r for r in tables['recovery_means'] if r['method'] == 'Jitter' and r['recovery'] == 'linear'], key=lambda r: r['m'])
        ax.plot(counts, [r['mean_snr'] for r in linear], '--', color='.3', label='Jitter + linear')
        ax.axhline(0, lw=.7, color='.4')
        ax.set(title='Fixed curvelet' if recovery == 'fixed' else 'Inner-selected: zero / linear / curvelet',
               xlabel='Retained measured traces (N=120)', ylabel='Mean common-H SNR across shots (dB)', xticks=counts)
        ax.grid(alpha=.2); ax.legend(fontsize=7)
    save(figure, 'recovery', tables['recovery_means'])
    figure, ax = plt.subplots(figsize=(9, 3.7), layout='constrained')
    count = counts[len(counts)//2]
    for recovery, marker in [('fixed', 'o'), ('selected', 's')]:
        a = sorted([r for r in tables['shot_means'] if r['m'] == count and r['method'] == 'Adaptive' and r['recovery'] == recovery], key=lambda r: r['ffid'])
        b = sorted([r for r in tables['shot_means'] if r['m'] == count and r['method'] == 'Jitter' and r['recovery'] == recovery], key=lambda r: r['ffid'])
        ax.plot(range(len(a)), [ra['mean_snr']-rb['mean_snr'] for ra, rb in zip(a, b)], marker+'-', label=recovery)
    ax.set_xticks(range(len(a)), [r['ffid'] for r in a], rotation=30)
    ax.axhline(0, color='.4', lw=.7); ax.grid(alpha=.2); ax.legend()
    ax.set(title=f'Adaptive minus Jitter, m={count}; mask seeds averaged first',
           xlabel='Shot FFID (spatially correlated within one survey)', ylabel='Common-H SNR difference (dB)')
    save(figure, 'shot_differences', tables['shot_means'])
    writer.gallery('Viking Line 12: one survey; descriptive measured-trace recovery')
    return writer.directory


def run(root, output, group):
    root, output = Path(root).resolve(), Path(output).resolve()
    if output.exists():
        raise FileExistsError(output)
    manifest, search, recovery, files = inputs(root)
    figure_path = (FIGURES/group).resolve()
    if not figure_path.is_relative_to(FIGURES.resolve()) or (figure_path.exists() and any(figure_path.iterdir())):
        raise ValueError('Use a new figure group inside figures/')
    tables = statistics(search, recovery)
    output.mkdir(parents=True)
    for name, records in tables.items():
        write_rows(output/(name+'.csv'), records)
    provenance = manifest['configuration']['dataset_provenance']
    state = dict(protocol='viking_results_v1', independent_surveys=1, source_status=provenance['status'],
                 shot_count=len(provenance['selected_ffids']), mask_seeds_averaged_before_shot_comparison=True,
                 inferential_tests_performed=False, input_sha256={str(p): digest(p) for p in files},
                 script_sha256=digest(__file__), table_sha256={p.name: digest(p) for p in output.glob('*.csv')})
    state['figure_directory'] = str(plot(tables, group, provenance))
    text = ['# Viking 真实资料实验结果', '',
            f"实际完成 {len(recovery)} 组恢复对照；{state['shot_count']} 炮来自同一个工区。",
            '先平均每炮的掩码种子，再报告炮间描述性结果；不把炮集数当作独立工区数，不计算独立工区显著性。', '',
            '| 保留道数 | Jitter 固定曲波 SNR | Adaptive 固定曲波 SNR | Jitter 内层选择 SNR | Adaptive 内层选择 SNR |',
            '| --- | --- | --- | --- | --- |']
    means = {(r['m'], r['method'], r['recovery']): r['mean_snr'] for r in tables['recovery_means']}
    for m in sorted({r['m'] for r in tables['search_means']}):
        values = [means[m, method, name] for name in ('fixed', 'selected') for method in ('Jitter', 'Adaptive')]
        text.append('| '+str(m)+' | '+' | '.join(f'{value:.4f}' for value in values)+' |')
    text += ['', '单位 dB；评价位置为统一外层隐藏集，负值保留。零预测为 0 dB，内部选择仅使用已观测道。',
             '外层隐藏振幅干预检查已通过。几何目标改善和恢复质量分别报告，不预设方法必须胜出。',
             '坐标单位标记存在历史不一致，距离尺度按公开采集文献与原始坐标增量解释；绝对振幅物理标定未建立。',
             '本批次为 Viking 新协议，不是旧 RRAW/v4/v5 真实实验的复现。', '']
    (output/'summary.md').write_text('\n'.join(text), encoding='utf-8')
    (output/'manifest.json').write_text(json.dumps(state, indent=2)+'\n', encoding='utf-8')
    return state


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--group', required=True)
    args = parser.parse_args()
    run(args.root, args.out, args.group)
