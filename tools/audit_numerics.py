"""Independent finite enumeration/NumPy checks of the native v5 numerical query interface."""
import argparse
import csv
import itertools
import json
from pathlib import Path
import subprocess
import time

import numpy as np

from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]


def query(identifier, mask, x=None, method='measure', seed=42, budget=300, minimum=32, window=12,
          gap=0., nn=0., hard=0, hard_m=0., wall=0., eligible=None):
    return ';'.join(map(str,[identifier,method,seed,budget,minimum,window,gap,nn,hard,hard_m,wall,
                            '' if x is None else ','.join(map(str,x)),
                            ''.join(str(int(v)) for v in mask),
                            '' if eligible is None else ''.join(str(int(v)) for v in eligible)]))


def metrics(mask, x, gap=0., nn=0., bandwidth=1., period_factor=1.):
    mask=np.asarray(mask,dtype=bool);x=np.asarray(x,dtype=float)
    kept=x[mask];n=len(x);m=len(kept);period=n*np.ptp(x)/(n-1)*period_factor
    dictionary=np.exp(2j*np.pi*np.outer(kept-x[0],np.arange(n)*bandwidth)/period)/np.sqrt(m)
    gram=dictionary.conj().T@dictionary;np.fill_diagonal(gram,0)
    mu=float(np.abs(gram).max())
    gaps=np.diff(kept)
    maximum=float(max(kept[0]-x[0],x[-1]-kept[-1],max(gaps,default=0)))
    nearest=np.minimum(np.r_[np.inf,gaps],np.r_[gaps,np.inf]) if m>1 else np.array([0.])
    cv=float(nearest.std()/nearest.mean()) if nearest.mean()>0 else 0.
    return dict(mu=mu,max_gap_m=maximum,nn_cv=cv,objective=mu+gap*max(0,maximum/(period/m)-1)+nn*cv)


def run(out, exe):
    out.mkdir(parents=True,exist_ok=False)
    cases=[];lines=[]
    for domain,x in [('uniform',np.arange(7.)),('physical',np.array([0.,1,3,4,8,9,12]))]:
        for code in range(1,127):
            mask=np.array([(code>>j)&1 for j in range(7)])
            for gap,nn in [(0.,0.),(.1,.3)]:
                name=f'{domain}_{code}_{int(gap*10)}'
                lines.append(query(name,mask,x,gap=gap,nn=nn));cases.append((name,metrics(mask,x,gap,nn)))
    for m in range(1,8):
        for bound in range(1,13):
            x=np.array([0.,1,3,4,8,9,12]);mask=np.arange(7)<m
            feasible=any(max(np.diff(np.r_[x[0],x[list(c)],x[-1]]))<=bound for c in itertools.combinations(range(7),m))
            name=f'dp_{m}_{bound}';lines.append(query(name,mask,x,method='dp',hard_m=bound))
            cases.append((name,dict(feasible=int(feasible))))
    path=out/'queries.txt';path.write_text('\n'.join(lines)+'\n')
    command=[str(exe.resolve()),'--audit-queries',str(path.resolve()),str((out/'native').resolve())]
    start=time.perf_counter()
    with (out/'native.log').open('wb') as log:run=subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
    record=dict(command=command,exit_code=run.returncode,seconds=time.perf_counter()-start,
                executable_sha256=digest(exe),query_sha256=digest(path),cases=len(cases))
    (out/'run.json').write_text(json.dumps(record,indent=2))
    if run.returncode:raise RuntimeError('native numerical probe failed')
    with (out/'native/queries.csv').open() as f:rows={r['id']:r for r in csv.DictReader(f)}
    largest={k:0. for k in ['mu','max_gap_m','nn_cv','objective']}
    for name,expected in cases:
        row=rows[name]
        for key,value in expected.items():
            error=abs(float(row[key])-value)
            if key in largest:largest[key]=max(largest[key],error)
            if error>3e-6:raise AssertionError((name,key,row[key],value))
        if name.startswith('dp_') and int(row['feasible']):
            mask=np.array(list(row['mask']),dtype=int);m,bound=map(int,name.split('_')[1:])
            assert mask.sum()==m and metrics(mask,[0,1,3,4,8,9,12])['max_gap_m']<=bound
    report=dict(passed=True,cases=len(cases),max_absolute_errors=largest,
                note='504 explicit dictionary/coverage/objective cases; 84 DP cases incl full cardinality')
    (out/'verification.json').write_text(json.dumps(report,indent=2))
    print(json.dumps(report))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True)
    p.add_argument('--exe',type=Path,default=ROOT/'x64/Release/IrrGeometryDesign.exe');a=p.parse_args();run(a.out,a.exe)
