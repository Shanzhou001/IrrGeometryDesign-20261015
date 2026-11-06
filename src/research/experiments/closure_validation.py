"""Frozen equal-evaluation controls and one prospectively held-out survey."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT,executable
from research.common.tabular import rows,write_rows
from research.data import closure_holdout
from research.data.confirmation import DEFAULT,freeze as known_freeze,volume_headers,decode
from research.experiments.audit_joint_studies import Recorder,initial_mask
from research.experiments.confirmation_spatial import fk3d
from research.experiments.spatial_replication import block_indices,masks
from research.experiments.thesis_workflow import source_identity,require_identity
from research.validation.artifacts import read_record,require_success,validate_native_steps,finite_number


def random_candidates(initial,hidden,seed,count,ordinal,budget):
    initial=np.asarray(initial,bool);hidden=np.asarray(hidden,bool)
    if (initial.shape!=(256,) or hidden.shape!=initial.shape or initial.sum()!=count or
            np.any(initial&hidden) or budget<1 or not 1<=count<np.sum(~hidden)):
        raise ValueError('Invalid random-search cardinality, hidden set or budget')
    rng=np.random.default_rng(np.random.SeedSequence([seed,count,ordinal,20261104]))
    yield initial.copy()
    eligible=np.flatnonzero(~hidden)
    for _ in range(budget-1):
        proposal=np.zeros(256,bool);proposal[rng.choice(eligible,count,replace=False)]=True
        yield proposal


def coherence(mask,full=False):
    mask=np.asarray(mask,bool).reshape(16,16)
    if not mask.any():raise ValueError('Empty Fourier mask')
    spectrum=abs(np.fft.fft2(mask))/mask.sum();spectrum[0,0]=0
    if not full:
        # Differences of 0..7 atoms cover all DFT bins except the Nyquist axes.
        spectrum[8,:]=0;spectrum[:,8]=0
    return float(spectrum.max())


def gram_coherence(mask):
    points=np.array([(x,y) for y in range(16) for x in range(16)])
    frequencies=np.array([(x/16,y/16) for y in range(8) for x in range(8)])
    atom=np.exp(2j*np.pi*(points[np.asarray(mask,bool)]@frequencies.T))
    gram=atom.conj().T@atom/len(atom);np.fill_diagonal(gram,0)
    return float(abs(gram).max())


def search(recorder,plan):
    hidden_sets=masks(plan);metadata={};lines=[];groups={}
    points='|'.join(f'{x},{y}' for y in range(16) for x in range(16))
    def add(key,initial,hidden,seed,budget,prefix):
        bits=''.join(map(str,initial.astype(int)))
        lines.append(';'.join(map(str,[key,seed+1,budget,prefix,bits,
            ''.join(map(str,(~hidden).astype(int))),points])))
        metadata[key]=bits
    for ordinal,(hname,hidden) in enumerate(hidden_sets.items()):
        for count in plan['counts']:
            for seed in plan['seeds']:
                group=f'{hname}_m{count}_s{seed}';initial=initial_mask(256,count,seed,~hidden)
                candidates=[]
                for index,proposal in enumerate(random_candidates(initial,hidden,seed,count,ordinal,plan['budget'])):
                    key=group+f'_random{index:03d}';add(key,proposal,hidden,seed,1,1);candidates.append(key)
                for method in ('SA','Hybrid'):
                    add(group+'_'+method,initial,hidden,seed,plan['budget'],
                        plan['budget'] if method=='SA' else plan['hybrid_sa_prefix'])
                groups[group]=dict(hidden_set=hname,m=count,seed=seed,candidates=candidates)
    path=recorder.out/'queries.txt';path.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    target=recorder.out/'search';recorder.call('equal_budget_search',['--spatial-queries',path,target],target)
    measured=rows(target/'queries.csv');lookup={r['id']:r for r in measured}
    if len(measured)!=len(metadata) or set(lookup)!=set(metadata):raise ValueError('Incomplete native query batch')
    layouts=[];trace=[];gram_error=fft_error=0.
    for group,info in groups.items():
        hidden=hidden_sets[info['hidden_set']];pool=[lookup[key] for key in info['candidates']]
        best=min(range(len(pool)),key=lambda i:float(pool[i]['mu']))
        running=float('inf')
        for index,row in enumerate(pool):
            if row['mask']!=metadata[row['id']] or int(row['evaluations'])!=1:
                raise ValueError('Random candidate changed or has an invalid evaluation count')
            mask=np.array(list(row['mask']),int).astype(bool)
            fft_error=max(fft_error,abs(coherence(mask)-float(row['mu'])))
            running=min(running,float(row['mu']))
            trace.append(dict(group=group,evaluation=index+1,mask=row['mask'],mu=float(row['mu']),best_mu=running))
        selected={'Jitter':pool[0],'RandomBudget':pool[best],
                  'SA':lookup[group+'_SA'],'Hybrid':lookup[group+'_Hybrid']}
        for method,row in selected.items():
            mask=np.array(list(row['mask']),int).astype(bool)
            budget=1 if method=='Jitter' else plan['budget']
            expected_native=1 if method in ('Jitter','RandomBudget') else budget
            if mask.sum()!=info['m'] or np.any(mask&hidden) or int(row['evaluations'])!=expected_native:
                raise ValueError('Native layout violates its cardinality, H or budget')
            gram_error=max(gram_error,abs(gram_coherence(mask)-float(row['mu'])))
            layouts.append(dict(layout=group+'_'+method,hidden_set=info['hidden_set'],m=info['m'],seed=info['seed'],
                method=method,mask=row['mask'],mu=float(row['mu']),full_fft_mu=coherence(mask,full=True),
                evaluations=budget,selected_candidate=best if method=='RandomBudget' else 0))
    if max(fft_error,gram_error)>2e-11:raise ValueError('Independent Fourier objective differs')
    write_rows(recorder.out/'layouts.csv',layouts);write_rows(recorder.out/'random_search_trace.csv',trace)
    return layouts,dict(gram_max_error=gram_error,fft_max_error=fft_error,
        native_queries=len(measured),random_candidates=len(trace),
        total_native_objective_evaluations=sum(int(r['evaluations']) for r in measured),
        scope='Jitter initial score is shared with RandomBudget; evaluation counts do not assert runtime equivalence')


def cubes(plan,data_directory=closure_holdout.DEFAULT):
    specs={s['id']:s for s in known_freeze()['protocol']['external']['surveys']}
    for survey in plan['known_surveys']:
        raw,info=volume_headers(specs[survey]);blocks=block_indices(info['inline'],info['crossline'],plan)
        for block in blocks:
            for start,stop in plan['time_windows']:
                data=decode(raw,info,block['indices'],start,stop).reshape(stop-start,16,16)
                yield survey,block['block'],start,data,dict(block=block,source=info['source'],
                    sample_interval_s=info['sample_interval_s'],role='previously_seen_survey',
                    input=str(DEFAULT/(survey+'.sgy')))
        del raw
    for block,volume,source,index in closure_holdout.load_blocks(data_directory):
        for start,stop in plan['time_windows']:
            data=np.asarray(volume[start:stop],float)
            if not np.isfinite(data).all() or not np.any(data):
                raise ValueError('Frozen holdout cube invalid; no replacement')
            yield plan['holdout']['id'],block['block'],start,data,dict(block=block,source=source,
                sample_interval_s=plan['holdout']['sample_interval_s'],role='prospective_project_unseen_survey',
                input=str(index))


def quality(truth,prediction,hidden):
    truth=np.asarray(truth);prediction=np.asarray(prediction);hidden=np.asarray(hidden)
    if (truth.ndim!=3 or prediction.shape!=truth.shape or hidden.shape!=truth.shape[1:] or
            hidden.dtype!=np.dtype(bool) or not hidden.any() or
            not np.isfinite(truth).all() or not np.isfinite(prediction).all()):
        raise ValueError('Finite matching truth/prediction and a Boolean hidden mask required')
    target=truth[:,hidden];value=prediction[:,hidden]
    energy=float(np.sum(target**2))
    if not np.isfinite(energy) or energy<=0:raise ValueError('Zero or nonfinite hidden energy; no case deletion')
    nmse=float(np.sum((value-target)**2)/energy)
    norm=float(np.linalg.norm(target)*np.linalg.norm(value))
    result=dict(nmse=nmse,snr=-10*np.log10(max(nmse,1e-30)),
                rms_ratio=float(np.linalg.norm(value)/np.linalg.norm(target)),
                cosine=float(np.sum(target*value)/norm) if norm else 0.)
    for name,value in result.items():finite_number(value,name)
    return result


def run(out,workers=4,data_directory=closure_holdout.DEFAULT):
    out=Path(out).resolve()
    if not out.is_relative_to((ROOT/'results').resolve()):raise ValueError('Results must stay inside results/')
    if not 1<=workers<=16:raise ValueError('Workers must be between 1 and 16')
    data_directory=Path(data_directory).resolve()
    frozen=closure_holdout.freeze(data_directory);plan=frozen['protocol'];identity=source_identity()
    recorder=Recorder(out,executable(),dict(plan=plan,protocol_sha256=digest(closure_holdout.PROTOCOL),
        freeze_sha256=digest(data_directory/'freeze.json'),workers=workers,
        algorithm_id='spatial_fixed_sa64_greedy_swap_v1',
        method_scope='Fixed spatial Hybrid; distinct from the 1D adaptive-switch/recycling method'),protocol=plan['protocol'])
    (out/'source_identity.json').write_text(json.dumps(identity,indent=2)+'\n',encoding='utf-8')
    try:
        layouts,search_audit=search(recorder,plan)
        (out/'truth').mkdir();(out/'predictions').mkdir();results=[];sources={}
        hidden_sets=masks(plan)
        for survey,block,start,data,provenance in cubes(plan,data_directory):
            cube=f'{survey}_{block}_w{start}';sources[cube]=provenance
            np.savez_compressed(out/'truth'/(cube+'.npz'),data=data)
            def evaluate(layout):
                known=np.array(list(layout['mask']),int).astype(bool).reshape(16,16)
                hidden=hidden_sets[layout['hidden_set']].reshape(16,16)
                prediction=fk3d(data,known,**plan['recovery'])
                intervention=data.copy();intervention[:,~known]=np.nan
                isolated=fk3d(intervention,known,**plan['recovery'])
                isolation=float(np.max(abs(prediction-isolated)))
                projection=float(np.max(abs(prediction[:,known]-data[:,known])))
                if isolation!=0 or projection!=0:raise ValueError('Observed-only recovery constraint failed')
                case=cube+'_'+layout['layout']
                path=out/'predictions'/(case+'.npz')
                np.savez_compressed(path,prediction=prediction,known=known,hidden=hidden)
                return dict(case=case,cube=cube,survey=survey,block=block,window_start=start,**layout,
                    **quality(data,prediction,hidden),projection_error=projection,isolation_error=isolation,
                    prediction_sha256=digest(path))
            with ThreadPoolExecutor(max_workers=workers) as pool:results.extend(pool.map(evaluate,layouts))
            print('completed',cube,flush=True)
        write_rows(out/'recovery.csv',results)
        (out/'sources.json').write_text(json.dumps(sources,indent=2)+'\n',encoding='utf-8')
        require_identity(identity)
        inputs={p['input']:p['source']['sha256'] for p in sources.values()}
        for p in sources.values():
            for name,value in p['source'].get('cache_sha256',{}).items():
                inputs[str(data_directory/name)]=value
        for name in ('freeze.json','transport_freeze.json','parihaka.source.json','publication.json','publication_after.json'):
            if (data_directory/name).is_file():inputs[str(data_directory/name)]=digest(data_directory/name)
        recorder.state.update(status='passed',cases=len(results),search_audit=search_audit,
            input_sha256=inputs,
            sources_sha256=digest(out/'sources.json'),
            derived_artifacts_sha256={p.relative_to(out).as_posix():digest(p)
                for folder in ('truth','predictions') for p in (out/folder).iterdir()})
        recorder.finish();verify(out)
    except BaseException as error:
        recorder.state.update(status='failed',error=str(error));recorder.save();raise
    return recorder.state


def verify(out):
    out=Path(out);state=read_record(out/'manifest.json')
    require_success(state);validate_native_steps(state,out)
    if not state.get('completed') or state.get('status')!='passed':raise ValueError('Incomplete closure experiment')
    plan=state['configuration']['plan']
    for name,value in state['input_sha256'].items():
        if digest(name)!=value:raise ValueError('Closure input changed')
    for name,value in {**state['derived_artifacts_sha256'],**state['output_tables_sha256'],
                       'sources.json':state['sources_sha256']}.items():
        if digest(out/name)!=value:raise ValueError('Closure artifact changed: '+name)
    # Validate exact paired keys and frozen masks as well as metric recomputation.
    layouts={r['layout']:r for r in rows(out/'layouts.csv')};hidden_sets=masks(plan)
    expected={(survey,f'b{y}{x}',start,h,m,seed,method)
        for survey in plan['known_surveys']+[plan['holdout']['id']] for y in range(3) for x in range(2)
        for start,_ in plan['time_windows'] for h in plan['hidden_sets'] for m in plan['counts']
        for seed in plan['seeds'] for method in plan['methods']}
    seen=set();truths={};error=projection=0.
    for row in rows(out/'recovery.csv'):
        key=(row['survey'],row['block'],int(row['window_start']),row['hidden_set'],int(row['m']),int(row['seed']),row['method'])
        if key in seen or key not in expected:raise ValueError('Duplicate or unexpected closure case')
        seen.add(key)
        if row['cube'] not in truths:
            with np.load(out/'truth'/(row['cube']+'.npz'),allow_pickle=False) as saved:truths[row['cube']]=saved['data']
        truth=truths[row['cube']];layout=layouts[row['layout']]
        with np.load(out/'predictions'/(row['case']+'.npz'),allow_pickle=False) as saved:
            known=saved['known'];hidden=saved['hidden'];prediction=saved['prediction']
            if (known.dtype!=np.dtype(bool) or hidden.dtype!=np.dtype(bool) or
                    known.shape!=truth.shape[1:] or hidden.shape!=known.shape or
                    ''.join(map(str,known.astype(int).ravel()))!=layout['mask'] or known.sum()!=key[4] or
                    np.any(known&hidden) or not np.array_equal(hidden.ravel(),hidden_sets[key[3]])):
                raise ValueError('Closure masks differ from frozen definitions')
            for name,value in quality(truth,prediction,hidden).items():
                error=max(error,abs(value-finite_number(row[name],name)))
            projection=max(projection,float(np.max(abs(prediction[:,known]-truth[:,known]))))
        if finite_number(row['isolation_error'],'isolation_error')!=0:raise ValueError('Recorded information leakage')
    if seen!=expected or error>1e-9 or projection!=0:raise ValueError('Independent closure audit failed')
    result=dict(status='passed',cases=len(seen),cubes=len(truths),score_max_error=error,
        projection_max_error=projection,isolation_interventions=len(seen),
        manifest_sha256=digest(out/'manifest.json'),
        scope='Execution and score verification only; prospective hypothesis outcomes are reported separately')
    (out/'verification.json').write_text(json.dumps(result,indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(json.dumps(result),flush=True);return result


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--workers',type=int,default=4);parser.add_argument('--verify',action='store_true')
    parser.add_argument('--data-directory',type=Path,default=closure_holdout.DEFAULT);args=parser.parse_args()
    if args.verify:verify(args.out)
    else:run(args.out,args.workers,args.data_directory)
