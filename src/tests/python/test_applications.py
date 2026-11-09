"""Regressions for source-level statistics and portable application acceptance."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.analysis.application_report import interval, layout_statistics, sign_flip, source_statistics
from research.common.hashing import digest
from research.experiments.field_layouts import spatial_cells
from research.experiments.mapped_field import hidden_mask
from research.recovery_local import warps
from research.validation.field_artifacts import audit, check_manifest, coordinate_idw
from research.validation.project_closure import gui_record
from research.validation.source_compatibility import compare_sources, THESIS, PIPELINE


def write_json(path, value):
    path.write_text(json.dumps(value), encoding='utf-8')


class ApplicationTests(unittest.TestCase):
    def test_protocol_formatting_preserves_semantics_and_bound_file_hashes(self):
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary)
            registered=root/'registered.json'
            registered.write_text('{"budget":200,"seeds":[17,43]}')
            out=root/'study'
            out.mkdir()
            copied=out/'protocol.json'
            copied.write_text(json.dumps(json.loads(registered.read_text()),indent=2))
            manifest=dict(status='passed',protocol_source='registered.json',
                          protocol_sha256=digest(registered),input_sha256={},source_sha256={},
                          output_sha256={'protocol.json':digest(copied)})
            write_json(out/'manifest.json',manifest)
            with patch('research.validation.field_artifacts.ROOT',root):
                self.assertEqual(check_manifest(out)['budget'],200)
                copied.write_text('{"budget":201,"seeds":[17,43]}')
                manifest['output_sha256']['protocol.json']=digest(copied)
                write_json(out/'manifest.json',manifest)
                with self.assertRaises(ValueError):check_manifest(out)

    def test_local_delay_predictor_excludes_unobserved_columns(self):
        xy = np.column_stack((np.arange(20.), np.zeros(20)))
        known = np.arange(20) % 2 == 0
        with self.assertRaises(ValueError):
            warps(np.ones((32,20)),xy,known,~known,4,20,.002)

    def test_source_statistics_do_not_promote_layout_repeats_to_independent_sources(self):
        records = []
        for source in range(4):
            for method in ('Hybrid', 'Jitter', 'SA', 'RandomBudget'):
                for _ in range(20):
                    records.append(dict(source=source, method=method, nmse=.8 if method=='Hybrid' else 1.))
        result = layout_statistics({'fixture': records})
        self.assertTrue(all(r['sources'] == 4 for r in result))
        self.assertTrue(all(abs(r['exact_two_sided_p']-.125) < 1e-12 for r in result))
        json.dumps(dict(sources=source_statistics(records), comparisons=result), allow_nan=False)
        self.assertEqual(sign_flip([0., 0., 0., 0.]), 1.)
        mean, low, high = interval([1., 1., 1.])
        self.assertEqual((mean, low, high), (1., 1., 1.))

    def test_integration_retention_rejects_existing_calculation_changes(self):
        original = b"def run():\n    step('old', ['pipeline.py', 'old'], 'marker')\n"
        integrated = original + b"    step('field_applications', ['pipeline.py', 'applications'], 'new')\n"
        compare_sources({THESIS: original}, {THESIS: integrated})
        with self.assertRaises(ValueError):
            compare_sources({THESIS: original}, {THESIS: integrated.replace(b"'old'", b"'other'", 1)})
        scientific = 'src/research/recovery_geometry.py'
        with self.assertRaises(ValueError):
            compare_sources({scientific: b'RATE=1'}, {scientific: b'RATE=2'})
        before = b"def main():\n    names=('field',)\n    route={'field':'one'}\n"
        after = b"def main():\n    names=('field','applications')\n    route={'field':'one','applications':'two'}\n"
        compare_sources({PIPELINE: before}, {PIPELINE: after})
        with self.assertRaises(ValueError):
            compare_sources({PIPELINE: before}, {PIPELINE: after.replace(b"'one'", b"'bad'")})

    def test_prediction_audit_rejects_missing_cases_and_changed_raw_targets(self):
        with tempfile.TemporaryDirectory() as temporary:
            out = Path(temporary)
            xy = np.column_stack((np.arange(20.), np.zeros(20)))
            data = np.sin(np.arange(32.)[:,None]*.1+np.arange(20.)[None]*.02)+1
            hidden = hidden_mask(20, 17)
            cells = spatial_cells(xy, np.flatnonzero(~hidden), 8)
            known = np.zeros(20, bool)
            known[[c[0] for c in cells]] = True
            truth = data[4:20, hidden]
            baseline = coordinate_idw(data[:,known], xy, known, hidden, 4, 20)
            value = float(np.sum((baseline-truth)**2)/np.sum(truth**2))
            target = out/'prediction.npz'
            np.savez(target, xy=xy, known=known, hidden=hidden, truth=truth,
                     prediction=np.zeros_like(truth), idw=baseline)
            write_json(out/'protocol.json', {'minimum_positive_shots':1})
            write_json(out/'manifest.json', {'status':'passed'})
            write_json(out/'performance.json', dict(cases=1,mean_nmse=1.,mean_idw_nmse=value,
                                                   positive_shots=0,performance_status='failed'))
            key = (1,0,0,17,8,21,'Jitter')
            case = (key,target.name,digest(target),17,8,4,20,1.,value,{})
            self.assertEqual(audit(out,{(1,0):(xy,data)},[case],{key})['status'],'passed')
            with self.assertRaises(ValueError):
                audit(out,{(1,0):(xy,data)},[],{key})
            changed = data.copy()
            changed[4,hidden] += 2
            with self.assertRaises(ValueError):
                audit(out,{(1,0):(xy,changed)},[case],{key})
            invalid = (*case[:7], float('nan'), *case[8:])
            with self.assertRaises(ValueError):
                audit(out,{(1,0):(xy,data)},[invalid],{key})

    def test_qt_proof_survives_directory_relocation(self):
        with tempfile.TemporaryDirectory() as temporary:
            base = Path(temporary)
            evidence = base/'qt_evidence'
            evidence.mkdir()
            identity = dict(executable_sha256='program', source_sha256={'source':'value'})
            documents = {
                'workflow.json':dict(status='passed',scope='all',steps={'field_applications':{}}),
                'source_identity.json':identity,
                'gui_verification.json':dict(workflow_executed=True,images_loaded=19),
                'catalog.json':dict(image_count=19,reports=[{}]*7),
                'primary_verification':dict(status='passed'),
                'application_verification':dict(status='passed'),
            }
            for name,value in documents.items():write_json(evidence/name,value)
            (evidence/'primary_scores').write_text('fixture')
            (base/'field_recovery_v2').mkdir()
            (base/'field_recovery_v2/recovery.csv').write_text('fixture')
            names=('soda_followup','viking_transfer','gvda','gvda_confirmation')
            scores={}
            for name in names:
                folder=base/'field_applications'/name
                folder.mkdir(parents=True)
                (folder/'recovery.csv').write_text('fixture')
                scores[name]=digest(folder/'recovery.csv')
            proof=dict(status='passed',scope='all',exit_code=0,images_loaded=19,gallery_report_count=7,
                       program_sha256='program',execution_directory='Z:/unavailable/old/location',
                       evidence_directory='qt_evidence',
                       evidence_sha256={p.name:digest(p) for p in evidence.iterdir()},
                       application_scores_sha256=scores,workflow_sha256=digest(evidence/'workflow.json'),
                       source_identity_sha256=digest(evidence/'source_identity.json'))
            write_json(base/'field_recovery_gui.json',proof)
            with patch('research.experiments.thesis_workflow.source_identity',return_value=identity):
                self.assertEqual(gui_record(base)['status'],'passed')
                write_json(base/'project_current_gui.json',dict(proof,application_scores_sha256={}))
                self.assertEqual(gui_record(base)['status'],'failed')
                write_json(base/'project_current_gui.json',proof)
                (evidence/'primary_scores').write_text('modified')
                self.assertEqual(gui_record(base)['status'],'failed')
