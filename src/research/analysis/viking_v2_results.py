"""Traceable, descriptive comparisons for the frozen pilot-assisted Viking v2 plan."""
import argparse
from collections import Counter, defaultdict
import itertools
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
from research.experiments.viking_v2 import METHODS, PROTOCOL, bits


def grouped(data, keys):
    result = defaultdict(list)
    for row in data:
        result[tuple(row[k] for k in keys)].append(row)
    return result


def inputs(root):
    manifest = json.loads((root/'manifest.json').read_text(encoding='utf8'))
    if manifest.get('protocol') != PROTOCOL or not manifest.get('completed') or manifest.get('status') != 'passed':
        raise ValueError('A completed Viking v2 batch is required')
    plan = manifest['configuration']['plan']
    if (digest(root/'frozen_plan.json') != manifest['plan_sha256'] or
            json.loads((root/'frozen_plan.json').read_text()) != plan or
            digest(root/'isolation.json') != manifest['isolation_sha256'] or not manifest['isolation']['passed']):
        raise ValueError('Plan or isolation records changed')
    recovery, search = rows(root/'recovery.csv'), rows(root/'search.csv')
    for name in ('recovery.csv', 'search.csv', 'bindings.csv'):
        recorded(root/name, root, manifest)
    by_search = {r['id']: r for r in search}
    if len(by_search) != len(search):
        raise ValueError('Duplicate search ids')
    expected = set(itertools.product(plan['ffids'], plan['window_starts'], [h['id'] for h in plan['hidden_sets']],
                                     plan['counts'], plan['seeds'], METHODS))
    actual = [(int(r['ffid']), int(r['window_start']), r['hidden_set'], int(r['m']), int(r['seed']), r['method']) for r in recovery]
    if len(actual) != len(set(actual)) or set(actual) != expected:
        raise ValueError('Unbalanced or incomplete shot/window/hidden/count/seed/method grid')
    hbits = {}
    for hset in plan['hidden_sets']:
        h = np.zeros(plan['n'], bool); h[hset['indices']] = True; hbits[hset['id']] = bits(h)
    paired_initials = {}
    for row in search:
        if row['method'] not in METHODS or row['H_bits'] != hbits[row['hidden_set']]:
            raise ValueError('Unexpected search method or hidden set')
        mask = np.array(list(row['mask']), int).astype(bool); pilot = np.array(list(row['pilot_bits']), int).astype(bool)
        hidden = np.array(list(row['H_bits']), int).astype(bool)
        if (len(mask) != plan['n'] or sum(mask) != int(row['m']) or np.any(mask & hidden) or
                sum(pilot) != plan['pilot_count'] or not np.all(mask[pilot]) or np.any(pilot & hidden)):
            raise ValueError('Count/pilot/hidden eligibility mismatch')
        key = (row['hidden_set'], row['m'], row['seed'])
        paired_initials.setdefault(key, row['initial_mask'])
        if paired_initials[key] != row['initial_mask']:
            raise ValueError('Search methods did not share initial masks')
        if row['method'] in ('Adaptive_Gap', 'Adaptive_Signal'):
            if float(row['max_gap_m']) > plan['hard_gap_m'][row['m']]+1e-6:
                raise ValueError('Coverage constraint was violated')
        trace_path = root/'search'/(row['id']+'.trace.csv'); recorded(trace_path, root, manifest)
        if len(rows(trace_path)) != int(row['evals']) or int(row['evals']) > plan['budget']:
            raise ValueError('Trace/evaluation budget mismatch')
    for record in recovery:
        source = by_search[record['search_id']]
        if any(record[key] != source[key] for key in ('method', 'mask', 'pilot_bits', 'H_bits', 'm', 'seed', 'hidden_set')):
            raise ValueError('Recovery and search records disagree')
        if float(record['time_origin_s']) != int(record['window_start'])*.004 or float(record['sample_interval_s']) != .004:
            raise ValueError('Time window metadata changed')
        for name in ('zero', 'linear', 'cold', 'fk', 'selected'):
            if not np.isfinite(float(record[name+'_snr'])) or not np.isfinite(float(record[name+'_nmse'])) or float(record[name+'_nmse']) < 0:
                raise ValueError('Invalid recovery score')
        if float(record['zero_snr']) != 0 or float(record['zero_nmse']) != 1:
            raise ValueError('Zero reference changed')
    return manifest, search, recovery


def statistics(search, recovery):
    # Each source/window/H receives equal weight after averaging mask seeds.
    cell = []
    for key, same in grouped(recovery, ['ffid', 'window_start', 'hidden_set', 'm', 'method']).items():
        for kind in ('zero', 'linear', 'cold', 'fk', 'selected'):
            cell.append(dict(zip(['ffid', 'window_start', 'hidden_set', 'm', 'method'], key), recovery=kind,
                             mean_snr=float(np.mean([float(r[kind+'_snr']) for r in same])),
                             mean_nmse=float(np.mean([float(r[kind+'_nmse']) for r in same])), mask_seed_count=len(same)))
    def aggregate(data, keys):
        return [dict(zip(keys, key), mean_snr=float(np.mean([r['mean_snr'] for r in same])),
                     mean_nmse=float(np.mean([r['mean_nmse'] for r in same])), condition_count=len(same), independent_surveys=1)
                for key, same in sorted(grouped(data, keys).items())]
    shot = aggregate(cell, ['ffid', 'm', 'method', 'recovery'])
    overall = aggregate(shot, ['m', 'method', 'recovery'])
    conditions = aggregate(cell, ['window_start', 'hidden_set', 'm', 'method', 'recovery'])
    # Mask seeds describe computational stability, never additional surveys.
    seed_cells = [dict(ffid=r['ffid'], window_start=r['window_start'], hidden_set=r['hidden_set'],
                       m=r['m'], method=r['method'], seed=r['seed'], recovery=kind,
                       mean_snr=float(r[kind+'_snr']), mean_nmse=float(r[kind+'_nmse']))
                  for r in recovery for kind in ('zero', 'linear', 'cold', 'fk', 'selected')]
    mask_seed_means = aggregate(seed_cells, ['seed', 'm', 'method', 'recovery'])
    effects = []
    for m in sorted({r['m'] for r in cell}, key=int):
        for kind in ('cold', 'fk', 'selected'):
            for a, b in [('Adaptive_Gap', 'Adaptive_Mu'), ('Adaptive_Signal', 'Adaptive_Gap'),
                         ('Adaptive_Mu', 'SA_Mu'), ('Adaptive_Signal', 'Jitter')]:
                aa = {(r['ffid'], r['window_start'], r['hidden_set']): r for r in cell if r['m']==m and r['method']==a and r['recovery']==kind}
                bb = {(r['ffid'], r['window_start'], r['hidden_set']): r for r in cell if r['m']==m and r['method']==b and r['recovery']==kind}
                if set(aa) != set(bb):
                    raise ValueError('Paired conditions differ')
                differences = [aa[key]['mean_snr']-bb[key]['mean_snr'] for key in sorted(aa)]
                effects.append(dict(m=m, recovery=kind, comparison=a+'-'+b, mean_delta_snr=float(np.mean(differences)),
                                    min_delta_snr=min(differences), max_delta_snr=max(differences), conditions=len(differences),
                                    positive_conditions=sum(d>1e-12 for d in differences), negative_conditions=sum(d<-1e-12 for d in differences),
                                    independent_surveys=1, inference='descriptive correlated within-survey conditions'))
    geometry = [dict(m=m, method=method, unique_searches=len(same), mean_full_mu=float(np.mean([float(r['mu']) for r in same])),
                     mean_max_gap_m=float(np.mean([float(r['max_gap_m']) for r in same])),
                     mean_evaluations=float(np.mean([int(r['evals']) for r in same])))
                for (m, method), same in sorted(grouped(search, ['m', 'method']).items())]
    return dict(cell_means=cell, shot_means=shot, overall_means=overall, condition_means=conditions,
                mask_seed_means=mask_seed_means,
                paired_effects=effects, geometry_means=geometry)


def plot(tables, plan, group):
    writer = FigureWriter(group); counts = sorted(plan['counts']); colors = ['#4c78a8','#f58518','#54a24b','#b279a2','#e45756']
    def save(figure, name, data):
        figure.suptitle('Viking v2: same survey; pilot cost included; paired H/windows; descriptive results', fontsize=9)
        writer.save(figure, name, dict(records=data, independent_surveys=1, source_status='documented_public_subset_with_header_caveat',
                                     pilot_count=plan['pilot_count']), dpi=220)
    fig, axes = plt.subplots(1,2,figsize=(10,3.7),layout='constrained')
    for method,color in zip(METHODS,colors):
        same = sorted([r for r in tables['geometry_means'] if r['method']==method],key=lambda r:int(r['m']))
        for ax, field in zip(axes,['mean_full_mu','mean_max_gap_m']):
            ax.plot(counts,[r[field] for r in same],'o-',label=method,color=color,ms=3)
    for ax in axes:
        ax.set(xlabel='Retained traces, including pilot',xticks=counts);ax.grid(alpha=.2);ax.legend(fontsize=6)
    axes[0].set(title='Full-frequency diagnostic',ylabel='Mean full Fourier coherence')
    axes[1].set(title='Coverage',ylabel='Mean maximum gap (m)')
    save(fig,'geometry',tables['geometry_means'])
    fig,axes=plt.subplots(1,3,figsize=(13,3.7),layout='constrained')
    for ax,kind,title in zip(axes,['cold','fk','selected'],['Fixed cold curvelet','Independent f-k l1 / FISTA','Observed-only model selection']):
        for method,color in zip(METHODS,colors):
            same=sorted([r for r in tables['overall_means'] if r['method']==method and r['recovery']==kind],key=lambda r:int(r['m']))
            ax.plot(counts,[r['mean_snr'] for r in same],'o-',label=method,color=color,ms=3)
        ax.axhline(0,color='.5',lw=.7);ax.set(title=title,xlabel='Retained traces',ylabel='Mean common-H SNR (dB)',xticks=counts)
        ax.grid(alpha=.2);ax.legend(fontsize=6)
    save(fig,'recovery',tables['overall_means'])
    count=60 if 60 in counts else counts[len(counts)//2]
    conditions=sorted({(r['window_start'],r['hidden_set']) for r in tables['condition_means']},key=lambda k:(int(k[0]),k[1]))
    lookup={(r['window_start'],r['hidden_set'],r['method']):r['mean_snr'] for r in tables['condition_means'] if int(r['m'])==count and r['recovery']=='selected'}
    matrix=np.array([[lookup[start,h,method] for method in METHODS] for start,h in conditions])
    fig,ax=plt.subplots(figsize=(9,max(3.5,.45*len(conditions)+1)),layout='constrained')
    im=ax.imshow(matrix,aspect='auto',cmap='viridis')
    ax.set_xticks(range(len(METHODS)),METHODS,rotation=20);ax.set_yticks(range(len(conditions)),[f'{int(s)*.004:.3f}s / {h}' for s,h in conditions])
    for i in range(len(conditions)):
        for j in range(len(METHODS)):
            ax.text(j,i,f'{matrix[i,j]:.2f}',ha='center',va='center',fontsize=8,color='white' if matrix[i,j]<(matrix.min()+matrix.max())/2 else 'black')
    ax.set(title=f'Inner-selected SNR by predefined window/H, m={count}',xlabel='Selection method',ylabel='Window / hidden set')
    fig.colorbar(im,ax=ax,label='SNR (dB)');save(fig,'conditions',tables['condition_means'])
    writer.gallery('Viking v2: pilot-assisted coverage and recovery across predefined H and time windows')
    return writer.directory


def run(root, output, group):
    root,output=Path(root).resolve(),Path(output).resolve()
    if output.exists():raise FileExistsError(output)
    manifest,search,recovery=inputs(root);plan=manifest['configuration']['plan']
    directory=(FIGURES/group).resolve()
    if not directory.is_relative_to(FIGURES.resolve()) or (directory.exists() and any(directory.iterdir())):
        raise ValueError('Use a new figure group under figures/')
    tables=statistics(search,recovery);output.mkdir(parents=True)
    for name,records in tables.items():write_rows(output/(name+'.csv'),records)
    counts=dict(Counter(r['selected'] for r in recovery))
    summary=['# Viking v2 四项任务实验结果','',f"已完成 {len(recovery)} 组恢复、{len(plan['ffids'])} 炮、{len(plan['window_starts'])} 个时间窗、{len(plan['hidden_sets'])} 组隐藏集。",
             f"配对掩码种子：{plan['seeds']}；每个案例使用 {plan['inner_folds']} 个不重叠内层验证分组。",
             f"每个布局含 {plan['pilot_count']} 道先导观测，计入保留数；这些道在内层训练中始终保留。",
             '同一工区中的炮、时间窗、隐藏集和随机种子均不作为独立工区样本；本批次是观察旧结果后冻结的新开发协议。','',
             '| 保留数 | 方法 | 固定冷启动 SNR | 独立 f-k SNR | 内层选择 SNR |','| --- | --- | --- | --- | --- |']
    lookup={(r['m'],r['method'],r['recovery']):r['mean_snr'] for r in tables['overall_means']}
    for m in sorted(plan['counts']):
        for method in METHODS:
            summary.append(f'| {m} | {method} | '+ ' | '.join(f'{lookup[str(m),method,k]:.4f}' for k in ['cold','fk','selected'])+' |')
    summary+=['','单位 dB；负值保留，先平均掩码种子与各预设条件，再按炮汇总。',
              '模型选择分布：'+json.dumps(counts,ensure_ascii=False)+'。',
              '覆盖和信号加权的逐条件差值见 paired_effects.csv；加权 PSF 属于先导观测估计的代理目标，未建立恢复保证。',
              '逐掩码种子均值见 mask_seed_means.csv；种子差异仅描述计算稳定性，不增加独立工区数。',
              '原始坐标单位码存在历史不一致，解释沿用文献道间距和已验证的相对几何；绝对振幅物理标定未建立。','']
    (output/'summary.md').write_text('\n'.join(summary),encoding='utf8')
    state=dict(protocol='viking_v2_results_v1',source_manifest_sha256=digest(root/'manifest.json'),
               plan_sha256=manifest['plan_sha256'],independent_surveys=1,inferential_tests_performed=False,
               candidate_selection_counts=counts,table_sha256={p.name:digest(p) for p in output.glob('*.csv')},
               figure_directory=str(plot(tables,plan,group)),script_sha256=digest(__file__))
    (output/'manifest.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf8')
    return state


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--root',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True);parser.add_argument('--group',required=True)
    args=parser.parse_args();run(args.root,args.out,args.group)
