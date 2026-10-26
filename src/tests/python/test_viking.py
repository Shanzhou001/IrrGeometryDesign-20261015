"""Independent byte, hidden-data isolation and single-survey analysis contracts."""
import argparse
import csv
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.analysis import viking_results as analysis
from research.common.hashing import digest
from research.common.paths import ROOT, executable
from research.common.tabular import rows, write_rows
from research.data import viking
from research.experiments import audit_joint_studies as joint
from research.experiments import viking_study as study
from research.experiments.review_studies import read_gather


def dataset_fixture(root):
    """Analytic IBM samples, not measured data or replacement experimental evidence."""
    for folder in ('raw', 'csv_256x120', 'geometry', 'source_docs'):
        (root/folder).mkdir()
    raw = bytearray(3600+120*6240)
    for offset, value in [(3216, 4000), (3220, 1500), (3224, 1), (3254, 1)]:
        struct.pack_into('>H', raw, offset, value)
    words = np.array([0x41100000, 0xc1200000, 0x40800000, 0x42100000], dtype='>u4')
    truth = np.tile([1., -2., .5, 16.], (375, 1)).ravel()
    truth = np.column_stack([np.roll(truth, j) for j in range(120)])
    geometry = []
    for j in range(120):
        start = 3600+j*6240
        for offset, value, fmt in [(8, 53, 'i'), (12, j+1, 'i'), (36, j*25-3237, 'i'),
                                   (70, 1, 'h'), (72, 3237, 'i'), (80, j*25, 'i'), (88, 3, 'h'),
                                   (114, 1500, 'H'), (116, 4000, 'H')]:
            struct.pack_into('>'+fmt, raw, start+offset, value)
        raw[start+240:start+6240] = np.roll(np.tile(words, 375), j).astype('>u4').tobytes()
        geometry.append(dict(output_column_zero_based=j, original_channel=j+1, source_x_m=3237,
                             receiver_x_m=j*25, offset_header_m=j*25-3237, coordinate_scalar_raw=1, coordinate_units_raw=3))
    (root/'raw/shot.sgy').write_bytes(raw)
    x = np.arange(120)*25.
    joint.write_gather(root/'csv_256x120/shot.csv', truth[512:768], x, 'analytic test fixture only')
    write_rows(root/'geometry/shot.csv', geometry)
    (root/'source_docs/README.md').write_text('Artificial test source, not field evidence')
    names = ['raw/shot.sgy', 'csv_256x120/shot.csv', 'geometry/shot.csv']
    registry = dict(reference_record='test fixture', citation='test fixture', original_format='IBM_BE',
                    coordinate_distance_scale_evidence='analytic 25 m fixture', coordinate_units_header_note='raw code 3 retained',
                    protocol='viking_line12_subset_v1', source_docs={'README.md': {'sha256': digest(root/'source_docs/README.md')}},
                    shots=[dict(ffid=53, source_x_m=3237, files={name: dict(bytes=(root/name).stat().st_size,
                                                                                 sha256=digest(root/name)) for name in names})])
    registry_path = root/'registry.json'
    registry_path.write_text(json.dumps(registry))
    return truth, registry_path


class VikingDatasetTests(unittest.TestCase):
    def test_big_endian_ibm_and_all_120_measured_columns_are_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); truth, registry = dataset_fixture(root)
            with patch.object(viking, 'REGISTRY', registry):
                shots, report = viking.load_dataset(root)
            np.testing.assert_array_equal(shots[0].data, truth[512:768])
            self.assertEqual(shots[0].data.shape, (256, 120))
            self.assertEqual(shots[0].sample_interval_s, .004)
            self.assertEqual(report['independent_surveys'], 1)
            self.assertFalse(report['full_file_sha256_locally_verified'])
            self.assertIn('header_caveat', report['status'])

    def test_wrong_encoding_header_and_mixed_shots_are_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); dataset_fixture(root); path = root/'raw/shot.sgy'; original = path.read_bytes()
            for offset, value in [(3224, 5), (3600+6240+8, 54)]:
                raw = bytearray(original)
                struct.pack_into('>H' if offset == 3224 else '>i', raw, offset, value)
                path.write_bytes(raw)
                with self.assertRaises(ValueError):
                    viking.decode_shot(path, 53)
            path.write_bytes(original[:-1])
            with self.assertRaisesRegex(ValueError, 'layout'):
                viking.decode_shot(path, 53)

    def test_local_manifest_cannot_bless_changed_csv(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); _, registry = dataset_fixture(root)
            changed = root/'csv_256x120/shot.csv'
            changed.write_bytes(changed.read_bytes()+b'\n')
            (root/'manifest.json').write_text(json.dumps({'sha256': digest(changed)}))
            with patch.object(viking, 'REGISTRY', registry):
                with self.assertRaisesRegex(ValueError, 'identity changed'):
                    viking.load_dataset(root)

    def test_csv_with_new_hash_still_must_match_raw_samples(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); _, registry = dataset_fixture(root)
            path = root/'csv_256x120/shot.csv'; data, x = read_gather(path)
            joint.write_gather(path, data*2, x, 'artificial changed fixture')
            meta = json.loads(registry.read_text())
            meta['shots'][0]['files']['csv_256x120/shot.csv'] = dict(bytes=path.stat().st_size, sha256=digest(path))
            registry.write_text(json.dumps(meta))
            with patch.object(viking, 'REGISTRY', registry):
                with self.assertRaisesRegex(ValueError, 'raw crop'):
                    viking.load_dataset(root)

    def test_all_missing_inputs_reported_together(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); _, registry = dataset_fixture(root)
            (root/'raw/shot.sgy').unlink(); (root/'geometry/shot.csv').unlink()
            with patch.object(viking, 'REGISTRY', registry):
                with self.assertRaises(FileNotFoundError) as error:
                    viking.load_dataset(root)
            self.assertIn('raw', str(error.exception)); self.assertIn('geometry', str(error.exception))


class VikingIsolationTests(unittest.TestCase):
    def test_nested_selection_and_serialized_inputs_ignore_outer_truth(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            truth = np.sin(np.arange(64)[:, None]*.13 + np.arange(120)[None, :]*.07)
            x = np.arange(120)*25.; hidden = study.hidden_mask(120)
            mask = joint.initial_mask(120, 60, 51, ~hidden)
            iterations = []
            class Recorder:
                def __init__(self):
                    self.out = root; self.state = {'protocol': study.PROTOCOL}
                def call(self, key, args, destination):
                    observed, _ = read_gather(args[1]); known = np.loadtxt(args[args.index('--mask')+1]).astype(bool)
                    np.testing.assert_allclose(observed[:, known], truth[:, known], atol=1e-11)
                    np.testing.assert_array_equal(observed[:, ~known], 0)
                    iterations.append(args[args.index('--iter')+1])
                    prediction = np.stack([np.interp(x, x[known], row[known]) for row in observed])
                    destination.mkdir(parents=True)
                    np.savetxt(destination/'prediction.csv', prediction, delimiter=',')
            recorder = Recorder()
            a = joint.nested(recorder, 'original', truth, x, mask, 9, iterations=7)
            changed = truth.copy(); changed[:, hidden] = 13*changed[:, hidden]+7
            b = joint.nested(recorder, 'intervention', changed, x, mask, 9, iterations=7)
            self.assertEqual(a[3], b[3])
            np.testing.assert_array_equal(a[0], b[0]); np.testing.assert_array_equal(a[1], b[1])
            self.assertTrue(all(value == 7 for value in iterations))

    def test_existing_output_is_rejected_before_loading_data(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); sentinel = root/'keep.txt'; sentinel.write_text('original')
            with patch.object(study, 'load_dataset') as load:
                with self.assertRaises(FileExistsError):
                    study.run(argparse.Namespace(out=root))
            load.assert_not_called(); self.assertEqual(sentinel.read_text(), 'original')

    def test_viking_shortcut_is_available_outside_project(self):
        with tempfile.TemporaryDirectory() as directory:
            result = subprocess.run([sys.executable, '-B', str(ROOT/'pipeline.py'), 'viking', '--help'],
                                    cwd=directory, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertIn(b'--validate-only', result.stdout)


@unittest.skipUnless(executable().is_file(), 'build the Release application first')
class NativeVikingTests(unittest.TestCase):
    def test_120_column_search_obeys_hidden_set_and_explicit_fourier_scores(self):
        with tempfile.TemporaryDirectory() as directory:
            recorder = joint.Recorder(Path(directory)/'run', executable(), {}, protocol=study.PROTOCOL)
            records = study.search(recorder, np.arange(120)*25., [36, 60, 84], [17, 29], 20)
            self.assertEqual(len(records), 30)
            for row in records:
                self.assertEqual(row['mask'].count('1'), row['m'])
                self.assertFalse(any(a == b == '1' for a, b in zip(row['mask'], row['evaluation_bits'])))
            recorder.finish()

    def test_real_native_solver_and_selection_are_invariant_to_hidden_intervention(self):
        with tempfile.TemporaryDirectory() as directory:
            recorder = joint.Recorder(Path(directory)/'run', executable(), {}, protocol=study.PROTOCOL)
            x = np.arange(120)*25.
            truth = np.sin(np.arange(64)[:, None]*.13 + np.arange(120)[None, :]*.07)
            shot = viking.Shot(53, truth, x, .004, 2.048)
            mask = joint.initial_mask(120, 60, 17, ~study.hidden_mask(120))
            row = dict(id='test', method='Jitter', seed=17, m=60, mask=study.bits(mask), mu=.3, evals=1)
            study.evaluate_case(recorder, shot, row, iterations=2)
            report = study.verify_isolation(recorder, shot, row, iterations=2)
            self.assertTrue(report['passed'])
            recorder.finish()


def analysis_fixture(root):
    """Synthetic result tables used only to test schema and aggregation, never as field results."""
    config = dict(n=120, counts=[36, 60], seeds=[11, 22], budget=20, strict_preprocessing_isolation=True,
                  model_selection_candidates=['zero', 'linear', 'default', 'cold'],
                  dataset_provenance=dict(independent_surveys=1, selected_ffids=[53, 153],
                                          status='documented_public_subset_with_header_caveat', coordinate_units_header_note='fixture code 3'))
    search, recovery, outputs = [], [], {}
    (root/'search').mkdir()
    hidden = study.hidden_mask(120)
    for m in config['counts']:
        mask = study.bits(joint.initial_mask(120, m, 17, ~hidden))
        for seed in config['seeds']:
            for index, method in enumerate(study.METHODS):
                key = f'm{m}_s{seed}_{method}'; mu = .6-index*.01+seed*.0001
                trace = [dict(best=mu+.1), dict(best=mu)] if method != 'Jitter' else [dict(best=mu)]
                path = root/'search'/(key+'.trace.csv'); write_rows(path, trace)
                outputs[path.relative_to(root).as_posix()] = digest(path)
                search.append(dict(id=key, m=m, seed=seed, method=method, mask=mask, initial_mask=mask,
                                   evaluation_bits=study.bits(hidden), budget_cap=20, mu=mu, evals=len(trace)))
                for ffid in config['dataset_provenance']['selected_ffids']:
                    record = dict(ffid=ffid, search_id=key, m=m, seed=seed, method=method, mask=mask,
                                  evaluation_bits=study.bits(hidden), n=120, independent_surveys=1,
                                  sample_interval_s=.004, time_origin_s=2.048, selected='linear', gain=.5)
                    for name in ('zero', 'linear', 'fixed', 'selected'):
                        snr = 0. if name == 'zero' else -10+index*.1+ffid*.001+seed*.0001
                        record[name+'_snr'] = snr; record[name+'_nmse'] = 10**(-snr/10)
                    recovery.append(record)
    write_rows(root/'search.csv', search); write_rows(root/'recovery.csv', recovery)
    (root/'dataset_audit.json').write_text(json.dumps(config['dataset_provenance']))
    (root/'isolation.json').write_text(json.dumps(dict(passed=True)))
    manifest = dict(protocol=study.PROTOCOL, status='passed', completed=True, configuration=config,
                    dataset_audit_sha256=digest(root/'dataset_audit.json'), isolation_sha256=digest(root/'isolation.json'),
                    steps={'search': dict(exit_code=0, outputs=outputs)},
                    output_tables_sha256={p.name: digest(p) for p in (root/'search.csv', root/'recovery.csv')})
    (root/'manifest.json').write_text(json.dumps(manifest))


class VikingAnalysisTests(unittest.TestCase):
    def test_seeds_are_pooled_before_descriptive_single_survey_comparison(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); analysis_fixture(root)
            images = root/'images'; images.mkdir()
            with patch('research.common.figures.central_figures', return_value=images):
                state = analysis.run(root, root/'analysis', 'research/viking_fixture')
            means = rows(root/'analysis/recovery_means.csv')
            self.assertTrue(all(r['independent_surveys'] == '1' and r['shot_count'] == '2' for r in means))
            self.assertTrue(all('ci_low' not in r and 'p_holm' not in r for r in means))
            self.assertTrue(all(float(r['mean_snr']) < 0 for r in means if r['recovery'] != 'zero'))
            self.assertFalse(state['inferential_tests_performed'])
            self.assertTrue(all(r['mask_seed_count'] == '2' for r in rows(root/'analysis/shot_means.csv')))
            self.assertEqual(len(list(images.glob('*.png'))), 3)

    def test_changed_raw_table_is_rejected_before_any_analysis_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); analysis_fixture(root)
            with (root/'recovery.csv').open('a') as stream:
                stream.write('\n')
            with self.assertRaisesRegex(ValueError, 'changed raw artifact'):
                analysis.run(root, root/'analysis', 'research/viking_fixture')
            self.assertFalse((root/'analysis').exists())

    def test_missing_paired_recovery_case_is_rejected_even_with_matching_hash(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory); analysis_fixture(root)
            write_rows(root/'recovery.csv', rows(root/'recovery.csv')[:-1])
            manifest = json.loads((root/'manifest.json').read_text())
            manifest['output_tables_sha256']['recovery.csv'] = digest(root/'recovery.csv')
            (root/'manifest.json').write_text(json.dumps(manifest))
            with self.assertRaisesRegex(ValueError, 'each shot'):
                analysis.run(root, root/'analysis', 'research/viking_fixture')
            self.assertFalse((root/'analysis').exists())


if __name__ == '__main__':
    unittest.main()
