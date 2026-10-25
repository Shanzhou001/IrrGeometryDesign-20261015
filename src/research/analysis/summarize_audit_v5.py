"""Generate traceable tables and Chinese thesis figures from frozen v4 and measured v5 CSVs."""
import argparse
from collections import defaultdict
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.analysis.ablation_stats import wilcoxon_signed_rank,holm
from research.common.tabular import rows, write_rows_if_any as write_rows
from research.common.hashing import digest
from research.common.figures import FigureWriter

from research.common.paths import ROOT, WORKSPACE
V4=WORKSPACE / 'results/review_20261024_v4/final'
TABLES='analysis'
STRUCTURAL=False


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
    write_rows(root/TABLES/'search_pairs.csv',correction(comparisons));write_rows(root/TABLES/'common_evaluation_pairs.csv',matched)
    write_rows(root/TABLES/'search_means.csv',summary)
    sensitivity=[]
    for (domain,rate,minimum,window),g in group([r for r in data if r['method']=='Adaptive'],['domain','rate','minimum','window']).items():
        sensitivity.append(dict(domain=domain,rate=rate,minimum=int(minimum),window=int(window),mu=float(np.mean([float(r['mu']) for r in g])),
                                switches_observed='see phase traces',n=len(g)))
    write_rows(root/TABLES/'switch_sensitivity.csv',sensitivity)
    return summary,comparisons,matched


def joint_tables(root):
    data=rows(root/'joint/joint.csv');common=[r for r in data if r['design']=='common_H']
    model=[]
    for (design,m,method,name),g in group(data,['design','m','method','model']).items():
        for recovery in ['fixed','selected']:
            values=[float(r[recovery+'_snr']) for r in g if r[recovery+'_snr']!='']
            if values:model.append(dict(design=design,m=m,method=method,model=name,family=g[0]['family'],noise=g[0]['noise'],structure=g[0]['structure'],
                                        recovery=recovery,snr=float(np.mean(values)),mask_seeds=len(values)))
    write_rows(root/TABLES/'joint_model_means.csv',model)
    stats=[];means=[]
    for (m,recovery),g in group([r for r in model if r['design']=='common_H'],['m','recovery']).items():
        by={method:sorted(v,key=lambda r:r['model']) for (method,),v in group(g,['method']).items()}
        if STRUCTURAL:
            # Two noise conditions share each clean template. Pool them before inference.
            by={method:[dict(model=family+'_'+structure,snr=float(np.mean([r['snr'] for r in same])))
                        for (family,structure),same in sorted(group(values,['family','structure']).items())]
                for method,values in by.items()}
        for method,g in by.items():
            v=[r['snr'] for r in g];lo,hi=confidence(v)
            means.append(dict(m=m,recovery=recovery,method=method,mean_snr=float(np.mean(v)),ci_low=lo,ci_high=hi,
                              analysis_units=len(g),unit='clean structural template' if STRUCTURAL else 'fixed model/noise condition'))
            if method!='Adaptive':stats.append(dict(m=m,recovery=recovery,comparison='Adaptive-'+method,
                **paired([r['snr'] for r in by['Adaptive']],[r['snr'] for r in g])))
    write_rows(root/TABLES/'joint_pairs.csv',correction(stats));write_rows(root/TABLES/'joint_means.csv',means)
    associations=[]
    for (design,m,family,noise,structure),g in group(data,['design','m','family','noise','structure']).items():
        # At each condition, deduplicate deterministic masks; no inferential p values across repeated masks.
        unique={r['mask']:r for r in g};g=list(unique.values())
        for metric in ['mu','max_gap','nn_cv']:
            associations.append(dict(design=design,m=m,family=family,noise=noise,structure=structure,metric=metric,n_unique_masks=len(g),
                                     spearman=spearman([float(r[metric]) for r in g],[float(r['fixed_snr']) for r in g])))
    write_rows(root/TABLES/'geometry_recovery_associations.csv',associations)
    return model,means,stats


def band_sensitivity(root):
    data=rows(V4/'search_physical_200/search.csv')+rows(V4/'search_uniform_200/search.csv');result=[]
    _,xreal=read_truth(WORKSPACE / 'data/real_seismic/real_gather_59x256.csv')
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
    write_rows(root/TABLES/'band_sensitivity.csv',result);summary=[]
    for (domain,rate),g in group(result,['domain','rate']).items():
        by={name:{r['mask_sha1']:r['mu'] for r in v} for (name,),v in group(g,['profile']).items()}
        keys=sorted(by['default'])
        for name,values in by.items():summary.append(dict(domain=domain,rate=rate,profile=name,masks=len(keys),
            rho_default=spearman([by['default'][k] for k in keys],[values[k] for k in keys])))
    write_rows(root/TABLES/'band_rank_sensitivity.csv',summary);return summary


def read_truth(path):
    lines=path.read_text(encoding='utf-8-sig').splitlines();x=np.array([float(v) for s in lines if s.startswith('# offsets_m=') for v in s.split('=',1)[1].split(',')])
    return np.loadtxt(path,comments='#',delimiter=','),x


def figures(root,search,matched,joint,band):
    plt.rcParams.update({'font.family':['Microsoft YaHei','SimSun','DejaVu Sans'],'font.size':10,'axes.unicode_minus':False,
                         'svg.fonttype':'none','svg.hashsalt':'audit_joint_v5','axes.spines.top':False,'axes.spines.right':False})
    writer=FigureWriter('research/v5');folder=writer.directory;index=[]
    def save(fig,name,source):
        writer.save(fig,name,source);index.append(name)
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
            ax.set_ylim(bottom=0)
            ax.legend(fontsize=8)
    fig.suptitle('v4 归档搜索：每组 20 个种子；误差线为条件 bootstrap 95% CI')
    save(fig,'01_v4_search',plot)
    fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
    for ax,domain in zip(axes,['uniform','physical']):
        for i,method in enumerate(['Hybrid','SA_Budget','NoRecycle']):
            g=sorted([r for r in matched if r['domain']==domain and r['comparison']=='Adaptive-'+method],key=lambda r:int(r['rate']))
            value=np.array([r['difference'] for r in g]);lo=np.array([r['ci_low'] for r in g]);hi=np.array([r['ci_high'] for r in g])
            ax.errorbar(np.arange(3)+(i-1)*.18,value,yerr=[value-lo,hi-value],fmt='o',capsize=3,label='对 '+method)
        ax.axhline(0,color='black',lw=.8)
        ax.set_xticks(range(3),['38/128','64/128','90/128'] if domain=='uniform' else ['20/59','30/59','40/59'])
        ax.set_xlabel('实际采样比例');ax.legend(fontsize=8)
        ax.set(title=('均匀候选' if domain=='uniform' else '头 offset 候选')+'：共同实际评价次数',ylabel='μ 差（Adaptive − 对照；负值更低）')
    fig.suptitle('v5：B=300，10 个固定几何种子；95% CI；并非独立工区')
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
    fig.suptitle('v5：共同 H=16 道，允许候选 112 道，评价上限 B=300\n6 个结构模板 ×2 噪声条件 ×3 掩码；结构模板 bootstrap 95% CI' if STRUCTURAL else
                 'v5：共同外层 H=16 道；12 个解析模型条件 × 每法 3 掩码；条件均值及 95% CI')
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
    fig.colorbar(im,ax=list(axes),label='缺失道平均 SNR / dB',shrink=.8);fig.suptitle('v4 九组权重，B=200：6 个解析实现 × 每组 3 掩码；统一恢复器；未按 SNR 选权重')
    save(fig,'05_v4_weight_tradeoff',plot)
    real=rows(root/'real/real.csv');fig,axes=plt.subplots(1,2,figsize=(11,4.5),layout='constrained')
    labels=['native_full','native_crop','upsampled_full','upsampled_crop']
    for field in ['fixed_snr','selected_snr']:
        for _,splits in group(real,['m','seed']).items():
            values={r['window']:float(r[field]) for r in splits}
            axes[0].plot(range(4),[values[name] for name in labels],color='gray',alpha=.25,lw=.6)
        means=[np.mean([float(r[field]) for r in real if r['window']==name]) for name in labels]
        axes[0].plot(range(4),means,'o-',label='固定默认恢复' if field=='fixed_snr' else '内部选择+幅度系数')
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
            if all(r['rho_default'] is None for r in g):
                ax.text(.02,.04,'aperture_period：μ 恒为 1，ρ 未定义',transform=ax.transAxes,fontsize=8)
                continue
            ax.plot(range(3),[r['rho_default'] for r in g],'o-',label={'half_band':'半带宽（0.5）','wide_band':'扩展带宽（1.5）','aperture_period':'周期取孔径'}[name])
        ax.set(ylim=(-1.05,1.05),title='均匀候选' if domain=='uniform' else '头 offset 候选',ylabel='与默认 μ 排名的 Spearman ρ',xlabel='采样档位');ax.set_xticks(range(3),['低','中','高']);ax.legend(fontsize=8)
    fig.suptitle('固定 v4 掩码的频带/周期敏感性；仅描述评分变化，不重新按测试恢复挑选定义')
    save(fig,'07_band_sensitivity',band)
    # Convergence uses only the common unpenalized target. Padding is display-only.
    fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained');plot=[]
    for ax,domain in zip(axes,['uniform','physical']):
        source=rows(V4/f'search_{domain}_200/search.csv')
        for method in ['SAOnly','Hybrid','SA_Budget','Adaptive']:
            sample=[r for r in source if r['method']==method and r['rate_index']=='1'];traces=[]
            for r in sample:
                t=np.array([float(v['best_objective']) for v in rows(V4/f'search_{domain}_200'/(r['mask_path']+'.trace.csv'))])
                traces.append(np.pad(t,(0,200-len(t)),mode='edge'))
            values=np.array(traces);mean=values.mean(axis=0);half=2.0930240544*values.std(axis=0,ddof=1)/np.sqrt(20)
            ax.plot(np.arange(1,201),mean,label=method);ax.fill_between(np.arange(1,201),mean-half,mean+half,alpha=.12)
            plot.append(dict(domain=domain,method=method,n=20,mean=mean.tolist(),pointwise_t95_halfwidth=half.tolist(),actual_evals=[r['evals'] for r in sample]))
        ax.set(title='均匀候选：64/128，B=200' if domain=='uniform' else '头 offset 候选：30/59，B=200',xlabel='目标评价次数（尾部延长仅供显示）',ylabel='历史最优 μ');ax.legend(fontsize=8)
        ax.set_ylim(bottom=0)
    fig.suptitle('v4：20 种子均值与逐点 t 区间；提前停止后的值延长不计作新评价')
    save(fig,'08_v4_convergence',plot)
    source=rows(V4/'search_uniform_200/search.csv')
    weight_names={'Adaptive','W_0_01','W_0_03','W_01_0','W_01_01','W_01_03','W_03_0','W_03_01','W_03_03'}
    sample=list({r['mask_sha1']:r for r in source if r['rate_index']=='1' and r['method'] in weight_names}.values())
    v=np.array([[float(r[k]) for k in ['mu','max_gap','nn_cv']] for r in sample])
    nondominated=[not np.any(np.all(v<=point+1e-8,axis=1)&np.any(v<point-1e-8,axis=1)) for point in v]
    fig,ax=plt.subplots(figsize=(7,4),layout='constrained');im=ax.scatter(v[:,0],v[:,1],c=v[:,2],cmap='viridis',alpha=.7,s=20)
    ax.scatter(v[nondominated,0],v[nondominated,1],facecolors='none',edgecolors='red',s=75,label='三指标未支配点')
    ax.set(xlabel='Fourier 互干度 μ',ylabel='最大含边界间隔 / 索引间距',title='v4 九组权重：64/128；20 种子；按掩码去重');ax.legend(fontsize=8)
    fig.colorbar(im,ax=ax,label='最近邻 CV')
    pareto=[dict(mask_sha1=r['mask_sha1'],method=r['method'],mu=float(r['mu']),max_gap=int(r['max_gap']),nn_cv=float(r['nn_cv']),nondominated=bool(flag)) for r,flag in zip(sample,nondominated)]
    save(fig,'09_v4_geometry_pareto',pareto)
    # Predefined first real case, not the most unfavorable result selected after inspection.
    case='native_full_m20_s0';truth,x=read_truth(root/'real/truth/native_full.csv')
    r=next(r for r in real if r['case']==case);mask=np.array(list(r['mask']),int).astype(bool)
    fixed=np.loadtxt(root/'real/native'/(case+'_fixed')/'prediction.csv',delimiter=',')
    selected=np.loadtxt(root/'real/selected'/(case+'.csv'),delimiter=',');hidden=np.flatnonzero(~mask)
    selected_columns=[int(hidden[0]),int(hidden[len(hidden)//2]),int(hidden[-1])]
    fig,axes=plt.subplots(3,1,figsize=(10,7),sharex=True,layout='constrained')
    for ax,j in zip(axes,selected_columns):
        ax.plot(np.arange(len(truth))*.008,truth[:,j],color='black',lw=.9,label='参考波形（编码未确认）')
        ax.plot(np.arange(len(truth))*.008,fixed[:,j],lw=.9,label='固定默认曲波')
        ax.plot(np.arange(len(truth))*.008,selected[:,j],lw=.9,alpha=.8,label='内部选择后')
        ax.set(title=f'隐藏列 {j}；头 offset={x[j]:g}（声明米）',ylabel='未标定原幅度');ax.legend(fontsize=8)
    axes[-1].set_xlabel('按道头 8 ms 定义的时间 / s')
    fig.suptitle(f"预设失败诊断案例 {case}：默认 SNR={float(r['fixed_snr']):.3f} dB；不据图调整恢复参数")
    save(fig,'10_real_failure_waveforms',dict(case=case,columns=selected_columns,record=r))
    writer.gallery('论文补充图件：由归档数据生成的 PNG 和数据说明')
    return index


def run(root):
    analysis=root/TABLES;analysis.mkdir(exist_ok=False)
    search,pairs,matched=search_tables(root);model,joint,statistics=joint_tables(root);band=band_sensitivity(root)
    names=[]  # Rendering is a separate, overwrite-safe pipeline stage.
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
               output_sha256={str(p.relative_to(root)):digest(p) for p in analysis.rglob('*') if p.is_file()},
               plots=names,search_holm_tests=len(pairs),joint_holm_tests=len(statistics),n_model_conditions=12,
               structural_templates=6,structural_analysis=STRUCTURAL,independent_real_sources=1)
    (root/(TABLES+'_manifest.json')).write_text(json.dumps(state,indent=2))
    print(json.dumps({k:v for k,v in state.items() if k not in ['input_sha256','output_sha256']}))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path)
    p.add_argument('--structural',action='store_true',help='pool the two noise conditions sharing each clean template')
    p.add_argument('--paper',action='store_true',help='final thesis edition, including convergence, Pareto and predefined failure case')
    a=p.parse_args();STRUCTURAL=a.structural or a.paper
    if STRUCTURAL:TABLES='analysis_structural'
    if a.paper:TABLES='analysis_thesis'
    run(a.root.resolve())
