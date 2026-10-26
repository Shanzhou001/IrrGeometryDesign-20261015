"""Post-hoc diagnostics only; never changes the frozen Viking experiment or selects a model."""
from collections import Counter, defaultdict
import json
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

from research.analysis.viking_results import inputs
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.paths import ROOT
from research.common.tabular import rows, write_rows
from research.data.viking import load_dataset
from research.experiments.viking_study import hidden_mask

RUN = ROOT/'results/viking_20261026'
OUT = ROOT/'results/viking_diagnosis_20261026'


def summarize(records, keys, fields):
    groups = defaultdict(list)
    for row in records:
        groups[tuple(row[k] for k in keys)].append(row)
    return [dict(zip(keys, key), count=len(same), **{f: float(np.mean([r[f] for r in same])) for f in fields})
            for key, same in sorted(groups.items())]


def snr(y, p):
    return float(10*np.log10(np.sum(y*y)/max(np.sum((y-p)**2), 1e-30)))


def waveform_stats(y, p):
    energy, predicted, cross = float(np.sum(y*y)), float(np.sum(p*p)), float(np.sum(y*p))
    gain = float(np.clip(cross/max(predicted, 1e-30), 0, 1))
    return dict(snr=snr(y, p), energy_ratio=predicted/energy,
                cosine=cross/np.sqrt(max(energy*predicted, 1e-30)),
                oracle_gain_on_H_diagnostic_only=gain, oracle_snr_on_H_diagnostic_only=snr(y, p*gain))


def main():
    manifest, search, recovery, used = inputs(RUN)
    shots, provenance = load_dataset()
    by_shot = {s.ffid: s for s in shots}; hidden = hidden_mask(120); hi = np.flatnonzero(hidden)
    native_hashes = {name.replace('\\', '/'): value for step in manifest['steps'].values() for name, value in step['outputs'].items()}
    def prediction(key):
        name = 'native/'+key+'/prediction.csv'
        if digest(RUN/name) != native_hashes[name]:
            raise ValueError('Changed prediction: '+name)
        return np.loadtxt(RUN/name, delimiter=',')
    geometry = []
    positions = np.arange(120)*127/119
    operator = np.zeros((120, 128))
    for j, pos in enumerate(positions):
        left = min(int(pos), 126); b = pos-left
        operator[j, left:left+2] = [1-b, b]
    for row in search:
        mask = np.array(list(row['mask']), int).astype(bool); kept = np.flatnonzero(mask)
        nearest = np.min(abs(hi[:, None]-kept[None, :]), axis=1)
        both = mask[hi-1]&mask[hi+1]
        neither = ~mask[hi-1]&~mask[hi+1]
        spans = []; extrapolated = 0
        for h in hi:
            left, right = kept[kept < h], kept[kept > h]
            if not len(left) or not len(right):
                extrapolated += 1
            else:
                spans.append(right[0]-left[-1])
        eig = np.linalg.eigvalsh(operator[mask]@operator[mask].T)
        residues = [int(np.sum(mask[np.arange(120)%8 == r])) for r in range(8)]
        geometry.append(dict(id=row['id'], m=int(row['m']), method=row['method'], seed=int(row['seed']),
                             mu=float(row['mu']), max_gap_m=float(row['max_gap_m']), nn_cv=float(row['nn_cv']),
                             H_mean_nearest_m=float(nearest.mean()*25), H_both_adjacent_fraction=float(both.mean()),
                             H_neither_adjacent_fraction=float(neither.mean()), H_mean_bracket_span_m=float(np.mean(spans)*25),
                             H_extrapolated_fraction=extrapolated/len(hi), projection_gram_condition=float(eig[-1]/eig[0]),
                             residue_counts=','.join(map(str, residues))))
    write_rows(OUT/'geometry.csv', geometry)
    geometry_means = summarize(geometry, ['m', 'method'], ['mu', 'max_gap_m', 'nn_cv', 'H_mean_nearest_m',
        'H_both_adjacent_fraction', 'H_neither_adjacent_fraction', 'H_mean_bracket_span_m', 'projection_gram_condition'])
    write_rows(OUT/'geometry_means.csv', geometry_means)
    waveforms, bands, convergence = [], [], []
    frequencies = np.fft.rfftfreq(256, .004)
    weights = np.full(len(frequencies), 2.); weights[[0, -1]] = 1
    for record in recovery:
        case = record['case']; shot = by_shot[int(record['ffid'])]
        y = shot.data[:, hidden]; fixed = prediction(case+'_fixed')[:, hidden]
        mask = np.array(list(record['mask']), int).astype(bool)
        selected_kind = record['selected']
        cold = prediction(case+'_selected_cold')[:, hidden] if selected_kind == 'cold' else None
        base = dict(case=case, ffid=shot.ffid, m=int(record['m']), method=record['method'], seed=int(record['seed']),
                    selected_kind=selected_kind, learned_gain=float(record['gain']))
        fixed_stats = waveform_stats(y, fixed)
        waveforms.append(dict(base, candidate='warm', **fixed_stats))
        predictions = [('warm', fixed)]
        if cold is not None:
            waveforms.append(dict(base, candidate='cold_on_inner_selected_subset', **waveform_stats(y, cold)))
            predictions.append(('cold', cold))
        yf = np.fft.rfft(y, axis=0)
        for name, pred in predictions:
            pf = np.fft.rfft(pred, axis=0)
            for lo, high in [(0, 20), (20, 40), (40, 80), (80, 126)]:
                keep = (frequencies >= lo)&(frequencies < high)
                e = float(np.sum(weights[keep, None]*abs(yf[keep])**2))
                err = float(np.sum(weights[keep, None]*abs(yf[keep]-pf[keep])**2))
                bands.append(dict(base, candidate=name, low_hz=lo, high_hz=min(high, 125),
                                  truth_energy_fraction=e/float(np.sum(weights[:, None]*abs(yf)**2)),
                                  nmse=err/e, snr=float(10*np.log10(e/max(err, 1e-30)))))
        for name, suffix in [('warm', '_fixed'), ('cold', '_selected_cold')]:
            if name == 'cold' and cold is None:
                continue
            trace = rows(RUN/'native'/(case+suffix)/'convergence.csv')
            convergence.append(dict(base, candidate=name, first_step=float(trace[0]['relative_step']),
                                    final_step=float(trace[-1]['relative_step'])))
    write_rows(OUT/'waveforms.csv', waveforms); write_rows(OUT/'bands.csv', bands); write_rows(OUT/'convergence.csv', convergence)
    warm_means = summarize([r for r in waveforms if r['candidate'] == 'warm'], ['m', 'method'],
                           ['snr', 'energy_ratio', 'cosine', 'oracle_gain_on_H_diagnostic_only', 'oracle_snr_on_H_diagnostic_only'])
    write_rows(OUT/'warm_means.csv', warm_means)
    spectral = []
    for shot in shots:
        data = shot.data-shot.data.mean(axis=0, keepdims=True)
        spectrum = abs(np.fft.fft2(data*np.hanning(256)[:, None]*np.hanning(120)[None, :]))**2
        wavenumbers = abs(np.fft.fftfreq(120, 25))
        for m in (36, 60, 84):
            cutoff = (m/120)/(2*25)
            spectral.append(dict(ffid=shot.ffid, m=m, regular_same_density_nyquist_cyc_m=cutoff,
                                 energy_above_reference=float(spectrum[:, wavenumbers > cutoff].sum()/spectrum.sum())))
    write_rows(OUT/'spatial_spectrum.csv', spectral)
    correlation = []
    for shot in shots:
        values = shot.data-shot.data.mean(axis=0, keepdims=True)
        for j in range(119):
            a, b = values[:, j], values[:, j+1]
            zero = float(np.dot(a, b)/np.sqrt(np.dot(a, a)*np.dot(b, b)))
            choices = []
            for lag in range(-12, 13):
                aa, bb = (a[lag:], b[:256-lag]) if lag >= 0 else (a[:256+lag], b[-lag:])
                choices.append((float(np.dot(aa, bb)/np.sqrt(np.dot(aa, aa)*np.dot(bb, bb))), lag))
            best, lag = max(choices)
            correlation.append(dict(ffid=shot.ffid, adjacent_column=j, zero_lag_correlation=zero,
                                    best_shifted_correlation=best, best_lag_samples=lag))
    write_rows(OUT/'neighbor_correlation.csv', correlation)
    floors = [r for r in search if int(r['m']) == 84 and abs(float(r['mu'])-1/7) < 1e-6]
    floor_scores = []
    for row in floors:
        cases = [r for r in recovery if r['search_id'] == row['id']]
        floor_scores.append(dict(id=row['id'], method=row['method'], mu=float(row['mu']),
                                 mean_fixed_snr=float(np.mean([float(r['fixed_snr']) for r in cases])),
                                 mean_selected_snr=float(np.mean([float(r['selected_snr']) for r in cases]))))
    write_rows(OUT/'floor_layouts.csv', floor_scores)
    cold_by_case = {r['case']: r for r in waveforms if r['candidate'] != 'warm'}
    warm_by_case = {r['case']: r for r in waveforms if r['candidate'] == 'warm'}
    paired_deltas = [r['snr']-warm_by_case[key]['snr'] for key, r in cold_by_case.items()]
    summary = dict(scope='post-hoc diagnosis, not independent confirmation or outer-H model selection',
                   source_manifest_sha256=digest(RUN/'manifest.json'), script_sha256=digest(__file__),
                   selected_candidates=dict(Counter(r['selected'] for r in recovery)),
                   gains_by_count={str(m): dict(mean=float(np.mean([float(r['gain']) for r in recovery if int(r['m']) == m])),
                                                below_one=sum(float(r['gain']) < 1-1e-12 for r in recovery if int(r['m']) == m)) for m in (36, 60, 84)},
                   cold_available_cases=len(cold_by_case), cold_minus_warm_mean_snr_on_selected_subset=float(np.mean(paired_deltas)),
                   cold_better_count=sum(d > 0 for d in paired_deltas),
                   periodic_hidden_set_fourier_lower_bound=1/7, floor_cases=len(floors),
                   floor_unique_masks=len({r['mask'] for r in floors}),
                   floor_fixed_snr_range=[min(r['mean_fixed_snr'] for r in floor_scores), max(r['mean_fixed_snr'] for r in floor_scores)],
                   spectral_energy_above_reference={str(m): float(np.mean([r['energy_above_reference'] for r in spectral if r['m'] == m])) for m in (36, 60, 84)},
                   neighbor_median_zero_lag=float(np.median([r['zero_lag_correlation'] for r in correlation])),
                   neighbor_median_best_shifted=float(np.median([r['best_shifted_correlation'] for r in correlation])),
                   neighbor_median_abs_best_lag_samples=float(np.median([abs(r['best_lag_samples']) for r in correlation])),
                   warm_final_step_median=float(np.median([r['final_step'] for r in convergence if r['candidate'] == 'warm'])))
    (OUT/'summary.json').write_text(json.dumps(summary, indent=2)+'\n', encoding='utf8')
    writer = FigureWriter('research/viking_diagnosis_20261026')
    figure, grid = plt.subplots(2, 2, figsize=(10, 7), layout='constrained')
    axes = grid.ravel()
    for method in ('Jitter', 'SAOnly', 'Hybrid', 'SA_Budget', 'Adaptive'):
        same = sorted([r for r in geometry_means if r['method'] == method], key=lambda r: r['m'])
        for ax, field in zip(axes, ('mu', 'max_gap_m', 'H_both_adjacent_fraction', 'H_neither_adjacent_fraction')):
            ax.plot([r['m'] for r in same], [r[field] for r in same], 'o-', label=method, ms=3)
    for ax, title, label in zip(axes, ('Fourier target', 'Maximum uncovered gap', 'Some hidden traces gain two neighbors', 'Other hidden traces lose both neighbors'),
                               ('Mean coherence', 'Mean maximum gap (m)', 'Fraction with both adjacent traces known', 'Fraction with neither adjacent trace known')):
        ax.set(title=title, ylabel=label, xlabel='Retained traces', xticks=[36, 60, 84]); ax.grid(alpha=.2)
    axes[0].legend(fontsize=7); figure.suptitle('Post-hoc Viking diagnosis: geometry and local coverage', fontsize=10)
    writer.save(figure, 'geometry_tradeoff', geometry_means)
    shot = by_shot[53]; seed = 20261024; count = 60
    figure, axes = plt.subplots(2, 1, figsize=(10, 6), layout='constrained', sharex=True)
    t = 2.048+np.arange(256)*.004
    for ax, method in zip(axes, ('Jitter', 'Adaptive')):
        case = f'ffid53_m60_seed{seed}_{method}'
        row = next(r for r in recovery if r['case'] == case)
        fixed = prediction(case+'_fixed'); cold = prediction(case+'_selected_cold')
        ax.plot(t, shot.data[:, 60], color='black', lw=1.2, label='Measured hidden trace')
        ax.plot(t, fixed[:, 60], lw=.9, label='Warm fixed curvelet')
        ax.plot(t, cold[:, 60]*float(row['gain']), lw=.9, label='Inner-selected cold + gain')
        ax.set(title=f'{method}; FFID 53; hidden column 60; m=60', ylabel='Recorded amplitude'); ax.legend(fontsize=8); ax.grid(alpha=.2)
    axes[-1].set_xlabel('Time (s)'); writer.save(figure, 'hidden_waveforms', dict(ffid=53, column_zero_based=60, m=60, seed=seed))
    writer.gallery('Viking diagnosis: post-hoc evidence, not confirmatory model tuning')
    print(json.dumps(summary, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
