"""Exercise preprocessing, optimization, nested reconstruction and scoring end to end."""
import json
from pathlib import Path

import numpy as np

from research.common.paths import executable
from research.experiments.audit_joint_studies import (Recorder,synthetic,real_raw,
    initial_mask,native_queries,nested)
from research.validation.audit_numerics import query
from research.experiments.review_studies import snr
from research.common.tabular import write_rows


def run(out):
    recorder=Recorder(out,executable(),dict(purpose='layout smoke, not a new thesis experiment',budget=64,seed=42))
    recorder.state['protocol']='layout_smoke_v1';recorder.save()
    models=synthetic(recorder.out)
    real,xreal,_,_=real_raw()
    records=[]
    for name,data,x,m in [('analytic',models[0][0],models[0][1],38),('real_unconfirmed',real,xreal,20)]:
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
    report=dict(passed=True,protocol='layout_smoke_v1',records=records,
                scope='Two end-to-end software checks, not additional independent field datasets or confirmation of scientific superiority.')
    (recorder.out/'verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report))


if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,required=True)
    run(parser.parse_args().out.resolve())
