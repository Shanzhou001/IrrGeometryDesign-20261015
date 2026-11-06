"""Portable dependencies and source drift must not escape the release identity."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from research.experiments import thesis_workflow as workflow
from research.validation import reproduction_kit as kit,run_audit_checks as checks


class DeliveryTests(unittest.TestCase):
    def test_kit_includes_full_workflow_auditor_and_does_not_activate_partial_evidence(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);(root/'tools').mkdir();auditor=root/'tools/verify_f3_transfer.py';auditor.write_text('# required auditor')
            runtime=root/'runtime';runtime.mkdir();study=root/'study';study.mkdir()
            (study/'workflow.json').write_text('{"status":"passed"}')
            (study/'acceptance.json').write_text('{"status":"passed"}')
            packed=kit.members(root,runtime,study)
            self.assertEqual(packed['tools/verify_f3_transfer.py'],auditor)
            self.assertIn('evidence/acceptance.json',packed)
            self.assertNotIn('evidence/workflow.json',packed)
            self.assertFalse(any(name.startswith('results/') for name in packed))

    def test_resource_and_tool_changes_are_detected_at_workflow_completion(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);resources=('src/gui/main.ui','src/resources/plots.qrc','src/resources/gui.qm','tools/verify_f3_transfer.py')
            for name in resources:
                p=root/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_text('original')
            (root/'src/gui/private.user').write_text('personal settings')
            program=root/'engine.exe';program.write_bytes(b'program')
            with patch.object(checks,'ROOT',root),patch.object(workflow,'ROOT',root),patch.object(workflow,'executable',return_value=program):
                actual={p.relative_to(root).as_posix() for p in checks.tracked_files()}
                self.assertTrue(set(resources)<=actual);self.assertNotIn('src/gui/private.user',actual)
                initial=workflow.source_identity();workflow.require_identity(initial)
                (root/resources[0]).write_text('changed resource')
                with self.assertRaisesRegex(RuntimeError,'changed during'):workflow.require_identity(initial)
                (root/resources[0]).write_text('original')
                (root/'tools/new_auditor.py').write_text('# newly required script')
                with self.assertRaisesRegex(RuntimeError,'changed during'):workflow.require_identity(initial)

    def test_kit_rejects_runtime_from_another_program_before_creating_output(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);runtime=root/'runtime';runtime.mkdir()
            (runtime/'engine.exe').write_bytes(b'old program')
            (runtime/'runtime_manifest.json').write_text(json.dumps(dict(status='passed',program='engine.exe',files={})))
            program=root/'current.exe';program.write_bytes(b'current program')
            out=root/'output/kit'
            with patch.object(kit,'ROOT',root),patch.object(kit,'executable',return_value=program):
                with self.assertRaisesRegex(ValueError,'does not match'):kit.create(out,runtime,root/'study')
            self.assertFalse(out.exists())
