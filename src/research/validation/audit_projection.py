"""Compare the actual cold, one-step, zero-shrink native projection with a dense NumPy operator."""
import json
from pathlib import Path
import argparse

import numpy as np

from research.experiments.audit_joint_studies import Recorder,write_gather,write_mask
from research.common.hashing import digest
from research.common.provenance import require_inputs

from research.common.paths import ROOT, WORKSPACE


def run(out, exe=None):
    program=Path(exe) if exe is not None else WORKSPACE / 'build/bin/Release/IrrGeometryDesign.exe'
    require_inputs([program])
    recorder=Recorder(out,program,dict(check='dense observation projection',synthetic_only=True,driver_sha256=digest(__file__)))
    result=[]
    for index,x in enumerate([np.array([0.,1,2,3,5,7,9,20]),np.arange(32.)*2.5]):
        n=len(x);mask=np.zeros(n,bool);mask[[1,2,3,n-2,n-1]]=True
        data=np.sin(np.arange(64)[:,None]*.13+np.arange(n)[None,:]*.27)
        observed=np.zeros_like(data);observed[:,mask]=data[:,mask]
        gather=recorder.out/f'input{index}.csv';mf=recorder.out/f'mask{index}.txt'
        write_gather(gather,observed,x,'independent dense projection probe');write_mask(mf,mask)
        target=recorder.out/f'native{index}'
        args=['--review-reconstruct',gather,'--mask',mf,'--out',target,'--iter',1,'--cold','--threshold-start',0,'--threshold-end',0]
        recorder.call(f'projection{index}',args,target)
        nx=32
        while np.ptp(x)/(nx-1)>np.diff(x).min():nx*=2
        positions=(x-x[0])/(np.ptp(x)/(nx-1));positions[-1]=nx-1
        a=np.zeros((n,nx))
        for j,p in enumerate(positions):
            left=min(int(np.floor(p)),nx-2);weight=p-left;a[j,left]=1-weight;a[j,left+1]=weight
        k=a[mask];gram=k@k.T
        # The engine uses float32 observations and a float32 returned grid; compare independently in float64.
        expected=(a@k.T@np.linalg.solve(gram,data[:,mask].astype(np.float32).T)).T
        actual=np.loadtxt(target/'prediction.csv',delimiter=',')
        error=float(abs(expected-actual).max());assert error<4e-7,error
        assert np.linalg.matrix_rank(k)==mask.sum()
        np.savetxt(recorder.out/f'operator{index}.csv',a,delimiter=',',fmt='%.15g')
        np.savetxt(recorder.out/f'expected{index}.csv',expected,delimiter=',',fmt='%.12g')
        result.append(dict(case=index,work_grid=nx,known=int(mask.sum()),max_abs_difference=error,gram_condition=float(np.linalg.cond(gram))))
    recorder.finish();(recorder.out/'verification.json').write_text(json.dumps(dict(passed=True,cases=result),indent=2));print(result)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True)
    p.add_argument('--exe',type=Path,default=WORKSPACE / 'build/bin/Release/IrrGeometryDesign.exe')
    args=p.parse_args();run(args.out,args.exe)
