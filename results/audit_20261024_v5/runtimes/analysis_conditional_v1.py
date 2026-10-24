"""Generate traceable tables and Chinese thesis figures from frozen v4 and measured v5 CSVs."""
import argparse
from collections import defaultdict
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from ablation_stats import wilcoxon_signed_rank,holm
from audit_joint_studies import rows,write_rows
from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]
V4=ROOT/'results/review_20261024_v4/final'


def group(data,keys):
    result=defaultdict(list)
    for r in data:result[tuple(r[k] for k in keys)].append(r)
    return result


def confidence(values,seed=54127):
    v=np.asarray(values,float);rng=np.random.default_rng(seed)
    means=v[rng.integers(len(v),size=(10000,len(v)))].mean(axis=1)
    low,high=np.quantile(means,[.025,.975]);return float(low),float(high)


def paired(a,b):
    a,b=np.asarray(a,float),np.asarray(b,float);d=a-b
    plus,minus,p,n=wilcoxon_signed_rank(a.tolist(),b.tolist());low,high=confidence(d)
    return dict(n=len(d),difference=float(d.mean()),ci_low=low,ci_high=high,p_raw=p,
                rank_biserial=(plus-minus)/(plus+minus) if plus+minus else 0.,n_nonzero=n)


def correction(records):
    for r,p in zip(records,holm([r['p_raw'] for r in records])):r['p_holm']=p;r['family_size']=len(records)
    return records


def spearman(a,b):
    def ranks(v):
        return np.array([sum(y<x for y in v)+.5*(sum(y==x for y in v)-1) for x in v])
    a,b=ranks(a),ranks(b)
    return float(np.corrcoef(a,b)[0,1]) if np.std(a)>1e-12 and np.std(b)>1e-12 else None


def search_tables(root):
    data=rows(root/'search/search.csv');base=[r for r in data if (r['minimum'],r['window'])==('32','12')]
    comparisons=[];matched=[];summary=[]
    for (domain,rate),g in group(base,['domain','rate']).items():
        by={method:sorted(v,key=lambda r:int(r['seed_index'])) for (method,),v in group(g,['method']).items()}
        for method,g in by.items():
            vals=[float(r['mu']) for r in g];lo,hi=confidence(vals)
            summary.append(dict(domain=domain,rate=rate,method=method,mu=float(np.mean(vals)),ci_low=lo,ci_high=hi,
                                evals=float(np.mean([int(r['evals']) for r in g])),attempts=float(np.mean([int(r['attempts']) for r in g]))))
        a=by['Adaptive']
        for method,b in by.items():
            if method=='Adaptive':continue
            comparisons.append(dict(domain=domain,rate=rate,comparison='Adaptive-'+method,**paired([float(r['mu']) for r in a],[float(r['mu']) for r in b])))
            aa=[];bb=[];evaluations=[]
            for ra,rb in zip(a,b):
                ta=rows(root/'search/sensitivity'/(ra['id']+'.trace.csv'));tb=rows(root/'search/sensitivity'/(rb['id']+'.trace.csv'))
                k=min(len(ta),len(tb));aa.append(float(ta[k-1]['best']));bb.append(float(tb[k-1]['best']));evaluations.append(k)
            matched.append(dict(domain=domain,rate=rate,comparison='Adaptive-'+method,mean_common_evals=float(np.mean(evaluations)),**paired(aa,bb)))
    write_rows(root/'analysis/search_pairs.csv',correction(comparisons));write_rows(root/'analysis/common_evaluation_pairs.csv',matched)
    write_rows(root/'analysis/search_means.csv',summary)
    sensitivity=[]
    for (domain,rate,minimum,window),g in group([r for r in data if r['method']=='Adaptive'],['domain','rate','minimum','window']).items():
        sensitivity.append(dict(domain=domain,rate=rate,minimum=int(minimum),window=int(window),mu=float(np.mean([float(r['mu']) for r in g])),
                                switches_observed='see phase traces',n=len(g)))
    write_rows(root/'analysis/switch_sensitivity.csv',sensitivity)
    return summary,comparisons,matched


def joint_tables(root):
    data=rows(root/'joint/joint.csv');common=[r for r in data if r['design']=='common_H']
    model=[]
    for (design,m,method,name),g in group(data,['design','m','method','model']).items():
        for recovery in ['fixed','selected']:
            values=[float(r[recovery+'_snr']) for r in g if r[recovery+'_snr']!='']
            if values:model.append(dict(design=design,m=m,method=method,model=name,family=g[0]['family'],noise=g[0]['noise'],structure=g[0]['structure'],
                                        recovery=recovery,snr=float(np.mean(values)),mask_seeds=len(values)))
    write_rows(root/'analysis/joint_model_means.csv',model)
    stats=[];means=[]
    for (m,recovery),g in group([r for r in model if r['design']=='common_H'],['m','recovery']).items():
        by={method:sorted(v,key=lambda r:r['model']) for (method,),v in group(g,['method']).items()}
        for method,g in by.items():
            v=[r['snr'] for r in g];lo,hi=confidence(v)
            means.append(dict(m=m,recovery=recovery,method=method,mean_snr=float(np.mean(v)),ci_low=lo,ci_high=hi,model_conditions=len(g)))
            if method!='Adaptive':stats.append(dict(m=m,recovery=recovery,comparison='Adaptive-'+method,
                **paired([r['snr'] for r in by['Adaptive']],[r['snr'] for r in g])))
    write_rows(root/'analysis/joint_pairs.csv',correction(stats));write_rows(root/'analysis/joint_means.csv',means)
    associations=[]
    for (design,m,family,noise,structure),g in group(data,['design','m','family','noise','structure']).items():
        # At each condition, deduplicate deterministic masks; no inferential p values across repeated masks.
        unique={r['mask']:r for r in g};g=list(unique.values())
        for metric in ['mu','max_gap','nn_cv']:
            associations.append(dict(design=design,m=m,family=family,noise=noise,structure=structure,metric=metric,n_unique_masks=len(g),
                                     spearman=spearman([float(r[metric]) for r in g],[float(r['fixed_snr']) for r in g])))
    write_rows(root/'analysis/geometry_recovery_associations.csv',associations)
    return model,means,stats


def band_sensitivity(root):
    data=rows(V4/'search_physical_200/search.csv')+rows(V4/'search_uniform_200/search.csv');result=[]
    _,xreal=read_truth(ROOT/'data/real_seismic/real_gather_59x256.csv')
    for (domain,rate),g in group(data,['domain','rate_index']).items():
        seen=set()
        for row in g:
            if row['mask_sha1'] in seen:continue
            seen.add(row['mask_sha1']);folder=V4/('search_'+domain+'_200')
            mask=np.loadtxt(folder/row['mask_path'],comments='#',dtype=int).astype(bool)
            x=xreal if domain=='physical' else np.arange(128);n=len(x);period=np.ptp(x)*n/(n-1)
            for name,band,scale in [('default',1,1),('half_band',.5,1),('wide_band',1.5,1),('aperture_period',1,(n-1)/n)]:
                atoms=np.exp(-2j*np.pi*np.outer(np.arange(1,n)*band,x[mask]-x[0])/(period*scale))
                mu=float(abs(atoms.sum(axis=1)).max()/mask.sum())
                if name=='default':assert abs(mu-float(row['mu']))<2e-6
                result.append(dict(domain=domain,rate=rate,mask_sha1=row['mask_sha1'],profile=name,mu=mu))
    write_rows(root/'analysis/band_sensitivity.csv',result);summary=[]
    for (domain,rate),g in group(result,['domain','rate']).items():
        by={name:{r['mask_sha1']:r['mu'] for r in v} for (name,),v in group(g,['profile']).items()}
        keys=sorted(by['default'])
        for name,values in by.items():summary.append(dict(domain=domain,rate=rate,profile=name,masks=len(keys),
            rho_default=spearman([by['default'][k] for k in keys],[values[k] for k in keys])))
    write_rows(root/'analysis/band_rank_sensitivity.csv',summary);return summary


def read_truth(path):
    lines=path.read_text(encoding='utf-8-sig').splitlines();x=np.array([float(v) for s in lines if s.startswith('# offsets_m=') for v in s.split('=',1)[1].split(',')])
    return np.loadtxt(path,comments='#',delimiter=','),x


def figures(root,search,matched,joint,band):
    plt.rcParams.update({'font.family':['Microsoft YaHei','SimSun','DejaVu Sans'],'font.size':10,'axes.unicode_minus':False,
                         'svg.fonttype':'none','svg.hashsalt':'audit_joint_v5','axes.spines.top':False,'axes.spines.right':False})
    folder=root/'figures';folder.mkdir(exist_ok=False);index=[]
    def save(fig,name,source):
        fig.savefig(folder/(name+'.png'),dpi=220,bbox_inches='tight');fig.savefig(folder/(name+'.svg'),bbox_inches='tight');plt.close(fig)
        (folder/(name+'.data.json')).write_text(json.dumps(source,ensure_ascii=False,indent=2),encoding='utf-8');index.append(name)
    # Recompute each plotted v4 mean from its original CSV and compare with its archived summary.
    frozen=json.loads((V4/'summary.json').read_text());plot=[]
    fig,axes=plt.subplots(2,2,figsize=(11,7),layout='constrained')
    for di,domain in enumerate(['uniform','physical']):
        for bi,budget in enumerate([200,600]):
            source=rows(V4/f'search_{domain}_{budget}/search.csv');ax=axes[di,bi]
            for method in ['SAOnly','Hybrid','SA_Budget','Adaptive']:
                means=[];low=[];high=[]
                for rate in range(3):
                    g=[r for r in source if r['method']==method and r['rate_index']==str(rate)];v=[float(r['mu']) for r in g];mean=float(np.mean(v));a,b=confidence(v)
                    old=next(r for r in frozen['search'] if r['domain']==domain and r['budget']==budget and r['rate']==rate and r['method']==method)
                    assert abs(old['mu']-mean)<1e-12
                    plot.append(dict(domain=domain,budget=budget,method=method,rate=rate,mean=mean,ci_low=a,ci_high=b,n=len(g)))
                    means.append(mean);low.append(mean-a);high.append(b-mean)
                ax.errorbar(np.arange(3),means,yerr=[low,high],marker='o',capsize=3,label=method)
            ax.set_xticks([0,1,2],['38/128','64/128','90/128'] if domain=='uniform' else ['20/59','30/59','40/59'])
            ax.set(title=('均匀候选' if domain=='uniform' else '头字段 offset 候选')+f'，评价上限 {budget}',xlabel='实际采样比例',ylabel='Fourier 互干度 μ')
            ax.legend(fontsize=8)
    fig.suptitle('v4 归档搜索：每组 20 个种子；误差线为条件 bootstrap 95% CI')
    save(fig,'01_v4_search',plot)
    fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
    for ax,domain in zip(axes,['uniform','physical']):
        g=[r for r in matched if r['domain']==domain and r['comparison'] in ['Adaptive-NoRecycle','Adaptive-Hybrid','Adaptive-SA_Budget']]
        for i,r in enumerate(g):
            ax.errorbar(i,r['difference'],yerr=[[r['difference']-r['ci_low']],[r['ci_high']-r['difference']]],fmt='o',capsize=3)
        ax.axhline(0,color='black',lw=.8);ax.set_xticks(range(len(g)),[r['comparison'].replace('Adaptive-','对 ')+f"\n档 {r['rate']}" for r in g],rotation=25,ha='right')
        ax.set(title=domain+'：各配对共同实际评价次数',ylabel='μ 差（Adaptive − 对照；负值更低）')
    fig.suptitle('v5：10 个固定几何种子；95% CI；并非独立工区')
    save(fig,'02_common_budget',matched)
    nested=[r for r in rows(V4/'real_nested.csv') if r['case'].startswith('real_keep')]
    fig,axes=plt.subplots(1,3,figsize=(11,4),sharey=True,layout='constrained')
    for ax,m in zip(axes,[20,30,40]):
        for r in nested:
            if int(r['keep'])==m:ax.plot([0,1],[float(r['snr_fixed']),float(r['snr_selected'])],'o-',alpha=.6,lw=.8)
        ax.axhline(0,color='black',ls='--',lw=.8);ax.set_xticks([0,1],['默认恢复','内部选择+幅度系数']);ax.set(title=f'保留 {m}/59',ylabel='外层隐藏道 SNR / dB')
    fig.suptitle('v4 条件性真实诊断：1 个未确认编码来源；每档 10 个相同外层划分')
    save(fig,'03_v4_nested_real',nested)
    fig,axes=plt.subplots(1,2,figsize=(11,4.5),sharey=True,layout='constrained')
    for ax,recovery in zip(axes,['fixed','selected']):
        for method in ['Jitter','SAOnly','Hybrid','SA_Budget','Adaptive']:
            g=sorted([r for r in joint if r['recovery']==recovery and r['method']==method],key=lambda r:int(r['m']))
            y=np.array([r['mean_snr'] for r in g]);a=np.array([r['ci_low'] for r in g]);b=np.array([r['ci_high'] for r in g])
            ax.errorbar(range(3),y,yerr=[y-a,b-y],marker='o',capsize=2,label=method)
        ax.set_xticks(range(3),['38/128','64/128','90/128']);ax.set(title='固定默认曲波' if recovery=='fixed' else '保留道内部选择',xlabel='实际保留数 / 全候选数',ylabel='共同 H 的 SNR / dB');ax.legend(fontsize=8)
        ax.axhline(0,color='black',lw=.8)
    fig.suptitle('v5：共同外层 H=16 道；12 个解析模型条件 × 每法 3 掩码；模型条件均值及 95% CI')
    save(fig,'04_common_H_recovery',joint)
    weight=rows(V4/'synthetic_reconstruction.csv');seeds=sorted({int(r['seed']) for r in weight})[:3];plot=[]
    fig,axes=plt.subplots(1,3,figsize=(12,4.5),layout='constrained');values_all=[]
    for rate in range(3):
        cells={}
        for g in [0.,.1,.3]:
            for n in [0.,.1,.3]:
                sample=[r for r in weight if int(r['rate_index'])==rate and int(r['seed']) in seeds and r['method'] in ['Adaptive','W_0_01','W_0_03','W_01_0','W_01_01','W_01_03','W_03_0','W_03_01','W_03_03'] and float(r['lambda_gap'])==g and float(r['lambda_nn'])==n]
                assert len(sample)==18
                record=dict(rate=rate,lambda_gap=g,lambda_nn=n,snr=float(np.mean([float(r['snr_missing']) for r in sample])),n_models=6,n_masks_per_model=3)
                plot.append(record);cells[g,n]=record['snr'];values_all.append(record['snr'])
        ax=axes[rate];z=np.array([[cells[g,n] for n in [0.,.1,.3]] for g in [0.,.1,.3]])
        ax.set_xticks(range(3),[0,.1,.3]);ax.set_yticks(range(3),[0,.1,.3]);ax.set(xlabel='最近邻 CV 权重',ylabel='最大间隔权重',title=['38/128','64/128','90/128'][rate]);ax._audit_z=z
    low,high=min(values_all),max(values_all)
    for ax in axes:
        z=ax._audit_z;im=ax.imshow(z,cmap='viridis',vmin=low,vmax=high)
        for i in range(3):
            for j in range(3):ax.text(j,i,f'{z[i,j]:.2f}',ha='center',va='center',color='white' if z[i,j]<(low+high)/2 else 'black')
    fig.colorbar(im,ax=list(axes),label='缺失道平均 SNR / dB',shrink=.8);fig.suptitle('v4 九组权重：6 个解析实现 × 每组 3 掩码；统一恢复器；探索性，未按 SNR 选权重')
    save(fig,'05_v4_weight_tradeoff',plot)
    real=rows(root/'real/real.csv');fig,axes=plt.subplots(1,2,figsize=(11,4.5),layout='constrained')
    labels=['native_full','native_crop','upsampled_full','upsampled_crop']
    for field in ['fixed_snr','selected_snr']:
        for _,splits in group(real,['m','seed']).items():
            values={r['window']:float(r[field]) for r in splits}
            axes[0].plot(range(4),[values[name] for name in labels],color='gray',alpha=.25,lw=.6)
        means=[np.mean([float(r[field]) for r in real if r['window']==name]) for name in labels]
        axes[0].plot(range(4),means,'o-',label=field)
    axes[0].axhline(0,color='black',lw=.8);axes[0].set_xticks(range(4),['8 ms 全段','8 ms 裁窗','4 ms 全段','4 ms 裁窗'],rotation=15);axes[0].set(ylabel='隐藏道 SNR / dB',title='预先固定窗口；每窗 6 个划分');axes[0].legend(fontsize=8)
    for m in [20,30,40]:
        g=[r for r in real if int(r['m'])==m];axes[1].scatter([float(r['prediction_energy_ratio']) for r in g],[2*float(r['cross_term']) for r in g],label=f'{m}/59')
    limits=axes[1].get_xlim();axes[1].plot(limits,limits,'k--',lw=.8,label='零预测误差边界');axes[1].set(xlabel='预测能量 / 真值能量',ylabel='2〈预测, 真值〉/ 真值能量',title='默认恢复误差：低于虚线时 SNR < 0');axes[1].legend(fontsize=8)
    fig.suptitle('v5：1 个未确认编码来源；每窗 20/30/40 道各 2 划分；细线为全部划分，跨窗差非算法增益')
    save(fig,'06_real_windows_and_failure',real)
    fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
    profiles=['half_band','wide_band','aperture_period']
    for ax,domain in zip(axes,['uniform','physical']):
        for name in profiles:
            g=sorted([r for r in band if r['domain']==domain and r['profile']==name],key=lambda r:int(r['rate']))
            ax.plot(range(3),[r['rho_default'] for r in g],'o-',label=name)
        ax.set(ylim=(-1.05,1.05),title=domain,ylabel='与默认 μ 排名的 Spearman ρ',xlabel='采样档位');ax.set_xticks(range(3),['低','中','高']);ax.legend(fontsize=8)
    fig.suptitle('固定 v4 掩码的频带/周期敏感性；仅描述评分变化，不重新按测试恢复挑选定义')
    save(fig,'07_band_sensitivity',band)
    (folder/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>论文补充图件 v5</title><style>body{font-family:sans-serif;max-width:1200px;margin:auto}img{width:100%}</style><h1>论文补充图件</h1>'+''.join(f'<h2>{n}</h2><p>PNG / SVG / data.json 均由归档数据生成</p><img src="{n}.png">' for n in index),encoding='utf-8')
    return index


def run(root):
    analysis=root/'analysis';analysis.mkdir(exist_ok=False)
    search,pairs,matched=search_tables(root);model,joint,statistics=joint_tables(root);band=band_sensitivity(root)
    names=figures(root,search,matched,joint,band)
    real=rows(root/'real/real.csv');realmeans=[]
    for (window,m),g in group(real,['window','m']).items():realmeans.append(dict(window=window,m=m,n=len(g),
        fixed_snr=float(np.mean([float(r['fixed_snr']) for r in g])),selected_snr=float(np.mean([float(r['selected_snr']) for r in g]))))
    write_rows(analysis/'real_means.csv',realmeans)
    timing=rows(root/'timing/timing.csv');times=[]
    for (domain,method),g in group(timing,['domain','method']).items():times.append(dict(domain=domain,method=method,n=len(g),
        mean_evals=float(np.mean([int(r['evals']) for r in g])),mean_wall_ms=float(np.mean([float(r['wall_ms']) for r in g])),mean_mu=float(np.mean([float(r['mu']) for r in g]))))
    write_rows(analysis/'time_limit_means.csv',times)
    inputfiles=[root/'search/search.csv',root/'joint/joint.csv',root/'real/real.csv',root/'timing/timing.csv',V4/'summary.json',V4/'synthetic_reconstruction.csv',V4/'real_nested.csv']
    state=dict(script_sha256=digest(__file__),input_sha256={str(p):digest(p) for p in inputfiles},
               output_sha256={str(p.relative_to(root)):digest(p) for directory in [analysis,root/'figures'] for p in directory.rglob('*') if p.is_file()},
               plots=names,search_holm_tests=len(pairs),joint_holm_tests=len(statistics),n_model_conditions=12,independent_real_sources=1)
    (root/'analysis_manifest.json').write_text(json.dumps(state,indent=2))
    print(json.dumps({k:v for k,v in state.items() if k not in ['input_sha256','output_sha256']}))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);run(p.parse_args().root.resolve())
