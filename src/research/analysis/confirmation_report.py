"""Publish the prespecified new results, including negative outcomes and their scope."""
import argparse
import html
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.common.hashing import digest
from research.common.tabular import rows
from research.data.confirmation import freeze


def run(root,out):
    root,out=Path(root).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    inputs={};figures=[]
    def read(path):
        inputs[str(path)]=digest(path);return rows(path)
    surveys=[s['id'] for s in freeze()['protocol']['external']['surveys']]
    required=[root/'synthetic_confirmation/verification.json',root/'spatial_confirmation/verification.json']
    required += [root/(survey+'_confirmation/verification.json') for survey in surveys]
    for path in required:
        state=json.loads(path.read_text())
        if state['status']!='passed':raise ValueError('An independent confirmation audit failed')
        inputs[str(path)]=digest(path)
    pairs=read(root/'synthetic_confirmation/paired_effects.csv');primary=[r for r in pairs if r['family']=='primary']
    out.mkdir(parents=True)
    fig,ax=plt.subplots(figsize=(9,5),layout='constrained')
    y=np.arange(len(primary));values=np.array([float(r['difference']) for r in primary])
    lower=np.array([float(r['ci_low']) for r in primary]);upper=np.array([float(r['ci_high']) for r in primary])
    ax.errorbar(values,y,xerr=[values-lower,upper-values],fmt='o',capsize=3)
    ax.set_yticks(y,[r['comparison']+' / m'+r['m']+' / Holm p='+format(float(r['p_holm']),'.3g') for r in primary])
    ax.axvline(0,color='.4');ax.set(xlabel='Paired common-H SNR difference (dB)',title='Prospective synthetic: 20 structures, 6 primary comparisons')
    fig.savefig(out/'synthetic.png',dpi=180);plt.close(fig);figures.append('synthetic.png')
    lines=['# 补充实验结果与修复验收','',
        '本轮协议在新增工区波形读取前冻结。历史六模板、Viking与F3结果保留，新增结果单独报告。',
        '统计单位为结构实现或工区；噪声、掩码种子、相邻剖面不增加独立工区数。','',
        '## 20个合成结构的主要比较','',
        '| 保留数 | 比较 | 平均差/dB | Holm p |','|---:|---|---:|---:|']
    lines.extend(f"|{r['m']}|{r['comparison']}|{float(r['difference']):.6f}|{float(r['p_holm']):.6g}|" for r in primary)
    lines.extend(['','正差表示Adaptive的共同隐藏道SNR较高；负差和不显著结果均保留。候选恢复选择仅使用内层观测。',''])
    source_urls=[]
    for survey in surveys:
        data=read(root/(survey+'_confirmation')/'means.csv')
        state=json.loads((root/(survey+'_confirmation')/'manifest.json').read_text())
        source_urls.append(state['configuration']['dataset_provenance']['source']['url'])
        fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
        for method in ('Jitter','SA_Mu','Adaptive_Mu','Adaptive_Gap','Adaptive_Signal'):
            same=[r for r in data if r['method']==method and r['recovery']=='selected']
            axes[0].plot([int(r['m']) for r in same],[float(r['mean_snr']) for r in same],'o-',label=method)
        for recovery in ('cold','fk','linear','selected'):
            averages=[np.mean([float(r['mean_snr']) for r in data if r['m']==str(m) and r['recovery']==recovery]) for m in (36,60,84)]
            axes[1].plot([36,60,84],averages,'o-',label=recovery)
        for ax in axes:ax.set(xlabel='Retained traces / 120',ylabel='Mean common-H SNR (dB)');ax.legend(fontsize=8);ax.grid(alpha=.2)
        axes[0].set_title(survey+': layouts with inner-selected recovery');axes[1].set_title('Fixed and observed-only selected baselines')
        fig.savefig(out/(survey+'.png'),dpi=180);plt.close(fig);figures.append(survey+'.png')
        lines.extend(['## '+survey+'新增工区','',
            '一个叠后工区、三个预设剖面；结果为工区内描述性比较，不把剖面当作独立工区。',
            '| 保留数 | 布局 | 内层选择SNR/dB |','|---:|---|---:|'])
        lines.extend(f"|{r['m']}|{r['method']}|{float(r['mean_snr']):.6f}|" for r in data if r['recovery']=='selected')
        lines.append('')
        source=state['configuration']['dataset_provenance']['source']
        lines.extend(['数据来源记录：'+source['source_record'],
            '数据署名：'+(source.get('attribution') or 'Stratton 3D公开数据；原始来源记录与完整卷身份保留。')])
        if source.get('license'):lines.append('原始许可：'+source['license'])
        lines.append('')
    spatial=read(root/'spatial_confirmation/recovery.csv')
    fig,axes=plt.subplots(1,2,figsize=(10,4),layout='constrained')
    for method in ('Jitter','SA','Hybrid'):
        counts=sorted({int(r['m']) for r in spatial})
        for ax,metric in zip(axes,('mu','snr')):
            values=[np.mean([float(r[metric]) for r in spatial if r['method']==method and int(r['m'])==m]) for m in counts]
            ax.plot(counts,values,'o-',label=method)
    axes[0].set(ylabel='Finite 2D Fourier coherence',title='2D spatial layout objective')
    axes[1].set(ylabel='Common-H SNR (dB)',title='Same-cube fixed 3D Fourier recovery')
    for ax in axes:ax.set_xlabel('Retained spatial traces / 256');ax.legend();ax.grid(alpha=.2)
    fig.savefig(out/'spatial.png',dpi=180);plt.close(fig);figures.append('spatial.png')
    lines.extend(['## 同一Stratton体的二维空间布局与波形验证','',
        '固定16×16空间网格、128时间样点、共同隐藏32个空间位置，27个预设布局案例。',
        '使用有限二维Fourier目标搜索，并用固定三维Fourier/FISTA恢复；显式Gram、隐藏值隔离及观测投影均独立检查。',
        '该空间算例与Stratton剖面实验属于同一工区，不能额外增加独立工区数量。',
        '原9072点SPS仍只有几何资料；没有与其对应的实测波形，未宣称其成像或实际降本已验证。','',
        '## 结论边界','',
        '当前共涉及Viking、F3、Stratton、Volve四个波形工区。F3为已见资料重复迁移。Teapot下载不可用，在读取替代资料波形前登记修订，改用Volve；算法及选窗规则保持冻结。',
        '算法排名和差值按实际结果报告，不承诺混合方法普遍占优。波形数字幅度诊断不等于物理保幅或下游成像验证。','',
        '## 数据来源','']+['- '+u for u in source_urls])
    markdown='\n'.join(lines)+'\n';(out/'report.md').write_text(markdown,encoding='utf-8')
    body='<pre style="white-space:pre-wrap">'+html.escape(markdown)+'</pre>'
    body+=''.join('<h2>'+name+'</h2><img style="max-width:100%" src="'+name+'">' for name in figures)
    (out/'report.html').write_text('<!doctype html><meta charset="utf-8"><title>补充实验结果</title><body style="max-width:1200px;margin:30px auto;font:17px sans-serif">'+body,encoding='utf-8')
    manifest=dict(status='passed',input_sha256=inputs,table_sha256={p.name:digest(p) for p in out.iterdir()},
        new_waveform_surveys=2,total_waveform_surveys=4,synthetic_templates=20,spatial_cases=len(spatial),
        primary_positive_significant=sum(float(r['difference'])>0 and float(r['p_holm'])<.05 for r in primary),
        primary_negative_significant=sum(float(r['difference'])<0 and float(r['p_holm'])<.05 for r in primary),
        script_sha256=digest(__file__))
    (out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n');return manifest


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();run(a.root,a.out)
