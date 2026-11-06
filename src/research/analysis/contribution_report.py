"""Separate layout, recovery and calibration effects without choosing outer-score winners."""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.tabular import rows, write_rows
from research.data.viking import load_windows
from research.data.f3 import load as load_f3


def design_check(units,family_size,alpha=.05):
    if units<1 or family_size<1 or not 0<alpha<1:raise ValueError('Invalid statistical design')
    minimum_p=2.0**(1-units)
    minimum_units=next(n for n in range(1,1024) if 2.0**(1-n)<=alpha/family_size)
    return dict(analysis_units=units,family_size=family_size,alpha=alpha,
        exact_two_sided_p_floor=minimum_p,first_holm_threshold=alpha/family_size,
        can_reject_any=minimum_p<=alpha/family_size,mathematical_minimum_nonzero_units=minimum_units,
        note='A mathematical feasibility check, not a power calculation. Repeated noise/mask seeds do not add structural units. No retrospective change to the historical testing family.')


def layout_effects(data):
    lookup={}
    for row in data:
        key=tuple(row.get(k,'') for k in ('ffid','section_index','window_start','hidden_set','m','seed'))
        pair=(key,row['method'])
        if pair in lookup:raise ValueError('Duplicate paired experimental condition')
        lookup[pair]=row
    result=[]
    for (key,method),row in lookup.items():
        baseline=lookup.get((key,'Jitter'))
        if baseline is None:raise ValueError('Missing matched Jitter baseline')
        for recovery in ('cold','fk','linear','selected'):
            result.append(dict(case=row['case'],m=row['m'],method=method,recovery=recovery,
                delta_snr_vs_jitter=float(row[recovery+'_snr'])-float(baseline[recovery+'_snr']),
                condition=repr(key),inference='descriptive correlated within-survey comparison'))
    return result


def score(target,prediction):
    return float(-10*np.log10(max(float(np.sum((target-prediction)**2)/np.sum(target**2)),1e-30)))


def recovery_effects(root,data,windows):
    result=[]
    for row in data:
        path=root/'predictions'/(row['case']+'.npz')
        section=int(row.get('ffid') or row['section_index']);start=int(row['window_start'])
        target=windows[start][section].data
        with np.load(path,allow_pickle=False) as saved:
            hidden=saved['hidden'];gain=float(row['gain'])
            selected=saved['selected'][:,hidden];reference=target[:,hidden]
            if abs(score(reference,selected)-float(row['selected_snr']))>1e-8:raise ValueError('Selected score changed')
            # The zero candidate is well-defined at gain=0. Other zero-gain
            # predictions cannot be inverted; leave that decomposition undefined.
            raw=selected/gain if gain>0 else np.zeros_like(selected) if row['selected']=='zero' else None
            raw_score=None if raw is None else score(reference,raw)
            result.append(dict(case=row['case'],m=row['m'],method=row['method'],candidate=row['selected'],gain=gain,
                selected_snr=float(row['selected_snr']),cold_snr=float(row['cold_snr']),
                selected_minus_cold=float(row['selected_snr'])-float(row['cold_snr']),
                selected_minus_linear=float(row['selected_snr'])-float(row['linear_snr']),
                selected_minus_fk=float(row['selected_snr'])-float(row['fk_snr']),
                selected_candidate_unscaled_snr=raw_score,
                candidate_configuration_effect=None if raw_score is None else raw_score-float(row['cold_snr']),
                amplitude_gain_effect=None if raw_score is None else float(row['selected_snr'])-raw_score))
    return result


def run(root,out):
    root,out=Path(root).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    inputs={};all_layout=[];all_recovery=[];selections={}
    for dataset in ('viking','f3'):
        base=root/dataset;state=json.loads((base/'manifest.json').read_text())
        if not state.get('completed') or state.get('status')!='passed':raise ValueError('Completed measured batch required')
        path=base/'recovery.csv'
        if digest(path)!=state['output_tables_sha256']['recovery.csv']:raise ValueError('Recovery table changed')
        inputs[str(path)]=digest(path);data=rows(path)
        if dataset=='viking':
            plan=state['configuration']['plan'];windows,_=load_windows(plan['window_starts'],shot_ids=plan['ffids'])
        else:windows,_=load_f3()
        for row in data:
            p=base/'predictions'/(row['case']+'.npz')
            if digest(p)!=state['derived_artifacts_sha256'][p.relative_to(base).as_posix()]:raise ValueError('Prediction changed')
            inputs[str(p)]=digest(p)
        all_layout.extend(dict(dataset=dataset,**r) for r in layout_effects(data))
        all_recovery.extend(dict(dataset=dataset,**r) for r in recovery_effects(base,data,windows))
        selections[dataset]=dict(Counter(r['selected'] for r in data))
    groups=defaultdict(list)
    for row in all_layout:groups[(row['dataset'],row['m'],row['method'],row['recovery'])].append(row['delta_snr_vs_jitter'])
    summary=[dict(dataset=k[0],m=k[1],method=k[2],recovery=k[3],mean_delta_snr_vs_jitter=float(np.mean(v)),
        paired_cases=len(v),positive_cases=sum(x>0 for x in v),negative_cases=sum(x<0 for x in v),independent_surveys=1)
        for k,v in sorted(groups.items())]
    synthetic=rows(root/'synthetic_analysis/joint_pairs.csv')
    n={int(r['n']) for r in synthetic};families={int(r['family_size']) for r in synthetic}
    if len(n)!=1 or len(families)!=1:raise ValueError('Mixed historical statistical designs')
    design=design_check(n.pop(),families.pop())
    claims=dict(status='passed',algorithm='SA with discrete local exchange search, adaptive switching and budget recycling',
        historical_waveform_surveys=2,f3_role='repeated transfer on previously accessed poststack data',
        geometry_role='9072-point SPS geometry; separate from Viking/F3 waveform surveys',
        no_universal_superiority_claim=True,no_physical_amplitude_or_imaging_claim=True,
        statistical_design=design,recovery_selection=selections)
    out.mkdir(parents=True)
    write_rows(out/'layout_effects.csv',all_layout);write_rows(out/'layout_summary.csv',summary)
    write_rows(out/'recovery_effects.csv',all_recovery)
    (out/'claims.json').write_text(json.dumps(claims,indent=2)+'\n')
    lines=['# 布局与恢复贡献及验证范围','',
        '当前方法为模拟退火与离散局部交换搜索，包含自适应切换和预算回收。历史CG函数名仅用于接口兼容。','',
        '布局贡献按同工区、剖面/炮、窗口、隐藏集、保留数和种子配对，对固定冷曲波、f-k、线性和内层选择分别报告。',
        '恢复贡献按同一布局计算，进一步分解候选配置变化和内层幅度系数的影响。这是数值差值分解，不是因果识别。','',
        f"历史合成恢复：{design['analysis_units']}个结构模板、{design['family_size']}项比较，精确双侧p值下限为{design['exact_two_sided_p_floor']:.6f}，Holm首阈值为{design['first_holm_threshold']:.6f}。",
        '当前设计无法通过Holm首阈值；保持历史检验不变，新协议单独增加模板并预先限定主要比较。','',
        'Viking与F3各为一个工区。F3是历史已见资料的重复迁移。SPS几何目标通过不建立保幅、实际采集降本或成像改善结论。','',
        '| 数据 | 保留数 | 布局 | 恢复方式 | 相对Jitter平均差/dB |','|---|---:|---|---|---:|']
    lines.extend(f"|{r['dataset']}|{r['m']}|{r['method']}|{r['recovery']}|{r['mean_delta_snr_vs_jitter']:.6f}|" for r in summary if r['recovery']=='selected')
    (out/'report.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')
    manifest=dict(status='passed',input_sha256=inputs,table_sha256={p.name:digest(p) for p in out.iterdir()},
                  script_sha256=digest(__file__),selection_used_outer_scores=False)
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(json.dumps(dict(status='passed',paired_cases=len(all_layout),recovery_cases=len(all_recovery))))
    return manifest


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();run(a.root,a.out)
