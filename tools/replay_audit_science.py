"""Replay all deterministic v5 search cases and prespecified predictions on the current binary."""
import argparse
import csv
import json
from pathlib import Path
import subprocess
import time

from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]


def table(p):
    with p.open(encoding='utf-8-sig') as f:return list(csv.DictReader(f))


def run(root,out):
    out.mkdir(parents=True,exist_ok=False);exe=ROOT/'x64/Release/IrrGeometryDesign.exe';records=[]
    def call(key,arguments,target):
        command=[str(exe),*map(str,arguments)];start=time.perf_counter()
        with (out/(key+'.log')).open('wb') as f:r=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        record=dict(command=command,exit_code=r.returncode,seconds=time.perf_counter()-start,
                    inputs={str(Path(v).resolve()):digest(v) for v in command[1:] if Path(v).is_file()},
                    outputs={str(p.relative_to(out)):digest(p) for p in target.rglob('*') if p.is_file()})
        records.append(record)
        if r.returncode:raise RuntimeError('replay failed: '+key)
    count=0
    for stage,name in [('search','sensitivity'),('joint','joint_search')]:
        target=out/name;call(name,['--audit-queries',root/stage/(name+'.txt'),target],target)
        original=table(root/stage/name/'queries.csv');current=table(target/'queries.csv')
        assert len(original)==len(current)
        for a,b in zip(original,current):
            for k in a:
                if k not in ['wall_ms','native_clock_ms']:assert a[k]==b[k],(name,k,a[k],b[k])
            ta=table(root/stage/name/(a['id']+'.trace.csv'));tb=table(target/(b['id']+'.trace.csv'))
            assert len(ta)==len(tb)
            for x,y in zip(ta,tb):
                for k in x:
                    if k!='wall_ms':assert x[k]==y[k],(name,a['id'],k)
            count+=1
    predictions=0
    for stage in ['joint','real']:
        state=json.loads((root/stage/'manifest.json').read_text())
        eligible=sorted(k for k in state['steps'] if k.endswith('_fixed'))
        for index in [0,len(eligible)//2,len(eligible)-1]:
            key=eligible[index];old=state['steps'][key];args=old['command'][1:];target=out/(stage+'_'+str(index))
            oldpath=Path(args[args.index('--out')+1]);args[args.index('--out')+1]=str(target)
            call(stage+'_'+str(index),args,target)
            for filename in ['prediction.csv','linear.csv','convergence.csv']:assert digest(target/filename)==digest(oldpath/filename),(stage,filename)
            a=table(target/'solver.csv')[0];b=table(oldpath/'solver.csv')[0]
            assert all(a[k]==b[k] for k in a if k!='runtime_ms')
            predictions+=1
    state=dict(passed=True,executable_sha256=digest(exe),search_cases=count,prediction_cases=predictions,records=records,
               scope='Full deterministic search replay, six prespecified fixed-recovery cases; full reconstruction batch not rerun after unused legacy codec repairs.')
    (out/'verification.json').write_text(json.dumps(state,indent=2));print(count,predictions)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);p.add_argument('--out',type=Path,required=True)
    a=p.parse_args();run(a.root.resolve(),a.out.resolve())
