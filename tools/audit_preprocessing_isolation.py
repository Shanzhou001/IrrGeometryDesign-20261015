"""End-to-end intervention through the historical six-significant-digit CSV preprocessing."""
import argparse
import json
from pathlib import Path

import numpy as np

from audit_joint_studies import Recorder,real_raw,initial_mask,nested
from make_59x256 import convert
from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]


def run(out):
    recorder=Recorder(out,ROOT/'x64/Release/IrrGeometryDesign.exe',dict(check='historical CSV rounding intervention',driver_sha256=digest(__file__)))
    data,x,_,_=real_raw();mask=initial_mask(59,30,20261024);changed=data.copy();changed[:,~mask]=13*changed[:,~mask]+7
    predictions=[];choices=[];scales=[]
    for name,values in [('A',data),('B',changed)]:
        preprocessed,_=convert(values,x)
        rounded=np.array([[float(format(v,'.6g')) for v in row] for row in preprocessed])
        # Exact same crop-time maximum used by the legacy linear-resampling conversion.
        scale=float(max(abs(values[:128]).max(),abs(.5*(values[127]+values[128])).max()))
        np.savetxt(recorder.out/('rounded_'+name+'.csv'),rounded,delimiter=',',fmt='%.6g')
        _,prediction,_,choice=nested(recorder,'rounded_'+name,rounded,x,mask,20261024+909)
        predictions.append(prediction*scale);choices.append(choice['selected']);scales.append(scale)
    difference=abs(predictions[0]-predictions[1])
    result=dict(known_preprocess_scaling=[float(v) for v in scales],selection=choices,
                same_candidate=choices[0]['candidate']==choices[1]['candidate'],
                prediction_max_abs_difference=float(difference.max()),
                relative_to_raw_peak=float(difference.max()/abs(data).max()),
                strict_preprocessing_isolation=False,
                interpretation='Historical full-field normalization sees H; approximate numerical cancellation is not strict data isolation. v5 raw pipeline removes this dependency.')
    recorder.finish();(recorder.out/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True);run(p.parse_args().out)
