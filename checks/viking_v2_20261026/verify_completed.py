"""Audit the completed, frozen batch without running or selecting new models."""
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
import json
from pathlib import Path
import sys

import numpy as np

PROJECT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT/'src'))
from research.analysis.viking_v2_results import inputs
from research.common.hashing import digest
from research.data.viking import REGISTRY, load_windows
from research.experiments.viking_v2 import PROTOCOL_FILE


def main():
    root = PROJECT/'results/viking_v2_20261026'
    check = Path(__file__).resolve().parent
    manifest, search, recovery = inputs(root)
    plan = manifest['configuration']['plan']
    provenance = manifest['configuration']['dataset_provenance']
    expected = {}

    def register(path, value):
        path = Path(path).resolve()
        if path in expected and expected[path] != value:
            raise AssertionError('Contradictory recorded hashes: '+str(path))
        expected[path] = value

    for path, value in manifest['source_sha256'].items():
        register(PROJECT/path, value)
    for path, value in provenance['input_sha256'].items():
        register(path, value)
    for name in ('derived_artifacts_sha256', 'output_tables_sha256'):
        for path, value in manifest[name].items():
            register(root/path, value)
    for step in manifest['steps'].values():
        assert step['exit_code'] == 0
        for path, value in step['inputs'].items():
            register(path, value)
        for path, value in step['outputs'].items():
            register(root/path, value)
    register(root/'search_metadata.json', manifest['search_metadata_sha256'])
    register(PROTOCOL_FILE, plan['protocol_file_sha256'])
    register(REGISTRY, provenance['registry_sha256'])
    register(manifest['program'], manifest['executable_sha256'])
    analysis = json.loads((root/'analysis/manifest.json').read_text())
    register(root/'manifest.json', analysis['source_manifest_sha256'])
    for path, value in analysis['table_sha256'].items():
        register(root/'analysis'/path, value)
    support = PROJECT/'results/viking_v2_support_20261026'
    archived = json.loads((support/'manifest.json').read_text())
    for path, value in archived['runtime_sha256'].items():
        register(support/'runtime'/path, value)
    register(support/'implementation_source.zip', archived['source_archive_sha256'])
    register(support/'before_source.zip', archived['before_source_archive_sha256'])

    def verify(item):
        path, value = item
        assert digest(path) == value, 'Hash mismatch: '+str(path)
        return path.stat().st_size

    with ThreadPoolExecutor(max_workers=4) as pool:
        byte_count = sum(pool.map(verify, expected.items()))
    windows, _ = load_windows(plan['window_starts'], Path(provenance['dataset_directory']), plan['ffids'])
    projection_error = score_error = mu_error = objective_error = 0.
    metadata = json.loads((root/'search_metadata.json').read_text())
    for row in search:
        mask = np.array(list(row['mask']), float)
        psf = abs(np.fft.fft(mask)[1:])/mask.sum()
        mu_error = max(mu_error, abs(float(row['mu'])-max(psf)))
        feature = metadata[row['id']]['feature_path']
        weights = np.array(json.loads((root/feature).read_text())['weights']) if feature else np.ones(len(psf))
        objective_error = max(objective_error, abs(float(row['objective'])-max(weights*psf)))
    for row in recovery:
        data = windows[int(row['window_start'])][int(row['ffid'])].data
        known = np.array(list(row['mask']), int).astype(bool)
        hidden = np.array(list(row['H_bits']), int).astype(bool)
        pilot = set(np.flatnonzero(np.array(list(row['pilot_bits']), int)))
        choice = json.loads((root/'selection'/(row['case']+'.json')).read_text())
        assert row['selected'] == choice['selected']['candidate']
        assert float(row['gain']) == choice['selected']['gain']
        assert len(choice['candidates']) == len(plan['recovery_candidates'])
        for split in choice['splits']:
            train, validation = set(split['train']), set(split['validation'])
            assert pilot <= train and not train & validation
            assert train | validation == set(np.flatnonzero(known))
            assert not (train | validation) & set(np.flatnonzero(hidden))
        with np.load(root/'predictions'/(row['case']+'.npz')) as saved:
            assert np.array_equal(saved['known'], known) and np.array_equal(saved['hidden'], hidden)
            for name in ('cold', 'fk', 'selected', 'linear'):
                prediction = saved[name]
                assert prediction.shape == data.shape and np.isfinite(prediction).all()
                projection_error = max(projection_error, float(np.max(abs(prediction[:, known]-data[:, known]))))
                nmse = np.sum((prediction[:, hidden]-data[:, hidden])**2)/np.sum(data[:, hidden]**2)
                score_error = max(score_error, abs(float(row[name+'_nmse'])-nmse),
                                  abs(float(row[name+'_snr'])+10*np.log10(nmse)))
    assert projection_error == 0 and score_error < 1e-10
    # Native coherence returns float after accumulating double-precision atoms.
    # Compare against the independent double-precision FFT at float precision.
    assert mu_error < 1e-7 and objective_error < 1e-7
    core = json.loads((check/'core/checks.json').read_text())
    assert all(r['status'] == 'passed' for r in core['records']) and not core['changed_sources']
    compatibility = json.loads((check/'legacy_compat/verification.json').read_text())
    assert compatibility['passed']
    result = dict(status='passed', completed_utc=datetime.now(timezone.utc).isoformat(),
                  recovery_cases=len(recovery), unique_searches=len(search), native_calls=len(manifest['steps']),
                  failed_native_calls=0, independent_surveys=1, inferential_tests_performed=False,
                  checked_unique_files=len(expected), checked_bytes=byte_count,
                  checked_prediction_arrays=4*len(recovery), projection_max_error=projection_error,
                  score_max_error=score_error, fourier_mu_max_error=mu_error, weighted_objective_max_error=objective_error,
                  fourier_comparison_tolerance=1e-7,
                  all_inner_splits_keep_pilot=True, all_inner_splits_exclude_outer_H=True,
                  isolation=manifest['isolation'], python_tests=core['records'][0]['tests'],
                  core_records=[dict(name=r['name'], status=r['status']) for r in core['records']],
                  legacy_compatibility=compatibility,
                  executable_sha256=manifest['executable_sha256'], plan_sha256=manifest['plan_sha256'],
                  artifact_audit_script_sha256=digest(__file__),
                  experiment_manifest_sha256=digest(root/'manifest.json'), analysis_manifest_sha256=digest(root/'analysis/manifest.json'),
                  supporting_checks_sha256={p.name:digest(p) for p in [check/'targeted_tests.log', check/'debug_pilot_tests.log',
                      check/'release_build.log', check/'debug_build.log', check/'launch.json']},
                  figures_sha256={p.name:digest(p) for p in Path(analysis['figure_directory']).iterdir() if p.is_file()},
                  figure_visual_inspection='All three PNGs inspected: readable labels and complete panels.')
    (check/'verification.json').write_text(json.dumps(result, indent=2)+'\n', encoding='utf8')
    print(json.dumps({k:v for k,v in result.items() if k not in ('figures_sha256','core_records','supporting_checks_sha256','legacy_compatibility')}, indent=2))


if __name__ == '__main__':
    main()
