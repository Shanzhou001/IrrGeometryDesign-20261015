"""Prespecified audit_joint_v5 experiments. Existing output directories are never reused."""
from __future__ import annotations
import argparse
from concurrent.futures import ThreadPoolExecutor
import csv
import datetime
import json
import os
os.environ.setdefault('OPENBLAS_NUM_THREADS','1')
from pathlib import Path
import platform
import subprocess
import threading
import time

import numpy as np

from audit_numerics import query, metrics
from convert_rraw_to_engine_csv import read_segy
from make_59x256 import convert as legacy_convert
from review_studies import validation_choice, linear_known, snr
from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]
PROTOCOL='audit_joint_v5'
SOURCE=ROOT/'data/real_seismic/Data/RRAW.SGY'
SOURCE_HASH='ddcb311b2406af17cf375b570b3eb8ea7ca2d81cdeb002f972e6d668cd7b2bc2'
METHODS=['Jitter','SAOnly','Hybrid','SA_Budget','Adaptive']
SEEDS=[20261024+179*s for s in range(10)]


def rows(path):
    with Path(path).open(encoding='utf-8-sig') as f:return list(csv.DictReader(f))


def write_rows(path, data):
    data=list(data)
    if not data:return
    with Path(path).open('w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=data[0]);w.writeheader();w.writerows(data)


def write_gather(path, data, x, label):
    path.parent.mkdir(parents=True,exist_ok=True)
    with path.open('w',encoding='utf-8',newline='\n') as f:
        f.write('# '+label+'\n# offsets_m='+','.join(format(v,'.14g') for v in x)+'\n')
        np.savetxt(f,data,delimiter=',',fmt='%.12g')


def write_mask(path, mask):
    path.parent.mkdir(parents=True,exist_ok=True);np.savetxt(path,np.asarray(mask,dtype=int),fmt='%d')


def initial_mask(n,m,seed,eligible=None):
    allowed=np.arange(n) if eligible is None else np.flatnonzero(eligible)
    rng=np.random.default_rng(seed);chosen=[]
    for k in range(m):
        a,b=k*len(allowed)//m,(k+1)*len(allowed)//m
        chosen.append(allowed[rng.integers(a,b)])
    result=np.zeros(n,dtype=bool);result[chosen]=True;return result


def real_raw():
    if digest(SOURCE)!=SOURCE_HASH:raise ValueError('RRAW identity changed; no implicit encoding override')
    data,x,cdp,meta=read_segy(SOURCE)
    order=np.argsort(x,kind='stable')
    return data[:,order],x[order].astype(float),cdp[order],meta


class Recorder:
    def __init__(self,out,exe,configuration):
        self.out=out.resolve();self.exe=exe.resolve();self.out.mkdir(parents=True,exist_ok=False)
        self.lock=threading.RLock();self.count=0
        paths=sorted(p for p in (ROOT/'IrrGeometryDesign').rglob('*') if p.suffix in ('.h','.cpp','.inc') and 'GeneratedFiles' not in p.parts)
        paths += [ROOT/'tools'/name for name in ['audit_joint_studies.py','audit_numerics.py','run_audit_checks.py',
                                               'review_studies.py','convert_rraw_to_engine_csv.py','make_59x256.py']]
        fingerprint={str(p.relative_to(ROOT)):digest(p) for p in paths}
        self.signature=(self.exe.stat().st_size,self.exe.stat().st_mtime_ns)
        self.state=dict(protocol=PROTOCOL,configuration=configuration,started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                        git_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT).decode().strip(),
                        program=str(self.exe),executable_sha256=digest(self.exe),source_sha256=fingerprint,
                        raw_sha256=digest(SOURCE),python=platform.python_version(),numpy=np.__version__,platform=platform.platform(),steps={})
        self.save()

    def save(self):
        with self.lock:
            path=self.out/'manifest.json';temporary=path.with_suffix('.tmp')
            temporary.write_text(json.dumps(self.state,indent=2,allow_nan=False),encoding='utf-8');temporary.replace(path)

    def call(self,key,args,destination):
        if (self.exe.stat().st_size,self.exe.stat().st_mtime_ns)!=self.signature:raise RuntimeError('program changed')
        command=[str(self.exe),*map(str,args)]
        inputs={str(Path(v).resolve()):digest(v) for v in command[1:] if Path(v).is_file()}
        if destination.exists():raise FileExistsError(destination)
        logfile=self.out/'logs'/(key+'.log');logfile.parent.mkdir(exist_ok=True)
        started=time.perf_counter()
        with logfile.open('wb') as log:r=subprocess.run(command,cwd=ROOT,stdout=log,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        record=dict(command=command,inputs=inputs,exit_code=r.returncode,seconds=time.perf_counter()-started,
                    outputs={str(p.relative_to(self.out)):digest(p) for p in sorted(destination.rglob('*')) if p.is_file()})
        with self.lock:
            if key in self.state['steps']:raise ValueError('duplicate run key')
            self.state['steps'][key]=record;self.count+=1
            with (self.out/'journal.jsonl').open('a',encoding='utf-8') as f:f.write(json.dumps(dict(key=key,step=record))+'\n')
            if self.count%50==0 or r.returncode:self.save();print('completed',self.count,key,flush=True)
        if r.returncode:raise RuntimeError(f'native call failed: {key}, rc={r.returncode}')

    def finish(self):
        if digest(self.exe)!=self.state['executable_sha256']:raise RuntimeError('program fingerprint changed')
        for name,value in self.state['source_sha256'].items():
            if digest(ROOT/name)!=value:raise RuntimeError('source fingerprint changed: '+name)
        for step in self.state['steps'].values():
            for name,value in step['inputs'].items():
                if digest(name)!=value:raise RuntimeError('input changed: '+name)
        self.state['completed_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();self.state['completed']=True;self.save()


def native_queries(recorder,key,lines):
    path=recorder.out/(key+'.txt');path.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    target=recorder.out/key;recorder.call(key,['--audit-queries',path,target],target)
    return rows(target/'queries.csv')


def reconstruct(recorder,key,data,x,mask,cold=False):
    # The native process receives only K. H and inner validation amplitudes are physically absent from its input.
    observed=np.zeros_like(data);observed[:,mask]=data[:,mask]
    gather=recorder.out/'observed'/(key+'.csv');maskfile=recorder.out/'masks'/(key+'.txt')
    write_gather(gather,observed,x,'observed only; unsampled amplitudes zero; '+PROTOCOL);write_mask(maskfile,mask)
    target=recorder.out/'native'/key
    args=['--review-reconstruct',gather,'--mask',maskfile,'--out',target,'--iter',40]
    if cold:args.append('--cold')
    recorder.call(key,args,target)
    return np.loadtxt(target/'prediction.csv',delimiter=',')


def nested(recorder,key,data,x,mask,seed):
    rng=np.random.default_rng(seed);known=np.flatnonzero(mask)
    validation=rng.permutation(known)[:max(2,len(known)//3)];train=mask.copy();train[validation]=False
    linear_inner=linear_known(data,x,train)
    inner=reconstruct(recorder,key+'_inner_default',data,x,train)
    cold=reconstruct(recorder,key+'_inner_cold',data,x,train,True)
    selected,candidates=validation_choice(data[:,validation],[('zero',np.zeros_like(inner[:,validation])),
        ('linear',linear_inner[:,validation]),('default',inner[:,validation]),('cold',cold[:,validation])])
    choice=dict(selected=selected,candidates=candidates,known=known.tolist(),validation=validation.tolist(),
                train=np.flatnonzero(train).tolist(),seed=seed)
    (recorder.out/'selection').mkdir(exist_ok=True)
    (recorder.out/'selection'/(key+'.json')).write_text(json.dumps(choice,indent=2))
    fixed=reconstruct(recorder,key+'_fixed',data,x,mask)
    name=selected['candidate'];linear=linear_known(data,x,mask)
    if name=='zero':prediction=np.zeros_like(data)
    elif name=='linear':prediction=linear.copy()
    elif name=='default':prediction=fixed.copy()
    else:prediction=reconstruct(recorder,key+'_selected_cold',data,x,mask,True)
    prediction=prediction*selected['gain'];prediction[:,mask]=data[:,mask]
    (recorder.out/'selected').mkdir(exist_ok=True)
    np.savetxt(recorder.out/'selected'/(key+'.csv'),prediction,delimiter=',',fmt='%.12g')
    return fixed,prediction,linear,choice


def synthetic(out):
    models=[];t=np.arange(256)[:,None]*.004;x=np.arange(128)*12.5;position=x-x.mean()
    for family in ['dipping','curved','faulted']:
        for structure in [0,1]:
            for noise in [.01,.05]:
                index=len(models);seed=65100+index;rng=np.random.default_rng(seed);data=np.zeros((256,128));events=[]
                for layer,t0 in enumerate([.22,.46,.70]):
                    dip=[.000035,.00012][structure]*(-1 if layer==1 else 1)
                    travel=np.sqrt(t0*t0+(position/(1800 if structure else 2600))**2) if family=='curved' else t0+dip*position
                    throw=[.025,.07][structure] if family=='faulted' else 0.
                    travel=travel+throw*(position>50);arg=(np.pi*(22+3*layer)*(t-travel))**2
                    data+=(1-.18*layer)*(1+.1*np.sin(x/400+layer))*(1-2*arg)*np.exp(-arg)
                    events.append(dict(t0=t0,dip=dip,throw=throw,frequency=22+3*layer))
                data+=rng.normal(0,noise,data.shape);name=f'model_{index:02d}'
                path=out/'truth'/(name+'.csv');write_gather(path,data,x,'analytic '+family+'; dt_s=.004; dx_m=12.5')
                meta=dict(model=name,family=family,structure=structure,noise=noise,seed=seed,events=events,truth_sha256=digest(path))
                path.with_suffix('.json').write_text(json.dumps(meta,indent=2));models.append((data,x,meta))
    return models


def joint(recorder,workers):
    models=synthetic(recorder.out);n=128;h=np.zeros(n,dtype=bool);h[4::8]=True
    all_queries=[];search_meta={}
    for design in ['common_H','complement']:
        eligible=~h if design=='common_H' else None
        for m in [38,64,90]:
            for si,seed in enumerate(SEEDS[:3]):
                initial=initial_mask(n,m,seed,eligible)
                for method in METHODS:
                    key=f'{design}_m{m}_s{si}_{method}'
                    all_queries.append(query(key,initial,method='measure' if method=='Jitter' else method,seed=seed+1,budget=300,eligible=eligible))
                    search_meta[key]=dict(design=design,m=m,seed=seed,method=method,seed_index=si)
    search=native_queries(recorder,'joint_search',all_queries)
    write_rows(recorder.out/'search.csv',search)
    tasks=[(data,x,meta,row) for data,x,meta in models for row in search]
    def case(task):
        data,x,meta,row=task;info=search_meta[row['id']];mask=np.array(list(row['mask']),dtype=int).astype(bool)
        key=meta['model']+'_'+row['id'];common=info['design']=='common_H';hidden=h if common else ~mask
        assert not np.any(mask&h) if common else True
        if common:
            fixed,selected,linear,choice=nested(recorder,key,data,x,mask,info['seed']+909)
        else:
            fixed=reconstruct(recorder,key+'_fixed',data,x,mask);linear=linear_known(data,x,mask);selected=None;choice=None
        reference=data[:,hidden];p=fixed[:,hidden];energy=float(np.sum(reference**2))
        result=dict(case=key,**{k:meta[k] for k in ['model','family','structure','noise']},**info,
                    n=n,mask=row['mask'],evaluation_bits=''.join(str(int(v)) for v in hidden),
                    mu=float(row['mu']),max_gap=int(row['max_gap']),nn_cv=float(row['nn_cv']),
                    zero_snr=0.,linear_snr=float(snr(data,linear,hidden)),fixed_snr=float(snr(data,fixed,hidden)),
                    selected_snr='' if selected is None else float(snr(data,selected,hidden)),
                    selected='' if choice is None else choice['selected']['candidate'],gain='' if choice is None else choice['selected']['gain'],
                    prediction_energy_ratio=float(np.sum(p*p)/energy),cross_term=float(np.sum(p*reference)/energy))
        return result
    with ThreadPoolExecutor(max_workers=workers) as pool:
        result=list(pool.map(case,tasks))
    write_rows(recorder.out/'joint.csv',result)


def search_study(recorder):
    _,physical,_,_=real_raw();lines=[]
    variants=[('Adaptive',32,12),('SAOnly',32,12),('Hybrid',32,12),('SA_Budget',32,12),
              ('FixedSwitch',32,12),('NoRecycle',32,12),('AdjacentOnly',32,12),('NoPerturb',32,12)]
    variants += [('Adaptive',minimum,window) for minimum,window in [(16,8),(16,24),(32,8),(32,24),(64,8),(64,24)]]
    metadata={}
    for domain,n,counts,x in [('uniform',128,[38,64,90],None),('physical',59,[20,30,40],physical)]:
        for ri,m in enumerate(counts):
            for si,seed in enumerate(SEEDS):
                initial=initial_mask(n,m,seed)
                for method,minimum,window in variants:
                    key=f'{domain}_r{ri}_s{si}_{method}_{minimum}_{window}'
                    lines.append(query(key,initial,x,method,seed+1,300,minimum,window))
                    metadata[key]=dict(domain=domain,rate=ri,seed_index=si,minimum=minimum,window=window)
    result=native_queries(recorder,'sensitivity',lines)
    write_rows(recorder.out/'search.csv',[dict(**r,**metadata[r['id']]) for r in result])


def timing(recorder):
    _,physical,_,_=real_raw();results=[]
    for domain,n,m,x in [('uniform',128,64,None),('physical',59,30,physical)]:
        for si,seed in enumerate(SEEDS):
            methods=['SA_Budget','Adaptive','NoPerturb'];methods=methods[si%3:]+methods[:si%3]
            for method in methods:
                key=f'{domain}_s{si}_{method}'
                line=query(key,initial_mask(n,m,seed),x,method,seed+1,100000,wall=30)
                result=native_queries(recorder,key,[line])[0];results.append(dict(**result,domain=domain,seed_index=si))
    write_rows(recorder.out/'timing.csv',results)


def real_study(recorder,workers):
    data,x,cdp,meta=real_raw();ts=np.arange(250)*.008;td=np.arange(499)*.004
    up=np.stack([np.interp(td,ts,row) for row in data.T],axis=1)
    windows=[('native_full',data,.008),('native_crop',data[:128],.008),('upsampled_full',up,.004),('upsampled_crop',up[:256],.004)]
    provenance=dict(profile='RRAW_IEEE_LE_UNCONFIRMED',source_sha256=meta['sha256'],offset_units='header-declared metres; geometry conflicts unresolved',
                    amplitude_units='unknown raw units',source_sample_rate_hz=125.,cdp_order=cdp.tolist(),windows=[])
    for name,values,dt in windows:
        path=recorder.out/'truth'/(name+'.csv');write_gather(path,values,x,f'{name}; assumed IEEE-LE; dt_s={dt}; no spatial interpolation/filter/normalization')
        provenance['windows'].append(dict(name=name,dt_s=dt,time_end_s=(len(values)-1)*dt,shape=list(values.shape),sha256=digest(path)))
    (recorder.out/'provenance.json').write_text(json.dumps(provenance,indent=2))
    tasks=[(name,values,dt,m,si,seed) for name,values,dt in windows for m in [20,30,40] for si,seed in enumerate(SEEDS[:2])]
    def case(task):
        name,values,dt,m,si,seed=task;mask=initial_mask(59,m,seed);key=f'{name}_m{m}_s{si}'
        fixed,selected,linear,choice=nested(recorder,key,values,x,mask,seed+909)
        ref=values[:,~mask];p=fixed[:,~mask];energy=float(np.sum(ref**2))
        return dict(case=key,window=name,dt_s=dt,m=m,seed=seed,mask=''.join(str(int(v)) for v in mask),
                    linear_snr=float(snr(values,linear,~mask)),fixed_snr=float(snr(values,fixed,~mask)),
                    selected_snr=float(snr(values,selected,~mask)),selected=choice['selected']['candidate'],gain=choice['selected']['gain'],
                    prediction_energy_ratio=float(np.sum(p*p)/energy),cross_term=float(np.sum(p*ref)/energy))
    with ThreadPoolExecutor(max_workers=workers) as pool:result=list(pool.map(case,tasks))
    write_rows(recorder.out/'real.csv',result)
    # A complete raw->preprocess->nested prediction intervention; selection must ignore every outer-hidden value.
    mask=initial_mask(59,30,SEEDS[0]);changed=data.copy();changed[:,~mask]=13*changed[:,~mask]+7
    a=nested(recorder,'isolation_raw_A',data,x,mask,SEEDS[0]+909)
    b=nested(recorder,'isolation_raw_B',changed,x,mask,SEEDS[0]+909)
    raw_error=float(np.max(abs(a[1]-b[1])));same_choice=a[3]['selected']==b[3]['selected']
    ca,_=legacy_convert(data,x);cb,_=legacy_convert(changed,x)
    la=nested(recorder,'isolation_legacy_A',ca,x,mask,SEEDS[0]+909)
    lb=nested(recorder,'isolation_legacy_B',cb,x,mask,SEEDS[0]+909)
    # Compare predictions in a common physical amplitude scale. Normalization includes known observations.
    crop=lambda value:np.vstack([value[:128],.5*(value[127]+value[128])])
    scale_a=float(np.max(abs(crop(data))));scale_b=float(np.max(abs(crop(changed))))
    legacy_error=float(np.max(abs(la[1]*scale_a-lb[1]*scale_b)))
    report=dict(raw_prediction_max_abs_difference=raw_error,raw_selection_unchanged=same_choice,
                legacy_physical_prediction_max_abs_difference=legacy_error,
                legacy_selection_same_candidate=la[3]['selected']['candidate']==lb[3]['selected']['candidate'],
                legacy_gains=[la[3]['selected']['gain'],lb[3]['selected']['gain']],
                note='legacy comparison here is unrounded; text export rounding is evaluated separately')
    if raw_error!=0 or not same_choice:raise AssertionError('raw outer leakage')
    (recorder.out/'isolation.json').write_text(json.dumps(report,indent=2))


def run(args):
    configuration=dict(protocol=PROTOCOL,stage=args.stage,search_budget=300,seeds=SEEDS,workers=args.workers,
                       methods=METHODS,common_H=list(range(4,128,8)),models=12,
                       real_profile='RRAW_IEEE_LE_UNCONFIRMED',configuration_document_sha256=digest(ROOT/'docs/EXPERIMENT_PROTOCOL.md'))
    recorder=Recorder(args.out,args.exe,configuration)
    if args.stage=='joint':joint(recorder,args.workers)
    elif args.stage=='search':search_study(recorder)
    elif args.stage=='timing':timing(recorder)
    elif args.stage=='real':real_study(recorder,args.workers)
    recorder.finish();print('completed',args.stage,recorder.count,'native steps',flush=True)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True)
    p.add_argument('--exe',type=Path,default=ROOT/'x64/Release/IrrGeometryDesign.exe')
    p.add_argument('--stage',choices=['search','joint','real','timing'],required=True)
    p.add_argument('--workers',type=int,default=4,choices=range(1,9));run(p.parse_args())
