"""Numerical checks against analytic results and exhaustive sign permutations."""
import itertools
import math
import unittest

import ablation_stats as stats
import numpy as np
import convert_rraw_to_engine_csv as conversion
from run_revised_studies import average_ranks


class StatisticsTests(unittest.TestCase):
    def test_t_cauchy_analytic(self):
        for value in (0.01, 0.5, 1.0, 2.0, 20.0):
            self.assertAlmostEqual(stats.student_t_two_sided(value, 1),
                                   1 - 2 * math.atan(value) / math.pi, places=12)

    def test_t_known_critical_values(self):
        self.assertAlmostEqual(stats.student_t_two_sided(2.093024054408263, 19), .05, places=11)
        self.assertAlmostEqual(stats.student_t_two_sided(12.706204736432095, 1), .05, places=11)

    def test_signed_rank_ties_matches_exhaustive_distribution(self):
        for values in ([1, 1, -1, 2], [2, -2, 3, 3, -4], [1, 2, 3, 4], [0, 1, -1]):
            diffs = [x for x in values if x]
            ranks = stats.rank_abs(diffs)
            wp = sum(r for r, d in zip(ranks, diffs) if d > 0)
            total = sum(ranks)
            threshold = min(wp, total - wp)
            count = sum(min(sum(r for r, sign in zip(ranks, signs) if sign),
                            sum(r for r, sign in zip(ranks, signs) if not sign)) <= threshold
                        for signs in itertools.product((False, True), repeat=len(ranks)))
            expected = count / 2 ** len(ranks)
            self.assertAlmostEqual(stats.wilcoxon_signed_rank(values, [0] * len(values))[2], expected)

    def test_degenerate_paired_differences(self):
        self.assertEqual(stats.paired_t([1, 1, 1], [0, 0, 0]), (math.inf, 0))
        self.assertEqual(stats.paired_t([1, 1, 1], [1, 1, 1]), (0, 1))
        self.assertEqual(stats.wilcoxon_signed_rank([1, 1], [1, 1]), (0, 0, 1, 0))

    def test_holm(self):
        actual = stats.holm([.01, .04, .03])
        for a, b in zip(actual, [.03, .06, .06]):
            self.assertAlmostEqual(a, b)

    def test_resampling_uses_physical_offsets(self):
        offsets = np.array([10, 0, 1])
        wave = np.sin(2 * np.pi * 25 * np.arange(250) * .008)
        gather = wave[:, None] * offsets[None, :]
        converted, _ = conversion.convert(gather, offsets)
        expected = np.linspace(0, 1, 128)
        np.testing.assert_allclose(np.max(np.abs(converted), axis=0), expected, atol=1e-10)

    def test_correlation_ranks_include_ties(self):
        np.testing.assert_array_equal(average_ranks([30, 10, 20, 20]), [4, 1, 2.5, 2.5])


if __name__ == '__main__':
    unittest.main()
