"""Observed-only recovery candidates and a convex f-k l1 baseline for Viking v2."""
import json
from pathlib import Path
import shutil

import numpy as np

from research.experiments.audit_joint_studies import write_gather, write_mask
from research.experiments.review_studies import linear_known, validation_choice

CANDIDATES = (
    dict(name='zero', kind='zero'), dict(name='linear', kind='linear'),
    dict(name='fk_weak80', kind='fk', iterations=80, fraction=.01),
    dict(name='fk_strong80', kind='fk', iterations=80, fraction=.05),
    dict(name='curvelet_warm40', kind='curvelet', iterations=40, start=.0265, end=.0005, cold=False),
    dict(name='curvelet_cold40', kind='curvelet', iterations=40, start=.0265, end=.0005, cold=True),
    dict(name='curvelet_cold_strong40', kind='curvelet', iterations=40, start=.08, end=.002, cold=True),
    dict(name='curvelet_cold80', kind='curvelet', iterations=80, start=.0265, end=.0005, cold=True),
)


def fk_fista(data, known, iterations=80, fraction=.01, debias_steps=0):
    """Minimize .5||P_K x-y||^2 + lambda||unitary FFT2(x)||_1; step size is one.

    Scale and lambda use K only. The returned latent solution is penalized; callers
    restore measured samples after predicting unknown columns. No H scores enter.
    """
    data = np.asarray(data, float); known = np.asarray(known, bool)
    if (data.ndim != 2 or known.shape != (data.shape[1],) or sum(known) < 2 or
            not np.isfinite(data[:, known]).all() or iterations < 1 or not 0 <= fraction <= 1 or debias_steps < 0):
        raise ValueError('Invalid observed Fourier recovery input')
    scale = max(float(np.max(abs(data[:, known]))), 1e-20)
    observed = np.zeros_like(data); observed[:, known] = data[:, known]/scale
    lam = fraction*float(np.max(abs(np.fft.fft2(observed, norm='ortho'))))
    current = np.zeros_like(data); extrapolated = current.copy(); momentum = 1.; history = []
    for _ in range(iterations):
        gradient = np.zeros_like(data); gradient[:, known] = (extrapolated-observed)[:, known]
        coefficients = np.fft.fft2(extrapolated-gradient, norm='ortho')
        coefficients *= np.maximum(1-lam/np.maximum(abs(coefficients), 1e-30), 0)
        updated = np.fft.ifft2(coefficients, norm='ortho').real
        residual = (updated-observed)[:, known]
        history.append(float(.5*np.sum(residual**2)+lam*np.sum(abs(np.fft.fft2(updated, norm='ortho')))))
        next_momentum = (1+np.sqrt(1+4*momentum**2))/2
        extrapolated = updated+(momentum-1)/next_momentum*(updated-current)
        current, momentum = updated, next_momentum
    if debias_steps > 0:
        spec = np.fft.fft2(current, norm='ortho')
        peak = float(np.max(abs(spec)))
        support = abs(spec) > max(1e-6 * peak, 1e-12)
        if np.any(support):
            for _ in range(debias_steps):
                grad = np.zeros_like(data); grad[:, known] = (current - observed)[:, known]
                current = np.fft.ifft2((np.fft.fft2(current - grad, norm='ortho')) * support, norm='ortho').real
    return current*scale, dict(objective='0.5*||P_K*x-y_K||^2 + lambda*||FFT2_ortho(x)||_1',
                                scale=scale, lambda_value=lam, iterations=iterations, fraction=fraction,
                                debias_steps=int(debias_steps), objective_trace=history)


def predict(recorder, key, data, x, known, configuration):
    kind = configuration['kind']
    if kind == 'zero':
        prediction = np.zeros_like(data)
    elif kind == 'linear':
        prediction = linear_known(data, x, known)
    elif kind == 'fk':
        prediction, diagnostic = fk_fista(data, known, configuration['iterations'], configuration['fraction'])
        folder = recorder.out/'python'; folder.mkdir(exist_ok=True)
        np.savez_compressed(folder/(key+'.npz'), prediction=prediction, known=known)
        (folder/(key+'.json')).write_text(json.dumps(diagnostic, indent=2)+'\n', encoding='utf-8')
    else:
        observed = np.zeros_like(data); observed[:, known] = data[:, known]
        gather = recorder.out/'observed'/(key+'.csv'); mask = recorder.out/'masks'/(key+'.txt')
        write_gather(gather, observed, x, 'Viking v2 observed only; unknown amplitudes zero')
        write_mask(mask, known)
        target = recorder.out/'native'/key
        args = ['--review-reconstruct', gather, '--mask', mask, '--out', target,
                '--iter', configuration['iterations'], '--threshold-start', configuration['start'],
                '--threshold-end', configuration['end']]
        if configuration['cold']:
            args.append('--cold')
        recorder.call(key, args, target)
        prediction = np.loadtxt(target/'prediction.csv', delimiter=',')
    prediction[:, known] = data[:, known]
    return prediction


def select_and_refit(recorder, key, data, x, known, pilot, seed, folds=1):
    """Pilot remains in every inner train set, because its values informed search."""
    available = np.flatnonzero(known & ~pilot)
    width = max(2, int(sum(known))//3)
    if pilot.shape != known.shape or not np.all(known[pilot]) or folds not in (1, 2) or len(available) < width*folds:
        raise ValueError('Inner folds must leave the pilot observed and use disjoint validation columns')
    order = np.random.default_rng(seed).permutation(available)
    targets, candidates = [], {c['name']: [] for c in CANDIDATES}; splits = []
    for fold in range(folds):
        validation = order[fold*width:(fold+1)*width]; train = known.copy(); train[validation] = False
        targets.append(data[:, validation]); splits.append(dict(train=np.flatnonzero(train).tolist(), validation=validation.tolist()))
        for cfg in CANDIDATES:
            prediction = predict(recorder, f"{key}_inner{fold}_{cfg['name']}", data, x, train, cfg)
            candidates[cfg['name']].append(prediction[:, validation])
    selected, records = validation_choice(np.concatenate(targets, axis=1),
        [(c['name'], np.concatenate(candidates[c['name']], axis=1)) for c in CANDIDATES])
    choice = dict(selected=selected, candidates=records, splits=splits, pilot=np.flatnonzero(pilot).tolist(), seed=seed)
    folder = recorder.out/'selection'; folder.mkdir(exist_ok=True)
    (folder/(key+'.json')).write_text(json.dumps(choice, indent=2)+'\n', encoding='utf-8')
    cache = {}
    def full(name):
        if name not in cache:
            cfg = next(c for c in CANDIDATES if c['name'] == name)
            cache[name] = predict(recorder, key+'_full_'+name, data, x, known, cfg)
        return cache[name]
    cold = full('curvelet_cold40'); fourier = full('fk_weak80')
    prediction = full(selected['candidate'])*selected['gain']; prediction[:, known] = data[:, known]
    return dict(cold=cold, fk=fourier, selected=prediction, linear=linear_known(data, x, known)), choice


def prune_intermediates(output_dir):
    """Remove temporary observed/masks/native solver working directories and inner-fold python caches."""
    out = Path(output_dir)
    removed = 0
    for dirname in ('observed', 'masks', 'native'):
        target = out / dirname
        if target.exists():
            shutil.rmtree(target, ignore_errors=True)
            removed += 1
    py_dir = out / 'python'
    if py_dir.is_dir():
        for item in list(py_dir.glob('*_inner*')):
            if item.is_file():
                item.unlink()
                removed += 1
    return removed
