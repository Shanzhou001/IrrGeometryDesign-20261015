"""Stratified effects and fixed-example waveform figures for mapped field data."""
import argparse
from collections import defaultdict
import html
import json
from pathlib import Path

import numpy as np
from research.common.hashing import digest
from research.common.tabular import rows,write_rows


def summarize(records):
    index={};groups=defaultdict(list)
    for row in records:
        key=tuple(row[k] for k in ('geometry','component','start','h','m','seed','recoverer','method'))
        if key in index:raise ValueError('Duplicate mapped-field case')
        index[key]=row
    for key,row in index.items():
        if row['method']!='Hybrid':continue
        for baseline in ('Jitter','SA','RandomBudget'):
            other=index.get((*key[:-1],baseline))
            if other is None:raise ValueError('Missing paired mapped-field control')
            value=float(row['snr'])-float(other['snr'])
            if not np.isfinite(value):raise ValueError('Nonfinite paired effect')
            groups[(row['survey'],row['recoverer'],int(row['m']),float(row['retained_fraction']),baseline)].append(value)
    return [dict(survey=survey,recoverer=kind,m=m,retained_fraction=fraction,comparison='Hybrid-'+baseline,
                 mean_delta=float(np.mean(v)),min_delta=min(v),max_delta=max(v),paired_cases=len(v),
                 positive_pairs=sum(x>0 for x in v))
            for (survey,kind,m,fraction,baseline),v in sorted(groups.items())]


def run(study,out):
    study,out=Path(study).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    audit=json.loads((study/'verification.json').read_text());manifest=json.loads((study/'manifest.json').read_text())
    if (audit.get('status')!='passed' or audit.get('manifest_sha256')!=digest(study/'manifest.json') or
            digest(study/'recovery.csv')!=manifest['output_tables_sha256']['recovery.csv']):
        raise ValueError('Mapped-field report requires verified unchanged scientific results')
    data=rows(study/'recovery.csv');summary=summarize(data)
    if len(data)!=1728 or len(summary)!=54 or any(r['paired_cases']!=24 for r in summary):
        raise ValueError('Incomplete frozen mapped-field comparisons')
    out.mkdir(parents=True);write_rows(out/'summary.csv',summary)
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig,axes=plt.subplots(1,2,figsize=(12,4.3),layout='constrained')
    for ax,survey in zip(axes,sorted({r['survey'] for r in summary})):
        for baseline in ('Jitter','SA','RandomBudget'):
            selected=[r for r in summary if r['survey']==survey and r['recoverer']=='fourier' and r['comparison']=='Hybrid-'+baseline]
            selected.sort(key=lambda r:r['m'])
            ax.plot([100*r['retained_fraction'] for r in selected],[r['mean_delta'] for r in selected],marker='o',label=baseline)
        ax.axhline(0,color='gray',linewidth=1);ax.set(xlabel='Retained receivers (%)',ylabel='Hybrid minus control SNR (dB)',title=survey)
        ax.legend(title='Control');ax.grid(alpha=.2)
    fig.suptitle('Mapped public field data: descriptive paired effects; fixed Fourier recovery')
    fig.savefig(out/'paired_effects.png',dpi=180);plt.close(fig)
    # The example is fixed by metadata, never selected by the recovery score.
    example=next(r for r in data if r['geometry']=='poland_f231' and int(r['component'])==0 and int(r['start'])==256
                 and int(r['h'])==1907 and int(r['m'])==128 and int(r['seed'])==17
                 and r['method']=='Hybrid' and r['recoverer']=='fourier')
    with np.load(study/'truth'/(example['cube']+'.npz'),allow_pickle=False) as saved:truth=saved['data']
    with np.load(study/'predictions'/(example['case']+'.npz'),allow_pickle=False) as saved:
        prediction=saved['prediction'];known=saved['known']
    observed=np.zeros_like(truth);observed[:,known]=truth[:,known]
    limit=max(float(np.percentile(abs(truth),98)),1e-20);fig,axes=plt.subplots(1,4,figsize=(14,4),layout='constrained')
    extent=[0,truth.shape[1],(256+truth.shape[0])*.002,256*.002]
    for ax,title,values in zip(axes,['Complete reference','Retained observations','Recovered','Recovery error'],
                               [truth,observed,prediction,prediction-truth]):
        ax.imshow(values,cmap='seismic',aspect='auto',vmin=-limit,vmax=limit,extent=extent)
        ax.set(title=title,xlabel='SPS-mapped receiver rank')
    axes[0].set_ylabel('Time (s)');fig.suptitle('Fixed Poland FFID 231 example; digital amplitude, common display scale')
    fig.savefig(out/'waveform_example.png',dpi=180);plt.close(fig)
    table='<table><tr><th>Survey</th><th>Recovery</th><th>Retained</th><th>Comparison</th><th>Mean dB</th><th>Positive pairs</th></tr>'
    for r in summary:
        table+='<tr>'+''.join('<td>'+html.escape(str(v))+'</td>' for v in [r['survey'],r['recoverer'],r['m'],r['comparison'],
                f"{r['mean_delta']:+.6f}",f"{r['positive_pairs']}/{r['paired_cases']}"])+'</tr>'
    page='''<!doctype html><meta charset="utf-8"><title>公开实测炮集验证</title>
<style>body{max-width:1250px;margin:30px auto;font:17px/1.6 sans-serif;padding:0 20px}img{width:100%}table{border-collapse:collapse;width:100%}td,th{padding:6px;border:1px solid #ccc}</style>
<h1>公开实测炮集：几何—采样—恢复—评分</h1>
<p>Poland和Soda Lake采用各自配套的波形、炮检映射与SPS坐标，构成本项目实测应用验证；原5151—5171资料作为几何案例。</p>
<p>共1728例；四布局、三恢复器、三档保留率。SA、Hybrid和随机搜索均200次几何评价；共同隐藏集合用于独立评分。全部负向结果保留。两个工区此前已访问，不宣称新的未见验证。</p>
<img src="paired_effects.png"><p>每点为24个依赖配对的描述性均值，重复炮、分量、窗口和种子不能解释为24个独立工区。</p>
<img src="waveform_example.png"><p>示例由固定炮号、窗口、隐藏集、保留数及种子确定，不按分数选例。</p>
<p>优化使用实测站点坐标；Fourier恢复使用测站序号网格，在弯曲或非规则几何上是近似。数字幅度未经物理标定；本项目不据此主张物理保幅、成像或实际成本改善。</p>
'''+table+'</table>'
    (out/'report.html').write_text(page,encoding='utf-8')
    record=dict(status='passed',cases=len(data),comparisons=len(summary),example_case=example['case'],
        input_sha256={str(study/name):digest(study/name) for name in ['recovery.csv','verification.json','manifest.json']},
        source_sha256={'src/research/analysis/mapped_field_report.py':digest(__file__)},
        output_sha256={p.name:digest(p) for p in out.iterdir() if p.is_file()})
    (out/'manifest.json').write_text(json.dumps(record,indent=2)+'\n');return record


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--study',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True);args=parser.parse_args();print(json.dumps(run(args.study,args.out),indent=2))
