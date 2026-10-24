"""Post hoc diagnostics: no values from this module feed parameter or mask selection."""  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24

import matplotlib.pyplot as plt  # add by wushanzhou in 2026/10/24
import numpy as np  # add by wushanzhou in 2026/10/24

from convert_rraw_to_engine_csv import read_segy  # add by wushanzhou in 2026/10/24
from review_studies import read_gather,read_mask,read_rows,sha,write_rows  # add by wushanzhou in 2026/10/24


def diagnose_real(output):  # add by wushanzhou in 2026/10/24
    source=Path(__file__).resolve().parents[1]/"data/real_seismic/Data/RRAW.SGY"  # add by wushanzhou in 2026/10/24
    raw,offsets,cdps,metadata=read_segy(source)  # add by wushanzhou in 2026/10/24
    payload=source.read_bytes()  # add by wushanzhou in 2026/10/24
    order=np.argsort(offsets,kind='stable');raw_ordered=raw[:,order];ordered_cdps=cdps[order]  # add by wushanzhou in 2026/10/24
    real,x=read_gather(source.parents[1]/"real_gather_59x256.csv")  # add by wushanzhou in 2026/10/24
    profiles={}  # add by wushanzhou in 2026/10/24
    for profile in ("IEEE_LE","IEEE_BE","IBM_LE","IBM_BE"):  # add by wushanzhou in 2026/10/24
        values=[]  # add by wushanzhou in 2026/10/24
        for j in range(metadata["ntraces"]):  # add by wushanzhou in 2026/10/24
            start=3600+j*(240+metadata["ns"]*4)+240  # add by wushanzhou in 2026/10/24
            block=payload[start:start+metadata["ns"]*4]  # add by wushanzhou in 2026/10/24
            if profile.startswith("IEEE"):  # add by wushanzhou in 2026/10/24
                trace=np.frombuffer(block,dtype="<f4" if profile.endswith("LE") else ">f4").astype(float)  # add by wushanzhou in 2026/10/24
            else:  # add by wushanzhou in 2026/10/24
                words=np.frombuffer(block,dtype="<u4" if profile.endswith("LE") else ">u4")  # add by wushanzhou in 2026/10/24
                exponent=((words>>24)&127).astype(int)-64  # add by wushanzhou in 2026/10/24
                trace=np.where(words>>31,-1.,1.)*(words&0x00ffffff)/float(2**24)*16.**exponent  # add by wushanzhou in 2026/10/24
            values.extend(trace)  # add by wushanzhou in 2026/10/24
        values=np.asarray(values);finite=values[np.isfinite(values)]  # add by wushanzhou in 2026/10/24
        profiles[profile]=dict(finite_fraction=float(np.isfinite(values).mean()),  # add by wushanzhou in 2026/10/24
                               absolute_p50=float(np.quantile(np.abs(finite),.5)),absolute_p99=float(np.quantile(np.abs(finite),.99)),  # add by wushanzhou in 2026/10/24
                               absolute_max=float(np.abs(finite).max()))  # add by wushanzhou in 2026/10/24
        matrix=values.reshape(metadata['ntraces'],metadata['ns']).T  # add by wushanzhou in 2026/10/24
        if np.isfinite(matrix).all():  # add by wushanzhou in 2026/10/24
            profiles[profile]['normalized_temporal_difference_rms']=float(np.sqrt(np.mean(np.diff(matrix,axis=0)**2))/max(np.sqrt(np.mean(matrix**2)),1e-30))  # add by wushanzhou in 2026/10/24
    words=np.concatenate([np.frombuffer(payload,dtype='<u4',count=metadata['ns'],offset=3600+j*(240+4*metadata['ns'])+240) for j in range(metadata['ntraces'])])  # add by wushanzhou in 2026/10/24
    nonzero=(words&0x7fffffff)!=0  # add by wushanzhou in 2026/10/24
    ibm_unnormalized=float(((words&0x00f00000)==0)[nonzero].mean())  # add by wushanzhou in 2026/10/24
    known_diagnostics=[]  # add by wushanzhou in 2026/10/24
    audit=json.loads((output/"data_diagnostics.json").read_text())  # add by wushanzhou in 2026/10/24
    for group in audit["groups"]:  # add by wushanzhou in 2026/10/24
        data,positions=read_gather(output/group["input"])  # add by wushanzhou in 2026/10/24
        training=np.array(group["diagnostic_training"])  # add by wushanzhou in 2026/10/24
        spectrum=np.abs(np.fft.rfft(data[:,training],axis=0))**2  # add by wushanzhou in 2026/10/24
        frequency=np.fft.rfftfreq(len(data),.004)  # add by wushanzhou in 2026/10/24
        peaks=frequency[np.argmax(spectrum[1:],axis=0)+1]  # add by wushanzhou in 2026/10/24
        known_diagnostics.append(dict(name=group["name"],training_traces=len(training),  # add by wushanzhou in 2026/10/24
                                      median_dominant_frequency_hz=float(np.median(peaks)),  # add by wushanzhou in 2026/10/24
                                      energy_above_raw_nyquist_fraction=float(spectrum[frequency>62.5].sum()/max(spectrum.sum(),1e-30)),  # add by wushanzhou in 2026/10/24
                                      zero_lag_mean=group["zero_lag_mean"],bounded_lag_mean=group["bounded_lag_mean"],  # add by wushanzhou in 2026/10/24
                                      median_absolute_lag_s=group["median_absolute_lag_s"]))  # add by wushanzhou in 2026/10/24
        source_subset=np.ones(len(x),dtype=bool) if group['name']=='mixed' else ordered_cdps==int(group['name'].split('_')[1])  # add by wushanzhou in 2026/10/24
        full_known=raw_ordered[:,source_subset][:,training]  # add by wushanzhou in 2026/10/24
        known_diagnostics[-1]['full_record_zero_lag_mean']=float(np.diag(np.corrcoef(full_known.T),1).mean())  # add by wushanzhou in 2026/10/24
    errors=[]  # add by wushanzhou in 2026/10/24
    for row in read_rows(output/"real_nested.csv"):  # add by wushanzhou in 2026/10/24
        mask=read_mask(output/row["outer_mask"])  # add by wushanzhou in 2026/10/24
        target=real[:,~mask]  # add by wushanzhou in 2026/10/24
        prediction=np.loadtxt(output/row["fixed_prediction"],delimiter=",")[:,~mask]  # add by wushanzhou in 2026/10/24
        energy=float(np.sum(target**2));ratio=float(np.sum(prediction**2)/energy)  # add by wushanzhou in 2026/10/24
        cross=float(np.sum(prediction*target)/energy)  # add by wushanzhou in 2026/10/24
        hidden_positions=x[~mask];known_positions=x[mask]  # add by wushanzhou in 2026/10/24
        exterior=(hidden_positions<known_positions[0]) | (hidden_positions>known_positions[-1])  # add by wushanzhou in 2026/10/24
        squared=(prediction-target)**2 # add by wushanzhou in 2026/10/24
        selection=json.loads((output/'selection'/(row['case']+'.json')).read_text()) # add by wushanzhou in 2026/10/24
        legacy_gain=next(c['gain'] for c in selection['candidates'] if c['candidate']=='legacy') # add by wushanzhou in 2026/10/24
        if row['selected']=='zero':raw_selected=np.zeros_like(target) # add by wushanzhou in 2026/10/24
        elif row['selected']=='linear':raw_selected=np.array([np.interp(x,x[mask],v[mask]) for v in real])[:,~mask] # add by wushanzhou in 2026/10/24
        else:raw_selected=np.loadtxt(output/'reconstruction'/(row['case']+'_outer_selected')/'prediction.csv',delimiter=',')[:,~mask] # add by wushanzhou in 2026/10/24
        gain_only_snr=float(10*np.log10(energy/max(np.square(legacy_gain*prediction-target).sum(),1e-30))) # add by wushanzhou in 2026/10/24
        selected_raw_snr=float(10*np.log10(energy/max(np.square(raw_selected-target).sum(),1e-30))) # add by wushanzhou in 2026/10/24
        corr=float(np.corrcoef(prediction.ravel(),target.ravel())[0,1]) if np.std(prediction)>0 else None  # add by wushanzhou in 2026/10/24
        trace=read_rows((output/row['fixed_prediction']).with_name('convergence.csv')) # add by wushanzhou in 2026/10/24
        steps=np.array([float(r['relative_step']) for r in trace]);assert len(steps)==40 and np.isfinite(steps).all() and (steps>=0).all() # add by wushanzhou in 2026/10/24
        errors.append(dict(case=row["case"],keep=int(row["keep"]),prediction_energy_ratio=ratio,  # add by wushanzhou in 2026/10/24
                           normalized_cross_term=cross,error_ratio=1+ratio-2*cross,  # add by wushanzhou in 2026/10/24
                           fixed_snr_from_energy=float(-10*np.log10(max(1e-30,1+ratio-2*cross))),  # add by wushanzhou in 2026/10/24
                           fixed_gain_only_snr=gain_only_snr,selected_before_gain_snr=selected_raw_snr,legacy_validation_gain=float(legacy_gain), # add by wushanzhou in 2026/10/24
                           flattened_prediction_correlation=corr,exterior_traces=int(exterior.sum()), # add by wushanzhou in 2026/10/24
                           fixed_relative_step_first=float(steps[0]),fixed_relative_step_last=float(steps[-1]), # add by wushanzhou in 2026/10/24
                           exterior_error_fraction=float(squared[:,exterior].sum()/max(squared.sum(),1e-30))))  # add by wushanzhou in 2026/10/24
        assert abs(errors[-1]["fixed_snr_from_energy"]-float(row["snr_fixed"]))<1e-7  # add by wushanzhou in 2026/10/24
    write_rows(output/"real_error_decomposition.csv",errors)  # add by wushanzhou in 2026/10/24
    diagnosis=dict(source_sha256=sha(source),trace_dt_us=metadata["dt_trace_field"],binary_dt_us=metadata["dt_raw_field"],  # add by wushanzhou in 2026/10/24
                   declared_format=metadata["declared_format"],payload_profile_used=metadata["payload_profile"],  # add by wushanzhou in 2026/10/24
                   encoding_descriptive_profiles=profiles,source_window_seconds=[0,(metadata["ns"]-1)*.008],  # add by wushanzhou in 2026/10/24
                   ibm_le_unnormalized_nonzero_word_fraction=ibm_unnormalized,  # add by wushanzhou in 2026/10/24
                   evaluated_window_seconds=[0,1.020],training_continuity_and_spectrum=known_diagnostics,  # add by wushanzhou in 2026/10/24
                   error_decomposition=errors,  # add by wushanzhou in 2026/10/24
                   limitation="Amplitude plausibility alone cannot prove a disputed encoding. The preserved IEEE-LE override was not selected by test SNR. Lag maxima and energy decompositions are descriptive, not causal identification or parameter selection.")  # add by wushanzhou in 2026/10/24
    window_path=output.parent/'full_window_final/window_rows.csv' # add by wushanzhou in 2026/10/24
    if not window_path.exists():window_path=output.parent/'full_window/window_rows.csv' # add by wushanzhou in 2026/10/24
    if window_path.exists():  # add by wushanzhou in 2026/10/24
        cropped={r['outer_mask_sha1']:r for r in read_rows(output/'real_nested.csv')}  # add by wushanzhou in 2026/10/24
        windows=[]  # add by wushanzhou in 2026/10/24
        for row in read_rows(window_path):  # add by wushanzhou in 2026/10/24
            match=cropped.get(row['outer_mask_sha1'])  # add by wushanzhou in 2026/10/24
            if match is not None:  # add by wushanzhou in 2026/10/24
                windows.append(dict(keep=int(row['keep']),inner_seed=int(row['seed']),mask_sha1=row['outer_mask_sha1'],  # add by wushanzhou in 2026/10/24
                                    crop_fixed=float(match['snr_fixed']),crop_selected=float(match['snr_selected']),  # add by wushanzhou in 2026/10/24
                                    full_fixed=float(row['snr_fixed']),full_selected=float(row['snr_selected'])))  # add by wushanzhou in 2026/10/24
        if windows:write_rows(output/'window_comparison.csv',windows)  # add by wushanzhou in 2026/10/24
        diagnosis['full_record_window_comparisons']=windows  # add by wushanzhou in 2026/10/24
    (output/"real_diagnosis.json").write_text(json.dumps(diagnosis,indent=2,allow_nan=False),encoding="utf-8")  # add by wushanzhou in 2026/10/24
    target_dir=output/"figures";target_dir.mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
    fig,axes=plt.subplots(1,2,figsize=(11,4))  # add by wushanzhou in 2026/10/24
    names=[r["name"] for r in known_diagnostics];positions=np.arange(len(names))  # add by wushanzhou in 2026/10/24
    axes[0].bar(positions-.18,[r["zero_lag_mean"] for r in known_diagnostics],.36,label="zero lag")  # add by wushanzhou in 2026/10/24
    axes[0].bar(positions+.18,[r["bounded_lag_mean"] for r in known_diagnostics],.36,label="best +/-48 ms")  # add by wushanzhou in 2026/10/24
    axes[0].set_xticks(positions,names,rotation=30)  # add by wushanzhou in 2026/10/24
    axes[0].set(ylabel="training adjacent correlation",title="Fixed training subsets; descriptive")  # add by wushanzhou in 2026/10/24
    axes[0].legend(fontsize=8)  # add by wushanzhou in 2026/10/24
    scatter=axes[1].scatter([r["prediction_energy_ratio"] for r in errors],[r["normalized_cross_term"] for r in errors], # add by wushanzhou in 2026/10/24
                    c=[r["keep"] for r in errors],cmap="viridis")  # add by wushanzhou in 2026/10/24
    axes[1].set(xlabel="prediction energy / test target energy",ylabel="prediction-target dot / target energy",  # add by wushanzhou in 2026/10/24
                title="Fixed POCS test-error decomposition")  # add by wushanzhou in 2026/10/24
    fig.colorbar(scatter,ax=axes[1],label='Retained traces',ticks=[20,30,40]) # add by wushanzhou in 2026/10/24
    fig.tight_layout()  # add by wushanzhou in 2026/10/24
    for extension in ("png","svg"):fig.savefig(target_dir/("06_real_diagnosis."+extension),dpi=180,bbox_inches="tight")  # add by wushanzhou in 2026/10/24
    plt.close(fig)  # add by wushanzhou in 2026/10/24
    return diagnosis  # add by wushanzhou in 2026/10/24
