"""Connect a 2D spatial mask to held-out waveform recovery on one frozen Stratton cube."""
import argparse
import json
from pathlib import Path

import numpy as np
from research.validation.artifacts import finite_arrays,finite_error,validate_native_steps

from research.common.hashing import digest
from research.common.paths import executable
from research.common.tabular import rows,write_rows
from research.data.confirmation import PROTOCOL,freeze,volume_headers,decode
from research.experiments.audit_joint_studies import Recorder,initial_mask


def fk3d(data,known,iterations=80,fraction=.01):
    data=np.asarray(data,float);known=np.asarray(known,bool)
    if (data.ndim!=3 or known.shape!=data.shape[1:] or known.sum()<2 or iterations<1 or
            not 0<=fraction<=1 or not np.isfinite(data[:,known]).all()):
        raise ValueError('Finite observed-only 3D input and matching spatial mask required')
    scale=max(float(np.max(abs(data[:,known]))),1e-20)
    observed=np.zeros_like(data);observed[:,known]=data[:,known]/scale
    lam=fraction*float(abs(np.fft.fftn(observed,norm='ortho')).max())
    current=np.zeros_like(data);extra=current.copy();momentum=1.
    for _ in range(iterations):
        gradient=np.zeros_like(data);gradient[:,known]=(extra-observed)[:,known]
        coefficients=np.fft.fftn(extra-gradient,norm='ortho')
        coefficients*=np.maximum(1-lam/np.maximum(abs(coefficients),1e-30),0)
        updated=np.fft.ifftn(coefficients,norm='ortho').real
        next_momentum=(1+np.sqrt(1+4*momentum**2))/2
        extra=updated+(momentum-1)/next_momentum*(updated-current)
        current,momentum=updated,next_momentum
    current*=scale;current[:,known]=data[:,known]
    return current


def load_cube():
    state=freeze();spec=next(s for s in state['protocol']['external']['surveys'] if s['id']=='stratton')
    raw,info=volume_headers(spec);lines=np.unique(info['inline']);crosslines=np.unique(info['crossline'])
    if len(lines)<16 or len(crosslines)<16:raise ValueError('Frozen spatial grid unavailable')
    lines=lines[(len(lines)-16)//2:(len(lines)-16)//2+16]
    crosslines=crosslines[(len(crosslines)-16)//2:(len(crosslines)-16)//2+16]
    lookup={tuple(k):i for i,k in enumerate(zip(info['inline'],info['crossline']))}
    try:indices=[lookup[int(y),int(x)] for y in lines for x in crosslines]
    except KeyError as error:raise ValueError('Frozen spatial cube has a missing trace; no score-based replacement') from error
    data=decode(raw,info,indices,512,640).reshape(128,16,16)
    return data,dict(inlines=lines.tolist(),crosslines=crosslines.tolist(),trace_indices=indices,source=info['source'])


def run(out):
    whole=json.loads(PROTOCOL.read_text());plan=whole['spatial_waveform'];data,provenance=load_cube()
    out=Path(out).resolve();recorder=Recorder(out,executable(),dict(plan=plan,provenance=provenance,
        protocol_sha256=digest(PROTOCOL),independent_surveys=1),protocol=whole['protocol']+'_spatial')
    np.savez_compressed(out/'truth.npz',data=data)
    points=np.array([(x,y) for y in range(16) for x in range(16)],float)
    hidden=np.zeros(256,bool);hidden[4::8]=True;lines=[];metadata={}
    for m in plan['counts']:
        for seed in plan['seeds']:
            initial=initial_mask(256,m,seed,~hidden)
            for method in plan['methods']:
                key=f'm{m}_s{seed}_{method}';budget=1 if method=='Jitter' else plan['budget'];prefix=budget if method=='SA' else 64
                fields=[key,seed+1,budget,prefix,''.join(map(str,initial.astype(int))),''.join(map(str,(~hidden).astype(int))),
                    '|'.join(f'{x},{y}' for x,y in points)]
                lines.append(';'.join(map(str,fields)));metadata[key]=dict(m=m,seed=seed,method=method,budget=budget)
    source=out/'spatial_queries.txt';source.write_text('\n'.join(lines)+'\n');target=out/'search'
    recorder.call('spatial_search',['--spatial-queries',source,target],target)
    searches=rows(target/'queries.csv');results=[];gram_error=0.
    frequency=np.array([(x/16,y/16) for y in range(8) for x in range(8)])
    dictionary=np.exp(2j*np.pi*(points@frequency.T))
    (out/'predictions').mkdir()
    for row in searches:
        info=metadata[row['id']];mask=np.array(list(row['mask']),int).astype(bool)
        if mask.sum()!=info['m'] or np.any(mask&hidden) or int(row['evaluations'])!=info['budget']:
            raise ValueError('Spatial search count, budget or hidden-set violation')
        a=dictionary[mask];gram=a.conj().T@a/mask.sum();np.fill_diagonal(gram,0)
        gram_error=max(gram_error,abs(float(abs(gram).max())-float(row['mu'])))
        known=mask.reshape(16,16);prediction=fk3d(data,known)
        reference=data.reshape(128,256)[:,hidden];predicted=prediction.reshape(128,256)[:,hidden]
        nmse=float(np.sum((predicted-reference)**2)/np.sum(reference**2))
        projection=float(np.max(abs(prediction[:,known]-data[:,known])))
        path=out/'predictions'/(row['id']+'.npz');np.savez_compressed(path,prediction=prediction,known=known,hidden=hidden.reshape(16,16))
        results.append(dict(case=row['id'],**info,mu=float(row['mu']),snr=-10*np.log10(max(nmse,1e-30)),nmse=nmse,
            rms_ratio=float(np.linalg.norm(predicted)/np.linalg.norm(reference)),projection_error=projection,prediction_sha256=digest(path),independent_surveys=1))
    # Change every unobserved amplitude while keeping observed data fixed.
    first=searches[0];mask=np.array(list(first['mask']),int).astype(bool).reshape(16,16)
    changed=data.copy();changed[:,~mask]=13*changed[:,~mask]+7
    isolation=float(np.max(abs(fk3d(changed,mask)-fk3d(data,mask))))
    if isolation!=0 or gram_error>2e-11 or any(r['projection_error']!=0 for r in results):raise ValueError('Spatial independent checks failed')
    write_rows(out/'recovery.csv',results)
    artifacts={p.relative_to(out).as_posix():digest(p) for p in (out/'predictions').iterdir()}
    artifacts['truth.npz']=digest(out/'truth.npz')
    recorder.state.update(status='passed',cases=len(results),gram_max_error=gram_error,isolation_max_error=isolation,
        derived_artifacts_sha256=artifacts,no_imaging_claim=True,protocol_file_sha256=digest(PROTOCOL))
    recorder.finish();verify(out)
    print(json.dumps(dict(status='passed',cases=len(results),gram_max_error=gram_error,isolation_max_error=isolation)),flush=True)
    return recorder.state


def verify(out):
    out=Path(out)
    validate_native_steps(json.loads((out/'manifest.json').read_text(encoding='utf-8')),out)
    with np.load(out/'truth.npz',allow_pickle=False) as saved:truth=saved['data']
    error=0.
    for row in rows(out/'recovery.csv'):
        path=out/'predictions'/(row['case']+'.npz')
        if digest(path)!=row['prediction_sha256']:raise ValueError('Changed spatial prediction')
        with np.load(path,allow_pickle=False) as saved:
            finite_arrays(truth,saved['prediction'])
            target=truth[:,saved['hidden']];prediction=saved['prediction'][:,saved['hidden']]
            nmse=float(np.sum((prediction-target)**2)/np.sum(target**2))
            error=max(error,finite_error(nmse,row['nmse']))
    if error>1e-12:raise ValueError('Independent spatial score mismatch')
    report=dict(status='passed',score_max_error=error,scope='One Stratton cube; no new independent survey beyond the Stratton section study')
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n');return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--out',type=Path,required=True);a=p.parse_args();run(a.out)
