"""Regression cases for cold starts and false-positive scientific acceptance."""
import ctypes
import errno
import io
import json
import os
import threading
from pathlib import Path
import tempfile
import unittest
import zipfile
from unittest.mock import patch

import numpy as np

from research.analysis import catalog,gallery
from research.common.hashing import digest
from research.common.tabular import write_rows
from research.data import closure_holdout as data
from research.experiments import closure_validation as study
from research.experiments.audit_joint_studies import initial_mask,Recorder
from research.experiments.spatial_replication import masks
from research.experiments.thesis_workflow import validate_retained
from research.validation.reproduce import extract
from research.validation.artifacts import finite_arrays,finite_error
from research.validation import source_compatibility as compatibility
from research.validation import results_retention as retention


def fixture(out):
    (out/'truth').mkdir();(out/'predictions').mkdir()
    plan=dict(known_surveys=[],holdout={'id':'probe'},time_windows=[[0,2]],hidden_sets=['periodic'],counts=[76],seeds=[11031],methods=['Jitter'])
    hidden=masks(plan)['periodic'].reshape(16,16)
    known=initial_mask(256,76,11031,~hidden.ravel()).reshape(16,16)
    mask=''.join(map(str,known.astype(int).ravel()));rows=[]
    write_rows(out/'layouts.csv',[dict(layout='layout',mask=mask)])
    for y in range(3):
        for x in range(2):
            block=f'b{y}{x}';cube='probe_'+block;case=cube+'_layout';truth=np.ones((2,16,16))
            np.savez(out/'truth'/(cube+'.npz'),data=truth)
            np.savez(out/'predictions'/(case+'.npz'),prediction=truth,known=known,hidden=hidden)
            rows.append(dict(case=case,cube=cube,survey='probe',block=block,window_start=0,hidden_set='periodic',m=76,
                seed=11031,method='Jitter',layout='layout',isolation_error=0,nmse=0,snr=300,rms_ratio=1,cosine=1))
    write_rows(out/'recovery.csv',rows);(out/'sources.json').write_text('{}')
    (out/'native.csv').write_text('complete native output')
    state=dict(completed=True,status='passed',configuration={'plan':plan},input_sha256={},
        steps={'search':dict(exit_code=0,command=['native','--query'],inputs={},outputs={'native.csv':digest(out/'native.csv')})},
        derived_artifacts_sha256={p.relative_to(out).as_posix():digest(p) for folder in ('truth','predictions') for p in (out/folder).iterdir()},
        output_tables_sha256={name:digest(out/name) for name in ('layouts.csv','recovery.csv')},sources_sha256=digest(out/'sources.json'))
    (out/'manifest.json').write_text(json.dumps(state));return state


class HardeningTests(unittest.TestCase):
    def test_retention_prunes_other_editions_and_preserves_active_nested_results(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);base=root/'results';out=base/'runs/new';out.mkdir(parents=True)
            (out/'workflow.json').write_text('{"status":"passed"}')
            (out/'accepted_source_snapshot.zip').write_bytes(b'preexisting archive')
            for path in (base/'old/a.txt',base/'runs/older/a.txt'):
                path.parent.mkdir(parents=True,exist_ok=True);path.write_text('old result')
            (base/'workflow.lock').write_text('0')
            with patch.object(retention,'ROOT',root):retention.keep_latest(out)
            self.assertEqual({p.name for p in base.iterdir()},{'runs','active.txt','workflow.lock'})
            self.assertEqual({p.name for p in (base/'runs').iterdir()},{'new'})
            self.assertEqual((base/'active.txt').read_text().strip(),'results/runs/new')
            (out/'workflow.json').write_text('{"status":"failed"}')
            with patch.object(retention,'ROOT',root),self.assertRaises(ValueError):retention.keep_latest(out)

    @unittest.skipUnless(os.name=='nt','Windows junction semantics')
    def test_retention_removes_junction_without_deleting_external_target(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);base=root/'results';base.mkdir();outside=root/'outside';outside.mkdir()
            payload=outside/'keep.txt';payload.write_text('keep')
            link=base/'old'
            # The creation helper deliberately limits targets to results; use a
            # broader test-only root to exercise deletion of an external link.
            with patch.object(compatibility,'ROOT',root.parent):
                import subprocess
                result=subprocess.run(f'cmd.exe /d /v:off /c mklink /J "{link}" "{outside}"',capture_output=True)
                self.assertEqual(result.returncode,0)
            retention.remove_entry(link,base)
            self.assertTrue(payload.is_file());self.assertFalse(link.exists())
            with self.assertRaises(ValueError):retention.remove_entry(outside,base)
            with self.assertRaises(ValueError):retention.remove_entry(base/'old/..',base)

    def test_relocated_artifacts_still_require_original_sha256(self):
        from research.validation.artifacts import check_hash
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);out=root/'results/latest';out.mkdir(parents=True)
            (root/'results/active.txt').write_text('results/latest')
            (out/'calculation_compatibility.json').write_text(json.dumps({'relocated_roots':['results/old']}))
            actual=out/'raw.csv';actual.write_text('original')
            old=root/'results/old/raw.csv'
            with patch.object(retention,'ROOT',root):
                check_hash(old,digest(actual));actual.write_text('changed')
                with self.assertRaises(ValueError):check_hash(old,'0'*64)
                with self.assertRaises(ValueError):check_hash(root/'results/unregistered/raw.csv',digest(actual))

    def test_source_retention_only_allows_exact_post_success_cleanup(self):
        old=b'def run():\n try:\n  calculate()\n except: raise\n'
        new=old.replace(b' except:',b'  from research.validation.results_retention import keep_latest\n  keep_latest(out)\n except:')
        compatibility.compare_sources({compatibility.THESIS:old},{compatibility.THESIS:new})
        with self.assertRaises(ValueError):
            compatibility.compare_sources({compatibility.THESIS:old},{compatibility.THESIS:new.replace(b'calculate()',b'other_calculation()')})
        first=b'def check_hash(): pass\ndef finite_error(): return 1\n'
        with self.assertRaises(ValueError):
            compatibility.compare_sources({compatibility.ARTIFACTS:first},{compatibility.ARTIFACTS:first.replace(b'return 1',b'return 0')})

    def test_legacy_spatial_summary_requires_verified_parent(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);marker=out/'verification.json'
            summary={'status':'passed','score_max_error':0,'scope':'legacy spatial audit'}
            marker.write_text(json.dumps(summary))
            with self.assertRaises(ValueError):validate_retained(marker)
            fixture(out);marker.write_text(json.dumps(summary));validate_retained(marker)
            parent=json.loads((out/'manifest.json').read_text());parent['status']='failed'
            (out/'manifest.json').write_text(json.dumps(parent))
            with self.assertRaises(ValueError):validate_retained(marker)

    def test_legacy_spatial_summary_rejects_nonfinite_score(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);fixture(out);marker=out/'verification.json'
            marker.write_text(json.dumps({'status':'passed','score_max_error':'NaN','scope':'legacy spatial audit'}))
            with self.assertRaises(ValueError):validate_retained(marker)

    def test_source_retention_allows_only_acceptance_function_changes(self):
        before={compatibility.THESIS:b'def validate_retained(x): return 1\ndef source_identity(): return 2\ndef run(): return 3\n',
                'src/research/recovery.py':b'algorithm = 1\n'}
        after=dict(before);after[compatibility.THESIS]=before[compatibility.THESIS].replace(b'return 1',b'return 4')
        after[compatibility.SELF]=b'acceptance_only = True\n'
        result=compatibility.compare_sources(before,after)
        self.assertEqual(result['changed'],[compatibility.THESIS]);self.assertEqual(result['added'],[compatibility.SELF])
        after[compatibility.THESIS]=after[compatibility.THESIS].replace(b'return 3',b'return 5')
        with self.assertRaises(ValueError):compatibility.compare_sources(before,after)

    def test_source_retention_rejects_algorithm_protocol_analysis_and_inventory_changes(self):
        for name in ['src/research/recovery.py','src/research/experiments/closure_protocol.json',
                     'src/native/optimization/spatialpsf.h','src/research/analysis/closure_report.py']:
            before={name:b'original'}
            with self.assertRaises(ValueError):compatibility.compare_sources(before,{name:b'changed'})
            with self.assertRaises(ValueError):compatibility.compare_sources(before,{})
            with self.assertRaises(ValueError):compatibility.compare_sources(before,{name:b'original','src/research/extra.py':b'added'})

    def test_source_retention_rejects_changed_native_program(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);study=root/'results/old';study.mkdir(parents=True);out=root/'results/new';out.mkdir()
            (study/'source_identity.json').write_text(json.dumps({'executable_sha256':'0'*64,'source_sha256':{}}))
            with patch.object(compatibility,'ROOT',root),patch('research.experiments.thesis_workflow.source_identity',
                    return_value={'executable_sha256':'1'*64,'source_sha256':{}}):
                with self.assertRaisesRegex(ValueError,'Native program changed'):compatibility.prepare(study,out,'all')
            self.assertEqual(list(out.iterdir()),[])

    def test_source_archive_must_match_its_registered_inventory_and_bytes(self):
        with tempfile.TemporaryDirectory() as folder:
            archive=Path(folder)/'source.zip';identity={'source_sha256':{'file.py':digest(__file__)}}
            with zipfile.ZipFile(archive,'w') as zipped:
                zipped.writestr('SOURCE_IDENTITY.json',json.dumps(identity));zipped.writestr('file.py','wrong source')
            with self.assertRaises(ValueError):compatibility._archive(archive,identity)

    @unittest.skipUnless(os.name=='nt','Windows junction semantics')
    def test_retained_junction_preserves_original_files(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);source=root/'results/source';source.mkdir(parents=True)
            (source/'payload.txt').write_text('unchanged calculation')
            destination=root/'results/acceptance/retained';destination.parent.mkdir()
            with patch.object(compatibility,'ROOT',root):compatibility._junction(destination,source)
            try:
                self.assertEqual(destination.resolve(),source.resolve())
                self.assertEqual((destination/'payload.txt').read_text(),'unchanged calculation')
            finally:destination.rmdir()
            self.assertEqual((source/'payload.txt').read_text(),'unchanged calculation')

    def test_changed_source_needs_an_explicit_archive_context(self):
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder)/'source.py';source.write_text('current')
            with self.assertRaisesRegex(ValueError,'without a calculation archive'):
                compatibility.check_retained_source(source,'0'*64,Path(folder)/'manifest.json')

    @staticmethod
    def snapshot_recorder(out):
        recorder=Recorder.__new__(Recorder);recorder.out=out
        recorder.lock=threading.RLock();recorder.state={'status':'running','steps':{}}
        return recorder

    @unittest.skipUnless(os.name=='nt','Windows file-sharing semantics')
    def test_snapshot_survives_real_windows_reader_lock(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);target=out/'manifest.json';target.write_text('{"status":"old"}')
            kernel=ctypes.WinDLL('kernel32',use_last_error=True)
            kernel.CreateFileW.argtypes=[ctypes.c_wchar_p,ctypes.c_uint32,ctypes.c_uint32,
                ctypes.c_void_p,ctypes.c_uint32,ctypes.c_uint32,ctypes.c_void_p]
            kernel.CreateFileW.restype=ctypes.c_void_p
            kernel.CloseHandle.argtypes=[ctypes.c_void_p];kernel.CloseHandle.restype=ctypes.c_int
            handle=kernel.CreateFileW(str(target),0x80000000,3,None,3,0x80,None)
            if handle==ctypes.c_void_p(-1).value:raise ctypes.WinError(ctypes.get_last_error())
            closed=[]
            timer=threading.Timer(.15,lambda:closed.append(bool(kernel.CloseHandle(handle))))
            timer.start();recorder=self.snapshot_recorder(out)
            try:recorder.save()
            finally:timer.join()
            self.assertEqual(closed,[True]);self.assertEqual(json.loads(target.read_text()),recorder.state)
            self.assertFalse(target.with_suffix('.tmp').exists())

    def test_snapshot_permanent_denial_is_bounded_and_retains_pending_state(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);target=out/'manifest.json';previous='{"status":"old"}'
            target.write_text(previous);recorder=self.snapshot_recorder(out)
            failure=PermissionError('locked');failure.winerror=5
            with patch.object(Path,'replace',side_effect=failure) as replace,patch('research.experiments.audit_joint_studies.time.sleep') as sleep:
                with self.assertRaises(PermissionError):recorder.save()
            self.assertGreater(replace.call_count,1);self.assertLessEqual(replace.call_count,8)
            self.assertEqual(sleep.call_count,replace.call_count-1)
            self.assertEqual(target.read_text(),previous)
            self.assertEqual(json.loads(target.with_suffix('.tmp').read_text()),recorder.state)

    def test_snapshot_other_io_errors_fail_without_retry(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);recorder=self.snapshot_recorder(out)
            failure=OSError(errno.ENOSPC,'disk full');failure.winerror=112
            with patch.object(Path,'replace',side_effect=failure) as replace,patch('research.experiments.audit_joint_studies.time.sleep') as sleep:
                with self.assertRaises(OSError):recorder.save()
            self.assertEqual(replace.call_count,1);sleep.assert_not_called()

    def test_empty_unknown_and_incomplete_records_are_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            marker=Path(folder)/'checks.json'
            for value in ({},{'status':'unknown'},{'status':'passed'},{'completed':False},{'status':'passed','scope':'project','records':[]}):
                marker.write_text(json.dumps(value))
                with self.assertRaises(ValueError):validate_retained(marker)

    def test_quality_rejects_nan_inf_and_shape_errors(self):
        truth=np.ones((2,16,16));hidden=np.zeros((16,16),bool);hidden[0,0]=True
        for value in (np.nan,np.inf,-np.inf):
            prediction=truth.copy();prediction[:,hidden]=value
            with self.assertRaises(ValueError):study.quality(truth,prediction,hidden)
        with self.assertRaises(ValueError):study.quality(truth,truth[:1],hidden)

    def test_shared_legacy_score_checks_reject_nonfinite_values(self):
        for value in (np.nan,np.inf,-np.inf):
            with self.assertRaises(ValueError):finite_error(value,0)
            with self.assertRaises(ValueError):finite_error(0,value)
            with self.assertRaises(ValueError):finite_arrays(np.ones((2,2)),np.full((2,2),value))

    def test_independent_audit_rejects_nonfinite_prediction_even_when_hashes_match(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);state=fixture(out);study.verify(out)
            path=next((out/'predictions').glob('*.npz'))
            with np.load(path) as saved:arrays=dict(saved)
            arrays['prediction'][:,arrays['hidden']]=np.nan;np.savez(path,**arrays)
            state['derived_artifacts_sha256'][path.relative_to(out).as_posix()]=digest(path)
            (out/'manifest.json').write_text(json.dumps(state))
            with self.assertRaises(ValueError):study.verify(out)

    def test_independent_audit_rejects_nonfinite_recorded_score(self):
        with tempfile.TemporaryDirectory() as folder:
            out=Path(folder);state=fixture(out);table=out/'recovery.csv'
            table.write_text(table.read_text().replace(',300,',',nan,'))
            state['output_tables_sha256']['recovery.csv']=digest(table);(out/'manifest.json').write_text(json.dumps(state))
            with self.assertRaisesRegex(ValueError,'Nonfinite'):study.verify(out)

    def test_native_failure_and_missing_or_modified_raw_outputs_are_rejected(self):
        for mode in ('failure','missing','modified'):
            with self.subTest(mode=mode),tempfile.TemporaryDirectory() as folder:
                out=Path(folder);state=fixture(out)
                if mode=='failure':
                    state['steps']['search']['exit_code']=1;(out/'manifest.json').write_text(json.dumps(state))
                elif mode=='missing':(out/'native.csv').unlink()
                else:(out/'native.csv').write_text('corrupted output')
                with self.assertRaises(ValueError):study.verify(out)

    def test_range_download_bootstraps_publication_without_prior_full_download(self):
        spec=json.loads(data.PROTOCOL.read_text())['holdout']
        record=json.dumps({'files':[{'key':'parihaka_dataset.npy','size':spec['bytes'],'checksum':'md5:'+spec['md5']}]}).encode()
        class Response:
            def __enter__(self):return self
            def __exit__(self,*args):pass
            def read(self):return record
        header=io.BytesIO();np.lib.format.write_array_header_1_0(header,dict(descr='<f4',fortran_order=False,shape=(590,782,1006)))
        def ranges(url,start,length,total):return header.getvalue() if start==0 else bytes(length)
        with tempfile.TemporaryDirectory() as folder,patch.object(data.urllib.request,'urlopen',return_value=Response()) as network,patch.object(data,'range_data',side_effect=ranges):
            root=Path(folder);source=data.download_ranges(root)
            self.assertTrue((root/'publication.json').is_file());self.assertEqual(network.call_count,2)
            self.assertEqual(len(list(data.load_blocks(root))),6)
            self.assertFalse(source['publisher_full_checksum_verified'])

    def test_full_volume_and_legacy_full_metadata_load_through_same_block_interface(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);data.freeze(root);path=root/'parihaka.npy';path.write_bytes(b'fixture full volume')
            (root/'parihaka.source.json').write_text(json.dumps(dict(sha256=digest(path))))
            array=np.broadcast_to(np.float32(1),(590,782,1006))
            with patch.object(data.np,'load',return_value=array):
                blocks=list(data.load_blocks(root));self.assertEqual(len(blocks),6)
                self.assertTrue(all(cube.shape==(1006,16,16) for _,cube,_,_ in blocks))
                self.assertEqual(data.download_ranges(root)['sha256'],digest(path))

    def test_catalog_refresh_preserves_reports_and_supports_fourteen_images(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);figures=root/'figures';figures.mkdir();batch=root/'results/batch';batch.mkdir(parents=True)
            report=batch/'report.html';report.write_text('verified report');images=[]
            for i in range(14):
                p=figures/f'{i}.png';p.write_bytes(b'image')
                images.append(dict(path=p.name,title=str(i),description='verified',sha256=digest(p)))
            with patch.object(catalog,'ROOT',root):catalog.publish(batch,images,[('Report',report)],'all',figures)
            with patch.object(gallery,'FIGURES',figures):gallery.render();gallery.render()
            self.assertIn('results/batch/report.html',(figures/'index.html').read_text(encoding='utf-8'))
            self.assertEqual(json.loads((figures/'current.catalog.json').read_text())['image_count'],14)
            self.assertIn('\t14\tall',(figures/'current.meta.tsv').read_text())

    def test_reproduction_rejects_archive_traversal(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);archive=root/'unsafe.zip'
            with zipfile.ZipFile(archive,'w') as zipped:
                zipped.writestr('../escaped.txt','must not be written')
            with self.assertRaisesRegex(ValueError,'escapes'):extract(archive,root/'probe')
            self.assertFalse((root/'escaped.txt').exists())

    def test_delivery_and_authoring_helpers_are_packaged_as_source(self):
        from research.common.paths import ROOT
        from research.validation.reproduction_kit import members
        from research.validation.run_audit_checks import tracked_files
        indexed={p.relative_to(ROOT).as_posix() for p in tracked_files()}
        packed=members(ROOT,ROOT/'output/runtime_current',ROOT/'results/completion_20261103')
        for name in ('src/research/validation/delivery.py','src/research/validation/reproduce.py',
                     'src/research/validation/binary_compatibility.py','tools/presentation/rebuild_reference.mjs'):
            self.assertIn(name,indexed);self.assertIn(name,packed)
