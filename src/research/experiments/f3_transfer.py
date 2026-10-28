"""One frozen, untuned transfer of the existing selector/recovery to F3 poststack data."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.analysis.waveform_quality import quality,summarize
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import ROOT,executable
from research.common.tabular import write_rows
from research.data.f3 import load,DEFAULT,REGISTRY
from research.experiments.audit_joint_studies import Recorder
from research.experiments.viking_v2 import METHODS,prepare_search,evaluate,isolation_checks
from research.recovery import CANDIDATES

PLAN=Path(__file__).with_name('f3_transfer_protocol.json')


def run(out,exe=None,workers=12,dataset=DEFAULT,keep_intermediates=False):
    out=Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    if workers<1:raise ValueError('Positive worker count required')
    plan=json.loads(PLAN.read_text());plan['recovery_candidates']=list(CANDIDATES)
    windows,provenance=load(dataset)
    if set(windows[0])!=set(plan['ffids']) or provenance['plan_sha256']!=digest(PLAN):
        raise ValueError('F3 data selection differs from the plan frozen before waveform access')
    recorder=Recorder(out,Path(exe) if exe else executable(),dict(plan=plan,dataset_provenance=provenance,
        worker_count=workers,independent_surveys=1,role='untuned external poststack transfer',
        time_unit='sample_index',spatial_unit='trace_interval',no_imaging_claim=True),protocol=plan['protocol'])
    (out/'frozen_plan.json').write_text(json.dumps(plan,indent=2)+'\n')
    recorder.state['plan_sha256']=digest(out/'frozen_plan.json');recorder.save()
    search,bindings=prepare_search(recorder,windows,plan)
    def case(binding):
        record=evaluate(recorder,windows[0][binding['ffid']],binding,search[binding['search_id']],plan)
        record['section_index']=record.pop('ffid');record.pop('sample_interval_s');record.pop('time_origin_s')
        record['time_unit']='sample_index';record['spatial_unit']='trace_interval'
        record['max_gap_trace_intervals']=record.pop('max_gap_m')/25.
        return record
    with ThreadPoolExecutor(max_workers=workers) as pool:records=list(pool.map(case,bindings))
    write_rows(out/'recovery.csv',records)
    isolation=isolation_checks(recorder,windows,search,bindings,plan)
    diagnostics=[];score_error=projection_error=0.;artifacts={}
    for row in records:
        data=windows[0][int(row['section_index'])].data
        path=out/'predictions'/(row['case']+'.npz')
        with np.load(path,allow_pickle=False) as saved:
            hidden=saved['hidden'];known=saved['known']
            for name in ('cold','fk','selected','linear'):
                q=quality(data[:,hidden],saved[name][:,hidden])
                projection_error=max(projection_error,float(np.max(abs(saved[name][:,known]-data[:,known]))))
                score_error=max(score_error,abs(q['nmse']-row[name+'_nmse']))
                diagnostics.append(dict(case=row['case'],m=str(row['m']),method=row['method'],recovery=name,**q))
    if projection_error!=0 or score_error>1e-12:raise AssertionError('Independent F3 reconstruction check failed')
    for name,value in provenance['input_sha256'].items():
        if digest(name)!=value:raise ValueError('F3 input changed during the frozen transfer')
    if digest(REGISTRY)!=provenance['registry_sha256']:raise ValueError('F3 registry changed')
    if not keep_intermediates:
        from research.recovery import prune_intermediates
        prune_intermediates(out)
    for folder in ('features','selection','python','predictions'):
        artifacts.update({p.relative_to(out).as_posix():digest(p) for p in (out/folder).rglob('*') if p.is_file()})
    means=[]
    for m in plan['counts']:
        for method in METHODS:
            same=[r for r in records if r['m']==m and r['method']==method]
            for kind in ('cold','fk','selected','linear'):
                means.append(dict(m=m,method=method,recovery=kind,mean_snr=float(np.mean([r[kind+'_snr'] for r in same])),
                                  mean_nmse=float(np.mean([r[kind+'_nmse'] for r in same])),cases=len(same),independent_surveys=1))
    write_rows(out/'means.csv',means);write_rows(out/'waveform_means.csv',summarize(diagnostics))
    recorder.state.update(status='passed',recovery_cases=len(records),search_cases=len(search),native_calls=recorder.count,
        isolation=isolation,projection_max_error=projection_error,score_max_error=score_error,
        derived_artifacts_sha256=artifacts,protocol_file_sha256=digest(PLAN))
    recorder.finish()
    writer=FigureWriter('research/'+out.name);fig,ax=plt.subplots(figsize=(8,4),layout='constrained')
    for method in METHODS:
        same=[r for r in means if r['method']==method and r['recovery']=='selected']
        ax.plot([r['m'] for r in same],[r['mean_snr'] for r in same],'o-',label=method)
    ax.set(xlabel='Retained traces',ylabel='Mean common-H SNR (dB)',title='F3: frozen external poststack transfer, one new survey')
    ax.legend(fontsize=8);ax.grid(alpha=.2)
    writer.save(fig,'recovery',dict(records=means,independent_surveys=1,time_unit='sample_index',spatial_unit='trace_interval'))
    writer.gallery('F3 frozen transfer: descriptive results, no imaging or physical calibration claim')
    summary=['# F3 frozen external transfer','',
      f'Completed {len(records)} cases, {len(search)} unique searches and {recorder.count} native calls.',
      'One additional poststack survey; three neighboring sections are dependent conditions, not three surveys.',
      'Candidate pool, thresholds, masks/seeds, H and counts frozen before waveform access; no external-score retuning.',
      'Coordinates/time are trace/sample indices. Original physical amplitude calibration and imaging quality are not inferred.','',
      '| m | method | selected SNR |','| --- | --- | --- |']
    summary += [f"| {r['m']} | {r['method']} | {r['mean_snr']:.4f} |" for r in means if r['recovery']=='selected']
    (out/'summary.md').write_text('\n'.join(summary)+'\n',encoding='utf8')
    print(json.dumps(dict(status='passed',cases=len(records),searches=len(search),native_calls=recorder.count,out=str(out))))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--exe',type=Path,default=executable());parser.add_argument('--workers',type=int,default=12)
    parser.add_argument('--dataset',type=Path,default=DEFAULT)
    parser.add_argument('--keep-intermediates',action='store_true',help='retain observed/masks/native inner-fold scratch files')
    args=parser.parse_args();run(args.out,args.exe,args.workers,args.dataset,args.keep_intermediates)
