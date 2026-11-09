"""Geometry and moveout predictors with a shot-held-out regression fit."""
from itertools import product

import numpy as np


# Signed apparent velocities are candidate slopes, not a subsurface velocity model.
SPEEDS = (0, 650, 850, 1100, 1500, 2000, 2800, 4200, -1200, -2500)
BANDS = (18, 36, 72, 144)
RIDGE = (.04, .2, 1., 5., 25.)


def predictors(observed, xy, source, known, targets, start, stop, dt):
    if (observed.ndim != 2 or known.dtype != bool or targets.dtype != bool or known.shape != (len(xy),) or
            targets.shape != known.shape or np.any(known & targets) or not targets.any() or
            observed.shape[1] != known.sum() or not np.isfinite(observed).all() or
            not 0 <= start < stop <= len(observed) or dt <= 0):
        raise ValueError('Prediction requires finite observed columns and disjoint target stations')
    sites = xy[known]
    target_xy = xy[targets]
    offsets = np.linalg.norm(xy - source, axis=1)
    distances = np.linalg.norm(target_xy[:, None] - sites[None, :], axis=2)
    neighbours = np.argsort(distances, axis=1, kind='stable')[:, :6]
    distance = np.take_along_axis(distances, neighbours, axis=1)
    weights = np.maximum(distance, 1e-6) ** -1.5
    weights /= weights.sum(axis=1, keepdims=True)
    shift = offsets[known][neighbours] - offsets[targets, None]
    time = np.arange(start, stop)[:, None, None]
    bins = np.fft.rfftfreq(len(observed), dt)
    spectrum = np.fft.rfft(observed, axis=0)
    bands = {band: np.fft.irfft(spectrum * np.clip((1.25 * band - bins) / (.5 * band), 0, 1)[:, None],
                               n=len(observed), axis=0) for band in BANDS}
    delta_xy = sites[neighbours] - target_xy[:, None, :]
    moves = [(time + (shift[None] / (speed * dt) if speed else 0), BANDS) for speed in SPEEDS]
    for axis, speed in product((0, 1), (-2800, -1200, 1200, 2800)):
        moves.append((time + delta_xy[None, :, :, axis] / (speed * dt), (18, 36, 72)))
    delta_squared = offsets[known][neighbours] ** 2 - offsets[targets, None] ** 2
    for speed in (1200, 1900, 3000):
        moves.append((np.sqrt(np.maximum(time ** 2 + delta_squared[None] / (speed * dt) ** 2, 0)), (18, 36, 72)))
    features = []
    for samples, selected_bands in moves:
        sample = np.clip(samples, 0, len(observed) - 1)
        low = np.floor(sample).astype(int)
        high = np.minimum(low + 1, len(observed) - 1)
        mix = sample - low
        for band in selected_bands:
            data = bands[band]
            a = data[low, neighbours[None]]
            b = data[high, neighbours[None]]
            features.append(np.sum((a + (b - a) * mix) * weights, axis=2))
    idw = np.sum(observed[start:stop, neighbours] * weights, axis=2)
    features.append(idw)
    strata = np.digitize(offsets[targets], np.quantile(offsets, [1/3, 2/3]))
    return np.asarray(features), idw, strata


def fit(equations):
    """Choose a ridge weight by leaving each development shot out in turn."""
    equations = list(equations)
    if len(equations) < 2:
        raise ValueError('At least two development shots are required')
    gram = np.sum([a for a, _, _ in equations], axis=0)
    cross = np.sum([b for _, b, _ in equations], axis=0)

    def solve(a, b, penalty):
        scale = max(1e-20, float(np.trace(a) / len(b)))
        return np.linalg.solve(a + np.eye(len(b)) * (scale * penalty), b)

    losses = []
    for penalty in RIDGE:
        errors = []
        for a, b, energy in equations:
            weights = solve(gram - a, cross - b, penalty)
            errors.append(1 + (weights @ a @ weights - 2 * weights @ b) / energy)
        losses.append(float(np.mean(errors)))
    best = int(np.argmin(losses))
    # A failed development validation falls back to the measured zero baseline.
    weights = solve(gram, cross, RIDGE[best]) if losses[best] < 1 else np.zeros(len(cross))
    return dict(ridge=RIDGE[best], development_loso_nmse=losses[best], weights=weights.tolist())


def combine(features, strata, models):
    prediction = np.empty(features.shape[1:])
    for group, model in enumerate(models):
        selected = strata == group
        prediction[:, selected] = np.einsum('f,ftx->tx', model['weights'], features[:, :, selected])
    return prediction
