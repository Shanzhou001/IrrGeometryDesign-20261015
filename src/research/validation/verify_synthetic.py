"""Re-read every synthetic prediction and verify scores and raw-output identities."""
import argparse
import json
from pathlib import Path
import numpy as np
from research.common.hashing import digest
from research.common.tabular import rows


def verify(root):
    root=Path(root); expected={}
    for name,filename in [('synthetic_search','run_manifest.json'),('synthetic_joint','manifest.json')]:
        base=root/name;state=json.loads((base/filename).read_text())
        if not state.get('completed'):raise ValueError('Incomplete synthetic experiment')
        for step in state.get('steps',{}).values():
            if step['exit_code']!=0:raise ValueError('Failed synthetic step')
            for key,value in step.get('outputs',{}).items():expected[base/key]=value
    for p,value in expected.items():
        if digest(p)!=value:raise ValueError('Changed synthetic raw output: '+str(p))
    base=root/'synthetic_joint';truth={};max_error=0.;cases=0
    for row in rows(base/'joint.csv'):
        model=row['model']
        if model not in truth:truth[model]=np.loadtxt(base/'truth'/(model+'.csv'),delimiter=',',comments='#')
        target=truth[model];mask=np.array(list(row['mask']),int).astype(bool);hidden=np.array(list(row['evaluation_bits']),int).astype(bool)
        if np.any(mask&hidden):raise ValueError('Synthetic target leakage')
        for kind,path in [('fixed',base/'native'/(row['case']+'_fixed')/'prediction.csv'),
                          ('selected',base/'selected'/(row['case']+'.csv'))]:
            if kind=='selected' and row['design']!='common_H':continue
            prediction=np.loadtxt(path,delimiter=',')
            nmse=np.sum((prediction[:,hidden]-target[:,hidden])**2)/np.sum(target[:,hidden]**2)
            error=abs(-10*np.log10(nmse)-float(row[kind+'_snr']));max_error=max(max_error,float(error))
            if error>1e-7:raise ValueError('Synthetic score mismatch')
        cases+=1
    result=dict(status='passed',cases=cases,raw_files=len(expected),score_max_error=max_error)
    (root/'synthetic_verification.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result),flush=True)
    return result


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--root',type=Path,required=True)
    verify(parser.parse_args().root)
