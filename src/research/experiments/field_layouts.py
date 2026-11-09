"""Paired spatial searches with one retained receiver per physical cell."""
import numpy as np

from research.experiments.mapped_field import coherence, dictionary


def spatial_cells(xy, eligible, count):
    if not 1 <= count <= len(eligible):
        raise ValueError('Infeasible receiver count')

    def split(indices, cells):
        if cells == 1:
            return [indices]
        axis = int(np.argmax(np.ptp(xy[indices], axis=0)))
        order = indices[np.argsort(xy[indices, axis], kind='stable')]
        left = cells // 2
        cut = len(order) * left // cells
        return split(order[:cut], left) + split(order[cut:], cells - left)

    return split(np.asarray(eligible), count)


def constrained_layouts(xy, hidden, count, seed, budget, prefix):
    eligible = np.flatnonzero(~hidden)
    if len(eligible) <= count or not 1 <= prefix <= budget:
        raise ValueError('Constrained search requires an exchangeable receiver cell')
    cells = spatial_cells(xy, eligible, count)
    rng = np.random.default_rng(seed)
    start = np.asarray([rng.choice(cell) for cell in cells])
    exchangeable = [i for i, cell in enumerate(cells) if len(cell) > 1]
    atoms = dictionary(xy)
    i, j = np.triu_indices(atoms.shape[1], 1)
    differences = atoms[:, i].conj() * atoms[:, j]
    initial_sum = differences[start].sum(axis=0)
    initial_score = float(abs(initial_sum).max() / count)
    selected = {'Jitter': (start, initial_score, 1)}

    candidates = np.stack([np.r_[start[i], rng.choice(cell, budget - 1)] for i, cell in enumerate(cells)], axis=1)
    # Compute the same full objective for all random candidates, including Jitter.
    random_scores = np.asarray([abs(differences[choice].sum(axis=0)).max() / count for choice in candidates])
    best = int(np.argmin(random_scores))
    selected['RandomBudget'] = candidates[best], float(random_scores[best]), budget

    for method in ('SA', 'Hybrid'):
        rng = np.random.default_rng(seed + 1)
        current, best_choice = start.copy(), start.copy()
        total = best_sum = initial_sum.copy()
        value = best_value = initial_score
        for evaluation in range(1, budget):
            greedy = method == 'Hybrid' and evaluation >= prefix
            if method == 'Hybrid' and evaluation == prefix:
                current, total, value = best_choice.copy(), best_sum.copy(), best_value
            cell = int(rng.choice(exchangeable))
            old = current[cell]
            candidate = rng.choice(cells[cell][cells[cell] != old])
            proposal = total - differences[old] + differences[candidate]
            score = float(abs(proposal).max() / count)
            temperature = .05 * (.001 / .05) ** min(1., evaluation / 63)
            if score < value or (not greedy and rng.random() < np.exp(min(0., (value - score) / temperature))):
                current[cell], total, value = candidate, proposal, score
                if value < best_value:
                    best_choice, best_sum, best_value = current.copy(), total.copy(), value
        selected[method] = best_choice, best_value, budget

    distances = np.linalg.norm(xy[:, None] - xy[None, :], axis=2)
    bound = float(max(np.max(distances[np.ix_(cell, cell)]) for cell in cells) + distances[:, eligible].min(axis=1).max())
    for method, (indices, measured, evaluations) in selected.items():
        known = np.zeros(len(xy), bool)
        known[indices] = True
        maximum = float(distances[:, indices].min(axis=1).max())
        if (abs(coherence(atoms, known) - measured) > 2e-11 or known.sum() != count or np.any(known & hidden) or
                any(known[cell].sum() != 1 for cell in cells) or maximum > bound + 1e-9):
            raise ValueError('Constrained spatial layout failed independent audit')
        yield method, known, dict(mu=measured, evaluations=evaluations, cells=count,
                                  max_nearest_distance=maximum, cell_cover_bound=bound)
