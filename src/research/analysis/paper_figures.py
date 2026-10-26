"""English manuscript figures, read only from the frozen v4/v5 result tables."""
import argparse
import json
from pathlib import Path

import numpy as np

from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import WORKSPACE
from research.common.tabular import rows

FIGURE_IDS = ('search', 'budget', 'recovery', 'real_windows')
SEARCH_METHODS = ('SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive')
COUNTS = {'uniform': (38, 64, 90), 'physical': (20, 30, 40)}
COMPARISONS = ('Adaptive-Hybrid', 'Adaptive-NoRecycle')
WINDOWS = ('native_full', 'native_crop', 'upsampled_full', 'upsampled_crop')


def source_status(v4, v5, real, additional=()):
    """Old archives without declarations remain sensitivity evidence."""
    records = [v4.get('fingerprint', {}).get('configuration', {}).get('real_source_provenance', {}),
               v5.get('configuration', {}).get('real_source_provenance', {}), real, *additional]
    statuses = [r.get('status') for r in records]
    confirmed = [r for r in records if r.get('status') == 'declared_confirmed']
    if all(status == 'not_used' for status in statuses):
        status = 'not_used'
    elif (all(status in ('declared_confirmed', 'not_used') for status in statuses) and
          confirmed and all(r.get('field_claims_eligible') is True and r.get('source_sha256') for r in confirmed) and
          len({r['source_sha256'] for r in confirmed}) == 1):
        status = 'declared_confirmed'
    else:
        status = 'unconfirmed_sensitivity'
    return dict(status=status, field_claims_eligible=status == 'declared_confirmed', records=records)


def number(record, key, path):
    try:
        value = float(record[key])
    except (KeyError, TypeError, ValueError) as error:
        raise ValueError(f'{path}: missing or invalid {key}') from error
    if not np.isfinite(value):
        raise ValueError(f'{path}: {key} must be finite')
    return value


def interval(record, mean_key, path):
    mean, low, high = (number(record, key, path) for key in (mean_key, 'ci_low', 'ci_high'))
    if not low <= mean <= high:
        raise ValueError(f'{path}: confidence interval does not contain {mean_key}')


def load_inputs(v4, v5, budget):
    """Read and validate every panel before creating an output directory or image."""
    table = v5 / 'analysis_thesis'
    inputs = {f'v4/search_{domain}_{budget}/search.csv': v4 / f'search_{domain}_{budget}/search.csv'
              for domain in COUNTS}
    inputs.update({f'v5/analysis_thesis/{name}.csv': table / (name + '.csv')
                   for name in ('search_pairs', 'common_evaluation_pairs', 'joint_means')})
    inputs['v5/real/real.csv'] = v5 / 'real/real.csv'
    inputs['v4/run_manifest.json'] = v4 / 'run_manifest.json'
    # The current driver writes one manifest per stage. Older archives used
    # a parent manifest; accept that layout only when stage manifests are absent.
    stage_manifests = {'v5/search/manifest.json': v5/'search/manifest.json',
                       'v5/joint/manifest.json': v5/'joint/manifest.json'}
    manifests = stage_manifests if (any(p.is_file() for p in stage_manifests.values()) or
                                    not (v5/'manifest.json').is_file()) else {'v5/manifest.json': v5/'manifest.json'}
    inputs.update(manifests)
    inputs['v5/real/provenance.json'] = v5 / 'real/provenance.json'
    missing = [str(path) for path in inputs.values() if not path.is_file()]
    if missing:
        raise FileNotFoundError('Paper figures require the original result tables. Missing inputs:\n' +
                                '\n'.join(missing))
    raw = {key: rows(path) for key, path in inputs.items() if path.suffix == '.csv'}
    stages = [json.loads(path.read_text(encoding='utf-8-sig')) for path in manifests.values()]
    provenance = source_status(json.loads(inputs['v4/run_manifest.json'].read_text(encoding='utf-8-sig')),
                               stages[0], json.loads(inputs['v5/real/provenance.json'].read_text(encoding='utf-8-sig')),
                               [stage.get('configuration', {}).get('real_source_provenance', {}) for stage in stages[1:]])
    from research.analysis.summarize_audit_v5 import confidence
    search = []
    for domain, counts in COUNTS.items():
        key = f'v4/search_{domain}_{budget}/search.csv'; path = inputs[key]
        for rate, count in enumerate(counts):
            seeds = None
            for method in SEARCH_METHODS:
                group = [r for r in raw[key] if r.get('method') == method and r.get('rate_index') == str(rate)]
                identities = [r.get('seed') for r in group]
                if len(group) < 2 or None in identities or len(set(identities)) != len(identities):
                    raise ValueError(f'{path}: {method}, rate {rate} needs distinct repeated seeds')
                if seeds is not None and seeds != set(identities):
                    raise ValueError(f'{path}: search methods must share the same seeds at rate {rate}')
                seeds = set(identities)
                group.sort(key=lambda r: r['seed'])
                values = []
                for record in group:
                    if (record.get('domain') != domain or number(record, 'm', path) != count or
                            number(record, 'budget_cap', path) != budget):
                        raise ValueError(f'{path}: inconsistent domain, retained count or evaluation budget')
                    value = number(record, 'mu', path)
                    if not 0 <= value <= 1:
                        raise ValueError(f'{path}: Fourier coherence must be in [0, 1]')
                    values.append(value)
                low, high = confidence(values)
                search.append(dict(domain=domain, budget=budget, method=method, rate=rate,
                                   mean=float(np.mean(values)), ci_low=low, ci_high=high, n=len(values)))

    paired = []
    expected = {(domain, str(rate), comparison) for domain in COUNTS for rate in range(3)
                for comparison in COMPARISONS}
    for name in ('search_pairs', 'common_evaluation_pairs'):
        key = f'v5/analysis_thesis/{name}.csv'; path = inputs[key]
        selected = [r for r in raw[key] if r.get('comparison') in COMPARISONS]
        identities = [(r.get('domain'), r.get('rate'), r.get('comparison')) for r in selected]
        if len(identities) != len(expected) or set(identities) != expected:
            raise ValueError(f'{path}: incomplete or duplicate domain/rate/comparison groups')
        for record in selected:
            interval(record, 'difference', path)
        paired.append(selected)

    key = 'v5/analysis_thesis/joint_means.csv'; path = inputs[key]; joint = raw[key]
    expected = {(str(count), recovery, method) for count in COUNTS['uniform']
                for recovery in ('fixed', 'selected') for method in ('Jitter', *SEARCH_METHODS)}
    identities = [(r.get('m'), r.get('recovery'), r.get('method')) for r in joint]
    if len(identities) != len(expected) or set(identities) != expected:
        raise ValueError(f'{path}: incomplete or duplicate common-H recovery groups')
    for record in joint:
        interval(record, 'mean_snr', path)
        if record.get('unit') != 'clean structural template' or number(record, 'analysis_units', path) < 2:
            raise ValueError(f'{path}: paper recovery intervals require independent clean structural templates')

    key = 'v5/real/real.csv'; path = inputs[key]; real = raw[key]
    expected = {(window, str(count)) for window in WINDOWS for count in COUNTS['physical']}
    if {(r.get('window'), r.get('m')) for r in real} != expected:
        raise ValueError(f'{path}: incomplete or unexpected real-window groups')
    for window, count in expected:
        selected = [r for r in real if (r.get('window'), r.get('m')) == (window, count)]
        seeds = [r.get('seed') for r in selected]
        if len(seeds) < 2 or None in seeds or len(set(seeds)) != len(seeds):
            raise ValueError(f'{path}: {window}, m={count} needs distinct repeated splits')
    for record in real:
        for field in ('fixed_snr', 'selected_snr'):
            number(record, field, path)
    sources = {key: digest(path) for key, path in inputs.items()}
    return search, paired[0], paired[1], joint, real, sources, provenance


def render(v4=None, v5=None, group='paper', budget=600):
    import matplotlib.pyplot as plt
    from matplotlib import rc_context
    if budget < 1:
        raise ValueError('evaluation budget must be positive')
    v4 = Path(v4 or WORKSPACE / 'results/review_20261024_v4/final').resolve()
    v5 = Path(v5 or WORKSPACE / 'results/audit_20261024_v5').resolve()
    search, final, common, joint, real, sources, provenance = load_inputs(v4, v5, budget)
    writer = FigureWriter(group)
    styles = {'font.family': 'serif', 'font.serif': ['Times New Roman', 'DejaVu Serif'],
              'font.size': 9, 'axes.labelsize': 9, 'axes.titlesize': 9,
              'legend.fontsize': 7.5, 'xtick.labelsize': 8, 'ytick.labelsize': 8,
              'axes.spines.top': False, 'axes.spines.right': False}
    colors = ['#4c78a8', '#f58518', '#54a24b', '#b279a2', '#e45756']
    methods = ['Jitter', 'SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive']
    labels = ['Jitter', 'SA-only', 'Legacy hybrid', 'SA-budget', 'Adaptive']
    def save(fig, name, data):
        status = provenance['status']
        if status != 'declared_confirmed':
            prior = fig._suptitle.get_text() if fig._suptitle is not None else ''
            label = ('Sensitivity diagnostic: real encoding / geometry unconfirmed'
                     if status == 'unconfirmed_sensitivity' else 'Synthetic-only inputs; no field-data evidence')
            fig.suptitle(label + ('\n' + prior if prior else ''), fontsize=8)
        writer.save(fig, name, {'sources_sha256': sources, 'records': data,
                               'source_provenance': provenance,
                               'selection_candidates': ['zero', 'linear', 'curvelet'],
                               'search_interval': 'conditional bootstrap 95% CI over paired seeds',
                               'recovery_interval': 'bootstrap 95% CI over clean structural templates',
                               'real_interval': 'range across splits of one real source'}, dpi=360)
    with rc_context(styles):
        data = search
        fig, axes = plt.subplots(1, 2, figsize=(7.1, 2.3), layout='constrained')
        for ax, domain, ms in zip(axes, ['uniform', 'physical'], [[38,64,90], [20,30,40]]):
            for method, label, color, shift in zip(methods[1:], labels[1:], colors[1:], [-.12,-.04,.04,.12]):
                rr = sorted([r for r in data if r['domain']==domain and r['method']==method], key=lambda r:r['rate'])
                y = np.array([r['mean'] for r in rr]); lo = np.array([r['ci_low'] for r in rr]); hi = np.array([r['ci_high'] for r in rr])
                ax.errorbar(np.arange(3)+shift, y, yerr=[y-lo,hi-y], fmt='o', ms=3, capsize=2, color=color, label=label)
            ax.set(xticks=range(3), xticklabels=ms, xlabel='Retained traces, m', ylabel='Fourier coherence', title=domain.capitalize()+f' domain; B = {budget}')
            ax.grid(axis='y',alpha=.2); ax.set_ylim(bottom=0)
        axes[0].legend(ncol=2, loc='upper right')
        save(fig, 'search', data)

        fig, axes = plt.subplots(1,2,figsize=(7.1,2.3),layout='constrained')
        for ax, comparison in zip(axes, ['Adaptive-Hybrid','Adaptive-NoRecycle']):
            for rr, label, color, shift in [(final,'Final evaluation',colors[4],-.08),(common,'Common evaluation count',colors[0],.08)]:
                rr = sorted([r for r in rr if r['comparison']==comparison],key=lambda r:(r['domain']!='uniform',int(r['rate'])))
                y = np.array([float(r['difference']) for r in rr]); lo = np.array([float(r['ci_low']) for r in rr]); hi = np.array([float(r['ci_high']) for r in rr])
                ax.errorbar(np.arange(6)+shift,y,yerr=[y-lo,hi-y],fmt='o',ms=3,capsize=2,label=label,color=color)
            ax.axhline(0,color='0.4',lw=.7); ax.grid(axis='y',alpha=.2)
            ax.set(xticks=range(6),xticklabels=['U38','U64','U90','P20','P30','P40'],ylabel='Paired coherence difference',title=comparison.replace('-', ' minus '))
        axes[0].legend(loc='lower left'); save(fig,'budget',{'final':final,'common_count':common})

        data = joint; fig,axes=plt.subplots(1,2,figsize=(7.1,2.8),layout='constrained')
        for ax, recovery in zip(axes,['fixed','selected']):
            for i,(method,label,color) in enumerate(zip(methods,labels,colors)):
                rr=sorted([r for r in data if r['method']==method and r['recovery']==recovery],key=lambda r:int(r['m']))
                y=np.array([float(r['mean_snr']) for r in rr]);lo=np.array([float(r['ci_low']) for r in rr]);hi=np.array([float(r['ci_high']) for r in rr])
                ax.errorbar(np.arange(3)+(i-2)*.055,y,yerr=[y-lo,hi-y],fmt='o-',ms=2.5,lw=.8,capsize=1.8,label=label,color=color)
            ax.set(xticks=range(3),xticklabels=[38,64,90],xlabel='Retained traces, m (N = 128)',ylabel='SNR on common H (dB)',title='Fixed curvelet' if recovery=='fixed' else 'Inner-selected recovery\n(zero / linear / curvelet)')
            ax.axhline(0,color='0.4',lw=.7)
            ax.grid(axis='y',alpha=.2)
        axes[0].legend(ncol=2,loc='lower right');save(fig,'recovery',data)

        data=real;fig,axes=plt.subplots(1,2,figsize=(7.1,2.8),layout='constrained',sharey=True)
        windows=WINDOWS
        for ax,key,title in zip(axes,['fixed_snr','selected_snr'],['Fixed curvelet','Inner-selected recovery']):
            for i,(window,color) in enumerate(zip(windows,colors)):
                group=[[float(r[key]) for r in data if r['window']==window and int(r['m'])==m] for m in [20,30,40]]
                y=np.mean(group,axis=1);lo=np.min(group,axis=1);hi=np.max(group,axis=1)
                ax.errorbar(np.arange(3)+(i-1.5)*.07,y,yerr=[y-lo,hi-y],fmt='o-',ms=3,lw=.8,capsize=2,color=color,label=window.replace('_',' '))
            ax.axhline(0,color='0.4',lw=.7);ax.grid(axis='y',alpha=.2)
            ax.set(xticks=range(3),xticklabels=[20,30,40],xlabel='Retained traces, m (N = 59)',ylabel='Hidden-trace SNR (dB)',title=title if key=='fixed_snr' else title+'\n(zero / linear / curvelet)')
        fig.suptitle(('Synthetic windows' if provenance['status']=='not_used' else 'One real source')+
                     '; bars show split ranges, not independent-source CIs',fontsize=8)
        axes[0].legend(ncol=2,loc='lower right',fontsize=6.8)
        save(fig,'real_windows',data)
    writer.gallery('English manuscript figures: '+provenance['status']+
                   '; original v4/v5 tables; selected recovery includes zero/linear/curvelet')
    return list(FIGURE_IDS)


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--v4-root',type=Path,help='v4 result directory containing search_<domain>_<budget>/')
    parser.add_argument('--v5-root',type=Path,help='v5 result directory containing analysis_thesis/ and real/')
    parser.add_argument('--group',default='paper',help='output group within the central figures directory')
    parser.add_argument('--budget',type=int,default=600,help='v4 search evaluation budget to plot')
    args=parser.parse_args()
    render(args.v4_root,args.v5_root,args.group,args.budget)
