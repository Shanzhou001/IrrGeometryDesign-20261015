"""Prespecified full-record check on the same source, with nested validation and fixed test masks."""  # add by wushanzhou in 2026/10/24
from research.common.paths import ROOT, WORKSPACE
import argparse  # add by wushanzhou in 2026/10/24
from concurrent.futures import ThreadPoolExecutor  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24

import numpy as np  # add by wushanzhou in 2026/10/24

from research.data.convert_rraw_to_engine_csv import read_segy  # add by wushanzhou in 2026/10/24
from research.experiments.review_studies import CONFIGURATIONS,Recorder,ROOT,REAL,nested_case,read_gather,read_rows,sha,write_gather,write_rows  # add by wushanzhou in 2026/10/24


def run(output,executable,resume):  # add by wushanzhou in 2026/10/24
    output=Path(output).resolve();output.mkdir(parents=True,exist_ok=True)  # add by wushanzhou in 2026/10/24
    driver_sha=sha(__file__);(output/'driver_snapshot.py').write_bytes(Path(__file__).read_bytes()) # add by wushanzhou in 2026/10/24
    raw,offsets,cdps,metadata=read_segy(WORKSPACE / "data/real_seismic/Data/RRAW.SGY")  # add by wushanzhou in 2026/10/24
    order=np.argsort(offsets,kind="stable");raw=raw[:,order];x=offsets[order].astype(float)  # add by wushanzhou in 2026/10/24
    old_t=np.arange(len(raw))*.008  # add by wushanzhou in 2026/10/24
    new_t=np.arange(2*len(raw)-1)*.004  # add by wushanzhou in 2026/10/24
    data=np.column_stack([np.interp(new_t,old_t,raw[:,j]) for j in range(raw.shape[1])])  # add by wushanzhou in 2026/10/24
    # Match the historical cropped input amplitude unit; solver scaling is known-only.  # add by wushanzhou in 2026/10/24
    data/=np.max(np.abs(data[:256]))  # add by wushanzhou in 2026/10/24
    crop,_=read_gather(REAL)  # add by wushanzhou in 2026/10/24
    prefix_error=float(np.max(np.abs(data[:256]-crop)))  # add by wushanzhou in 2026/10/24
    if prefix_error>1e-6:raise AssertionError("full record does not match the archived crop")  # add by wushanzhou in 2026/10/24
    path=output/"full_499x59.csv"  # add by wushanzhou in 2026/10/24
    write_gather(path,data,x,"same_RRAW_source_full_0_to_1.992s; dt_s=.004; no spatial interpolation")  # add by wushanzhou in 2026/10/24
    configuration=dict(study="full_record_window_diagnostic",independent_real_sources=1,driver_sha256=driver_sha, # add by wushanzhou in 2026/10/24
                       input_sha256=sha(path),source_sha256=metadata["sha256"],shape=list(data.shape),  # add by wushanzhou in 2026/10/24
                       prefix_max_error=prefix_error,outer_seeds=2,keeps=[20,30,40],configurations=CONFIGURATIONS,  # add by wushanzhou in 2026/10/24
                       no_window_selection_by_test=True)  # add by wushanzhou in 2026/10/24
    recorder=Recorder(output,executable,configuration,resume)  # add by wushanzhou in 2026/10/24
    def case(job):  # add by wushanzhou in 2026/10/24
        keep,index=job;seed=2026102400+index*179  # add by wushanzhou in 2026/10/24
        mask=np.zeros(len(x),dtype=bool);mask[np.random.default_rng(seed).choice(len(x),keep,replace=False)]=True  # add by wushanzhou in 2026/10/24
        name=f"full_keep{keep}_seed{seed}"  # add by wushanzhou in 2026/10/24
        row,_,_=nested_case(recorder,name,path,data,x,mask,seed+1,CONFIGURATIONS)  # add by wushanzhou in 2026/10/24
        print("Full-record case complete:",name,flush=True)  # add by wushanzhou in 2026/10/24
        return row  # add by wushanzhou in 2026/10/24
    with ThreadPoolExecutor(max_workers=2) as pool:  # add by wushanzhou in 2026/10/24
        rows=list(pool.map(case,[(keep,index) for keep in (20,30,40) for index in range(2)]))  # add by wushanzhou in 2026/10/24
    write_rows(output/"window_rows.csv",rows)  # add by wushanzhou in 2026/10/24
    recorder.state["completed"]=True;recorder.save()  # add by wushanzhou in 2026/10/24
    # Independently recalculate each reported SNR from the serialized predictions and raw-derived targets.  # add by wushanzhou in 2026/10/24
    checked=[]  # add by wushanzhou in 2026/10/24
    for row in rows:  # add by wushanzhou in 2026/10/24
        mask=np.loadtxt(output/row["outer_mask"],comments="#").astype(bool)  # add by wushanzhou in 2026/10/24
        serialized=np.loadtxt(path,delimiter=",",comments="#")  # add by wushanzhou in 2026/10/24
        prediction=np.loadtxt(output/row["selected_prediction"],delimiter=",")  # add by wushanzhou in 2026/10/24
        fixed=np.loadtxt(output/row["fixed_prediction"],delimiter=",")  # add by wushanzhou in 2026/10/24
        for values,column in [(prediction,"snr_selected"),(fixed,"snr_fixed")]:  # add by wushanzhou in 2026/10/24
            score=float(10*np.log10(np.square(serialized[:,~mask]).sum()/max(np.square(values[:,~mask]-serialized[:,~mask]).sum(),1e-30)))  # add by wushanzhou in 2026/10/24
            if abs(score-float(row[column]))>1e-6:raise AssertionError("full-window score mismatch")  # add by wushanzhou in 2026/10/24
        if np.max(np.abs(prediction[:,mask]-serialized[:,mask]))>1e-7:raise AssertionError("full-window consistency mismatch")  # add by wushanzhou in 2026/10/24
        checked.append(row["case"])  # add by wushanzhou in 2026/10/24
    result=dict(passed=True,cases=checked,configuration=configuration,  # add by wushanzhou in 2026/10/24
                limitation="Same source and same geometry splits. Different time windows have different target energies; SNR differences across windows are not method gains on the same target.")  # add by wushanzhou in 2026/10/24
    (output/"verification.json").write_text(json.dumps(result,indent=2),encoding="utf-8")  # add by wushanzhou in 2026/10/24
    print(json.dumps(dict(passed=True,full_window_cases=len(checked),prefix_max_error=prefix_error)),flush=True)  # add by wushanzhou in 2026/10/24


if __name__=="__main__":  # add by wushanzhou in 2026/10/24
    parser=argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/24
    parser.add_argument("--out",type=Path,default=WORKSPACE / "results/review_20261024_v4/full_window_final") # add by wushanzhou in 2026/10/24
    parser.add_argument("--exe",type=Path,default=WORKSPACE / "build/bin/Release/IrrGeometryDesign.exe")  # add by wushanzhou in 2026/10/24
    parser.add_argument("--resume",action="store_true")  # add by wushanzhou in 2026/10/24
    args=parser.parse_args();run(args.out,args.exe,args.resume)  # add by wushanzhou in 2026/10/24
