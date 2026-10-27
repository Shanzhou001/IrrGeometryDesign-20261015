"""Independently verify the frozen F3 archive, masks and reconstruction scores."""
import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import json
from pathlib import Path
import sys

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'src'))
import numpy as np
from research.common.hashing import digest
from research.common.paths import ROOT
from research.common.tabular import rows
from research.data.f3 import load,REGISTRY


def verify(root,out):
    root,out=Path(root).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    state=json.loads((root/'manifest.json').read_text())
    if not state.get('completed') or state.get('status')!='passed' or state['protocol']!='f3_frozen_transfer_v1':
        raise ValueError('A completed frozen F3 batch is required')
    expected={}
    def add(path,value):
        path=Path(path).resolve()
        if path in expected and expected[path]!=value:raise ValueError('Conflicting file identity')
        expected[path]=value
    for name,value in state['source_sha256'].items():add(ROOT/name,value)
    for section in ('derived_artifacts_sha256','output_tables_sha256'):
        for name,value in state[section].items():add(root/name,value)
    for step in state['steps'].values():
        if step['exit_code']:raise ValueError('Native call failed')
        for name,value in step['inputs'].items():add(name,value)
        for name,value in step['outputs'].items():add(root/name,value)
    provenance=state['configuration']['dataset_provenance']
    for name,value in provenance['input_sha256'].items():add(name,value)
    add(REGISTRY,provenance['registry_sha256']);add(state['program'],state['executable_sha256'])
    add(root/'frozen_plan.json',state['plan_sha256'])
    add(ROOT/'src/research/experiments/f3_transfer_protocol.json',state['protocol_file_sha256'])
    def check(item):
        path,value=item
        if digest(path)!=value:raise ValueError('Changed file: '+str(path))
        return path.stat().st_size
    with ThreadPoolExecutor(max_workers=4) as pool:byte_count=sum(pool.map(check,expected.items()))
    windows,_=load();plan=state['configuration']['plan']
    search=rows(root/'search.csv');recovery=rows(root/'recovery.csv')
    metadata=json.loads((root/'search_metadata.json').read_text())
    mu_error=objective_error=score_error=projection_error=0.
    for row in search:
        mask=np.array(list(row['mask']),int).astype(bool);hidden=np.array(list(row['H_bits']),int).astype(bool)
        pilot=np.array(list(row['pilot_bits']),int).astype(bool)
        if mask.sum()!=int(row['m']) or np.any(mask&hidden) or np.any(pilot&~mask):raise ValueError('Invalid acquired mask')
        psf=abs(np.fft.fft(mask.astype(float))[1:])/mask.sum();feature=metadata[row['id']]['feature_path']
        weights=np.array(json.loads((root/feature).read_text())['weights']) if feature else np.ones(len(psf))
        mu_error=max(mu_error,abs(float(row['mu'])-max(psf)))
        objective_error=max(objective_error,abs(float(row['objective'])-max(weights*psf)))
    for row in recovery:
        data=windows[0][int(row['section_index'])].data
        known=np.array(list(row['mask']),int).astype(bool);hidden=np.array(list(row['H_bits']),int).astype(bool)
        pilot=set(np.flatnonzero(np.array(list(row['pilot_bits']),int)))
        choice=json.loads((root/'selection'/(row['case']+'.json')).read_text());previous=set()
        if len(choice['splits'])!=2 or row['selected']!=choice['selected']['candidate'] or float(row['gain'])!=choice['selected']['gain']:
            raise ValueError('Invalid inner selection record')
        for split in choice['splits']:
            train,validation=set(split['train']),set(split['validation'])
            if not pilot<=train or train&validation or train|validation!=set(np.flatnonzero(known)) or previous&validation:
                raise ValueError('Invalid disjoint pilot-preserving split')
            previous|=validation
        with np.load(root/'predictions'/(row['case']+'.npz'),allow_pickle=False) as saved:
            if not np.array_equal(saved['known'],known) or not np.array_equal(saved['hidden'],hidden):raise ValueError('Mask mismatch')
            for kind in ('cold','fk','selected','linear'):
                prediction=saved[kind]
                if prediction.shape!=data.shape or not np.isfinite(prediction).all():raise ValueError('Invalid prediction')
                projection_error=max(projection_error,float(np.max(abs(prediction[:,known]-data[:,known]))))
                nmse=float(np.sum((prediction[:,hidden]-data[:,hidden])**2)/np.sum(data[:,hidden]**2))
                score_error=max(score_error,abs(nmse-float(row[kind+'_nmse'])),abs(-10*np.log10(nmse)-float(row[kind+'_snr'])))
    expected_cases=len(plan['ffids'])*len(plan['hidden_sets'])*len(plan['counts'])*len(plan['seeds'])*len(plan['methods'])
    if len(recovery)!=expected_cases or projection_error!=0 or score_error>1e-10 or max(mu_error,objective_error)>1e-7 or not state['isolation']['passed']:
        raise ValueError('F3 independent numerical checks failed')
    result=dict(status='passed',completed_utc=datetime.now(timezone.utc).isoformat(),
        experiment_manifest_sha256=digest(root/'manifest.json'),script_sha256=digest(__file__),
        checked_unique_files=len(expected),checked_bytes=byte_count,cases=len(recovery),prediction_arrays=4*len(recovery),
        searches=len(search),native_calls=len(state['steps']),projection_max_error=projection_error,
        score_max_error=score_error,fourier_mu_max_error=mu_error,weighted_objective_max_error=objective_error,
        independent_surveys=1,scope='One additional poststack survey; masks, hashes, inner splits, scores and information intervention; no imaging claim')
    out.mkdir(parents=True);(out/'verification.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--root',type=Path,required=True);parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args();verify(args.root,args.out)
