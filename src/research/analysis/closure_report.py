"""Report frozen hypothesis outcomes separately from successful execution."""
import argparse
from collections import defaultdict
import html
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.tabular import rows,write_rows


def summarize(data,plan):
    keys=('survey','block','window_start','hidden_set','m','seed','method')
    index={}
    for row in data:
        key=tuple(str(row[k]) for k in keys)
        if key in index:raise ValueError('Duplicate recovery case')
        index[key]=row
    groups=defaultdict(list);alignment=defaultdict(list)
    for key,row in index.items():
        if row['method']!='Hybrid':continue
        for baseline in ('Jitter','SA','RandomBudget'):
            other=index.get(key[:-1]+(baseline,))
            if other is None:raise ValueError('Missing frozen baseline')
            delta=float(row['snr'])-float(other['snr'])
            groups[(str(row['survey']),str(row['block']),int(row['m']),baseline)].append(delta)
            alignment[(str(row['survey']),int(row['m']),baseline)].append(
                (float(row['mu'])-float(other['mu']),float(row['full_fft_mu'])-float(other['full_fft_mu']),delta))
    repetitions=len(plan['time_windows'])*len(plan['hidden_sets'])*len(plan['seeds'])
    blocks=[]
    for (survey,block,m,baseline),values in sorted(groups.items()):
        if len(values)!=repetitions:raise ValueError('Incomplete within-block repetitions')
        blocks.append(dict(survey=survey,block=block,m=m,comparison='Hybrid-'+baseline,
                           mean_delta=float(np.mean(values)),paired_cases=len(values)))
    summary=[]
    for (survey,m,baseline),values in sorted(alignment.items()):
        effects=[r['mean_delta'] for r in blocks if r['survey']==survey and r['m']==m and r['comparison']=='Hybrid-'+baseline]
        if len(effects)!=6:raise ValueError('Incomplete frozen six-block comparison')
        primary=survey==plan['primary']['survey'] and m==plan['primary']['count']
        summary.append(dict(survey=survey,m=m,comparison='Hybrid-'+baseline,mean_delta=float(np.mean(effects)),
            min_block_delta=min(effects),max_block_delta=max(effects),positive_blocks=sum(v>0 for v in effects),blocks=6,
            role='primary' if primary else 'secondary_descriptive',all_blocks_positive=all(v>0 for v in effects)))
    diagnostic=[]
    for (survey,m,baseline),values in sorted(alignment.items()):
        diagnostic.append(dict(survey=survey,m=m,comparison='Hybrid-'+baseline,cases=len(values),
            lower_finite_mu=sum(mu< -1e-12 for mu,_,_ in values),
            lower_finite_mu_and_lower_snr=sum(mu< -1e-12 and delta< -1e-12 for mu,_,delta in values),
            lower_full_mu=sum(mu< -1e-12 for _,mu,_ in values),
            lower_full_mu_and_lower_snr=sum(mu< -1e-12 and delta< -1e-12 for _,mu,delta in values)))
    return blocks,summary,diagnostic


def run(study,out):
    study,out=Path(study).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    audit=json.loads((study/'verification.json').read_text(encoding='utf-8'))
    state=json.loads((study/'manifest.json').read_text(encoding='utf-8'))
    if audit['status']!='passed' or audit['manifest_sha256']!=digest(study/'manifest.json'):
        raise ValueError('Report requires verified matching scientific results')
    if digest(study/'recovery.csv')!=state['output_tables_sha256']['recovery.csv']:
        raise ValueError('Recovery table changed after scientific verification')
    plan=state['configuration']['plan'];data=rows(study/'recovery.csv')
    if len(data)!=audit['cases']:raise ValueError('Unverified case count')
    blocks,summary,diagnostic=summarize(data,plan);primary=[r for r in summary if r['role']=='primary']
    if len(primary)!=len(plan['primary']['comparisons']):raise ValueError('Incomplete primary family')
    out.mkdir(parents=True)
    write_rows(out/'block_effects.csv',blocks);write_rows(out/'summary.csv',summary)
    write_rows(out/'objective_alignment.csv',diagnostic)
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    fig,ax=plt.subplots(figsize=(10,5),layout='constrained')
    chosen=[r for r in summary if r['role']=='primary'];positions=np.arange(len(chosen))
    ax.bar(positions,[r['mean_delta'] for r in chosen],color='#167582')
    for i,row in enumerate(chosen):
        effects=[b['mean_delta'] for b in blocks if b['survey']==row['survey'] and b['m']==row['m'] and b['comparison']==row['comparison']]
        ax.scatter(np.full(len(effects),i),effects,color='#b24d27',zorder=3)
    ax.set_xticks(positions,[r['comparison'] for r in chosen]);ax.axhline(0,color='gray')
    ax.set(ylabel='Block-averaged SNR difference (dB)',title='Frozen Parihaka condition: 128/256 traces; six correlated blocks')
    fig.savefig(out/'primary_effects.png',dpi=180);plt.close(fig)
    claims=dict(execution_status='passed',hypothesis_outcomes=primary,
        primary_design='Prospectively frozen, descriptive six-block consistency in one project-unseen survey; no population p-values',
        claim_status={
            'software_and_numerical_verification':'supported_within_checked_scope',
            'equal_objective_evaluation_comparison':'completed',
            'prospective_new_survey_validation':'completed_with_outcomes_reported',
            'universal_algorithm_superiority':'not_established',
            'monotone_coherence_snr_relation':'not_established',
            'original_SPS_waveform_imaging':'blocked_missing_corresponding_waveforms',
            'physical_amplitude_preservation':'not_validated',
            'actual_acquisition_cost_saving':'not_validated',
            'CNN_GCN_system':'outside_current_implemented_scope',
            'fresh_machine_full_reproduction':'not_validated'},
        new_survey_count=0,total_project_waveform_surveys=5,
        source_transport='Exact byte ranges with per-range hashes and stable publication identity; full-volume publisher MD5 not locally verified',
        case_count=audit['cases'],independent_survey_count_for_primary=1)
    (out/'claims.json').write_text(json.dumps(claims,indent=2)+'\n',encoding='utf-8')
    lines=['# 公平预算对照与前瞻外部验证','',
        '当前实现为模拟退火与离散局部交换搜索。项目研究采样布局、预算利用与缺失道恢复的适用条件。',
        '原协议在首次读取Parihaka波形之前冻结；本轮重算沿用原协议，不增加新工区。Stratton/Volve继续作为已见工区复验。',
        '新资料是发布者整理的数值体，使用道序号几何与未标定数字幅度，不宣称原始SEG-Y字节、物理保幅或成像验证。','',
        '## 设计与独立核验','',
        f"本轮{audit['cases']}例恢复，36个空间—时间块。每个工区六个不重叠空间块，块内聚合两个时间窗、两个隐藏集和三个种子。",
        'SA、Hybrid、RandomBudget均为300次目标评价，Jitter是其中共同的初始单布局。随机搜索只按互干度选择，全部300个候选及最佳轨迹留存。',
        '目标评价次数一致不表示运行时间相等。原生随机候选逐个初始化与搜索器增量更新存在不同开销，不作速度优势结论。',
        f"独立评分最大差{audit['score_max_error']:.3g}，观测投影差{audit['projection_max_error']:.3g}；全部{audit['isolation_interventions']}例执行未观测值NaN干预。",
        '恢复规则为80次Fourier/FISTA迭代与0.01阈值比例，归一化和实际阈值仍从各布局观测计算。','',
        '## 主要外部验证结果','',
        '主要条件预定为Parihaka保留128/256道，比较Hybrid与三种对照。六块均为正差是描述性一致性标准，不是总体显著性检验。',
        '| 比较 | 平均差/dB | 块差范围/dB | 正差块 | 六块均正 |','|---|---:|---|---:|---|']
    for r in primary:
        lines.append(f"|{r['comparison']}|{r['mean_delta']:+.6f}|[{r['min_block_delta']:+.6f}, {r['max_block_delta']:+.6f}]|{r['positive_blocks']}/6|{'满足' if r['all_blocks_positive'] else '未满足'}|")
    lines+=['','## 全部条件','',
        '| 工区 | 保留数 | 比较 | 平均差/dB | 正差块 |','|---|---:|---|---:|---:|']
    lines.extend(f"|{r['survey']}|{r['m']}|{r['comparison']}|{r['mean_delta']:+.6f}|{r['positive_blocks']}/6|" for r in summary)
    lines+=['','## 目标与恢复的对应关系','',
        'objective_alignment.csv同时检查原64原子目标及全空间Fourier目标。目标降低而SNR下降的案例全部保留。',
        '全空间目标仅用于预定的次要诊断，没有根据外层SNR重新选布局或修改恢复参数。','',
        '## 结论与尚缺证据','',
        '执行和数值验收通过与研究假设获得支持分别记录。负差、无差和不满足一致性标准均为正式结果。',
        '一个新工区不能建立跨工区总体优势；条件发现后的确认仅限本轮预定主条件。',
        '当前可以报告预算利用机制与条件性恢复效果。Adaptive/Hybrid普遍优于所有基线及互干度与SNR单调对应尚未建立。',
        '原9072点SPS暂无对应波形。物理保幅、下游成像与实际采集成本仍缺证据；CNN/GCN不属于当前已实现范围。',
        '下一步输入验收要求见根目录RESEARCH_SCOPE.md，不用代码补造实测结论。','',
        '## 数据与传输记录','',
        'Parihaka来源：https://zenodo.org/records/18988527 ，原始提供方见冻结协议与publication.json。',
        '完整下载速度较慢，传输修订改为同一文件的预定块字节范围。每个响应严格核对206、Content-Range及长度，下载前后发布者文件版本/MD5一致。',
        '源字节与解码块均保存本地SHA-256。发布者完整文件MD5仅作为版本身份，未宣称局部缓存完成全卷MD5核验。']
    markdown='\n'.join(lines)+'\n';(out/'report.md').write_text(markdown,encoding='utf-8')
    (out/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>公平预算与前瞻验证</title>'
        '<body style="max-width:1200px;margin:30px auto;font:17px sans-serif"><pre style="white-space:pre-wrap">'+html.escape(markdown)+'</pre>',encoding='utf-8')
    manifest=dict(status='passed',input_sha256={str(study/name):digest(study/name)
        for name in ('manifest.json','verification.json','recovery.csv')},script_sha256=digest(__file__),
        output_sha256={p.name:digest(p) for p in out.iterdir()})
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(status='passed',cases=audit['cases'],primary=primary)),flush=True)
    return claims


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--study',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True);args=parser.parse_args();run(args.study,args.out)
