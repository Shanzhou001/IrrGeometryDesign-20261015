"""Validation status and audit records, isolated from compilers and native programs."""
from contextlib import ExitStack
import io
import json
from pathlib import Path
import sys
import tempfile
import zipfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import numpy as np

from research import pipeline
from research.validation import run_audit_checks as runner
from research.validation import pipeline_smoke, audit_projection
from research.validation import reproducibility_bundle as bundle
from research.common.hashing import digest


class ValidationRunnerTests(unittest.TestCase):
    def fixture(self, folder):
        root=Path(folder)/'project';workspace=Path(folder)/'workspace'
        source=root/'src/native/algorithm.cpp';source.parent.mkdir(parents=True);source.write_text('original')
        executable=workspace/'build/bin/Release/IrrGeometryDesign.exe'
        executable.parent.mkdir(parents=True);executable.write_bytes(b'fixture program')
        return root,workspace,source,executable

    def run_fixture(self, folder, *, full=False, legacy=False, python_only=False, skip=0, missing=False, launch_error=False,
                    source_change=None, summary=True, confirmed=True,legacy_skip=False):
        root,workspace,source,program=self.fixture(folder)
        out=Path(folder)/'checks'
        current_inputs=[workspace/'data/current/input.sgy']
        if (full or legacy) and not missing:
            for path in (current_inputs if full else runner.external_inputs(workspace)):
                path.parent.mkdir(parents=True,exist_ok=True);path.write_bytes(b'fixture input')
        calls=[]
        def run(command, **kwargs):
            name=Path(kwargs['stdout'].name).stem;calls.append(name)
            if name=='python' and summary:
                reason='external RRAW.SGY dataset is not installed' if legacy_skip else 'fixture required dependency unavailable'
                for index in range(skip):kwargs['stdout'].write((f'test_{index} (fixture.Tests) ... skipped \'{reason}\'\n').encode())
                kwargs['stdout'].write(('Ran 5 tests in 0.001s\n\nOK'+(f' (skipped={skip})' if skip else '')+'\n').encode())
            if name=='release_core':
                if source_change=='modified':source.write_text('changed')
                elif source_change=='deleted':source.unlink()
                elif source_change=='added':source.with_name('new.cpp').write_text('new')
                if launch_error:raise FileNotFoundError('native launch fixture failure')
            return SimpleNamespace(returncode=0)
        with ExitStack() as stack:
            stack.enter_context(patch.object(runner,'ROOT',root));stack.enter_context(patch.object(runner,'WORKSPACE',workspace))
            stack.enter_context(patch.object(runner,'source_files',side_effect=lambda:list((root/'src/native').glob('*.cpp'))))
            stack.enter_context(patch.object(runner,'project_inputs',return_value=current_inputs))
            stack.enter_context(patch.object(runner,'executable',return_value=program))
            stack.enter_context(patch.object(runner,'native_environment',return_value={}))
            stack.enter_context(patch.object(runner.subprocess,'run',side_effect=run))
            stack.enter_context(patch.object(runner,'real_source_provenance',return_value=dict(status='declared_confirmed' if confirmed else 'unconfirmed_sensitivity',field_claims_eligible=confirmed)))
            stack.enter_context(patch('sys.stdout',new=io.StringIO()))
            try:runner.checks(out,full=full,legacy=legacy,python_only=python_only,modes=('Release',))
            except RuntimeError:pass
        return json.loads((out/'checks.json').read_text()),calls

    def test_core_records_python_skips_without_claiming_external_coverage(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,skip=2)
        self.assertEqual(state['status'],'passed');self.assertEqual(state['scope'],'core')
        tests=state['records'][0]['tests']
        self.assertEqual((tests['run'],tests['passed'],tests['skipped']),(5,3,2))
        self.assertTrue(state['excluded']);self.assertNotIn('release_integration',calls)

    def test_legacy_requires_zero_python_skips(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,legacy=True,skip=1,legacy_skip=True)
        self.assertEqual(state['status'],'incomplete')
        self.assertEqual(state['records'][0]['status'],'incomplete')
        self.assertIn('release_integration',calls)

    def test_legacy_with_all_inputs_confirmed_and_no_skips_passes(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,legacy=True)
        self.assertEqual(state['status'],'passed');self.assertIn('release_integration',calls)

    def test_legacy_missing_inputs_is_incomplete_and_records_every_missing_path(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,legacy=True,missing=True)
        self.assertEqual(state['status'],'incomplete');self.assertEqual(len(state['missing_external_inputs']),6)
        self.assertNotIn('release_integration',calls)

    def test_unconfirmed_sensitivity_run_cannot_pass_full_validation(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,legacy=True,confirmed=False)
        self.assertEqual(state['status'],'incomplete');self.assertIn('unconfirmed',state['external_preflight_error'])

    def test_current_project_allows_only_declared_optional_legacy_skips(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,full=True,skip=2,legacy_skip=True)
        self.assertEqual(state['status'],'passed');self.assertEqual(state['scope'],'project')
        self.assertIn('release_registered_data',calls);self.assertIn('sps_integration',calls)
        self.assertNotIn('release_integration',calls)
        self.assertTrue(all(item['optional_legacy'] for item in state['records'][0]['skips']))

    def test_current_project_required_skip_or_missing_inputs_is_incomplete(self):
        with tempfile.TemporaryDirectory() as folder:state,_=self.run_fixture(folder,full=True,skip=1)
        self.assertEqual(state['status'],'incomplete')
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,full=True,missing=True)
        self.assertEqual(state['status'],'incomplete');self.assertEqual(len(state['missing_project_inputs']),1)
        self.assertNotIn('release_registered_data',calls);self.assertNotIn('sps_integration',calls)

    def test_python_only_records_exclusions_and_does_not_claim_native_or_data_coverage(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,python_only=True,skip=2)
        self.assertEqual(calls,['python']);self.assertEqual(state['scope'],'python')
        self.assertEqual(state['status'],'passed');self.assertEqual(len(state['excluded']),3)

    def test_launch_failure_is_preserved_in_completed_audit_record(self):
        with tempfile.TemporaryDirectory() as folder:state,calls=self.run_fixture(folder,launch_error=True)
        self.assertEqual(state['status'],'failed');self.assertIn('completed_utc',state)
        record=next(r for r in state['records'] if r['name']=='release_core')
        self.assertIsNone(record['exit_code']);self.assertIn('native launch fixture failure',record['error'])

    def test_zero_exit_without_python_summary_is_not_a_pass(self):
        with tempfile.TemporaryDirectory() as folder:state,_=self.run_fixture(folder,summary=False)
        self.assertEqual(state['status'],'failed');self.assertIsNone(state['records'][0]['tests'])

    def test_source_addition_modification_and_deletion_are_detected(self):
        for change in ['added','modified','deleted']:
            with self.subTest(change=change),tempfile.TemporaryDirectory() as folder:
                state,_=self.run_fixture(folder,source_change=change)
            self.assertEqual(state['status'],'failed');self.assertEqual(len(state['changed_sources']),1)

    def test_failed_unittest_footer_keeps_error_and_skip_counts(self):
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'python.log'
            path.write_text('Ran 9 tests in 0.001s\n\nFAILED (failures=1, errors=2, skipped=3)\n')
            result=runner.unittest_summary(path)
        self.assertEqual((result['run'],result['passed'],result['failures'],result['errors'],result['skipped']),(9,3,1,2,3))


class ValidationEntrypointTests(unittest.TestCase):
    def test_check_forwards_direct_options_and_legacy_separator(self):
        for separator in [[],['--']]:
            with self.subTest(separator=separator),tempfile.TemporaryDirectory() as folder:
                arguments=['pipeline.py','check',*separator,'--out','checks/fixture','--mode','Release']
                with patch.object(sys,'argv',arguments),patch.object(pipeline,'WORKSPACE',Path(folder)),patch.object(pipeline,'native_environment',return_value={}),patch.object(pipeline.subprocess,'call',return_value=0) as call:
                    with self.assertRaises(SystemExit) as error:pipeline.main()
                self.assertEqual(error.exception.code,0)
                self.assertEqual(call.call_args.args[0][2:],['-m','research.validation.run_audit_checks','--out','checks/fixture','--mode','Release'])

    def test_unknown_options_on_other_commands_still_fail(self):
        with patch.object(sys,'argv',['pipeline.py','test','--unrecognized']),patch('sys.stderr',new=io.StringIO()),patch.object(pipeline.subprocess,'call') as call:
            with self.assertRaises(SystemExit) as error:pipeline.main()
        self.assertEqual(error.exception.code,2);call.assert_not_called()

    def test_projection_missing_program_fails_before_creating_output(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder)/'out'
            with self.assertRaises(FileNotFoundError):audit_projection.run(out,Path(folder)/'missing.exe')
            self.assertFalse(out.exists())

    def test_smoke_default_has_no_real_data_dependency(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder)/'out';program=Path(folder)/'fixture.exe';program.write_bytes(b'fixture')
            data=np.sin(np.arange(64)[:,None]*.13+np.arange(128)[None,:]*.27);x=np.arange(128.)
            class Recorder:
                def __init__(self,out,exe,configuration):self.out=out;out.mkdir();self.state=dict(configuration=configuration)
                def save(self):pass
                def finish(self):pass
            def queries(recorder,name,lines):
                return [dict(mask=lines[0].split(';')[12])]
            def nested(recorder,key,truth,x,mask,seed):
                return truth.copy(),truth.copy(),np.zeros_like(truth),dict(selected=dict(candidate='default'))
            with patch.object(pipeline_smoke,'Recorder',Recorder),patch.object(pipeline_smoke,'synthetic',return_value=[(data,x,{})]),patch.object(pipeline_smoke,'native_queries',side_effect=queries),patch.object(pipeline_smoke,'nested',side_effect=nested),patch.object(pipeline_smoke,'real_source_provenance',side_effect=AssertionError('unexpected real input')),patch('sys.stdout',new=io.StringIO()):
                pipeline_smoke.run(out,program)
            report=json.loads((out/'verification.json').read_text())
            self.assertTrue(report['passed']);self.assertEqual(report['real_source_provenance']['status'],'not_used')
            self.assertEqual(len(report['records']),1)


class ReproductionBundleTests(unittest.TestCase):
    def fixture(self,folder):
        root=Path(folder)/'project';dataset=root/'data/public/viking_line12_small'
        for name,content in [('source_docs/TERMS.txt','original terms'),('raw/shot.sgy','registered input')]:
            path=dataset/name;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(content)
        registry=dict(source_docs={'TERMS.txt':dict(sha256=digest(dataset/'source_docs/TERMS.txt'))},
                      shots=[dict(files={'raw/shot.sgy':dict(sha256=digest(dataset/'raw/shot.sgy'))})])
        path=root/'src/research/data/viking_line12_subset.json';path.parent.mkdir(parents=True);path.write_text(json.dumps(registry))
        (root/'src/research/example.py').write_text('pass\n')
        run=root/'results/current';(run/'native/large').mkdir(parents=True)
        (run/'manifest.json').write_text(json.dumps(dict(status='passed',completed=True,executable_sha256='fixture')))
        (run/'recovery.csv').write_text('value\n1\n');(run/'native/large/prediction.csv').write_bytes(b'bulk'*100)
        return root,run,dataset

    def test_compact_package_verifies_and_excludes_bulk_predictions(self):
        with tempfile.TemporaryDirectory() as folder:
            root,run,_=self.fixture(folder);out=Path(folder)/'review.zip'
            with patch.object(bundle.subprocess,'check_output',side_effect=OSError('no git fixture')):
                report=bundle.create(out,run,project=root)
            self.assertEqual(report['status'],'passed')
            with zipfile.ZipFile(out) as archive:
                self.assertIn('results/current/recovery.csv',archive.namelist())
                self.assertNotIn('results/current/native/large/prediction.csv',archive.namelist())
                state=json.loads(archive.read(bundle.MANIFEST))
                self.assertFalse(state['availability']['compiled_runtime_included'])
                self.assertIn('Original notices',state['rights_note'])
            self.assertTrue(out.with_suffix('.zip.sha256').is_file())
            with self.assertRaises(FileExistsError):bundle.create(out,project=root)

    def test_changed_registered_input_is_rejected_before_writing_archive(self):
        with tempfile.TemporaryDirectory() as folder:
            root,_,dataset=self.fixture(folder);out=Path(folder)/'invalid.zip'
            (dataset/'raw/shot.sgy').write_text('changed')
            with self.assertRaises(ValueError):bundle.create(out,project=root)
            self.assertFalse(out.exists())

    def test_verify_rejects_tampering_extra_members_and_unsafe_paths(self):
        with tempfile.TemporaryDirectory() as folder:
            root,_,_=self.fixture(folder);original=Path(folder)/'valid.zip'
            with patch.object(bundle.subprocess,'check_output',side_effect=OSError('no git fixture')):
                bundle.create(original,project=root)
            for extra in ['../outside','unexpected.txt']:
                modified=Path(folder)/('unsafe.zip' if '..' in extra else 'extra.zip')
                modified.write_bytes(original.read_bytes())
                with zipfile.ZipFile(modified,'a') as archive:archive.writestr(extra,'unexpected')
                with self.assertRaises(ValueError):bundle.verify(modified)
            corrupt=Path(folder)/'corrupt.zip'
            with zipfile.ZipFile(original) as source,zipfile.ZipFile(corrupt,'w') as target:
                for name in source.namelist():
                    target.writestr(name,b'wrong' if name.endswith('example.py') else source.read(name))
            with self.assertRaises(ValueError):bundle.verify(corrupt)


if __name__=='__main__':unittest.main()
