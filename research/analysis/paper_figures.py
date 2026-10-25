"""English manuscript figures, read only from the frozen v4/v5 result tables."""
import argparse
import json
from pathlib import Path

import numpy as np

from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import ROOT
from research.common.tabular import rows

FIGURE_IDS = ('search', 'budget', 'recovery', 'real_windows')


def render():
    import matplotlib.pyplot as plt
    from matplotlib import rc_context
    v5 = ROOT / 'results/audit_20261024_v5'
    table = v5 / 'analysis_thesis'
    inputs = [ROOT / 'figures/research/v5/01_v4_search.data.json',
              table / 'search_pairs.csv', table / 'common_evaluation_pairs.csv',
              table / 'joint_means.csv', v5 / 'real/real.csv']
    sources = {p.relative_to(ROOT).as_posix(): digest(p) for p in inputs}
    writer = FigureWriter('paper')
    styles = {'font.family': 'serif', 'font.serif': ['Times New Roman', 'DejaVu Serif'],
              'font.size': 9, 'axes.labelsize': 9, 'axes.titlesize': 9,
              'legend.fontsize': 7.5, 'xtick.labelsize': 8, 'ytick.labelsize': 8,
              'axes.spines.top': False, 'axes.spines.right': False}
    colors = ['#4c78a8', '#f58518', '#54a24b', '#b279a2', '#e45756']
    methods = ['Jitter', 'SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive']
    labels = ['Jitter', 'SA-only', 'Legacy hybrid', 'SA-budget', 'Adaptive']
    def save(fig, name, data):
        writer.save(fig, name, {'sources_sha256': sources, 'records': data}, dpi=360)
    with rc_context(styles):
        data = json.loads(inputs[0].read_text(encoding='utf-8'))
        fig, axes = plt.subplots(1, 2, figsize=(7.1, 2.3), layout='constrained')
        for ax, domain, ms in zip(axes, ['uniform', 'physical'], [[38,64,90], [20,30,40]]):
            for method, label, color, shift in zip(methods[1:], labels[1:], colors[1:], [-.12,-.04,.04,.12]):
                rr = sorted([r for r in data if r['domain']==domain and r['budget']==600 and r['method']==method], key=lambda r:r['rate'])
                y = np.array([r['mean'] for r in rr]); lo = np.array([r['ci_low'] for r in rr]); hi = np.array([r['ci_high'] for r in rr])
                ax.errorbar(np.arange(3)+shift, y, yerr=[y-lo,hi-y], fmt='o', ms=3, capsize=2, color=color, label=label)
            ax.set(xticks=range(3), xticklabels=ms, xlabel='Retained traces, m', ylabel='Fourier coherence', title=domain.capitalize()+' domain; B = 600')
            ax.grid(axis='y',alpha=.2); ax.set_ylim(bottom=0)
        axes[0].legend(ncol=2, loc='upper right')
        save(fig, 'search', data)

        final = rows(inputs[1]); common = rows(inputs[2])
        fig, axes = plt.subplots(1,2,figsize=(7.1,2.3),layout='constrained')
        for ax, comparison in zip(axes, ['Adaptive-Hybrid','Adaptive-NoRecycle']):
            for rr, label, color, shift in [(final,'Final evaluation',colors[4],-.08),(common,'Common evaluation count',colors[0],.08)]:
                rr = sorted([r for r in rr if r['comparison']==comparison],key=lambda r:(r['domain']!='uniform',int(r['rate'])))
                y = np.array([float(r['difference']) for r in rr]); lo = np.array([float(r['ci_low']) for r in rr]); hi = np.array([float(r['ci_high']) for r in rr])
                ax.errorbar(np.arange(6)+shift,y,yerr=[y-lo,hi-y],fmt='o',ms=3,capsize=2,label=label,color=color)
            ax.axhline(0,color='0.4',lw=.7); ax.grid(axis='y',alpha=.2)
            ax.set(xticks=range(6),xticklabels=['U38','U64','U90','P20','P30','P40'],ylabel='Paired coherence difference',title=comparison.replace('-', ' minus '))
        axes[0].legend(loc='lower left'); save(fig,'budget',{'final':final,'common_count':common})

        data = rows(inputs[3]); fig,axes=plt.subplots(1,2,figsize=(7.1,2.5),layout='constrained')
        for ax, recovery in zip(axes,['fixed','selected']):
            for i,(method,label,color) in enumerate(zip(methods,labels,colors)):
                rr=sorted([r for r in data if r['method']==method and r['recovery']==recovery],key=lambda r:int(r['m']))
                y=np.array([float(r['mean_snr']) for r in rr]);lo=np.array([float(r['ci_low']) for r in rr]);hi=np.array([float(r['ci_high']) for r in rr])
                ax.errorbar(np.arange(3)+(i-2)*.055,y,yerr=[y-lo,hi-y],fmt='o-',ms=2.5,lw=.8,capsize=1.8,label=label,color=color)
            ax.set(xticks=range(3),xticklabels=[38,64,90],xlabel='Retained traces, m (N = 128)',ylabel='SNR on common H (dB)',title='Fixed curvelet' if recovery=='fixed' else 'Inner-selected recovery',ylim=(0,18))
            ax.grid(axis='y',alpha=.2)
        axes[0].legend(ncol=2,loc='lower right');save(fig,'recovery',data)

        data=rows(inputs[4]);fig,axes=plt.subplots(1,2,figsize=(7.1,2.4),layout='constrained',sharey=True)
        windows=['native_full','native_crop','upsampled_full','upsampled_crop']
        for ax,key,title in zip(axes,['fixed_snr','selected_snr'],['Fixed curvelet','Inner-selected recovery']):
            for i,(window,color) in enumerate(zip(windows,colors)):
                group=[[float(r[key]) for r in data if r['window']==window and int(r['m'])==m] for m in [20,30,40]]
                y=np.mean(group,axis=1);lo=np.min(group,axis=1);hi=np.max(group,axis=1)
                ax.errorbar(np.arange(3)+(i-1.5)*.07,y,yerr=[y-lo,hi-y],fmt='o-',ms=3,lw=.8,capsize=2,color=color,label=window.replace('_',' '))
            ax.axhline(0,color='0.4',lw=.7);ax.grid(axis='y',alpha=.2)
            ax.set(xticks=range(3),xticklabels=[20,30,40],xlabel='Retained traces, m (N = 59)',ylabel='Hidden-trace SNR (dB)',title=title)
        axes[0].legend(ncol=2,loc='lower right',fontsize=6.8)
        save(fig,'real_windows',data)
    writer.gallery('English manuscript figures: frozen v4/v5 data')
    return list(FIGURE_IDS)


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.parse_args()
    render()
