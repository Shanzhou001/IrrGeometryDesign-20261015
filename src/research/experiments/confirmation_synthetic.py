"""Prospective twenty-structure common-H experiment; historical six-template results stay unchanged."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np
from research.validation.artifacts import finite_arrays,finite_error,validate_native_steps

from research.analysis.contribution_report import design_check
from research.analysis.summarize_audit_v5 import paired, correction
from research.common.hashing import digest
from research.common.paths import executable
from research.common.tabular import rows, write_rows
from research.data.confirmation import PROTOCOL, freeze
from research.experiments.audit_joint_studies import Recorder, initial_mask, native_queries, nested, write_gather
from research.experiments.review_studies import snr
from research.validation.audit_numerics import query


def structures(plan):
    t=np.arange(plan['samples'])[:,None]*.004;x=np.arange(plan['n'])*12.5
    position=x-x.mean();result=[]
    for family in plan['families']:
        for variant in range(plan['structures_per_family']):
            index=len(result);rng=np.random.default_rng(np.random.SeedSequence([plan['structure_seed'],index]))
            data=np.zeros((len(t),len(x)));events=[]
            for layer in range(4):
                t0=.15+.19*layer+rng.uniform(-.025,.025);dip=rng.uniform(.000025,.00015)*(-1 if layer%2 else 1)
                frequency=rng.uniform(18,38);amplitude=rng.uniform(.5,1.)*(-1 if layer%3==1 else 1)
                velocity=rng.uniform(1700,3200);apex=rng.uniform(-250,250);throw=rng.uniform(.025,.075)
                travel=t0+dip*position
                if family=='curved':travel=np.sqrt(t0*t0+((position-apex)/velocity)**2)
                elif family=='faulted':travel+=throw*(position>apex)
                elif family=='crossing':travel=t0+dip*position*(-1 if variant%2 else 1)+rng.uniform(-1e-7,1e-7)*position**2
                elif family=='diffraction':travel=np.sqrt(t0*t0+((position-apex)/(velocity*.65))**2)
                arg=(np.pi*frequency*(t-travel))**2
                data+=amplitude*(1+.12*np.sin(x/rng.uniform(250,550)+layer))*(1-2*arg)*np.exp(-arg)
                events.append(dict(t0=t0,dip=dip,frequency=frequency,amplitude=amplitude,velocity=velocity,apex=apex,throw=throw))
            result.append((data,x,dict(template=f't{index:02d}',family=family,variant=variant,events=events)))
    if len(result)!=plan['templates']:raise ValueError('Frozen template count mismatch')
    return result


def summarize(data,plan):
    template_means=[];comparisons=[]
    for m in plan['counts']:
        for recovery in ('fixed','selected'):
            by={}
            for method in plan['methods']:
                values=[]
                for template in sorted({r['template'] for r in data}):
                    same=[r for r in data if r['m']==m and r['method']==method and r['template']==template]
                    expected=len(plan['noise_levels'])*len(plan['mask_seeds'])
                    if len(same)!=expected:raise ValueError('Incomplete noise/mask repetition')
                    value=float(np.mean([r[recovery+'_snr'] for r in same]));values.append(value)
                    template_means.append(dict(m=m,recovery=recovery,method=method,template=template,snr=value))
                by[method]=values
            for baseline in ('Jitter','SA_Budget'):
                comparisons.append(dict(m=m,recovery=recovery,comparison='Adaptive-'+baseline,
                    family='primary' if recovery=='selected' else 'secondary',**paired(by['Adaptive'],by[baseline])))
    primary=correction([r for r in comparisons if r['family']=='primary'])
    secondary=correction([r for r in comparisons if r['family']=='secondary'])
    return template_means,primary+secondary


def run(out,workers=8,exe=None):
    freeze();whole=json.loads(PROTOCOL.read_text());plan=whole['synthetic'];out=Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    recorder=Recorder(out,Path(exe) if exe else executable(),dict(plan=plan,protocol_sha256=digest(PROTOCOL),
        workers=workers,synthetic_only=True,analysis_unit='separately seeded clean structural realization',
        target='same noisy realization on common hidden traces; noise and masks pooled by clean structural realization'),protocol=whole['protocol']+'_synthetic')
    (out/'protocol.json').write_text(json.dumps(whole,indent=2)+'\n')
    n=plan['n'];hidden=np.zeros(n,bool);hidden[4::8]=True;lines=[];metadata={}
    for m in plan['counts']:
        for seed in plan['mask_seeds']:
            initial=initial_mask(n,m,seed,~hidden)
            for method in plan['methods']:
                key=f'm{m}_s{seed}_{method}'
                lines.append(query(key,initial,method='measure' if method=='Jitter' else method,seed=seed+1,budget=plan['budget'],eligible=~hidden))
                metadata[key]=dict(m=m,seed=seed,method=method)
    searches=native_queries(recorder,'search',lines);write_rows(out/'search.csv',searches)
    models=[]
    for clean,x,meta in structures(plan):
        for level in plan['noise_levels']:
            seed=np.random.SeedSequence([plan['structure_seed'],int(meta['template'][1:]),int(level*1e6),1])
            data=clean+np.random.default_rng(seed).normal(0,level,clean.shape)
            name=meta['template']+'_noise'+str(int(level*1000));path=out/'truth'/(name+'.csv')
            write_gather(path,data,x,'prospective synthetic; common H; dt=.004; no field-data claim')
            models.append((data,x,dict(meta,model=name,noise=level,truth_sha256=digest(path))))
    tasks=[(data,x,meta,search) for data,x,meta in models for search in searches]
    def case(task):
        data,x,meta,search=task;info=metadata[search['id']];mask=np.array(list(search['mask']),int).astype(bool)
        if mask.sum()!=info['m'] or np.any(mask&hidden):raise ValueError('Invalid common-H mask')
        key=meta['model']+'_'+search['id']
        fixed,selected,linear,choice=nested(recorder,key,data,x,mask,info['seed']+909)
        return dict(case=key,template=meta['template'],family=meta['family'],variant=meta['variant'],noise=meta['noise'],
            model=meta['model'],**info,mask=search['mask'],evaluation_bits=''.join(map(str,hidden.astype(int))),
            mu=float(search['mu']),fixed_snr=float(snr(data,fixed,hidden)),selected_snr=float(snr(data,selected,hidden)),
            linear_snr=float(snr(data,linear,hidden)),selected=choice['selected']['candidate'],gain=choice['selected']['gain'])
    with ThreadPoolExecutor(max_workers=workers) as pool:data=list(pool.map(case,tasks))
    write_rows(out/'recovery.csv',data)
    means,pairs=summarize(data,plan);write_rows(out/'template_means.csv',means);write_rows(out/'paired_effects.csv',pairs)
    design=design_check(plan['templates'],plan['primary_family_size'])
    recorder.state.update(status='passed',cases=len(data),primary_design=design,
        primary_significant=sum(r['p_holm']<.05 for r in pairs if r['family']=='primary'),
        independent_real_surveys=0,protocol_file_sha256=digest(PROTOCOL),
        derived_artifacts_sha256={p.relative_to(out).as_posix():digest(p) for name in ('truth','selected','selection')
            for p in (out/name).rglob('*') if p.is_file()})
    recorder.finish();verify(out)
    print(json.dumps(dict(status='passed',cases=len(data),templates=plan['templates'],primary_can_reject=design['can_reject_any'])),flush=True)
    return recorder.state


def verify(out):
    out=Path(out);manifest=json.loads((out/'manifest.json').read_text());error=0.;count=0
    validate_native_steps(manifest,out)
    for step in manifest['steps'].values():
        if step['exit_code']!=0:raise ValueError('Failed native step')
        for name,value in step['outputs'].items():
            if digest(out/name)!=value:raise ValueError('Changed native prediction')
    truths={}
    for row in rows(out/'recovery.csv'):
        if row['model'] not in truths:truths[row['model']]=np.loadtxt(out/'truth'/(row['model']+'.csv'),comments='#',delimiter=',')
        truth=truths[row['model']];hidden=np.array(list(row['evaluation_bits']),int).astype(bool)
        for kind,path in [('fixed',out/'native'/(row['case']+'_fixed')/'prediction.csv'),('selected',out/'selected'/(row['case']+'.csv'))]:
            values=np.loadtxt(path,delimiter=',');finite_arrays(truth,values)
            error=max(error,finite_error(snr(truth,values,hidden),row[kind+'_snr']))
        count+=1
    if error>1e-7:raise ValueError('Independent prospective score mismatch')
    report=dict(status='passed',cases=count,score_max_error=error,manifest_sha256=digest(out/'manifest.json'))
    (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--out',type=Path,required=True);p.add_argument('--workers',type=int,default=8)
    a=p.parse_args();run(a.out,a.workers)
