"""Bind maintained field experiments to their inputs, models and saved predictions."""
import json
from collections import defaultdict
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT
from research.experiments.field_layouts import spatial_cells
from research.experiments.mapped_field import hidden_mask
from research.validation.run_audit_checks import tracked_files


def source_snapshot():
    return {p.relative_to(ROOT).as_posix(): digest(p) for p in tracked_files()}


def seal(out, protocol, inputs, filenames, initial_sources):
    out, protocol = Path(out), Path(protocol)
    files = ['protocol.json', *filenames]
    model = out / 'frozen_model.json'
    if model.is_file():
        files.append(model.name)
    if source_snapshot() != initial_sources:
        raise ValueError('Sources changed during field calculation')
    manifest = dict(status='passed', protocol_source=protocol.relative_to(ROOT).as_posix(),
                    protocol_sha256=digest(protocol), input_sha256=inputs,
                    source_sha256=initial_sources,
                    output_sha256={name: digest(out / name) for name in files})
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')


def check_manifest(out):
    out = Path(out)
    manifest = json.loads((out / 'manifest.json').read_text())
    if manifest.get('status') != 'passed':
        raise ValueError('Incomplete field manifest')
    protocol = ROOT / manifest['protocol_source']
    if (digest(protocol) != manifest['protocol_sha256'] or
            json.loads((out/'protocol.json').read_text()) != json.loads(protocol.read_text())):
        raise ValueError('Field protocol changed')
    for section, base in [('input_sha256', ROOT), ('source_sha256', ROOT), ('output_sha256', out)]:
        for name, expected in manifest[section].items():
            if digest(base / name) != expected:
                raise ValueError('Field evidence changed: ' + name)
    return json.loads((out / 'protocol.json').read_text())


def coordinate_idw(observed, xy, known, hidden, start, stop):
    distances = np.linalg.norm(xy[hidden, None] - xy[None, known], axis=2)
    nearest = np.argsort(distances, axis=1, kind='stable')[:, :6]
    weights = np.maximum(np.take_along_axis(distances, nearest, axis=1), 1e-6) ** -1.5
    weights /= weights.sum(axis=1, keepdims=True)
    return np.sum(observed[start:stop, nearest] * weights, axis=2)


def audit(out, acquisitions, cases, expected_keys):
    """Check standardized case descriptions against independently read recordings."""
    out = Path(out)
    seen, deployments = set(), {}
    source_scores = defaultdict(list)
    maximum = 0.
    for item in cases:
        key, path, expected_hash, h, count, start, stop, nmse, baseline, full_scores = item
        if not np.isfinite([float(nmse), float(baseline), *map(float, full_scores.values())]).all():
            raise ValueError('Nonfinite reported field score')
        if key in seen or key not in expected_keys:
            raise ValueError('Duplicate or unregistered scientific case')
        seen.add(key)
        target = (out / path).resolve()
        if not target.is_relative_to(out.resolve()) or digest(target) != expected_hash:
            raise ValueError('Missing or modified field prediction')
        xy, samples = acquisitions[key[:2]]
        with np.load(target, allow_pickle=False) as saved:
            known, hidden = saved['known'], saved['hidden']
            if (known.dtype != bool or hidden.dtype != bool or known.shape != (len(xy),) or
                    hidden.shape != known.shape or known.sum() != count or np.any(known & hidden) or
                    not np.array_equal(hidden, hidden_mask(len(xy), h)) or not np.array_equal(xy, saved['xy'])):
                raise ValueError('Receiver identities, mask or common holdout differs')
            cells = spatial_cells(xy, np.flatnonzero(~hidden), count)
            if any(known[cell].sum() != 1 for cell in cells):
                raise ValueError('One-receiver-per-cell constraint failed')
            truth = samples[start:stop, hidden]
            if not np.array_equal(truth, saved['truth']):
                raise ValueError('Saved target differs from published raw samples')
            reference = coordinate_idw(samples[:, known], xy, known, hidden, start, stop)
            if not np.array_equal(reference, saved['idw']):
                raise ValueError('Coordinate baseline differs from independently read observations')
            energy = float(np.sum(truth ** 2))
            if not energy > 0:
                raise ValueError('Zero target energy')
            for values, expected in [(saved['prediction'], nmse), (saved['idw'], baseline)]:
                if values.shape != truth.shape or not np.isfinite(values).all():
                    raise ValueError('Invalid prediction shape or values')
                score = float(np.sum((values - truth) ** 2) / energy)
                maximum = max(maximum, abs(score - float(expected)))
            if 'prediction_all' in saved:
                targets = ~known
                all_truth = samples[start:stop, targets]
                all_baseline = coordinate_idw(samples[:, known], xy, known, targets, start, stop)
                if (not np.array_equal(all_truth, saved['truth_all']) or
                        not np.array_equal(all_baseline, saved['idw_all'])):
                    raise ValueError('Full unobserved-station evidence differs')
                if not np.isfinite(saved['prediction_all']).all():
                    raise ValueError('Nonfinite full reconstruction')
                for field, column in [('prediction_all', 'all_nmse'), ('idw_all', 'all_idw_nmse')]:
                    score = float(np.sum((saved[field] - all_truth)**2) / np.sum(all_truth**2))
                    maximum = max(maximum, abs(score - float(full_scores[column])))
            elif full_scores:
                raise ValueError('Missing full unobserved-station predictions')
            # Repeated source locations share a deployment when their coordinates match.
            deployment = (xy.tobytes(), key[2:])
            if deployment in deployments and not np.array_equal(deployments[deployment], known):
                raise ValueError('A supposedly fixed deployment changed between sources')
            deployments[deployment] = known.copy()
        source_scores[key[0]].append((float(nmse), float(baseline)))
    if seen != expected_keys or maximum > 1e-12:
        raise ValueError('Missing field cases or independent score mismatch')
    expected_count = len(expected_keys)
    performance = json.loads((out / 'performance.json').read_text())
    if not np.isfinite([performance['mean_nmse'], performance['mean_idw_nmse']]).all():
        raise ValueError('Nonfinite reported source aggregate')
    if performance['cases'] != expected_count:
        raise ValueError('Reported field case count differs')
    plan = json.loads((out / 'protocol.json').read_text())
    means = np.array([np.mean(v, axis=0) for v in source_scores.values()])
    mean, baseline = np.mean(means, axis=0)
    positive = int(np.sum(means[:, 0] < 1))
    passed = mean < min(1, baseline) and positive >= plan['minimum_positive_shots']
    if (abs(mean - performance['mean_nmse']) > 1e-12 or
            abs(baseline - performance['mean_idw_nmse']) > 1e-12 or
            positive != performance['positive_shots'] or
            performance['performance_status'] != ('passed' if passed else 'failed')):
        raise ValueError('Reported effect differs from independent source aggregation')
    return dict(status='passed', cases=len(seen), raw_target_matches=len(seen),
                score_max_error=maximum, performance_status=performance['performance_status'],
                manifest_sha256=digest(out / 'manifest.json'))


def write_verification(out, result):
    (Path(out) / 'verification.json').write_text(json.dumps(result, indent=2) + '\n')
    return result
