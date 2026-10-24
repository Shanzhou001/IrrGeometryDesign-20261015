"""Read-only independent audit of v5 hashes, masks, observations, predictions and selections."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import struct

import numpy as np

from research.common.hashing import digest

from research.common.paths import ROOT


def table(path):
    with Path(path).open(encoding='utf-8-sig') as f:return list(csv.DictReader(f))


def gather(path):
    text=Path(path).read_text(encoding='utf-8-sig').splitlines()
    coordinates=[s for s in text if s.startswith('# offsets_m=')]
    assert len(coordinates)==1
    x=np.array(list(map(float,coordinates[0].split('=',1)[1].split(','))))
    data=np.loadtxt(path,delimiter=',',comments='#')
    assert np.isfinite(data).all() and data.shape[1]==len(x) and np.all(np.diff(x)>0)
    return data,x


def binary_mask(text):
    assert text and set(text)<=set('01')
    return np.fromiter((v=='1' for v in text),bool)


def objective(mask,x):
    n=len(mask);m=int(mask.sum());period=np.ptp(x)*n/(n-1)
    atom=np.exp(-2j*np.pi*np.outer(np.arange(1,n),x-x[0])/period)
    mu=float(abs(atom[:,mask].sum(axis=1)).max()/m)
    selected=x[mask];dist=np.diff(selected)
    maximum=max([selected[0]-x[0],x[-1]-selected[-1],*dist])
    near=np.minimum(np.r_[np.inf,dist],np.r_[dist,np.inf])
    cv=float(near.std()/near.mean()) if len(selected)>1 else 0.
    return mu,maximum,cv


def snr(truth,prediction,h):
    energy=np.square(truth[:,h]).sum();error=np.square(truth[:,h]-prediction[:,h]).sum()
    assert energy>0 and np.isfinite(prediction).all()
    return float(10*np.log10(energy/max(error,1e-30)))


def verify_stage(path,runtime=None):
    manifest=json.loads((path/'manifest.json').read_text());assert manifest.get('completed')
    assert manifest['protocol']=='audit_joint_v5'
    program=runtime/'bin/IrrGeometryDesign.exe' if runtime else Path(manifest['program'])
    assert digest(program)==manifest['executable_sha256']
    for name,value in manifest['source_sha256'].items():assert digest((runtime/'sources' if runtime else ROOT)/name)==value,name
    hashed=0
    for step in manifest['steps'].values():
        assert step['exit_code']==0 and step['outputs']
        for name,value in step['inputs'].items():assert digest(name)==value,name
        for name,value in step['outputs'].items():assert digest(path/name)==value,name;hashed+=1
    checks={'stage':path.name,'native_steps':len(manifest['steps']),'hashed_native_outputs':hashed}
    raw=(ROOT/'data/real_seismic/Data/RRAW.SGY').read_bytes()
    xreal=np.sort([struct.unpack_from('<i',raw,3600+j*1240+36)[0] for j in range(59)]).astype(float)
    query_count=0
    for file in path.glob('*/queries.csv'):
        input_file=path/(file.parent.name+'.txt')
        if not input_file.exists():continue
        specs={line.split(';')[0]:line.split(';') for line in input_file.read_text().splitlines()}
        for r in table(file):
            mask=binary_mask(r['mask']);spec=specs[r['id']];x=np.array(list(map(float,spec[11].split(',')))) if spec[11] else np.arange(len(mask))
            assert int(r['ok'])==1 and mask.sum()==int(r['m'])
            if spec[13]:assert not np.any(mask&~binary_mask(spec[13]))
            mu,gap,cv=objective(mask,x)
            assert abs(mu-float(r['mu']))<3e-6,(file,r['id'],'mu')
            assert abs(cv-float(r['nn_cv']))<3e-6
            assert abs(float(r['objective'])-(mu+float(spec[6])*max(0,gap/(len(mask)*np.ptp(x)/(len(mask)-1)/mask.sum())-1)+float(spec[7])*cv))<3e-6
            assert float(r['evals'])<=float(r['budget'])
            trace=table(file.parent/(r['id']+'.trace.csv'));assert len(trace)==int(r['evals'])
            best=np.array([float(t['best']) for t in trace]);assert np.all(np.diff(best)<=1e-7)
            assert abs(best[-1]-float(r['objective']))<2e-6
            if trace[-1]['accepted_current']:
                assert int(r['attempts'])>=len(trace)-1
                assert all(float(t['best'])<=float(t['accepted_current'])+1e-6 for t in trace)
            query_count+=1
    checks['search_queries']=query_count
    checked=0
    for csvfile in [path/'joint.csv',path/'real.csv']:
        if not csvfile.exists():continue
        for r in table(csvfile):
            key=r['case'];mask=binary_mask(r['mask'])
            truth,x=gather(path/'truth'/((r['model'] if 'model' in r else r['window'])+'.csv'))
            h=binary_mask(r['evaluation_bits']) if 'evaluation_bits' in r else ~mask
            assert not np.any(mask&h) and mask.sum()==int(r['m'])
            if r.get('design')=='common_H':assert np.array_equal(np.flatnonzero(h),np.arange(4,128,8))
            fixed=np.loadtxt(path/'native'/(key+'_fixed')/'prediction.csv',delimiter=',')
            assert abs(snr(truth,fixed,h)-float(r['fixed_snr']))<1e-7
            assert np.max(abs(fixed[:,mask]-truth[:,mask]))<2e-5*max(1,abs(truth[:,mask]).max())
            linear=np.array([np.interp(x,x[mask],line[mask]) for line in truth])
            assert abs(snr(truth,linear,h)-float(r['linear_snr']))<1e-7
            if r['selected_snr']!='':
                selection=json.loads((path/'selection'/(key+'.json')).read_text())
                train=np.array(selection['train']);v=np.array(selection['validation']);known=np.array(selection['known'])
                assert set(train).isdisjoint(v) and set(train)|set(v)==set(np.flatnonzero(mask)) and set(known)==set(np.flatnonzero(mask))
                candidates=[('zero',np.zeros_like(truth[:,v])),('linear',np.array([np.interp(x[v],x[train],t[train]) for t in truth]))]
                for name,suffix in [('default','_inner_default'),('cold','_inner_cold')]:
                    prediction=np.loadtxt(path/'native'/(key+suffix)/'prediction.csv',delimiter=',');candidates.append((name,prediction[:,v]))
                    observed,ox=gather(path/'observed'/(key+suffix+'.csv'))
                    absent=np.ones(len(x),bool);absent[train]=False
                    assert np.count_nonzero(observed[:,absent])==0
                    assert np.max(abs(observed[:,train]-truth[:,train]))<1e-7*max(1,abs(truth[:,train]).max())
                estimates=[];target=truth[:,v].ravel()
                for name,p in candidates:
                    p=p.ravel();gain=float(np.clip((p@target)/(p@p),0,1)) if p@p>1e-30 else 0.
                    estimates.append((float(np.mean((target-gain*p)**2)),name,gain))
                chosen=min(range(4),key=lambda i:(estimates[i][0],i));_,name,gain=estimates[chosen]
                assert name==r['selected']==selection['selected']['candidate'] and abs(gain-float(r['gain']))<1e-9
                prediction=np.loadtxt(path/'selected'/(key+'.csv'),delimiter=',')
                assert abs(snr(truth,prediction,h)-float(r['selected_snr']))<1e-7
                if name=='zero':expected=np.zeros_like(truth)
                elif name=='linear':expected=linear
                elif name=='default':expected=fixed
                else:expected=np.loadtxt(path/'native'/(key+'_selected_cold')/'prediction.csv',delimiter=',')
                expected=gain*expected;expected[:,mask]=truth[:,mask]
                assert np.max(abs(expected-prediction))<1e-7*max(1,abs(truth).max())
            checked+=1
    checks['reconstruction_records']=checked
    return checks


def verify(root,report,runtime=None):
    assert not report.exists(),'verification report output already exists'
    if runtime:
        snapshot=json.loads((runtime/'snapshot.json').read_text(encoding='utf-8'))
        for original,value in snapshot['dependency_sha256'].items():assert digest(runtime/'bin'/Path(original).name)==value
    result={'passed':True,'verified_runtime':str(runtime) if runtime else 'current checkout',
            'stages':[verify_stage(root/name,runtime) for name in ['search','joint','real','timing']]}
    baseline=json.loads((root/'baseline_manifest.json').read_text(encoding='utf-8'));count=0
    for name,value in baseline['protected_files'].items():
        assert digest(ROOT/name)==value['sha256'],name;count+=1
    result['unchanged_protected_files']=count
    report.write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);p.add_argument('--report',type=Path,required=True)
    p.add_argument('--runtime',type=Path,help='Explicit immutable source/binary snapshot; never silently substitute for a changed checkout')
    a=p.parse_args();verify(a.root.resolve(),a.report.resolve(),a.runtime.resolve() if a.runtime else None)
