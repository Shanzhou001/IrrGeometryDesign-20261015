"""Observed-pair spatial bracketing with measured local time delays."""
from itertools import combinations

import numpy as np


def warps(observed, xy, known, targets, start, stop, dt):
    if (observed.ndim != 2 or known.dtype != bool or targets.dtype != bool or
            known.shape != (len(xy),) or targets.shape != known.shape or
            observed.shape[1] != known.sum() or known.sum() < 8 or np.any(known & targets) or
            not targets.any() or not np.isfinite(observed).all() or dt <= 0 or
            not 0 <= start < stop <= len(observed)):
        raise ValueError('Local-delay prediction requires disjoint targets and finite observed columns')
    coords = xy[known]
    txy = xy[targets]
    distance = np.linalg.norm(coords[None] - txy[:, None], axis=2)
    nearest = np.argsort(distance, axis=1, kind='stable')[:, :8]
    spec = np.fft.rfft(observed, axis=0)
    f = np.fft.rfftfreq(len(observed), dt)
    ts = np.arange(start, stop)
    ii, jj = np.asarray(list(combinations(range(8), 2))).T
    u, v = nearest[:, ii], nearest[:, jj]
    a, b = coords[u], coords[v]
    ab = b - a
    alpha = np.sum((txy[:, None] - a) * ab, axis=2) / np.sum(ab**2, axis=2)
    weight = np.clip(alpha, 0, 1)
    gap = np.linalg.norm(txy[:, None] - (a + weight[:, :, None] * ab), axis=2)
    spread = np.linalg.norm(ab, axis=2)
    step = np.median(np.min(np.where(distance>0,distance,np.inf), axis=1))
    result = []
    for cut in (12, 25, 50, 100):
        smooth = np.fft.irfft(spec * np.exp(-.5*(f/cut)**4)[:, None], n=len(observed), axis=0)
        expected = smooth[ts[:, None, None], u[None]]
        mag = np.sum(expected**2, axis=0)
        score = np.full(alpha.shape, -1.)
        lag = np.zeros(alpha.shape, int)
        for shift in range(-48, 49, 2):
            moved = smooth[np.clip(ts+shift,0,len(observed)-1)[:,None,None], v[None]]
            corr = np.sum(moved*expected, axis=0) / np.sqrt(np.maximum(np.sum(moved**2, axis=0)*mag, 1e-30))
            corr = np.where(abs(shift)*dt*550 < spread, corr, -2.)
            better = corr > score
            score[better], lag[better] = corr[better], shift
        merit = np.maximum(score,0) * np.exp(-gap/max(step,1e-5) - spread/(4*max(step,1e-5)))
        merit[(alpha < 0) | (alpha > 1)] = 0
        best = np.argmax(merit, axis=1)
        j = np.arange(len(txy))
        p, q, fraction, delta = u[j,best], v[j,best], weight[j,best], lag[j,best]
        def interp(shift, channel):
            t = np.clip(ts[:,None]+shift,0,len(observed)-1)
            lo = np.floor(t).astype(int);hi=np.minimum(lo+1,len(observed)-1)
            return smooth[lo,channel]*(1-(t-lo)) + smooth[hi,channel]*(t-lo)
        prediction = interp(-fraction*delta,p)*(1-fraction) + interp((1-fraction)*delta,q)*fraction
        result.append(prediction*np.maximum(score[j,best],0)*(merit[j,best]>0))
    return np.asarray(result)
