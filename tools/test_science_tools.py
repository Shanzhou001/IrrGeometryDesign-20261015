"""Numerical checks against analytic results and exhaustive sign permutations."""  # add by wushanzhou in 2026/10/23
import itertools  # add by wushanzhou in 2026/10/23
import math  # add by wushanzhou in 2026/10/23
import unittest  # add by wushanzhou in 2026/10/23
import tempfile  # add by wushanzhou in 2026/10/23
import struct  # add by wushanzhou in 2026/10/23
from pathlib import Path  # add by wushanzhou in 2026/10/23
from run_revised_studies import cache_fingerprint, reusable_step, sha256  # add by wushanzhou in 2026/10/23
from audit_change_annotations import annotate  # add by wushanzhou in 2026/10/23
from verify_revised_results import physical_metrics, coherence, matches_float32_export  # add by wushanzhou in 2026/10/24
# add by wushanzhou in 2026/10/23

import ablation_stats as stats  # add by wushanzhou in 2026/10/23
import numpy as np  # add by wushanzhou in 2026/10/23
import convert_rraw_to_engine_csv as conversion  # add by wushanzhou in 2026/10/23
from run_revised_studies import average_ranks  # add by wushanzhou in 2026/10/23


class StatisticsTests(unittest.TestCase):  # add by wushanzhou in 2026/10/23
    def test_t_cauchy_analytic(self):  # add by wushanzhou in 2026/10/23
        for value in (0.01, 0.5, 1.0, 2.0, 20.0):  # add by wushanzhou in 2026/10/23
            self.assertAlmostEqual(stats.student_t_two_sided(value, 1),  # add by wushanzhou in 2026/10/23
                                   1 - 2 * math.atan(value) / math.pi, places=12)  # add by wushanzhou in 2026/10/23

    def test_t_known_critical_values(self):  # add by wushanzhou in 2026/10/23
        self.assertAlmostEqual(stats.student_t_two_sided(2.093024054408263, 19), .05, places=11)  # add by wushanzhou in 2026/10/23
        self.assertAlmostEqual(stats.student_t_two_sided(12.706204736432095, 1), .05, places=11)  # add by wushanzhou in 2026/10/23

    def test_signed_rank_ties_matches_exhaustive_distribution(self):  # add by wushanzhou in 2026/10/23
        for values in ([1, 1, -1, 2], [2, -2, 3, 3, -4], [1, 2, 3, 4], [0, 1, -1]):  # add by wushanzhou in 2026/10/23
            diffs = [x for x in values if x]  # add by wushanzhou in 2026/10/23
            ranks = stats.rank_abs(diffs)  # add by wushanzhou in 2026/10/23
            wp = sum(r for r, d in zip(ranks, diffs) if d > 0)  # add by wushanzhou in 2026/10/23
            total = sum(ranks)  # add by wushanzhou in 2026/10/23
            threshold = min(wp, total - wp)  # add by wushanzhou in 2026/10/23
            count = sum(min(sum(r for r, sign in zip(ranks, signs) if sign),  # add by wushanzhou in 2026/10/23
                            sum(r for r, sign in zip(ranks, signs) if not sign)) <= threshold  # add by wushanzhou in 2026/10/23
                        for signs in itertools.product((False, True), repeat=len(ranks)))  # add by wushanzhou in 2026/10/23
            expected = count / 2 ** len(ranks)  # add by wushanzhou in 2026/10/23
            self.assertAlmostEqual(stats.wilcoxon_signed_rank(values, [0] * len(values))[2], expected)  # add by wushanzhou in 2026/10/23

    def test_degenerate_paired_differences(self):  # add by wushanzhou in 2026/10/23
        self.assertEqual(stats.paired_t([1, 1, 1], [0, 0, 0]), (math.inf, 0))  # add by wushanzhou in 2026/10/23
        self.assertEqual(stats.paired_t([1, 1, 1], [1, 1, 1]), (0, 1))  # add by wushanzhou in 2026/10/23
        self.assertEqual(stats.wilcoxon_signed_rank([1, 1], [1, 1]), (0, 0, 1, 0))  # add by wushanzhou in 2026/10/23

    def test_holm(self):  # add by wushanzhou in 2026/10/23
        actual = stats.holm([.01, .04, .03])  # add by wushanzhou in 2026/10/23
        for a, b in zip(actual, [.03, .06, .06]):  # add by wushanzhou in 2026/10/23
            self.assertAlmostEqual(a, b)  # add by wushanzhou in 2026/10/23

    def test_resampling_uses_physical_offsets(self):  # add by wushanzhou in 2026/10/23
        offsets = np.array([10, 0, 1])  # add by wushanzhou in 2026/10/23
        wave = np.sin(2 * np.pi * 25 * np.arange(250) * .008)  # add by wushanzhou in 2026/10/23
        gather = wave[:, None] * offsets[None, :]  # add by wushanzhou in 2026/10/23
        converted, _ = conversion.convert(gather, offsets)  # add by wushanzhou in 2026/10/23
        expected = np.linspace(0, 1, 128)  # add by wushanzhou in 2026/10/23
        np.testing.assert_allclose(np.max(np.abs(converted), axis=0), expected, atol=1e-10)  # add by wushanzhou in 2026/10/23

    def test_correlation_ranks_include_ties(self):  # add by wushanzhou in 2026/10/23
        np.testing.assert_array_equal(average_ranks([30, 10, 20, 20]), [4, 1, 2.5, 2.5])  # add by wushanzhou in 2026/10/23

    def test_cache_invalidation_and_missing_outputs(self):  # add by wushanzhou in 2026/10/23
        state = {key: {} for key in ("protocol", "binary_sha256", "runtime_versions", "tools_sha256", "source_sha256", "input_sha256")}  # add by wushanzhou in 2026/10/23
        before = cache_fingerprint(state)  # add by wushanzhou in 2026/10/23
        state["tools_sha256"] = {"statistics.py": "new version"}  # add by wushanzhou in 2026/10/23
        self.assertNotEqual(before, cache_fingerprint(state))  # add by wushanzhou in 2026/10/23
        with tempfile.TemporaryDirectory() as directory:  # add by wushanzhou in 2026/10/23
            root = Path(directory); artifact = root / "result.csv"; artifact.write_text("old")  # add by wushanzhou in 2026/10/23
            record = {"returncode": 0, "command": ["test"], "artifacts": {"result.csv": sha256(artifact)}}  # add by wushanzhou in 2026/10/23
            self.assertTrue(reusable_step(record, ["test"], root))  # add by wushanzhou in 2026/10/23
            self.assertFalse(reusable_step(record, ["changed"], root))  # add by wushanzhou in 2026/10/23
            artifact.write_text("changed")  # add by wushanzhou in 2026/10/23
            self.assertFalse(reusable_step(record, ["test"], root))  # add by wushanzhou in 2026/10/23
            artifact.unlink()  # add by wushanzhou in 2026/10/23
            self.assertFalse(reusable_step(record, ["test"], root))  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23

    def test_physical_dictionary_uniform_equivalence_and_gram(self):  # add by wushanzhou in 2026/10/23
        bits = np.array([1, 0, 0, 1, 0, 1, 0, 1])  # add by wushanzhou in 2026/10/23
        self.assertAlmostEqual(physical_metrics(bits, np.arange(8)*12.5)[0], coherence(bits))  # add by wushanzhou in 2026/10/23
        x = np.array([0, 1, 3, 4, 8, 9, 15, 20.])  # add by wushanzhou in 2026/10/23
        mu, gap, cv, period = physical_metrics(bits, x)  # add by wushanzhou in 2026/10/23
        dictionary = np.exp(2j*np.pi*np.outer(x[bits.astype(bool)], np.arange(8)/period))/np.sqrt(bits.sum())  # add by wushanzhou in 2026/10/23
        gram = dictionary.conj().T @ dictionary  # add by wushanzhou in 2026/10/23
        np.fill_diagonal(gram, 0)  # add by wushanzhou in 2026/10/23
        self.assertAlmostEqual(mu, np.abs(gram).max())  # add by wushanzhou in 2026/10/23
        self.assertEqual(gap, 11)  # add by wushanzhou in 2026/10/23
        self.assertAlmostEqual(cv, np.sqrt(8.5)/6)  # add by wushanzhou in 2026/10/23
        np.testing.assert_allclose(physical_metrics(bits, x*10+100)[:3], [mu, gap*10, cv])  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23

    def test_segy_rejects_truncation_and_nonfinite_samples(self):  # add by wushanzhou in 2026/10/23
        with tempfile.TemporaryDirectory() as directory:  # add by wushanzhou in 2026/10/23
            path = Path(directory) / "bad.sgy"  # add by wushanzhou in 2026/10/23
            path.write_bytes(b"short")  # add by wushanzhou in 2026/10/23
            with self.assertRaises(ValueError): conversion.read_segy(path)  # add by wushanzhou in 2026/10/23
            raw = bytearray(3600+240+8)  # add by wushanzhou in 2026/10/23
            struct.pack_into("<H", raw, 3220, 2)  # add by wushanzhou in 2026/10/23
            path.write_bytes(raw[:-1])  # add by wushanzhou in 2026/10/23
            with self.assertRaises(ValueError): conversion.read_segy(path)  # add by wushanzhou in 2026/10/23
            struct.pack_into("<f", raw, 3840, float("nan")); path.write_bytes(raw)  # add by wushanzhou in 2026/10/23
            with self.assertRaises(ValueError): conversion.read_segy(path)  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23


    def test_annotation_preserves_legacy_bytes_and_is_idempotent(self):  # add by wushanzhou in 2026/10/23
        legacy = "// legacy byte \x85 stays in its original comment\nint value=1;\n"  # add by wushanzhou in 2026/10/23
        self.assertEqual(annotate(Path("sample.h"), legacy, legacy)[0], legacy)  # add by wushanzhou in 2026/10/23
        for name, old, new in (("sample.py", "value=1\n", "value=2\n"),  # add by wushanzhou in 2026/10/23
                               ("build.bat", "exit /b 1\n", "exit /b 0\n")):  # add by wushanzhou in 2026/10/23
            once = annotate(Path(name), old, new)[0]  # add by wushanzhou in 2026/10/23
            self.assertEqual(annotate(Path(name), old, once)[0], once)  # add by wushanzhou in 2026/10/23
    # add by wushanzhou in 2026/10/23

        previous = "exit /b 0\nrem add by wushanzhou in 2026/10/23\n"  # add by wushanzhou in 2026/10/24
        self.assertEqual(annotate(Path("build.bat"), "exit /b 1\n", previous)[0], previous)  # add by wushanzhou in 2026/10/24

        previous_doc = '"""Updated\nmodule\n"""  # add by wushanzhou in 2026/10/23\n'  # add by wushanzhou in 2026/10/24
        old_doc = '"""Before\nmodule\n"""\n'  # add by wushanzhou in 2026/10/24
        self.assertEqual(annotate(Path("sample.py"), old_doc, previous_doc)[0], previous_doc)  # add by wushanzhou in 2026/10/24

    def test_float_export_comparison_uses_recorded_precision(self):  # add by wushanzhou in 2026/10/24
        value = 1.0291332542045954  # add by wushanzhou in 2026/10/24
        self.assertGreater(abs(value-1.02913), 2e-6)  # add by wushanzhou in 2026/10/24
        self.assertTrue(matches_float32_export(value, "1.02913"))  # add by wushanzhou in 2026/10/24
        self.assertFalse(matches_float32_export(value, "1.02914"))  # add by wushanzhou in 2026/10/24
        self.assertTrue(matches_float32_export(1.086376656922328, "1.08638"))  # add by wushanzhou in 2026/10/24

if __name__ == '__main__':  # add by wushanzhou in 2026/10/23
    unittest.main()  # add by wushanzhou in 2026/10/23
