"""Replicate the frozen spatial experiment over disjoint blocks and two known surveys."""
import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime,timezone
import json
from pathlib import Path

import numpy as np
from research.validation.artifacts import finite_arrays,finite_error,validate_native_steps

from research.common.hashing import digest
from research.common.paths import ROOT,executable
from research.common.tabular import rows,write_rows
from research.data.confirmation import DEFAULT,freeze as original_freeze,volume_headers,decode
from research.experiments.audit_joint_studies import Recorder,initial_mask
from research.experiments.confirmation_spatial import fk3d

PROTOCOL=Path(__file__).with_name('spatial_replication_protocol.json')
FREEZE=ROOT/'data/public/spatial_replication_20261103/freeze.json'


def freeze(marker=FREEZE):
    marker=Path(marker);plan=json.loads(PROTOCOL.read_text(encoding='utf-8'))
    if marker.exists():
        state=json.loads(marker.read_text(encoding='utf-8'))
        if state['protocol_sha256']!=digest(PROTOCOL):raise ValueError('Spatial replication protocol changed after freezing')
    else:
        marker.parent.mkdir(parents=True,exist_ok=True)
        state=dict(frozen_utc=datetime.now(timezone.utc).isoformat(),protocol_sha256=digest(PROTOCOL),protocol=plan,
            role=plan['role'],extension_samples_decoded=False)
        marker.write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    return state


def block_indices(inline,crossline,plan):
    axes=[np.unique(inline),np.unique(crossline)];widths=plan['shape']
    if any(len(axis)<width for axis,width in zip(axes,widths)):raise ValueError('Spatial axes are too short')
    lookup={tuple(k):i for i,k in enumerate(zip(inline,crossline))};used=set();result=[]
    for iy,qy in enumerate(plan['inline_quantiles']):
        for ix,qx in enumerate(plan['crossline_quantiles']):
            selected=[]
            for axis,width,q in zip(axes,widths,(qy,qx)):
                start=max(0,min(int((len(axis)-1)*q)-width//2,len(axis)-width))
                selected.append(axis[start:start+width])
            try:indices=[lookup[int(y),int(x)] for y in selected[0] for x in selected[1]]
            except KeyError as error:raise ValueError('A frozen block has missing traces; no replacement') from error
            if used.intersection(indices):raise ValueError('Frozen spatial blocks overlap')
            used.update(indices);result.append(dict(block=f'b{iy}{ix}',indices=indices,
                inlines=selected[0].tolist(),crosslines=selected[1].tolist()))
    return result


def masks(plan):
    result={}
    for name in plan['hidden_sets']:
        hidden=np.zeros(256,bool)
        if name=='periodic':hidden[4::8]=True
        elif name=='r271828':hidden[np.random.default_rng(271828).choice(256,32,replace=False)]=True
        else:raise ValueError('Unknown frozen hidden set')
        result[name]=hidden
    return result


def run(out,workers=4):
    if not 1<=workers<=16:raise ValueError('Workers must be between 1 and 16')
    frozen=freeze();plan=frozen['protocol'];out=Path(out).resolve()
    recorder=Recorder(out,executable(),dict(plan=plan,protocol_sha256=digest(PROTOCOL),
        freeze_sha256=digest(FREEZE),workers=workers,independent_surveys=2),protocol=plan['protocol'])
    (out/'protocol.json').write_text(json.dumps(plan,indent=2)+'\n',encoding='utf-8')
    points=np.array([(x,y) for y in range(16) for x in range(16)],float)
    hidden_sets=masks(plan);queries=[];metadata={}
    for hname,hidden in hidden_sets.items():
        for count in plan['counts']:
            for seed in plan['seeds']:
                initial=initial_mask(256,count,seed,~hidden)
                for method in plan['methods']:
                    key=f'{hname}_m{count}_s{seed}_{method}';budget=1 if method=='Jitter' else plan['budget']
                    fields=[key,seed+1,budget,budget if method=='SA' else plan['hybrid_sa_prefix'],
                        ''.join(map(str,initial.astype(int))),''.join(map(str,(~hidden).astype(int))),
                        '|'.join(f'{x},{y}' for x,y in points)]
                    queries.append(';'.join(map(str,fields)))
                    metadata[key]=dict(hidden_set=hname,m=count,seed=seed,method=method,budget=budget)
    query=out/'queries.txt';query.write_text('\n'.join(queries)+'\n',encoding='utf-8');search=out/'search'
    recorder.call('spatial_search',['--spatial-queries',query,search],search)
    measured=rows(search/'queries.csv')
    if len(measured)!=len(metadata) or {r['id'] for r in measured}!=set(metadata):raise ValueError('Incomplete spatial search')
    frequency=np.array([(x/16,y/16) for y in range(8) for x in range(8)])
    dictionary=np.exp(2j*np.pi*(points@frequency.T));gram_error=0.
    for row in measured:
        info=metadata[row['id']];known=np.array(list(row['mask']),int).astype(bool)
        if known.sum()!=info['m'] or np.any(known&hidden_sets[info['hidden_set']]) or int(row['evaluations'])!=info['budget']:
            raise ValueError('Search violated its frozen budget or masks')
        atom=dictionary[known];gram=atom.conj().T@atom/known.sum();np.fill_diagonal(gram,0)
        gram_error=max(gram_error,abs(float(abs(gram).max())-float(row['mu'])))
    if gram_error>2e-11:raise ValueError('Independent Gram differs')
    (out/'truth').mkdir();(out/'predictions').mkdir();results=[];sources={};isolation=0.
    specs={s['id']:s for s in original_freeze()['protocol']['external']['surveys']}
    for survey in plan['surveys']:
        raw,info=volume_headers(specs[survey]);blocks=block_indices(info['inline'],info['crossline'],plan)
        sources[survey]=dict(source=info['source'],blocks=blocks,sample_interval_s=info['sample_interval_s'])
        for block in blocks:
            for start,stop in plan['time_windows']:
                cube=f'{survey}_{block["block"]}_w{start}'
                data=decode(raw,info,block['indices'],start,stop).reshape(stop-start,16,16)
                truth=out/'truth'/(cube+'.npz');np.savez_compressed(truth,data=data)
                def evaluate(row):
                    meta=metadata[row['id']];key=cube+'_'+row['id'];known=np.array(list(row['mask']),int).astype(bool).reshape(16,16)
                    hidden=hidden_sets[meta['hidden_set']].reshape(16,16)
                    prediction=fk3d(data,known,**plan['recovery']);reference=data[:,hidden];target=prediction[:,hidden]
                    nmse=float(np.sum((target-reference)**2)/np.sum(reference**2))
                    path=out/'predictions'/(key+'.npz');np.savez_compressed(path,prediction=prediction,known=known,hidden=hidden)
                    return dict(case=key,cube=cube,survey=survey,block=block['block'],window_start=start,**meta,
                        mu=float(row['mu']),snr=-10*np.log10(max(nmse,1e-30)),nmse=nmse,
                        projection_error=float(np.max(abs(prediction[:,known]-data[:,known]))),prediction_sha256=digest(path))
                with ThreadPoolExecutor(max_workers=workers) as pool:results.extend(pool.map(evaluate,measured))
                for hname in hidden_sets:
                    row=next(r for r in measured if metadata[r['id']]['hidden_set']==hname)
                    known=np.array(list(row['mask']),int).astype(bool).reshape(16,16)
                    changed=data.copy();changed[:,~known]=np.nan
                    isolation=max(isolation,float(np.max(abs(fk3d(changed,known,**plan['recovery'])-fk3d(data,known,**plan['recovery'])))))
                print('completed cube',cube,flush=True)
        del raw
    if isolation!=0 or any(r['projection_error']!=0 for r in results):raise ValueError('Observed-only constraints failed')
    write_rows(out/'recovery.csv',results)
    (out/'sources.json').write_text(json.dumps(sources,indent=2)+'\n',encoding='utf-8')
    recorder.state.update(status='passed',cases=len(results),gram_max_error=gram_error,isolation_max_error=isolation,
        input_sha256={str(DEFAULT/(survey+'.sgy')):source['source']['sha256'] for survey,source in sources.items()},
        derived_artifacts_sha256={p.relative_to(out).as_posix():digest(p) for folder in ('truth','predictions') for p in (out/folder).iterdir()},
        sources_sha256=digest(out/'sources.json'),protocol_file_sha256=digest(PROTOCOL))
    recorder.finish();verify(out)
    return recorder.state


def verify(out):
    out=Path(out);state=json.loads((out/'manifest.json').read_text(encoding='utf-8'))
    validate_native_steps(state,out)
    if not state.get('completed') or state.get('status')!='passed':raise ValueError('Incomplete spatial replication')
    for name,value in state['input_sha256'].items():
        if digest(name)!=value:raise ValueError('Replication source volume changed')
    for name,value in {**state['derived_artifacts_sha256'],**state['output_tables_sha256'],'sources.json':state['sources_sha256']}.items():
        if digest(out/name)!=value:raise ValueError('Spatial replication artifact changed: '+name)
    count=0;error=projection=0.;truths={}
    for row in rows(out/'recovery.csv'):
        if row['cube'] not in truths:
            with np.load(out/'truth'/(row['cube']+'.npz'),allow_pickle=False) as saved:truths[row['cube']]=saved['data']
        truth=truths[row['cube']]
        with np.load(out/'predictions'/(row['case']+'.npz'),allow_pickle=False) as saved:
            if saved['known'].sum()!=int(row['m']) or np.any(saved['known']&saved['hidden']):raise ValueError('Invalid saved masks')
            target=truth[:,saved['hidden']];prediction=saved['prediction']
            finite_arrays(truth,prediction)
            nmse=float(np.sum((prediction[:,saved['hidden']]-target)**2)/np.sum(target**2))
            error=max(error,finite_error(nmse,row['nmse']),finite_error(-10*np.log10(max(nmse,1e-30)),row['snr']))
            projection=max(projection,float(np.max(abs(prediction[:,saved['known']]-truth[:,saved['known']]))))
        count+=1
    plan=state['configuration']['plan'];expected=len(plan['surveys'])*len(plan['inline_quantiles'])*len(plan['crossline_quantiles'])*len(plan['time_windows'])*len(plan['hidden_sets'])*len(plan['counts'])*len(plan['seeds'])*len(plan['methods'])
    if count!=expected or error>1e-10 or projection!=0:raise ValueError('Independent spatial replication audit failed')
    report=dict(status='passed',cases=count,cubes=len(truths),surveys=2,blocks_per_survey=6,score_max_error=error,
        projection_max_error=projection,manifest_sha256=digest(out/'manifest.json'),scope=plan['role'])
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8');print(json.dumps(report),flush=True)
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True);parser.add_argument('--workers',type=int,default=4)
    args=parser.parse_args();run(args.out,args.workers)
