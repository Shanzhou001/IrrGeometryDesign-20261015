"""Research pairing, immutable acceptance identity and spatial block contracts."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.analysis.applicability_report import paired_effects
from research.common.hashing import digest
from research.common.paths import ROOT
from research.experiments import spatial_replication as spatial
from research.experiments.confirmation_spatial import fk3d
from research.experiments.thesis_workflow import validate_retained
from research.validation.run_audit_checks import tracked_files


class CompletionTests(unittest.TestCase):
    def test_six_spatial_blocks_are_disjoint_and_stable_under_trace_order(self):
        plan=json.loads(spatial.PROTOCOL.read_text(encoding='utf-8'))
        y,x=np.indices((120,128));inline,crossline=y.ravel(),x.ravel()
        blocks=spatial.block_indices(inline,crossline,plan)
        self.assertEqual(len(blocks),6)
        self.assertEqual(len(set(i for b in blocks for i in b['indices'])),6*256)
        for b in blocks:self.assertEqual(len(b['indices']),256)
        order=np.random.default_rng(25).permutation(len(inline))
        reordered=spatial.block_indices(inline[order],crossline[order],plan)
        self.assertEqual([b['inlines'] for b in blocks],[b['inlines'] for b in reordered])
        self.assertEqual([b['crosslines'] for b in blocks],[b['crosslines'] for b in reordered])

    def test_missing_or_overlapping_frozen_blocks_are_rejected(self):
        plan=json.loads(spatial.PROTOCOL.read_text(encoding='utf-8'))
        y,x=np.indices((120,128));inline,crossline=y.ravel(),x.ravel()
        missing=spatial.block_indices(inline,crossline,plan)[0]['indices'][0]
        with self.assertRaisesRegex(ValueError,'missing traces'):
            spatial.block_indices(np.delete(inline,missing),np.delete(crossline,missing),plan)
        y,x=np.indices((20,20))
        with self.assertRaisesRegex(ValueError,'overlap'):spatial.block_indices(y.ravel(),x.ravel(),plan)

    def test_replication_protocol_cannot_change_after_freeze(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);protocol=root/'protocol.json';protocol.write_text('{"role":"fixture"}')
            with patch.object(spatial,'PROTOCOL',protocol):
                marker=root/'freeze.json';spatial.freeze(marker)
                protocol.write_text('{"role":"changed"}')
                with self.assertRaisesRegex(ValueError,'after freezing'):spatial.freeze(marker)

    def test_condition_effects_pair_before_aggregating_and_reject_duplicates(self):
        data=[dict(unit=u,method=m,score=100*u+(2 if m=='Hybrid' else 0),mu=.2)
              for u in (1,2) for m in ('Jitter','Hybrid')]
        paired=paired_effects(data,('unit',),'score')
        self.assertEqual([r['delta'] for r in paired if r['method']=='Hybrid'],[2,2])
        with self.assertRaisesRegex(ValueError,'Duplicate'):paired_effects(data+[data[0]],('unit',),'score')
        with self.assertRaisesRegex(ValueError,'Missing'):paired_effects([data[1]],('unit',),'score')

    def test_verification_rejects_a_changed_manifest_even_if_outputs_match(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);manifest=root/'manifest.json';payload=root/'result.csv';payload.write_text('verified')
            manifest.write_text(json.dumps(dict(completed=True,output_tables_sha256={'result.csv':digest(payload)})))
            marker=root/'verification.json';marker.write_text(json.dumps(dict(status='passed',manifest_sha256=digest(manifest))))
            validate_retained(marker)
            manifest.write_text('{"completed":true,"configuration":"changed"}')
            with self.assertRaisesRegex(ValueError,'another manifest'):validate_retained(marker)

    def test_acceptance_identity_includes_tests_build_configuration_and_launcher(self):
        actual={p.relative_to(ROOT).as_posix() for p in tracked_files()}
        for name in ('pipeline.py','build.bat','requirements.txt','src/tests/python/test_completion.py',
                     'src/build/native.bat','src/research/experiments/spatial_replication_protocol.json'):
            self.assertIn(name,actual)

    def test_spatial_solver_rejects_invalid_controls(self):
        data=np.ones((8,4,4));known=np.ones((4,4),bool)
        for options in (dict(iterations=0),dict(fraction=-.1),dict(fraction=1.1)):
            with self.assertRaises(ValueError):fk3d(data,known,**options)
