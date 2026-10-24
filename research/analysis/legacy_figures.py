"""Render legacy displays from their own archived CSVs, with explicit replay traces.

Deduplication uses the figure kind and input hashes. Equal old PNG bytes alone
cannot establish equality: some old display files were stale copies.
"""
import csv
import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import ROOT, executable


def table(path):
    with Path(path).open(encoding='utf-8-sig') as stream:
        return list(csv.DictReader(line for line in stream if not line.startswith('#')))


def logical(old):
    for before, after in [('figures/enhanced_20261023/', 'legacy/v3/'),
                          ('figures/revised_20261023/', 'legacy/v2/'),
                          ('figures/real/', 'legacy/real/')]:
        if old.startswith(before):
            return after + old[len(before):]
    return 'legacy/baseline/' + Path(old).name


def describe(old):
    name = logical(old).replace('legacy/v3/validation/', 'legacy/v3/')
    kind = Path(name).name[:2]
    base = ROOT/('results/enhanced_20261023' if '/v3/' in name else
                 'results/revised_20261023' if '/v2/' in name else 'results')
    benchmark = base/'synthetic/01_sampling_benchmark'
    attributes = base/'real_sps/attribute_evaluation'
    reconstruction = base/'synthetic/03_wavefield_reconstruction'
    if 'real_reference' in name or name.startswith('legacy/real/'):
        reconstruction = base/'real_field'
    for label, folder in [('mask_builtin','mask_recon_builtin'),('mask_dipping','mask_recon_dipping'),('mask_faulted','mask_recon_faulted')]:
        if '/' + label + '/' in name:
            # The old native loop left its last case in these two display files.
            row = table(base/folder/'mask_recon_study.csv')[-1]
            tag = f"{row['mask_type']}_r{round(float(row['rate'])*100)}_seed{row['seed']}"
            reconstruction = base/folder/('run_' + tag)
    files = {
        '00': [benchmark/'benchmark_comparison_table.csv', reconstruction/'recon_metrics_summary.csv', attributes/'attribute_fold_map_matrix.csv'],
        '01': [benchmark/'benchmark_convergence_curves.csv'],
        '02': [benchmark/'benchmark_comparison_table.csv'],
        '03': [attributes/'attribute_fold_map_matrix.csv'],
        '04': [attributes/'attribute_offset_distribution.csv', attributes/'attribute_azimuth_rose.csv'],
        '05': [reconstruction/'recon_metrics_summary.csv', reconstruction/'cmp_mask.txt'],
        '06': [reconstruction/'recon_metrics_summary.csv', reconstruction/'cmp_mask.txt'],
    }[kind]
    if kind in ['05', '06']:
        files += [p for p in reconstruction.glob('recon_*gather.csv')]
    assert all(p.is_file() for p in files), files
    return dict(name=name, kind=kind, benchmark=benchmark, attributes=attributes,
                reconstruction=reconstruction, inputs=files)


def plan():
    baseline = json.loads((ROOT/'results/refactor_20261024/baseline.json').read_text(encoding='utf-8'))
    old = [p for p in baseline['images'] if p.startswith('figures/') and not p.startswith('figures/gui/')]
    jobs, owners, aliases = {}, {}, {}
    for path in sorted(old, key=lambda p: ('validation' in p, 0 if 'enhanced' in p else 1 if 'revised' in p else 2, p)):
        item = describe(path)
        identity = (item['kind'], tuple(digest(p) for p in item['inputs']))
        if identity not in owners:
            owners[identity] = item['name']; jobs[item['name']] = item
        aliases[path] = 'figures/' + owners[identity]
        aliases['figures/' + item['name']] = 'figures/' + owners[identity]
    return jobs, aliases


def gather(path):
    lines = Path(path).read_text(encoding='utf-8-sig').splitlines()
    first = next(i for i, line in enumerate(lines) if line and not line.startswith('#'))
    data = np.loadtxt(path, delimiter=',', skiprows=first+1)
    return data[:, 1:], data[:, 0]


def render(out, run_native):
    jobs, aliases = plan(); replay = {};records=[]
    for name, spec in jobs.items():
        group = str(Path(name).parent).replace('\\','/')
        kind = spec['kind']; data = dict(input_sha256={str(p.relative_to(ROOT)):digest(p) for p in spec['inputs']})
        if kind in ['05', '06']:
            source = spec['reconstruction']; key = source.relative_to(ROOT).as_posix()
            if key not in replay:
                target = out/'legacy_replay'/key.removeprefix('results/')
                mask = np.loadtxt(source/'cmp_mask.txt',comments='#',dtype=int)
                arguments=['--replay-reference',str(target),str(source/'cmp_mask.txt'),str(float(mask.mean()))]
                # Use the original input/model, never upsample a decimated display export.
                if source.name=='real_field':arguments.append(str(ROOT/'data/real_seismic/real_gather_256x128.csv'))
                elif 'mask_recon_dipping' in source.parts:arguments.append(str(ROOT/'data/synthetic_models/dipping.csv'))
                elif 'mask_recon_faulted' in source.parts:arguments.append(str(ROOT/'data/synthetic_models/faulted.csv'))
                run_native('legacy_' + str(len(replay)),arguments)
                replay[key] = target
            target = replay[key]
            old_truth=source/'recon_original_gather.csv'
            archived=old_truth.exists() and gather(old_truth)[0].shape==gather(target/'recon_original_gather.csv')[0].shape
            array_source = source if archived else target
            truth,t = gather(array_source/'recon_original_gather.csv')
            prediction,_ = gather(array_source/'recon_recovered_gather.csv')
            observed,_ = gather(array_source/'recon_subsampled_gather.csv')
            mask = np.loadtxt(source/'cmp_mask.txt',comments='#',dtype=int).astype(bool)
            current,_ = gather(target/'recon_recovered_gather.csv')
            data.update(source_mode='archived exported matrices' if archived else 'current deterministic replay; original matrices were not archived',
                        replay_path=str(target.relative_to(ROOT)), replay_max_abs_prediction_difference=float(abs(current-prediction).max()),
                        mask_sha256=digest(source/'cmp_mask.txt'), hidden_snr_db=float(10*np.log10(np.square(truth[:,~mask]).sum()/np.square(truth[:,~mask]-prediction[:,~mask]).sum())))
            if old_truth.exists():
                old_prediction,_=gather(source/'recon_recovered_gather.csv')
                sy,sx=current.shape[0]//old_prediction.shape[0],current.shape[1]//old_prediction.shape[1]
                assert current[::sy,::sx].shape==old_prediction.shape
                data['archived_export_shape']=list(old_prediction.shape)
                data['replay_difference_on_archived_grid']=float(abs(current[::sy,::sx]-old_prediction).max())
                if not archived:data['source_mode']='current full-resolution replay; old display export was decimated'
            if kind=='05':
                fig,axes=plt.subplots(1,4,figsize=(13,5),sharey=True,layout='constrained')
                limit=float(np.quantile(abs(truth),.995))
                for ax,array,title in zip(axes,[truth,observed,prediction,truth-prediction],['参考场','采样观测','曲波恢复','误差']):
                    image=ax.imshow(array,aspect='auto',cmap='RdBu_r',vmin=-limit,vmax=limit,extent=[0,array.shape[1]-1,t[-1],t[0]])
                    ax.set(title=title,xlabel='道索引')
                axes[0].set_ylabel('时间 / s');fig.colorbar(image,ax=axes,shrink=.6,label='参考场振幅单位')
                fig.suptitle(f"{group}：{mask.sum()}/{len(mask)} 道；缺失道 SNR={data['hidden_snr_db']:.3f} dB")
            else:
                trace=table(target/'recon_convergence.csv');hidden=np.flatnonzero(~mask);j=int(hidden[len(hidden)//2])
                fig,axes=plt.subplots(1,2,figsize=(11,4),layout='constrained')
                axes[0].plot([int(r['iteration']) for r in trace],[float(r['relative_step']) for r in trace])
                axes[0].set(title='当前程序重放：迭代相对步长',xlabel='迭代次数',ylabel='相对步长（不是隐藏误差）')
                axes[1].plot(t,truth[:,j],label='参考场');axes[1].plot(t,prediction[:,j],label='归档恢复' if archived else '当前重放恢复')
                axes[1].set(title=f'预设隐藏道 {j}',xlabel='时间 / s',ylabel='振幅');axes[1].legend();fig.suptitle(group)
                data['trace_column']=j
        elif kind=='01':
            rows=table(spec['inputs'][0]);fig,ax=plt.subplots(figsize=(9,4),layout='constrained')
            for rate in ['30','50','70']:ax.plot([int(r['Iteration']) for r in rows],[float(r['Best_Miu_'+rate+'Pct']) for r in rows],label=rate+'%')
            ax.set(title=group+'：归档历史最优互干度轨迹',xlabel='原始迭代索引',ylabel='μ',ylim=(0,None));ax.legend()
        elif kind in ['00','02']:
            rows=table(spec['benchmark']/'benchmark_comparison_table.csv');fig,ax=plt.subplots(figsize=(10,4),layout='constrained');x=np.arange(len(rows))
            for i,(column,label) in enumerate([('Regular_Miu','规则'),('Random_Miu_Mean','随机'),('Mean_Miu_SA','混合搜索')]):
                ax.bar(x+(i-1)*.24,[float(r[column]) for r in rows],.24,label=label)
            ax.set_xticks(x,[r['Sampling_Rate'] for r in rows]);ax.set(xlabel='归档采样比例',ylabel='μ',title=group+('：工作流程汇总' if kind=='00' else '：采样对照'));ax.legend()
            if kind=='00':
                metrics={r['Metric_Name']:r['Value'] for r in table(spec['reconstruction']/'recon_metrics_summary.csv')}
                text='；'.join(k+'='+metrics[k] for k in ['Active_Traces','Total_Traces','SNR_POCS_MissingOnly'] if k in metrics)
                fig.supxlabel('归档恢复：'+text+'；几何目标与恢复精度分别评价',fontsize=10)
                data['archived_metrics']=metrics
        elif kind=='03':
            array=np.loadtxt(spec['inputs'][0],delimiter=',',comments='#');fig,ax=plt.subplots(figsize=(10,4),layout='constrained')
            image=ax.imshow(array,origin='lower',aspect='auto',cmap='viridis');fig.colorbar(image,ax=ax,label='覆盖次数')
            ax.set(title=group+'：实测 SPS 的 CMP 覆盖',xlabel='CMP 列索引',ylabel='CMP 行索引')
        else:
            offsets=table(spec['inputs'][0]);angles=table(spec['inputs'][1]);fig=plt.figure(figsize=(11,4),layout='constrained')
            ax=fig.add_subplot(121);ax.bar([.5*(float(r['Offset_Min_m'])+float(r['Offset_Max_m'])) for r in offsets],[int(r['Trace_Count']) for r in offsets],width=float(offsets[0]['Offset_Max_m'])-float(offsets[0]['Offset_Min_m']))
            ax.set(xlabel='炮检距 / m',ylabel='道数',title=group+'：炮检距')
            ax=fig.add_subplot(122,projection='polar');ax.bar([np.deg2rad(.5*(float(r['Azimuth_Start_Deg'])+float(r['Azimuth_End_Deg']))) for r in angles],[int(r['Trace_Count']) for r in angles],width=np.deg2rad(15))
            ax.set_title('方位角 / 道数');ax.set_theta_zero_location('N');ax.set_theta_direction(-1)
        writer=FigureWriter(group);writer.save(fig,Path(name).stem,data)
        records.append(dict(path='figures/'+name,**data))
    return records,aliases
