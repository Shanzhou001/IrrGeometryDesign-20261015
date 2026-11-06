"""First project evaluation of the prospectively frozen additional surveys."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np
from research.validation.artifacts import finite_arrays,finite_error,validate_native_steps

from research.analysis.waveform_quality import quality,summarize
from research.common.hashing import digest
from research.common.paths import executable
from research.common.tabular import rows,write_rows
from research.data.confirmation import PROTOCOL,load,freeze
from research.experiments.audit_joint_studies import Recorder
from research.experiments.viking_v2 import PROTOCOL_FILE,prepare_search,evaluate,isolation_checks
from research.recovery import CANDIDATES


def run(survey,out,workers=8):
    freeze();whole=json.loads(PROTOCOL.read_text());spec=whole['external'];out=Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    windows,provenance=load(survey)
    plan=json.loads(PROTOCOL_FILE.read_text());plan.update(protocol=whole['protocol']+'_'+survey,
        ffids=[0,1,2],window_starts=[512],seeds=spec['seeds'],inner_folds=spec['inner_folds'],
        sample_interval_s=provenance['sample_interval_s'],recovery_candidates=list(CANDIDATES),
        scope='First project evaluation of a newly accessed poststack survey under a previously frozen protocol; descriptive within-survey results')
    recorder=Recorder(out,executable(),dict(plan=plan,dataset_provenance=provenance,independent_surveys=1,
        worker_count=workers,unseen_before_protocol_freeze=True,no_imaging_claim=True,
        geometry='nominal trace ranks; gap carrier uses 25 units per rank'),protocol=plan['protocol'])
    try:
        (out/'frozen_plan.json').write_text(json.dumps(plan,indent=2)+'\n')
        search,bindings=prepare_search(recorder,windows,plan)
        with ThreadPoolExecutor(max_workers=workers) as pool:
            data=list(pool.map(lambda b:evaluate(recorder,windows[512][b['ffid']],b,search[b['search_id']],plan),bindings))
        write_rows(out/'recovery.csv',data)
        isolation=isolation_checks(recorder,windows,search,bindings,plan)
        means=[];diagnostics=[]
        for row in data:
            target=windows[512][row['ffid']].data
            with np.load(out/'predictions'/(row['case']+'.npz'),allow_pickle=False) as saved:
                for kind in ('cold','fk','selected','linear'):
                    diagnostics.append(dict(case=row['case'],m=str(row['m']),method=row['method'],recovery=kind,
                        **quality(target[:,saved['hidden']],saved[kind][:,saved['hidden']])))
        for m in plan['counts']:
            for method in plan['methods']:
                same=[r for r in data if r['m']==m and r['method']==method]
                for kind in ('cold','fk','selected','linear'):
                    means.append(dict(m=m,method=method,recovery=kind,mean_snr=float(np.mean([r[kind+'_snr'] for r in same])),
                        mean_nmse=float(np.mean([r[kind+'_nmse'] for r in same])),cases=len(same),independent_surveys=1))
        write_rows(out/'means.csv',means);write_rows(out/'waveform_means.csv',summarize(diagnostics))
        artifacts={p.relative_to(out).as_posix():digest(p) for name in ('features','selection','python','predictions') for p in (out/name).rglob('*') if p.is_file()}
        recorder.state.update(status='passed',recovery_cases=len(data),search_cases=len(search),isolation=isolation,
            derived_artifacts_sha256=artifacts,protocol_file_sha256=digest(PROTOCOL))
        recorder.finish()
        verify(survey,out)
    except Exception as error:
        recorder.state.update(status='failed',completed=False,error=str(error));recorder.save();raise
    print(json.dumps(dict(status='passed',survey=survey,cases=len(data))),flush=True)
    return recorder.state


def verify(survey,out):
    out=Path(out);state=json.loads((out/'manifest.json').read_text());windows,_=load(survey)
    validate_native_steps(state,out)
    if not state.get('completed'):raise ValueError('Incomplete confirmation batch')
    expected={out/n:v for n,v in state['derived_artifacts_sha256'].items()}
    expected.update({out/n:v for n,v in state['output_tables_sha256'].items()})
    for step in state['steps'].values():
        if step['exit_code']!=0:raise ValueError('Failed confirmation native call')
        expected.update({out/n:v for n,v in step['outputs'].items()})
    for path,value in expected.items():
        if digest(path)!=value:raise ValueError('Confirmation artifact changed: '+str(path))
    error=projection=0.;count=0
    for row in rows(out/'recovery.csv'):
        target=windows[512][int(row['ffid'])].data;path=out/'predictions'/(row['case']+'.npz')
        with np.load(path,allow_pickle=False) as saved:
            hidden=saved['hidden'];known=saved['known']
            if np.any(hidden&known) or int(known.sum())!=int(row['m']):raise ValueError('Invalid confirmation masks')
            pilot=set(np.flatnonzero(np.array(list(row['pilot_bits']),int)))
            choice=json.loads((out/'selection'/(row['case']+'.json')).read_text())
            for split in choice['splits']:
                train,validation=set(split['train']),set(split['validation'])
                if not pilot<=train or train&validation or train|validation!=set(np.flatnonzero(known)):
                    raise ValueError('Invalid pilot-preserving split')
            for kind in ('cold','fk','selected','linear'):
                prediction=saved[kind];finite_arrays(target,prediction)
                projection=max(projection,float(np.max(abs(prediction[:,known]-target[:,known]))))
                nmse=float(np.sum((prediction[:,hidden]-target[:,hidden])**2)/np.sum(target[:,hidden]**2))
                error=max(error,finite_error(nmse,row[kind+'_nmse']),finite_error(-10*np.log10(nmse),row[kind+'_snr']))
            count+=1
    if error>1e-9 or projection!=0:raise ValueError('Independent external score/projection mismatch')
    report=dict(status='passed',survey=survey,cases=count,checked_files=len(expected),score_max_error=error,
        projection_max_error=projection,independent_surveys=1,scope='One field survey; sections and seeds remain dependent')
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--survey',choices=['stratton','volve'],required=True)
    p.add_argument('--out',type=Path,required=True);p.add_argument('--workers',type=int,default=8)
    a=p.parse_args();run(a.survey,a.out,a.workers)
