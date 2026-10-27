"""Verify frozen Viking v2 results, including archived rather than current source."""
import argparse
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
from zipfile import ZipFile

import numpy as np

from research.analysis.viking_v2_results import inputs
from research.common.hashing import digest
from research.common.paths import ROOT
from research.data.viking import load_windows


def verify(root, output, source_archive=None):
    root, output = Path(root).resolve(), Path(output).resolve()
    if output.exists():
        raise FileExistsError(output)
    manifest, search, recovery = inputs(root)
    plan=manifest['configuration']['plan']; expected={}
    def register(path, value):
        path=Path(path).resolve()
        if path in expected and expected[path]!=value:
            raise ValueError('Conflicting hashes for '+str(path))
        expected[path]=value
    if source_archive:
        with ZipFile(source_archive) as archive:
            for name, value in manifest['source_sha256'].items():
                if hashlib.sha256(archive.read(name.replace('\\','/'))).hexdigest()!=value:
                    raise ValueError('Archived source changed: '+name)
    else:
        for name,value in manifest['source_sha256'].items():register(ROOT/name,value)
    for section in ('derived_artifacts_sha256','output_tables_sha256'):
        for name,value in manifest[section].items():register(root/name,value)
    for step in manifest['steps'].values():
        if step['exit_code']!=0:raise ValueError('Failed native call')
        for name,value in step['inputs'].items():register(name,value)
        for name,value in step['outputs'].items():register(root/name,value)
    provenance=manifest['configuration']['dataset_provenance']
    for name,value in provenance['input_sha256'].items():register(name,value)
    register(manifest['program'],manifest['executable_sha256'])
    register(root/'search_metadata.json',manifest['search_metadata_sha256'])
    analysis=json.loads((root/'analysis/manifest.json').read_text())
    register(root/'manifest.json',analysis['source_manifest_sha256'])
    for name,value in analysis['table_sha256'].items():register(root/'analysis'/name,value)
    def file_check(item):
        path,value=item
        if digest(path)!=value:raise ValueError('File changed: '+str(path))
        return path.stat().st_size
    with ThreadPoolExecutor(max_workers=4) as pool:byte_count=sum(pool.map(file_check,expected.items()))
    windows,_=load_windows(plan['window_starts'],Path(provenance['dataset_directory']),plan['ffids'])
    metadata=json.loads((root/'search_metadata.json').read_text())
    mu_error=objective_error=projection_error=score_error=0.
    for row in search:
        mask=np.array(list(row['mask']),float); psf=abs(np.fft.fft(mask)[1:])/mask.sum()
        feature=metadata[row['id']]['feature_path']
        weights=np.array(json.loads((root/feature).read_text())['weights']) if feature else np.ones(len(psf))
        mu_error=max(mu_error,abs(float(row['mu'])-max(psf)))
        objective_error=max(objective_error,abs(float(row['objective'])-max(weights*psf)))
    for row in recovery:
        data=windows[int(row['window_start'])][int(row['ffid'])].data
        known=np.array(list(row['mask']),int).astype(bool);hidden=np.array(list(row['H_bits']),int).astype(bool)
        pilot=set(np.flatnonzero(np.array(list(row['pilot_bits']),int)))
        choice=json.loads((root/'selection'/(row['case']+'.json')).read_text())
        if len(choice['splits'])!=plan['inner_folds']:raise ValueError('Wrong inner fold count')
        if row['selected']!=choice['selected']['candidate'] or float(row['gain'])!=choice['selected']['gain']:
            raise ValueError('Selection table differs from saved choice')
        validations=[]
        for split in choice['splits']:
            train,validation=set(split['train']),set(split['validation'])
            if not pilot<=train or train&validation or train|validation!=set(np.flatnonzero(known)):
                raise ValueError('Invalid pilot-preserving inner split')
            if any(validation&other for other in validations):raise ValueError('Overlapping inner validation folds')
            validations.append(validation)
        with np.load(root/'predictions'/(row['case']+'.npz')) as saved:
            if not np.array_equal(saved['known'],known) or not np.array_equal(saved['hidden'],hidden):
                raise ValueError('Prediction masks changed')
            for name in ('cold','fk','selected','linear'):
                prediction=saved[name]
                if prediction.shape!=data.shape or not np.isfinite(prediction).all():raise ValueError('Invalid prediction')
                projection_error=max(projection_error,float(np.max(abs(prediction[:,known]-data[:,known]))))
                nmse=np.sum((prediction[:,hidden]-data[:,hidden])**2)/np.sum(data[:,hidden]**2)
                score_error=max(score_error,abs(float(row[name+'_nmse'])-nmse),abs(float(row[name+'_snr'])+10*np.log10(nmse)))
    if projection_error!=0 or score_error>1e-10 or max(mu_error,objective_error)>1e-7:
        raise ValueError('Independent prediction or Fourier scores differ')
    result=dict(status='passed',completed_utc=datetime.now(timezone.utc).isoformat(),root=str(root),
                experiment_manifest_sha256=digest(root/'manifest.json'),analysis_manifest_sha256=digest(root/'analysis/manifest.json'),
                source_archive_sha256=digest(source_archive) if source_archive else None,
                source_files_verified=len(manifest['source_sha256']),checked_unique_files=len(expected),checked_bytes=byte_count,
                recovery_cases=len(recovery),unique_searches=len(search),native_calls=len(manifest['steps']),
                checked_prediction_arrays=4*len(recovery),projection_max_error=projection_error,score_max_error=score_error,
                fourier_mu_max_error=mu_error,weighted_objective_max_error=objective_error,
                seeds=plan['seeds'],inner_folds=plan['inner_folds'],independent_surveys=1,
                isolation=manifest['isolation'],script_sha256=digest(__file__))
    output.mkdir(parents=True);(output/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result,indent=2));return result


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,required=True);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--source-archive',type=Path,help='frozen source ZIP, permitting verification after later source edits')
    args=parser.parse_args();verify(args.root,args.out,args.source_archive)
