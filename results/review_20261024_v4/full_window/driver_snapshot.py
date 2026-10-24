"""Prespecified full-record check on the same source, with nested validation and fixed test masks."""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path

import numpy as np

from convert_rraw_to_engine_csv import read_segy
from review_studies import CONFIGURATIONS,Recorder,ROOT,REAL,nested_case,read_gather,read_rows,sha,write_gather,write_rows


def run(output,executable,resume):
    output=Path(output).resolve();output.mkdir(parents=True,exist_ok=True)
    raw,offsets,cdps,metadata=read_segy(ROOT/"data/real_seismic/Data/RRAW.SGY")
    order=np.argsort(offsets,kind="stable");raw=raw[:,order];x=offsets[order].astype(float)
    old_t=np.arange(len(raw))*.008
    new_t=np.arange(2*len(raw)-1)*.004
    data=np.column_stack([np.interp(new_t,old_t,raw[:,j]) for j in range(raw.shape[1])])
    # Match the historical cropped input amplitude unit; solver scaling is known-only.
    data/=np.max(np.abs(data[:256]))
    crop,_=read_gather(REAL)
    prefix_error=float(np.max(np.abs(data[:256]-crop)))
    if prefix_error>1e-6:raise AssertionError("full record does not match the archived crop")
    path=output/"full_499x59.csv"
    write_gather(path,data,x,"same_RRAW_source_full_0_to_1.992s; dt_s=.004; no spatial interpolation")
    configuration=dict(study="full_record_window_diagnostic",independent_real_sources=1,
                       input_sha256=sha(path),source_sha256=metadata["sha256"],shape=list(data.shape),
                       prefix_max_error=prefix_error,outer_seeds=2,keeps=[20,30,40],configurations=CONFIGURATIONS,
                       no_window_selection_by_test=True)
    recorder=Recorder(output,executable,configuration,resume)
    def case(job):
        keep,index=job;seed=2026102400+index*179
        mask=np.zeros(len(x),dtype=bool);mask[np.random.default_rng(seed).choice(len(x),keep,replace=False)]=True
        name=f"full_keep{keep}_seed{seed}"
        row,_,_=nested_case(recorder,name,path,data,x,mask,seed+1,CONFIGURATIONS)
        print("Full-record case complete:",name,flush=True)
        return row
    with ThreadPoolExecutor(max_workers=2) as pool:
        rows=list(pool.map(case,[(keep,index) for keep in (20,30,40) for index in range(2)]))
    write_rows(output/"window_rows.csv",rows)
    recorder.state["completed"]=True;recorder.save()
    # Independently recalculate each reported SNR from the serialized predictions and raw-derived targets.
    checked=[]
    for row in rows:
        mask=np.loadtxt(output/row["outer_mask"],comments="#").astype(bool)
        serialized=np.loadtxt(path,delimiter=",",comments="#")
        prediction=np.loadtxt(output/row["selected_prediction"],delimiter=",")
        fixed=np.loadtxt(output/row["fixed_prediction"],delimiter=",")
        for values,column in [(prediction,"snr_selected"),(fixed,"snr_fixed")]:
            score=float(10*np.log10(np.square(serialized[:,~mask]).sum()/max(np.square(values[:,~mask]-serialized[:,~mask]).sum(),1e-30)))
            if abs(score-float(row[column]))>1e-6:raise AssertionError("full-window score mismatch")
        if np.max(np.abs(prediction[:,mask]-serialized[:,mask]))>1e-7:raise AssertionError("full-window consistency mismatch")
        checked.append(row["case"])
    result=dict(passed=True,cases=checked,configuration=configuration,
                limitation="Same source and same geometry splits. Different time windows have different target energies; SNR differences across windows are not method gains on the same target.")
    (output/"verification.json").write_text(json.dumps(result,indent=2),encoding="utf-8")
    print(json.dumps(dict(passed=True,full_window_cases=len(checked),prefix_max_error=prefix_error)),flush=True)


if __name__=="__main__":
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out",type=Path,default=ROOT/"results/review_20261024_v4/full_window")
    parser.add_argument("--exe",type=Path,default=ROOT/"x64/Release/IrrGeometryDesign.exe")
    parser.add_argument("--resume",action="store_true")
    args=parser.parse_args();run(args.out,args.exe,args.resume)
