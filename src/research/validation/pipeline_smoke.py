"""Exercise preprocessing, optimization, nested reconstruction and scoring end to end."""
import json
from pathlib import Path

import numpy as np

from research.common.paths import executable, WORKSPACE
from research.common.provenance import add_real_data_arguments, real_source_provenance, require_inputs
from research.experiments.audit_joint_studies import (Recorder,synthetic,real_raw,
    initial_mask,native_queries,nested)
from research.validation.audit_numerics import query
from research.experiments.review_studies import snr
from research.common.tabular import write_rows


def run(out, exe=None, include_real=False, source_confirmation=None, allow_unconfirmed_real=False):
    program=Path(exe) if exe is not None else executable()
    require_inputs([program])
    provenance=dict(status='not_used',field_claims_eligible=False,independent_real_sources=0)
    if include_real:
        provenance=real_source_provenance(WORKSPACE/'data/real_seismic/Data/RRAW.SGY',
                                         source_confirmation,allow_unconfirmed_real)
    recorder=Recorder(out,program,dict(purpose='software smoke, not a thesis experiment',budget=64,seed=42,
                                     synthetic_only=not include_real,real_source_provenance=provenance))
    recorder.state['protocol']='layout_smoke_v1';recorder.save()
    models=synthetic(recorder.out)
    cases=[('analytic',models[0][0],models[0][1],38)]
    if include_real:
        real,xreal,_,_=real_raw();cases.append(('real_'+provenance['status'],real,xreal,20))
    records=[]
    for name,data,x,m in cases:
        h=np.zeros(len(x),bool);h[4::8]=True;eligible=~h
        mask=initial_mask(len(x),m,42,eligible)
        rows=native_queries(recorder,name,[query(name,mask,x,method='Adaptive',budget=64,eligible=eligible)])
        selected=np.array(list(rows[0]['mask']),int).astype(bool)
        assert selected.sum()==m and not np.any(selected&h)
        fixed,prediction,linear,choice=nested(recorder,name,data,x,selected,42)
        records.append(dict(case=name,known=m,evaluation_points=int(h.sum()),
                            fixed_snr=snr(data,fixed,h),selected_snr=snr(data,prediction,h),
                            linear_snr=snr(data,linear,h),candidate=choice['selected']['candidate']))
        assert np.isfinite([records[-1][k] for k in ['fixed_snr','selected_snr','linear_snr']]).all()
    write_rows(recorder.out/'pipeline.csv',records);recorder.finish()
    report=dict(passed=True,protocol='layout_smoke_v1',records=records,real_source_provenance=provenance,
                scope='End-to-end software checks; these do not establish scientific superiority or independent field replication.')
    (recorder.out/'verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report))


if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--exe',type=Path,default=executable())
    parser.add_argument('--include-real',action='store_true',help='also test external real data with an explicit source declaration')
    add_real_data_arguments(parser);args=parser.parse_args()
    run(args.out.resolve(),args.exe,args.include_real,args.source_confirmation,args.allow_unconfirmed_real)
