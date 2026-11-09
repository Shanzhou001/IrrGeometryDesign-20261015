"""Source-level uncertainty, layout contrasts and measured applicability limits."""
from collections import defaultdict
from itertools import product
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT
from research.common.tabular import rows, write_rows
from research.experiments.field_recovery import dump


def interval(values, seed=20261108):
    values = np.asarray(values, float)
    if len(values) < 2 or not np.isfinite(values).all():
        raise ValueError('Source-level uncertainty requires finite repeated sources')
    rng = np.random.default_rng(seed)
    samples = rng.choice(values, size=(20000, len(values)), replace=True).mean(axis=1)
    return float(np.mean(values)), *map(float, np.quantile(samples, [.025, .975]))


def sign_flip(values):
    values = np.asarray(values, float)
    if not 1 <= len(values) <= 16:
        raise ValueError('Exact source-level sign flips support 1..16 sources')
    signs = np.array(list(product((-1., 1.), repeat=len(values))))
    statistic = abs(float(np.mean(values)))
    return float(np.mean(abs(signs @ values / len(values)) >= statistic - 1e-14))


def source_statistics(records):
    groups = defaultdict(list)
    for row in records:
        identity = row.get('source', row.get('ffid'))
        groups[identity].append(float(row['nmse']))
    values = [np.mean(v) for _, v in sorted(groups.items())]
    mean, low, high = interval(values)
    return dict(sources=len(values), cases=len(records), nmse=mean, ci_low=low, ci_high=high,
                positive_sources=int(sum(v < 1 for v in values)),
                interpretation='Source-cluster percentile bootstrap within this recorded survey and protocol.')


def layout_statistics(datasets):
    output = []
    for survey, records in datasets.items():
        groups = defaultdict(lambda: defaultdict(list))
        for row in records:
            source = row.get('source', row.get('ffid'))
            groups[source][row['method']].append(float(row['nmse']))
        for reference in ('Jitter', 'RandomBudget', 'SA'):
            differences = [np.mean(methods[reference]) - np.mean(methods['Hybrid'])
                           for _, methods in sorted(groups.items())]
            mean, low, high = interval(differences)
            output.append(dict(survey=survey, reference=reference, sources=len(differences),
                               nmse_improvement=mean, ci_low=low, ci_high=high,
                               positive_sources=int(sum(d > 0 for d in differences)),
                               exact_two_sided_p=sign_flip(differences)))
    previous = 0.
    for rank, row in enumerate(sorted(output, key=lambda r: r['exact_two_sided_p'])):
        previous = max(previous, min(1., row['exact_two_sided_p'] * (len(output) - rank)))
        row['holm_p'] = previous
        row['family'] = 'Nine secondary exploratory source-level layout comparisons'
    return output


def run(base, suite=None, model_study=None):
    base = Path(base)
    suite = Path(suite or base / 'field_applications')
    model_study = Path(model_study or base / 'field_recovery_v2')
    out = suite / 'report'
    out.mkdir()
    primary = rows(model_study / 'recovery.csv')
    tables = {
        'Poland': [r for r in primary if r['survey'] == 'poland'],
        'Soda Lake': [r for r in primary if r['survey'] == 'soda'],
        'Soda Lake local delay': rows(suite / 'soda_followup/recovery.csv'),
        'Viking transfer': rows(suite / 'viking_transfer/recovery.csv'),
        'Garner Valley original': rows(suite / 'gvda/recovery.csv'),
        'Garner Valley prospective': rows(suite / 'gvda_confirmation/recovery.csv'),
    }
    summaries = {name: source_statistics(values) for name, values in tables.items()}
    comparisons = layout_statistics({name: tables[name] for name in
                                     ('Poland', 'Viking transfer', 'Garner Valley prospective')})
    write_rows(out / 'source_statistics.csv', [dict(survey=name, **record) for name, record in summaries.items()])
    write_rows(out / 'layout_statistics.csv', comparisons)
    confirmation = tables['Garner Valley prospective']
    full = source_statistics([dict(row, nmse=row['all_nmse']) for row in confirmation])
    datasets = {name: json.loads((suite / folder / 'performance.json').read_text())
                for name, folder in [('viking', 'viking_transfer'), ('gvda', 'gvda'),
                                     ('gvda_confirmation', 'gvda_confirmation'), ('soda_followup', 'soda_followup')]}
    poland = json.loads((model_study / 'performance.json').read_text())['poland']
    positive = poland['passed'] and all(datasets[k]['performance_status'] == 'passed'
                                      for k in ('viking', 'gvda', 'gvda_confirmation'))
    boundary = dict(survey='Soda Lake', classification='unsupported_in_tested_conditions',
                    scope='Recorded receiver subsets, 30/50/70 percent retention and the two fixed score windows',
                    main_passed=json.loads((model_study / 'performance.json').read_text())['soda']['passed'],
                    local_delay_passed=datasets['soda_followup']['performance_status'] == 'passed')
    state = dict(status='passed', application_effect_status='passed' if positive else 'failed',
                 primary_summary=summaries, all_unobserved_confirmation=full,
                 soda_boundary=boundary, layout_comparisons=comparisons,
                 claims='Condition-specific measured recovery and acquisition-constraint evaluation. Universal Hybrid superiority and a monotone coherence/recovery relationship remain unsupported.')
    dump(out / 'summary.json', state)
    figures(out, summaries, comparisons)
    body = ''.join(f"<tr><td>{name}</td><td>{r['sources']}</td><td>{r['positive_sources']}</td>"
                   f"<td>{r['nmse']:.6f}</td><td>[{r['ci_low']:.6f}, {r['ci_high']:.6f}]</td></tr>"
                   for name, r in summaries.items())
    contrasts = ''.join(f"<tr><td>{r['survey']}</td><td>Hybrid / {r['reference']}</td>"
                        f"<td>{r['nmse_improvement']:.6f}</td><td>{r['holm_p']:.6f}</td></tr>"
                        for r in comparisons)
    text = f"""<!doctype html><meta charset="utf-8"><title>实测应用与适用条件</title>
<main style="max-width:1100px;margin:30px auto;font:17px sans-serif">
<h1>实测应用、统计不确定性与适用条件</h1>
<p>正向应用采用 Poland、Viking 与 Garner Valley。Soda Lake 保留为指定条件下的不适用案例。
新确认包含十二个预先登记的震源，每个条件使用二十个布局种子。原始数字幅值在共同隐藏道及全部未观测道评分。</p>
<table><tr><th>资料</th><th>震源数</th><th>优于零填充</th><th>平均 NMSE</th><th>震源重采样区间</th></tr>{body}</table>
<p>置信区间以震源为聚类单位，同一震源的窗口、掩码与布局先聚合。各配置数量不增加独立工区数。
Viking 是已登记工区的模型迁移，Garner Valley 新确认是工区内的新震源确认。</p>
<p>Garner Valley 全部未观测道的平均 NMSE 为 {full['nmse']:.6f}，
95% 震源重采样区间为 [{full['ci_low']:.6f}, {full['ci_high']:.6f}]。</p>
<img src="source_effects.png" style="max-width:100%" alt="Source-level raw errors">
<h2>布局贡献与目标对应关系</h2>
<p>几何感知恢复采用时移预测与开发炮回归，布局目标采用实际坐标上的 Fourier Gram 互干度。
原曲波单尺度入口保留为历史计算协议。当前结论限定为按协议实测的条件性结果。
互干度、空间覆盖与波形误差各自保留，布局优化器的优势由配对误差检验。</p>
<table><tr><th>资料</th><th>对照</th><th>NMSE 改善</th><th>Holm p</th></tr>{contrasts}</table>
<p>正改善表示 Hybrid 误差更低。九项布局比较属于次要探索家族，精确双侧符号翻转检验以震源为单位。</p>
<img src="layout_effects.png" style="max-width:100%" alt="Paired layout effects">
<p><a href="source_statistics.csv">震源统计</a> · <a href="layout_statistics.csv">布局统计</a> ·
<a href="summary.json">完整范围与结果</a></p></main>"""
    (out / 'report.html').write_text(text, encoding='utf-8')
    outputs = ['source_statistics.csv', 'layout_statistics.csv', 'summary.json', 'report.html',
               'source_effects.png', 'layout_effects.png']
    dump(out / 'manifest.json', dict(status='passed', source_sha256={
         Path(__file__).relative_to(ROOT).as_posix(): digest(__file__)},
         input_sha256={(model_study/'recovery.csv').relative_to(ROOT).as_posix():
                       digest(model_study/'recovery.csv'),
                       **{(suite/folder/'recovery.csv').relative_to(ROOT).as_posix(): digest(suite/folder/'recovery.csv')
                          for folder in ('soda_followup', 'viking_transfer', 'gvda', 'gvda_confirmation')}},
         output_sha256={name: digest(out/name) for name in outputs}))
    return state


def figures(out, summaries, comparisons):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    names, values = list(summaries), list(summaries.values())
    fig, ax = plt.subplots(figsize=(11, 5), layout='constrained')
    means = np.array([v['nmse'] for v in values])
    errors = np.array([[v['nmse']-v['ci_low'], v['ci_high']-v['nmse']] for v in values]).T
    ax.errorbar(range(len(names)), means, yerr=errors, fmt='o', capsize=4)
    ax.axhline(1, color='black', linestyle='--')
    ax.set(xticks=range(len(names)), xticklabels=[n.replace(' ', '\n') for n in names],
           ylabel='Raw hidden-trace NMSE with source bootstrap interval')
    fig.savefig(out/'source_effects.png', dpi=150)
    plt.close(fig)
    fig, ax = plt.subplots(figsize=(11, 5), layout='constrained')
    labels = [f"{r['survey']}\n{r['reference']}" for r in comparisons]
    means = np.array([r['nmse_improvement'] for r in comparisons])
    errors = np.array([[r['nmse_improvement']-r['ci_low'], r['ci_high']-r['nmse_improvement']]
                       for r in comparisons]).T
    ax.errorbar(range(len(labels)), means, yerr=errors, fmt='o', capsize=4)
    ax.axhline(0, color='black', linestyle='--')
    ax.set(xticks=range(len(labels)), xticklabels=labels, ylabel='Paired NMSE improvement of Hybrid')
    ax.tick_params(axis='x', labelsize=8)
    fig.savefig(out/'layout_effects.png', dpi=150)
    plt.close(fig)
