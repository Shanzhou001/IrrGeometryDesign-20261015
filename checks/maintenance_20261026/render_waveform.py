"""A fixed illustrative case, chosen before the robustness batch completed."""
import json
from pathlib import Path
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'src'))
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.data.viking import load_windows


def main():
    root=ROOT/'results/viking_robust_20261026'
    state=json.loads((root/'manifest.json').read_text())
    if not state.get('completed') or state.get('status')!='passed':raise ValueError('Batch not complete')
    # Same middle shot/window, random H and middle density; no ranking of outer
    # scores is used to select the illustration.
    key='f453_w512_hr271828_m60_s94217_'
    methods=['Jitter','SA_Mu','Adaptive_Gap'];arrays={};hashes={}
    for method in methods:
        path=root/'predictions'/(key+method+'.npz')
        with np.load(path) as data:
            arrays[method]=data['selected'];hidden=data['hidden']
        hashes[path.relative_to(ROOT).as_posix()]=digest(path)
    windows,_=load_windows([512],shot_ids=[453]);shot=windows[512][453]
    target=shot.data[:,hidden];limit=np.quantile(abs(target),.99)
    fig,axes=plt.subplots(1,4,figsize=(12,4.2),sharey=True,layout='constrained')
    extent=[-.5,target.shape[1]-.5,shot.time_origin_s+len(target)*.004,shot.time_origin_s]
    image=None
    for ax,title,values in zip(axes,['Measured H']+methods,[target]+[arrays[m][:,hidden] for m in methods]):
        image=ax.imshow(values,aspect='auto',extent=extent,cmap='gray',vmin=-limit,vmax=limit)
        ax.set(title=title,xlabel='Common hidden-trace order')
    axes[0].set_ylabel('Time (s)');fig.colorbar(image,ax=list(axes),label='Recorded digital amplitude',shrink=.8)
    fig.suptitle('Fixed illustration: FFID 453, 2.048-3.072 s, random H, m=60, seed=94217',fontsize=10)
    writer=FigureWriter('research/viking_robust_waveforms_20261026')
    writer.save(fig,'hidden_traces',dict(case_prefix=key,prediction_sha256=hashes,
        hidden_indices=np.flatnonzero(hidden).tolist(),scope='Preset within-survey illustration; not a selected best case'),dpi=220)
    indices=np.flatnonzero(hidden);j=indices[len(indices)//2];time=shot.time_origin_s+np.arange(len(shot.data))*.004
    fig,axes=plt.subplots(1,2,figsize=(11,3.5),layout='constrained')
    axes[0].plot(time,shot.data[:,j],color='black',lw=1.2,label='Measured target')
    for method in methods:
        axes[0].plot(time,arrays[method][:,j],lw=.9,label=method)
        axes[1].plot(time,arrays[method][:,j]-shot.data[:,j],lw=.9,label=method)
    for ax in axes:ax.set_xlabel('Time (s)');ax.grid(alpha=.2);ax.legend(fontsize=7)
    axes[0].set(title=f'Preset middle H trace, index {j}',ylabel='Digital amplitude')
    axes[1].set(title='Prediction minus measured target',ylabel='Residual amplitude')
    writer.save(fig,'waveform',dict(trace_index=int(j),case_prefix=key,prediction_sha256=hashes),dpi=220)
    writer.gallery('Fixed illustrative hidden traces and waveform errors')
    print(writer.directory)


if __name__=='__main__':main()
