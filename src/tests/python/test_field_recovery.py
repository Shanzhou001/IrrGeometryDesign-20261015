"""Regressions for the surveyed-geometry performance stage."""
import unittest

import numpy as np

from research.data.mapped_shots import receiver_span
from research.experiments.field_layouts import constrained_layouts, spatial_cells
from research.recovery_geometry import fit, predictors


class FieldRecoveryTests(unittest.TestCase):
    def test_sparse_xps_span_uses_registered_rps_stations(self):
        points = [6059, 6060, 6061, 6062, 6064, 6068, 6071]
        self.assertEqual(receiver_span(points, 6064, 6068, 2), [6064, 6068])
        self.assertEqual(receiver_span(points, 6068, 6064, 2), [6068, 6064])
        with self.assertRaises(ValueError):
            receiver_span(points, 6064, 6068, 5)

    def test_layouts_share_physical_constraints_and_budget(self):
        xy = np.asarray([(x * 25., y * 60. + x * .5) for y in range(6) for x in range(8)])
        hidden = np.arange(len(xy)) % 7 == 0
        cells = spatial_cells(xy, np.flatnonzero(~hidden), 17)
        layouts = list(constrained_layouts(xy, hidden, 17, 21, 40, 16))
        self.assertEqual(len(layouts), 4)
        for method, known, metrics in layouts:
            self.assertEqual(known.sum(), 17)
            self.assertFalse(np.any(known & hidden))
            self.assertTrue(all(known[cell].sum() == 1 for cell in cells))
            self.assertEqual(metrics['evaluations'], 1 if method == 'Jitter' else 40)
            self.assertLessEqual(metrics['max_nearest_distance'], metrics['cell_cover_bound'])

    def test_predictor_rejects_full_data_with_hidden_columns(self):
        xy = np.column_stack((np.arange(16.) * 25, np.zeros(16)))
        known = np.arange(16) % 2 == 0
        data = np.ones((128, 16))
        with self.assertRaises(ValueError):
            predictors(data, xy, xy[0], known, ~known, 8, 32, .002)
        features, baseline, _ = predictors(data[:, known], xy, xy[0], known, ~known, 8, 32, .002)
        np.testing.assert_allclose(baseline, 1.)
        self.assertEqual(features.shape[1:], (24, 8))

    def test_unpredictable_development_falls_back_to_zero(self):
        model = fit([(np.eye(2), np.zeros(2), np.array(1.))] * 3)
        self.assertEqual(model['weights'], [0., 0.])
