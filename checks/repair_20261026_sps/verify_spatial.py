"""Recompute the exported SPS Fourier objectives using an explicit complex Gram."""
import csv
import hashlib
import json
from pathlib import Path

import numpy as np


def records(path):
    with path.open(encoding='utf-8-sig', newline='') as stream:
        return list(csv.DictReader(stream))


base = Path(__file__).resolve().parent
root = base/'real_sps/attribute_evaluation'
definition = json.loads((root/'sps_spatial_definition.json').read_text())
points = np.array([[float(row['x']), float(row['y'])]
                   for row in records(root/'sps_spatial_coordinates.csv')])
atoms = records(root/'sps_spatial_atoms.csv')
frequencies = np.array([[float(row['cycles_per_x_unit']), float(row['cycles_per_y_unit'])]
                        for row in atoms])
shifted = points - [definition['origin_x'], definition['origin_y']]
dictionary = np.exp(2j*np.pi*(shifted@frequencies.T))
checks = []
for row in records(root/'sps_spatial_sampling_comparison.csv'):
    mask = np.loadtxt(root/'masks'/('sps_'+row['method']+'.txt'), comments='#', dtype=int)
    assert set(mask) <= {0, 1} and mask.sum() == int(row['receivers'])
    selected = dictionary[mask.astype(bool)]
    gram = selected.conj().T@selected/mask.sum()
    np.fill_diagonal(gram, 0)
    value = float(abs(gram).max())
    error = abs(value-float(row['spatial_fourier_mu']))
    assert error < 1e-11
    assert hashlib.sha1(''.join(map(str, mask)).encode()).hexdigest() == row['mask_sha1']
    checks.append(dict(method=row['method'], independent_mu=value, max_abs_difference=error))
result = dict(passed=True, points=len(points), dictionary_atoms=len(atoms), checks=checks,
              generator_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
              scope='One recovered SPS geometry; no waveform-recovery or cross-survey claim')
(base/'independent_verification.json').write_text(json.dumps(result, indent=2), encoding='utf-8')
print(json.dumps(result))
