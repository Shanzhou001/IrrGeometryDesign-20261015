"""Mapped public prestack waveforms: frozen geometry search, recovery and audit."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT,current_results,executable
from research.common.tabular import rows,write_rows
from research.data.field_segy import FieldSegy,mapping
from research.experiments.audit_joint_studies import Recorder,initial_mask
from research.experiments.confirmation_spatial import fk3d
from research.experiments.thesis_workflow import source_identity,require_identity
from research.recovery import fk_fista
from research.validation.artifacts import validate_native_steps

PROTOCOL=Path(__file__).with_name('mapped_field_protocol.json')


def hidden_mask(n,seed):
    result=np.zeros(n,bool);result[np.random.default_rng(seed).choice(n,n//10,replace=False)]=True
    return result


def dictionary(xy):
    xy=np.asarray(xy,float)
    if xy.ndim!=2 or xy.shape[1]!=2 or not np.isfinite(xy).all() or len(np.unique(xy,axis=0))!=len(xy):
        raise ValueError('Invalid surveyed coordinates')
    axes=[np.unique(xy[:,i]) for i in range(2)]
    periods=[float(a[-1]-a[0]+np.median(np.diff(a))) if len(a)>1 else 1. for a in axes]
    frequencies=np.array([(x/periods[0],y/periods[1]) for y in range(min(8,len(axes[1])))
                          for x in range(min(8,len(axes[0])))])
    return np.exp(2j*np.pi*((xy-xy.min(axis=0))@frequencies.T))


def coherence(atoms,mask):
    a=atoms[np.asarray(mask,bool)];gram=a.conj().T@a/len(a);np.fill_diagonal(gram,0)
    return float(abs(gram).max())


def selections(plan):
    base=ROOT/'data/public';geometries={};sources={}
    for survey,wave,mapname in [('poland_line001_field','Line_001.sgy','trace_source_receiver_mapping.csv'),
                                ('soda_lake_2010_field','F7733R1.SGY','F7733R1_trace_source_receiver_mapping.csv')]:
        segy=FieldSegy(base/survey/wave);joined=mapping(base/survey/mapname,segy);sources[survey]=segy
        if survey.startswith('poland'):
            spec=plan['poland']
            for ffid in spec['ffids']:
                lookup={int(r['channel']):r for r in joined if int(r['field_record'])==ffid}
                group=[lookup[k] for k in range(spec['first_channel'],spec['first_channel']+spec['receivers'])]
                xy=np.array([[float(r['receiver_x']),float(r['receiver_y'])] for r in group])
                geometries[f'poland_f{ffid}']=dict(survey=survey,shape=[spec['receivers']],xy=xy,
                    components={0:group},ffid=ffid)
        else:
            spec=plan['soda'];lookup={(int(r['component_block']),int(r['receiver_line']),int(r['receiver_point'])):r for r in joined}
            components={c:[lookup[c,line,point] for line in spec['receiver_lines'] for point in spec['receiver_points']]
                        for c in spec['components']}
            xy=np.array([[float(r['receiver_x_sps']),float(r['receiver_y_sps'])] for r in components[1]])
            for group in components.values():
                if not np.array_equal(xy,[[float(r['receiver_x_sps']),float(r['receiver_y_sps'])] for r in group]):
                    raise ValueError('Component station correspondence differs')
            geometries['soda_f7733']=dict(survey=survey,shape=[len(spec['receiver_lines']),len(spec['receiver_points'])],
                xy=xy,components=components,ffid=spec['ffid'])
    return geometries,sources


def search(recorder,plan,geometries):
    queries=[];groups={};initials={}
    for geometry,info in geometries.items():
        n=len(info['xy']);points='|'.join(','.join(format(v,'.17g') for v in xy) for xy in info['xy'])
        for h in plan['hidden_seeds']:
            hidden=hidden_mask(n,h);eligible=np.flatnonzero(~hidden)
            for fraction in plan['keep_fractions']:
                count=int(round(n*fraction))
                for seed in plan['seeds']:
                    key=f'{geometry}_h{h}_m{count}_s{seed}';initial=initial_mask(n,count,seed,~hidden)
                    initials[key]=initial;groups[key]=dict(geometry=geometry,h=h,m=count,seed=seed)
                    rng=np.random.default_rng(np.random.SeedSequence([h,count,seed,20261105]))
                    for i in range(plan['budget']):
                        mask=initial.copy() if i==0 else np.zeros(n,bool)
                        if i:mask[rng.choice(eligible,count,replace=False)]=True
                        bits=''.join(map(str,mask.astype(int)))
                        queries.append(';'.join(map(str,[key+f'_r{i:03d}',seed+1,1,1,bits,
                            ''.join(map(str,(~hidden).astype(int))),points])))
                    for method,prefix in [('SA',plan['budget']),('Hybrid',plan['hybrid_sa_prefix'])]:
                        queries.append(';'.join(map(str,[key+'_'+method,seed+1,plan['budget'],prefix,
                            ''.join(map(str,initial.astype(int))),''.join(map(str,(~hidden).astype(int))),points])))
    queryfile=recorder.out/'queries.txt';queryfile.write_text('\n'.join(queries)+'\n')
    target=recorder.out/'search';recorder.call('mapped_geometry_search',['--spatial-queries',queryfile,target],target)
    measured=rows(target/'queries.csv');lookup={r['id']:r for r in measured}
    if len(lookup)!=len(queries) or len(measured)!=len(queries):raise ValueError('Incomplete or duplicate native queries')
    layouts=[];audit=0.;trace=[]
    for key,info in groups.items():
        pool=[lookup[key+f'_r{i:03d}'] for i in range(plan['budget'])]
        best=min(range(len(pool)),key=lambda i:float(pool[i]['mu']));running=float('inf')
        for i,row in enumerate(pool):
            mask=np.array(list(row['mask']),int)
            if (int(row['evaluations'])!=1 or len(mask)!=len(geometries[info['geometry']]['xy']) or
                    set(mask)-{0,1} or mask.sum()!=info['m'] or
                    np.any(mask.astype(bool)&hidden_mask(len(mask),info['h']))):
                raise ValueError('Random candidate budget or eligibility changed')
            running=min(running,float(row['mu']));trace.append(dict(group=key,evaluation=i+1,
                mu=row['mu'],best_mu=running,mask=row['mask']))
        chosen={'Jitter':pool[0],'RandomBudget':pool[best],'SA':lookup[key+'_SA'],'Hybrid':lookup[key+'_Hybrid']}
        atoms=dictionary(geometries[info['geometry']]['xy']);hidden=hidden_mask(len(atoms),info['h'])
        for method,row in chosen.items():
            values=np.array(list(row['mask']),int)
            if len(values)!=len(atoms) or set(values)-{0,1}:raise ValueError('Invalid native mask encoding')
            mask=values.astype(bool)
            if (mask.sum()!=info['m'] or np.any(mask&hidden) or
                    int(row['evaluations'])!=(1 if method in ('Jitter','RandomBudget') else plan['budget'])):
                raise ValueError('Mask, holdout or native budget violation')
            audit=max(audit,abs(coherence(atoms,mask)-float(row['mu'])))
            layouts.append(dict(layout=key+'_'+method,**info,method=method,mask=row['mask'],mu=float(row['mu']),
                evaluations=1 if method=='Jitter' else plan['budget'],selected_random_index=best if method=='RandomBudget' else 0))
    if audit>2e-11:raise ValueError('Independent surveyed-coordinate Gram differs')
    write_rows(recorder.out/'layouts.csv',layouts);write_rows(recorder.out/'random_candidates.csv',trace)
    return layouts,dict(explicit_gram_max_error=audit,native_queries=len(queries),groups=len(groups))


def predict(data,known,kind,plan):
    data=np.asarray(data,float);known=np.asarray(known,bool)
    flat=data.reshape(len(data),-1);mask=known.ravel()
    if mask.shape!=(flat.shape[1],) or mask.sum()<2 or not np.isfinite(flat[:,mask]).all():
        raise ValueError('Invalid observed-only recovery input')
    if kind=='zero':result=np.zeros_like(flat)
    elif kind=='linear':
        # Fixed rank-axis interpolation baseline; no hidden values enter.
        if data.ndim==2:
            result=np.array([np.interp(np.arange(len(mask)),np.flatnonzero(mask),row[mask]) for row in flat])
        else:
            sites=np.array(list(np.ndindex(known.shape)),float);observed=sites[mask]
            distances=np.linalg.norm(sites[:,None,:]-observed[None,:,:],axis=2)
            neighbours=np.argsort(distances,axis=1,kind='stable')[:,:4]
            weights=1/np.maximum(np.take_along_axis(distances,neighbours,axis=1),1e-12)**2
            weights/=weights.sum(axis=1,keepdims=True)
            result=np.sum(flat[:,mask][:,neighbours]*weights,axis=2)
    elif kind=='fourier':
        if data.ndim==2:result=fk_fista(data,mask,plan['iterations'],plan['fraction'])[0]
        else:result=fk3d(data,known,plan['iterations'],plan['fraction']).reshape(flat.shape)
    else:raise ValueError('Unknown frozen recoverer')
    result[:,mask]=flat[:,mask]
    if not np.isfinite(result).all():raise ValueError('Nonfinite recovery')
    return result.reshape(data.shape)


def score(truth,prediction,hidden):
    truth=np.asarray(truth,float);prediction=np.asarray(prediction,float);hidden=np.asarray(hidden)
    if (truth.shape!=prediction.shape or hidden.dtype!=np.dtype(bool) or hidden.shape!=truth.shape[1:] or
            not hidden.any() or not np.isfinite(truth).all() or not np.isfinite(prediction).all()):
        raise ValueError('Invalid finite score inputs')
    a=truth[:,hidden];b=prediction[:,hidden];energy=float(np.sum(a*a))
    if energy<=0 or not np.isfinite(energy):raise ValueError('Zero/nonfinite hidden energy; no case deletion')
    nmse=float(np.sum((a-b)**2)/energy);norm=float(np.linalg.norm(a)*np.linalg.norm(b))
    return dict(snr=float(-10*np.log10(max(nmse,1e-30))),nmse=nmse,
                rms_ratio=float(np.linalg.norm(b)/np.linalg.norm(a)),cosine=float(np.sum(a*b)/norm) if norm else 0.,
                relative_bias=float(np.mean(b-a)/np.sqrt(np.mean(a*a))))


def report(out,records,plan):
    lines=['# 公开实测炮集与炮检映射联合验证','',
        '状态仅指公开炮集恢复执行与独立审计。两工区已在本项协议之前访问，不宣称未见工区；原5151—5171测区仍未完成。','',
        f'共{len(records)}例评价；Poland三炮，Soda Lake一炮的三个未定轴向分量；两个窗口、两个隐藏集、三档保留率、两个种子。',
        'SA、Hybrid、RandomBudget均200次几何目标评价，Jitter为共享初始布局。所有布局使用相同恢复器和共同隐藏集合。',
        '优化读取真实SPS坐标；恢复使用测站序号网格，在弯曲/非规则几何上是近似。linear在Poland为线性插值，在Soda为逻辑网格四邻点反距离插值。','',
        '|工区|恢复器|Hybrid−Jitter/dB|Hybrid−SA/dB|Hybrid−RandomBudget/dB|','|---|---|---:|---:|---:|']
    index={(r['geometry'],r['component'],r['start'],r['h'],r['m'],r['seed'],r['recoverer'],r['method']):r for r in records}
    for survey in sorted(set(r['survey'] for r in records)):
        for kind in plan['recoverers']:
            effects=[]
            for baseline in ('Jitter','SA','RandomBudget'):
                diffs=[]
                for key,row in index.items():
                    if row['survey']==survey and row['recoverer']==kind and row['method']=='Hybrid':
                        diffs.append(row['snr']-index[(*key[:-1],baseline)]['snr'])
                effects.append(float(np.mean(diffs)))
            lines.append('|'+survey+'|'+kind+'|'+'|'.join(f'{v:+.6f}' for v in effects)+'|')
    lines+=['','上述平均混合了不同保留率与依赖重复，仅为描述性汇总。逐保留率、逐炮/分量和负向结果均保存在recovery.csv，不作总体显著性或普遍优越结论。',
        '数字幅度RMS比例、相关性和偏差均按共同隐藏道计算；未经仪器标定，不能解释为物理保幅。',
        '波形道头、外部SPS及XPS关联已经检查；Poland原道头坐标为零，采用外部几何；Soda存在0.1坐标缩放、0.2米源位置差和分量轴向未知限制。',
        '本流程未完成原测区实测恢复、物理标定、速度模型下成像或实际现场成本验证。']
    (out/'report.md').write_text('\n'.join(lines)+'\n',encoding='utf-8')


def run(out,workers=4):
    out=Path(out).resolve()
    if not out.is_relative_to((ROOT/'results').resolve()) or out.exists():raise ValueError('Use a new directory inside results/')
    if not 1<=workers<=16:raise ValueError('Workers must be between 1 and 16')
    plan=json.loads(PROTOCOL.read_text());identity=source_identity()
    for name,value in plan['input_sha256'].items():
        if digest(ROOT/name)!=value:raise ValueError('Frozen field input changed: '+name)
    recorder=Recorder(out,executable(),dict(plan=plan,protocol_sha256=digest(PROTOCOL),workers=workers),protocol=plan['protocol'])
    (out/'frozen_plan.json').write_text(json.dumps(plan,indent=2)+'\n')
    (out/'source_identity.json').write_text(json.dumps(identity,indent=2)+'\n')
    try:
        geometries,sources=selections(plan);layouts,audit=search(recorder,plan,geometries)
        (out/'truth').mkdir();(out/'predictions').mkdir();records=[];selection={}
        for geometry,info in geometries.items():
            selection[geometry]={str(c):group for c,group in info['components'].items()}
            for component,group in info['components'].items():
                for start,stop in plan['windows']:
                    cube=f'{geometry}_c{component}_w{start}'
                    data=sources[info['survey']].decode([int(r['trace_index_zero_based']) for r in group],start,stop)
                    data=data.reshape((stop-start,*info['shape']));np.savez_compressed(out/'truth'/(cube+'.npz'),data=data)
                    jobs=[(layout,kind) for layout in layouts if layout['geometry']==geometry for kind in plan['recoverers']]
                    def evaluate(job):
                        layout,kind=job;known=np.array(list(layout['mask']),int).astype(bool).reshape(info['shape'])
                        hidden=hidden_mask(known.size,layout['h']).reshape(known.shape)
                        prediction=predict(data,known,kind,plan);intervention=data.copy();intervention[:,~known]=np.nan
                        isolated=predict(intervention,known,kind,plan)
                        leakage=float(np.max(abs(prediction-isolated)));projection=float(np.max(abs(prediction[:,known]-data[:,known])))
                        if leakage!=0 or projection!=0:raise ValueError('Information isolation or observation projection failed')
                        case=f'{cube}_{layout["layout"]}_{kind}';path=out/'predictions'/(case+'.npz')
                        np.savez_compressed(path,prediction=prediction,known=known,hidden=hidden)
                        return dict(case=case,cube=cube,survey=info['survey'],component=component,start=start,recoverer=kind,
                            **layout,**score(data,prediction,hidden),projection_error=projection,isolation_error=leakage,
                            retained_fraction=layout['m']/known.size,prediction_sha256=digest(path))
                    with ThreadPoolExecutor(max_workers=workers) as pool:records.extend(pool.map(evaluate,jobs))
                    print('completed mapped field',cube,flush=True)
        write_rows(out/'recovery.csv',records);report(out,records,plan)
        (out/'selected_trace_mapping.json').write_text(json.dumps(selection,indent=2)+'\n')
        require_identity(identity)
        recorder.state.update(status='passed',cases=len(records),search_audit=audit,
            input_sha256={str(ROOT/name):value for name,value in plan['input_sha256'].items()},
            derived_artifacts_sha256={p.relative_to(out).as_posix():digest(p) for p in out.rglob('*') if p.is_file()
                and p.name not in ('manifest.json','manifest.tmp','journal.jsonl')})
        recorder.finish();verify(out)
        for segy in sources.values():segy.close()
    except BaseException as error:
        recorder.state.update(status='failed',error=str(error));recorder.save();raise
    return recorder.state


def verify(out):
    out=Path(out);state=json.loads((out/'manifest.json').read_text());plan=state['configuration']['plan']
    if state.get('status')!='passed' or state.get('completed') is not True:raise ValueError('Incomplete mapped-field run')
    if plan!=json.loads(PROTOCOL.read_text()) or state['configuration']['protocol_sha256']!=digest(PROTOCOL):
        raise ValueError('Frozen field protocol changed')
    validate_native_steps(state,out)
    for name,value in state['input_sha256'].items():
        if digest(name)!=value:raise ValueError('Field input hash changed')
    for name,value in state['derived_artifacts_sha256'].items():
        if digest(out/name)!=value:raise ValueError('Field artifact changed: '+name)
    geometries,sources=selections(plan)
    selected={g:{str(c):group for c,group in info['components'].items()} for g,info in geometries.items()}
    if selected!=json.loads((out/'selected_trace_mapping.json').read_text()):raise ValueError('Selected station mapping changed')
    layouts={r['layout']:r for r in rows(out/'layouts.csv')};truths={};seen=set();error=projection=0.
    records=rows(out/'recovery.csv');expected=set()
    for layout in layouts.values():
        for component in ([0] if layout['geometry'].startswith('poland') else plan['soda']['components']):
            for start,_ in plan['windows']:
                for kind in plan['recoverers']:expected.add((layout['layout'],component,start,kind))
    for row in records:
        key=(row['layout'],int(row['component']),int(row['start']),row['recoverer'])
        if key in seen or key not in expected:raise ValueError('Duplicate or unexpected field case')
        seen.add(key);layout=layouts[row['layout']]
        if row['cube'] not in truths:
            with np.load(out/'truth'/(row['cube']+'.npz'),allow_pickle=False) as data:truths[row['cube']]=data['data']
            info=geometries[layout['geometry']];group=info['components'][key[1]]
            start,stop=next(w for w in plan['windows'] if w[0]==key[2])
            direct=sources[info['survey']].decode([int(r['trace_index_zero_based']) for r in group],start,stop)
            if not np.array_equal(direct.reshape(stop-start,*info['shape']),truths[row['cube']]):
                raise ValueError('Stored truth differs from the fixed source traces')
        truth=truths[row['cube']]
        with np.load(out/'predictions'/(row['case']+'.npz'),allow_pickle=False) as data:
            prediction,known,hidden=(data[k] for k in ('prediction','known','hidden'))
            if (known.dtype!=np.dtype(bool) or hidden.dtype!=np.dtype(bool) or known.shape!=truth.shape[1:] or
                    not np.array_equal(hidden.ravel(),hidden_mask(known.size,int(layout['h']))) or
                    ''.join(map(str,known.astype(int).ravel()))!=layout['mask'] or
                    known.sum()!=int(layout['m']) or np.any(known&hidden)):
                raise ValueError('Frozen field masks changed')
            for name,value in score(truth,prediction,hidden).items():
                actual=float(row[name])
                if not np.isfinite(actual):raise ValueError('Nonfinite recorded field score')
                error=max(error,abs(value-actual))
            projection=max(projection,float(np.max(abs(prediction[:,known]-truth[:,known]))))
        if float(row['isolation_error'])!=0:raise ValueError('Recorded field leakage')
    if seen!=expected or len(seen)!=1728 or error>1e-9 or projection!=0:raise ValueError('Independent mapped-field verification failed')
    for segy in sources.values():segy.close()
    result=dict(status='passed',scope=plan['scope'],cases=len(seen),waveform_surveys=2,
        score_max_error=error,projection_max_error=projection,isolation_interventions=len(seen),
        original_site_cases=0,manifest_sha256=digest(out/'manifest.json'))
    (out/'verification.json').write_text(json.dumps(result,indent=2,allow_nan=False)+'\n')
    print(json.dumps(result),flush=True);return result


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,default=current_results()/'mapped_field')
    parser.add_argument('--workers',type=int,default=4);parser.add_argument('--verify',action='store_true');args=parser.parse_args()
    if args.verify:verify(args.out)
    else:run(args.out,args.workers)
