"""Post-evaluation waveform diagnostics; no metric here selects a recovery model."""
import argparse
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.tabular import rows, write_rows
from research.data.viking import load_windows


def quality(target,prediction):
    target=np.asarray(target,float);prediction=np.asarray(prediction,float)
    if target.ndim!=2 or target.shape!=prediction.shape or len(target)<8 or not np.isfinite(target).all() or not np.isfinite(prediction).all():
        raise ValueError('Finite time-by-trace arrays of equal shape are required')
    energy=float(np.sum(target**2));pred_energy=float(np.sum(prediction**2))
    if energy<=0:raise ValueError('A nonzero measured target is required')
    a=target-target.mean(axis=0);b=prediction-prediction.mean(axis=0)
    denominator=float(np.linalg.norm(a)*np.linalg.norm(b))
    correlation=float(np.sum(a*b)/denominator) if denominator>1e-30 else None
    window=np.hanning(len(a))[:,None];fa=np.fft.rfft(a*window,axis=0);fb=np.fft.rfft(b*window,axis=0)
    frequency=np.fft.rfftfreq(len(a));band=(frequency>=.02)&(frequency<=.24)
    power=abs(fa[band])**2;cross=fb[band]*fa[band].conj()
    threshold=max(float(np.max(abs(fb[band])))*1e-12,1e-30)
    valid=(abs(fb[band])>threshold)&(power>float(power.max())*1e-12)
    covered=float(np.sum(power[valid]));total=float(np.sum(power))
    phase=float(np.sum(power[valid]*abs(np.angle(cross[valid])))/covered*180/np.pi) if covered>0 else None
    return dict(nmse=float(np.sum((prediction-target)**2)/energy),rms_ratio=float(np.sqrt(pred_energy/energy)),
                centered_waveform_correlation=correlation,band_phase_mae_degrees=phase,
                phase_target_energy_coverage=covered/total if total>0 else None)


def summarize(records):
    groups={}
    for row in records:groups.setdefault((row['m'],row['method'],row['recovery']),[]).append(row)
    result=[]
    for (m,method,recovery),same in sorted(groups.items()):
        row=dict(m=m,method=method,recovery=recovery,cases=len(same),independent_surveys=1)
        for key in ('nmse','rms_ratio','centered_waveform_correlation','band_phase_mae_degrees','phase_target_energy_coverage'):
            values=[r[key] for r in same if r[key] is not None]
            row['mean_'+key]=float(np.mean(values)) if values else None
        result.append(row)
    return result


def run(root,out):
    root,out=Path(root).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    state=json.loads((root/'manifest.json').read_text())
    if not state.get('completed') or state.get('status')!='passed':raise ValueError('Completed passed batch required')
    if digest(root/'recovery.csv')!=state['output_tables_sha256']['recovery.csv']:
        raise ValueError('Frozen recovery table changed')
    plan=state['configuration']['plan'];windows,_=load_windows(plan['window_starts'],shot_ids=plan['ffids'])
    records=[];hashes={}
    for row in rows(root/'recovery.csv'):
        path=root/'predictions'/(row['case']+'.npz');hashes[path.relative_to(root).as_posix()]=digest(path)
        if hashes[path.relative_to(root).as_posix()]!=state['derived_artifacts_sha256'][path.relative_to(root).as_posix()]:
            raise ValueError('Frozen prediction changed: '+str(path))
        with np.load(path) as saved:
            target=windows[int(row['window_start'])][int(row['ffid'])].data[:,saved['hidden']]
            for kind in ('cold','fk','selected','linear'):
                records.append(dict(case=row['case'],m=row['m'],method=row['method'],seed=row['seed'],recovery=kind,
                                    **quality(target,saved[kind][:,saved['hidden']])))
    out.mkdir(parents=True);write_rows(out/'waveform_cases.csv',records);write_rows(out/'waveform_means.csv',summarize(records))
    report=dict(status='passed',scope='Post-hoc measured-waveform diagnostics; not physical amplitude calibration or imaging validation',
                prediction_arrays=len(records),experiment_manifest_sha256=digest(root/'manifest.json'),prediction_sha256=hashes,
                phase_band_cycles_per_sample=[.02,.24],phase_definition='Target-power-weighted absolute cross-spectrum phase, after temporal demeaning and Hann window; coverage reported for nonzero predicted bins',
                rms_definition='Predicted/target RMS on common hidden traces; ideal ratio 1, not absolute physical amplitude',
                independent_surveys=1,used_for_model_selection=False,script_sha256=digest(__file__))
    (out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(status='passed',prediction_arrays=len(records),out=str(out))));return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--root',type=Path,required=True);parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args();run(args.root,args.out)
