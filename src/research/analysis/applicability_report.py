"""Condition-level effects and bounded research claims, retaining every result."""
import argparse
from collections import Counter,defaultdict
import html,json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.common.hashing import digest
from research.common.tabular import rows,write_rows


def paired_effects(data,keys,metric,baseline='Jitter'):
    lookup={}
    for row in data:
        key=tuple(row[k] for k in keys)+(row['method'],)
        if key in lookup:raise ValueError('Duplicate paired condition')
        lookup[key]=row
    result=[]
    for key,row in lookup.items():
        reference=lookup.get(key[:-1]+(baseline,))
        if reference is None:raise ValueError('Missing paired baseline')
        result.append(dict(row,delta=float(row[metric])-float(reference[metric]),
            delta_mu=float(row['mu'])-float(reference['mu'])))
    return result


def aggregate(data,keys):
    groups=defaultdict(list)
    for row in data:groups[tuple(row[k] for k in keys)].append(float(row['delta']))
    return [dict(zip(keys,k),mean_delta=float(np.mean(v)),min_delta=min(v),max_delta=max(v),
        cases=len(v),positive_cases=sum(x>1e-12 for x in v),negative_cases=sum(x< -1e-12 for x in v))
        for k,v in sorted(groups.items())]


def run(root,out):
    root,out=Path(root).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    inputs={};conditions=[];selections=[];proxy=[]
    def read(path):inputs[str(path)]=digest(path);return rows(path)
    for dataset in ('viking','f3','stratton_confirmation','volve_confirmation'):
        data=read(root/dataset/'recovery.csv')
        for candidate,count in sorted(Counter(r['selected'] for r in data).items()):
            selections.append(dict(dataset=dataset,candidate=candidate,count=count,total=len(data),fraction=count/len(data)))
        for kind in ('cold','fk','linear','selected'):
            section='ffid' if 'ffid' in data[0] else 'section_index'
            pairs=paired_effects(data,(section,'window_start','hidden_set','m','seed'),kind+'_snr')
            for r in pairs:
                conditions.append(dict(dataset=dataset,recovery=kind,**{k:r[k] for k in ('m','method','window_start','hidden_set')},
                    delta=r['delta']))
            for r in aggregate(pairs,('m','method')):
                same=[p for p in pairs if p['m']==r['m'] and p['method']==r['method']]
                lower=[p for p in same if p['delta_mu']< -1e-12]
                proxy.append(dict(dataset=dataset,recovery=kind,m=r['m'],method=r['method'],cases=len(same),
                    lower_mu_cases=len(lower),lower_mu_but_lower_snr=sum(p['delta']< -1e-12 for p in lower),
                    mean_delta_snr=r['mean_delta']))
    synthetic=read(root/'synthetic_confirmation/recovery.csv')
    synth=[]
    for kind in ('fixed','selected'):
        synth.extend(dict(r,recovery=kind) for r in paired_effects(synthetic,('template','noise','m','seed'),kind+'_snr'))
    spatial=read(root/'spatial_replication/recovery.csv')
    audit=root/'spatial_replication/verification.json';inputs[str(audit)]=digest(audit)
    if json.loads(audit.read_text(encoding='utf-8'))['status']!='passed':raise ValueError('Replication audit failed')
    pairs=paired_effects(spatial,('survey','block','window_start','hidden_set','m','seed'),'snr')
    blocks=aggregate(pairs,('survey','block','m','method'))
    summary=[]
    for survey in sorted({r['survey'] for r in blocks}):
        for m in ('76','128','179'):
            for method in ('SA','Hybrid'):
                same=[r for r in blocks if r['survey']==survey and r['m']==m and r['method']==method]
                values=[r['mean_delta'] for r in same]
                summary.append(dict(survey=survey,m=m,method=method,mean_delta=float(np.mean(values)),
                    min_block_delta=min(values),max_block_delta=max(values),positive_blocks=sum(v>0 for v in values),
                    blocks=len(values),scope='Six correlated blocks within one previously seen survey; descriptive'))
    out.mkdir(parents=True)
    write_rows(out/'field_conditions.csv',aggregate(conditions,('dataset','m','method','recovery','window_start','hidden_set')))
    write_rows(out/'synthetic_conditions.csv',aggregate(synth,('family','noise','m','method','recovery')))
    write_rows(out/'recovery_selection.csv',selections);write_rows(out/'objective_alignment.csv',proxy)
    write_rows(out/'spatial_block_effects.csv',blocks);write_rows(out/'spatial_summary.csv',summary)
    fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
    for ax,survey in zip(axes,sorted({r['survey'] for r in blocks})):
        for method in ('SA','Hybrid'):
            same=sorted([r for r in summary if r['survey']==survey and r['method']==method],key=lambda r:int(r['m']))
            ax.plot([int(r['m']) for r in same],[r['mean_delta'] for r in same],'o-',label=method)
        ax.axhline(0,color='.5');ax.set(title=survey+' / six spatial blocks',xlabel='Retained spatial traces / 256',ylabel='Block-averaged SNR gain vs Jitter (dB)')
        ax.legend();ax.grid(alpha=.2)
    fig.savefig(out/'spatial_replication.png',dpi=180);plt.close(fig)
    lines=['# 适用条件、空间复验与结论边界','',
        '本报告补充条件分析与跨块复验。Stratton和Volve已被本项目访问，当前扩展不恢复未见工区身份。',
        '空间协议在读取扩展块波形前冻结；每个工区选择六个互不重叠的空间块、两个时间窗、两组共同隐藏位置和三个掩码种子。',
        '窗口、隐藏集及种子先在块内聚合，再等权平均六块；块之间仍可能存在地质相关性，不作工区总体显著性结论。','',
        '## 相对Jitter的空间复验','',
        '| 工区 | 保留数 | 方法 | 块均值差/dB | 块差值范围/dB | 正差块 |','|---|---:|---|---:|---|---:|']
    lines.extend(f"|{r['survey']}|{r['m']}|{r['method']}|{r['mean_delta']:.6f}|[{r['min_block_delta']:.6f}, {r['max_block_delta']:.6f}]|{r['positive_blocks']}/{r['blocks']}|" for r in summary)
    lines+=['','## 恢复器选择','', '| 数据 | 候选 | 案例数 | 比例 |','|---|---|---:|---:|']
    lines.extend(f"|{r['dataset']}|{r['candidate']}|{r['count']}/{r['total']}|{100*r['fraction']:.2f}%|" for r in selections)
    lines+=['','## 研究主张','',
        '1. 已支持：在本轮输入和固定协议下，软件、评分、信息隔离与观测约束可以独立核验；预算回收贡献可按共同评价次数分析。',
        '2. 条件性结果：空间块收益按工区、保留数和块范围报告；完整负结果保留。条件分层是观察结果后的描述性分析，不能包装成事先指定的确认性显著性发现。',
        '3. 尚未建立：Adaptive普遍优于SA/Jitter、互干度与恢复SNR单调对应、内层选模总优于固定恢复器。',
        '4. 待输入：原9072点SPS仍无对应波形；实际采集降本、物理保幅和下游成像不在已验证结论内。','',
        '## 条件分析文件','',
        '- synthetic_conditions.csv：按结构族、噪声、保留数和恢复器分层的配对差值。',
        '- field_conditions.csv：按工区、保留数、恢复器、窗口和隐藏集分层的配对差值。',
        '- objective_alignment.csv：互干度降低而SNR也降低的案例数；分别在同一保留数、同一恢复器下比较。',
        '- spatial_block_effects.csv：保留全部空间块，禁止按效果筛选块。','',
        '补充合成图的95% bootstrap均值区间未做多重校正；主要显著性只依据冻结的精确Wilcoxon与Holm结果。',
        '当前SA与离散局部搜索属于实际实现；旧CG字段为兼容名。当前范围不含开题中的CNN/GCN系统。']
    markdown='\n'.join(lines)+'\n';(out/'report.md').write_text(markdown,encoding='utf-8')
    (out/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>适用条件与空间复验</title><body style="max-width:1200px;margin:30px auto;font:17px sans-serif"><pre style="white-space:pre-wrap">'+html.escape(markdown)+'</pre><img style="width:100%" src="spatial_replication.png">',encoding='utf-8')
    claims=dict(supported=['software_and_score_verification','observed_only_information_isolation','budget_recycling_mechanism'],
        conditional=['spatial_block_recovery_effects','coverage_constraint_effects'],
        not_established=['universal_algorithm_superiority','monotone_coherence_recovery_relation','physical_amplitude_or_migration_gain','actual_field_cost_saving'],
        blocked_by_input=['9072_point_SPS_waveform_imaging_validation'],new_independent_surveys=0,
        exploratory_condition_analysis=True,spatial_cases=len(spatial))
    (out/'claims.json').write_text(json.dumps(claims,indent=2)+'\n',encoding='utf-8')
    state=dict(status='passed',input_sha256=inputs,table_sha256={p.name:digest(p) for p in out.iterdir()},
        spatial_cases=len(spatial),script_sha256=digest(__file__))
    (out/'manifest.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8');print(json.dumps(dict(status='passed',spatial_cases=len(spatial))))
    return state


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--out',type=Path,required=True)
    args=p.parse_args();run(args.root,args.out)
