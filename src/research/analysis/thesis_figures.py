"""Measured figures, explicit provenance and a UTF-8 catalog for the Qt viewer."""
import html
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
from PIL import Image

from research.common.paths import ROOT, FIGURES
from research.common.hashing import digest
from research.common.tabular import rows
from research.common.figures import FigureWriter
from research.data.viking import load_windows


def run(out):
    out=Path(out); writer=FigureWriter('research/current/diagnostics')
    plt.rcParams.update({'font.size':10, 'axes.spines.top':False, 'axes.spines.right':False})
    sources={}
    def table(path):
        sources[path.relative_to(ROOT).as_posix()]=digest(path)
        return rows(path)
    def save(fig,name,data):
        writer.save(fig,name,dict(records=data,source_sha256=dict(sources)),dpi=250)
    for dataset,file in [('viking',out/'viking/analysis/overall_means.csv'),('f3',out/'f3/means.csv')]:
        data=table(file)
        fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
        for kind in ('cold','fk','selected','linear'):
            ys=[np.mean([float(r['mean_snr']) for r in data if r['m']==str(m) and r['recovery']==kind]) for m in (36,60,84)]
            axes[0].plot([36,60,84],ys,'o-',label=kind)
        axes[0].set(xlabel='Retained traces / 120',ylabel='Mean common-H SNR (dB)',title=dataset.upper()+': recovery baselines')
        axes[0].legend();axes[0].grid(alpha=.2)
        q=table(out/('viking_quality' if dataset=='viking' else 'f3')/'waveform_means.csv')
        for kind in ('cold','fk','selected','linear'):
            ys=[np.mean([float(r['mean_rms_ratio']) for r in q if r['m']==str(m) and r['recovery']==kind]) for m in (36,60,84)]
            axes[1].plot([36,60,84],ys,'o-',label=kind)
        axes[1].axhline(1,color='.4',linestyle='--',label='Target RMS')
        axes[1].set(xlabel='Retained traces / 120',ylabel='Prediction / target RMS',title='Amplitude diagnostic; no physical calibration')
        axes[1].legend(fontsize=8);axes[1].grid(alpha=.2)
        save(fig,dataset+'_baselines',dict(scores=data,quality=q))
    # The case is fixed by indices, independent of the outer evaluation score.
    case='f453_w512_hr271828_m60_s94217_Adaptive_Gap'
    path=out/'viking/predictions'/(case+'.npz'); sources[path.relative_to(ROOT).as_posix()]=digest(path)
    windows,provenance=load_windows([512],shot_ids=[453]);shot=windows[512][453]
    with np.load(path,allow_pickle=False) as prediction:
        known=prediction['known'];hidden=prediction['hidden'];selected=prediction['selected']
        observed=np.zeros_like(shot.data);observed[:,known]=shot.data[:,known]
        limit=float(np.max(abs(shot.data)))
        fig,axes=plt.subplots(1,4,figsize=(13,5),layout='constrained',sharey=True)
        for ax,values,title in zip(axes,[shot.data,observed,selected,selected-shot.data],['Measured target','Retained observations','Inner-selected prediction','Prediction - target']):
            im=ax.imshow(values,cmap='seismic',vmin=-limit,vmax=limit,aspect='auto',
                extent=[0,119,shot.time_origin_s+len(values)*shot.sample_interval_s,shot.time_origin_s])
            ax.set(title=title,xlabel='Trace index')
        axes[0].set_ylabel('Time (s)');fig.colorbar(im,ax=list(axes),shrink=.7,label='Uncalibrated amplitude; shared color scale')
        fig.suptitle('Predefined Viking case: FFID453, window512, m60, seed94217; no score-based case selection',fontsize=10)
        save(fig,'viking_gathers',dict(case=case,prediction_sha256=digest(path),target_provenance=provenance))
        trace=int(np.flatnonzero(hidden)[0]);t=shot.time_origin_s+np.arange(len(shot.data))*shot.sample_interval_s
        fig,axes=plt.subplots(1,2,figsize=(12,4),layout='constrained')
        for label,values in [('target',shot.data),('selected',selected),('fixed cold',prediction['cold']),('fixed f-k',prediction['fk'])]:
            axes[0].plot(t,values[:,trace],label=label,linewidth=1)
        axes[0].set(xlabel='Time (s)',ylabel='Uncalibrated amplitude',title=f'First predefined hidden trace: {trace}')
        axes[0].legend(fontsize=8);axes[0].grid(alpha=.2)
        axes[1].scatter(np.flatnonzero(known),np.ones(known.sum()),marker='|',s=100,label='Retained K')
        axes[1].scatter(np.flatnonzero(hidden),np.zeros(hidden.sum()),marker='x',label='Common hidden H')
        axes[1].set(xlabel='Trace index',ylabel='Set membership',yticks=[0,1],yticklabels=['H','K'],title='K and H are disjoint; pilot included in K')
        axes[1].legend();save(fig,'viking_hidden_trace',dict(case=case,trace=trace,known=np.flatnonzero(known).tolist(),hidden=np.flatnonzero(hidden).tolist()))
    fig,ax=plt.subplots(figsize=(8,4),layout='constrained'); traces={}
    for method in ('Jitter','SA_Mu','Adaptive_Mu','Adaptive_Gap','Adaptive_Signal'):
        key=('f453_w512_' if method=='Adaptive_Signal' else '')+'hr271828_m60_s94217_'+method
        p=out/'viking/search'/(key+'.trace.csv');data=table(p);traces[method]=data
        ax.plot([int(r['evaluation']) for r in data],[float(r['best']) for r in data],label=method,marker='o' if len(data)==1 else None)
    ax.set(xlabel='Consumed objective evaluations',ylabel='Best objective (weighted for Adaptive_Signal)',title='Fixed condition: weighted and full-frequency objectives differ')
    ax.legend(fontsize=8);ax.grid(alpha=.2);save(fig,'search_trajectories',traces)
    # Independent explicit Gram verification for the exported actual SPS geometry.
    base=out/'checks/real_sps/attribute_evaluation'
    definition=json.loads((base/'sps_spatial_definition.json').read_text());coords=table(base/'sps_spatial_coordinates.csv');atoms=table(base/'sps_spatial_atoms.csv');summary=table(base/'sps_spatial_sampling_comparison.csv')
    xy=np.array([[float(r['x']),float(r['y'])] for r in coords]);frequencies=np.array([[float(r['cycles_per_x_unit']),float(r['cycles_per_y_unit'])] for r in atoms])
    dictionary=np.exp(2j*np.pi*((xy-[definition['origin_x'],definition['origin_y']])@frequencies.T));errors=[]
    for r in summary:
        p=base/'masks'/('sps_'+r['method']+'.txt');mask=np.loadtxt(p,comments='#',dtype=int)
        if set(mask)-{0,1} or mask.sum()!=int(r['receivers']):raise ValueError('Invalid SPS mask')
        a=dictionary[mask.astype(bool)];gram=a.conj().T@a/mask.sum();np.fill_diagonal(gram,0)
        error=abs(abs(gram).max()-float(r['spatial_fourier_mu']));errors.append(float(error))
        if error>1e-11:raise ValueError('SPS explicit Gram mismatch')
    fig,axes=plt.subplots(1,2,figsize=(12,4),layout='constrained')
    axes[0].scatter(xy[:,0],xy[:,1],s=1);axes[0].set(xlabel='X (source coordinate units)',ylabel='Y (source coordinate units)',title=f'SPS: {len(xy)} actual receivers');axes[0].set_aspect('equal')
    axes[1].bar(np.arange(len(summary)),[float(r['spatial_fourier_mu']) for r in summary]);axes[1].set_xticks(np.arange(len(summary)),[r['method'] for r in summary],rotation=25,ha='right')
    axes[1].set(ylabel='Finite 2D Fourier dictionary coherence',title='Geometry result; no waveform/imaging inference')
    save(fig,'sps_geometry',dict(comparison=summary,gram_absolute_errors=errors))
    (out/'sps_verification.json').write_text(json.dumps(dict(status='passed',points=len(xy),max_gram_error=max(errors)),indent=2)+'\n')
    # Catalog only this verified edition; old plots and GUI screenshots are excluded.
    names={'synthetic/search':'合成：等预算搜索','synthetic/budget':'合成：预算回收与共同评价次数',
        'synthetic/recovery':'合成：共同隐藏道恢复','viking/recovery':'Viking：恢复比较',
        'viking/geometry':'Viking：覆盖与互干度','viking/conditions':'Viking：预设窗口与隐藏集',
        'f3/recovery':'F3：重复迁移布局对照','diagnostics/viking_baselines':'Viking：基线与幅度诊断',
        'diagnostics/f3_baselines':'F3：基线与幅度诊断','diagnostics/viking_gathers':'Viking：预设道集与残差',
        'diagnostics/viking_hidden_trace':'Viking：隐藏道波形与布局','diagnostics/search_trajectories':'搜索：实际评价轨迹',
        'diagnostics/sps_geometry':'SPS：真实几何与二维目标'}
    catalog=[]
    for p in sorted((FIGURES/'research/current').rglob('*.png')):
        data=p.with_suffix('.data.json')
        if not data.is_file():raise ValueError('Image has no numerical provenance: '+str(p))
        with Image.open(p) as im:
            im.verify()
        with Image.open(p) as im:
            if min(im.size)<500:raise ValueError('Insufficient image resolution')
            size=list(im.size)
        key=p.relative_to(FIGURES/'research/current').with_suffix('').as_posix()
        title=names[key]
        description='数据来自本轮完整实验与独立核验。SNR仅在共同隐藏道计算，负值保留；种子和相邻窗口不增加独立工区数。'
        if key.startswith('f3') or key=='diagnostics/f3_baselines':description+=' F3为已见资料的重复迁移，不能当作新的未见工区。'
        catalog.append(dict(path=p.relative_to(FIGURES).as_posix(),title=title,description=description,
            sha256=digest(p),data_sha256=digest(data),pixels=size))
    if len(catalog)!=13:raise ValueError('Incomplete current figure edition: '+str(len(catalog)))
    (FIGURES/'current.tsv').write_text(''.join(f"{r['path']}\t{r['title']}\t{r['description']}\n" for r in catalog),encoding='utf8')
    (out/'figure_verification.json').write_text(json.dumps(dict(status='passed',images=catalog,source_sha256=sources),ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    cards=''.join('<h2>'+html.escape(r['title'])+'</h2><p>'+html.escape(r['description'])+'</p><img src="'+r['path']+'">' for r in catalog)
    (FIGURES/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>毕业实验最新结果</title><style>body{max-width:1200px;margin:30px auto;font:16px sans-serif}img{width:100%}</style><h1>当前已核验实验结果</h1>'+cards,encoding='utf8')
    return catalog


if __name__=='__main__':
    run(ROOT/'results/current')
