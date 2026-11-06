"""Failure evidence, prospective statistics and observed-only spatial recovery contracts."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.analysis.contribution_report import design_check,layout_effects
from research.common import paths
from research.experiments import audit_joint_studies as audit
from research.experiments.confirmation_spatial import fk3d
from research.experiments.confirmation_synthetic import structures
from research.data.confirmation import PROTOCOL,volume_headers
from research.common.hashing import digest


class ConfirmationTests(unittest.TestCase):
    def test_exact_test_floor_exposes_impossible_historical_family(self):
        old=design_check(6,24)
        self.assertEqual(old['exact_two_sided_p_floor'],.03125)
        self.assertFalse(old['can_reject_any'])
        self.assertEqual(old['mathematical_minimum_nonzero_units'],10)
        self.assertTrue(design_check(20,6)['can_reject_any'])

    def test_layout_effects_pair_conditions_before_comparing_methods(self):
        data=[]
        for seed in (1,2):
            for method in ('Adaptive','Jitter'):
                row=dict(case=f'{seed}_{method}',ffid=53,window_start=512,hidden_set='H',m=60,seed=seed,method=method)
                row.update({kind+'_snr':seed*100+(2 if method=='Adaptive' else 0) for kind in ('cold','fk','linear','selected')})
                data.append(row)
        self.assertTrue(all(r['delta_snr_vs_jitter']==2 for r in layout_effects(data) if r['method']=='Adaptive'))
        with self.assertRaisesRegex(ValueError,'Missing matched'):layout_effects(data[:1])

    def test_spatial_recovery_never_reads_unobserved_values(self):
        data=np.random.default_rng(31).normal(size=(16,4,4));known=np.indices((4,4)).sum(axis=0)%2==0
        changed=data.copy();changed[:,~known]=np.nan
        a=fk3d(data,known,iterations=8);b=fk3d(changed,known,iterations=8)
        np.testing.assert_array_equal(a,b)
        np.testing.assert_array_equal(a[:,known],data[:,known])

    def test_new_structures_are_distinct_and_reproducible(self):
        plan=json.loads(PROTOCOL.read_text())['synthetic'];a=structures(plan);b=structures(plan)
        self.assertEqual(len(a),20)
        self.assertEqual(len({x[0].tobytes() for x in a}),20)
        for x,y in zip(a,b):np.testing.assert_array_equal(x[0],y[0])

    def test_native_failure_evidence_survives_temporary_batch_cleanup(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);exe=root/'program.exe';exe.write_bytes(b'fixture')
            with patch.object(audit,'FAILURES',root/'failures'):
                with tempfile.TemporaryDirectory() as batch:
                    recorder=audit.Recorder(Path(batch)/'run',exe,{},protocol='test')
                    target=recorder.out/'native/shared/case'
                    def fail(command,**kwargs):
                        self.assertTrue(target.parent.is_dir())
                        kwargs['stdout'].write(b'deterministic injected write failure')
                        return subprocess.CompletedProcess(command,1)
                    with patch.object(audit.subprocess,'run',side_effect=fail) as call:
                        with self.assertRaisesRegex(RuntimeError,'evidence='):recorder.call('failed_case',['--out',target],target)
                    self.assertEqual(call.call_count,1)
                    state=json.loads((recorder.out/'manifest.json').read_text())
                    archive=Path(state['steps']['failed_case']['failure_archive'])
                self.assertEqual((archive/'native.log').read_bytes(),b'deterministic injected write failure')
                failure=json.loads((archive/'failure.json').read_text())
                self.assertEqual(failure['exit_code'],1)
                self.assertIn('inputs',failure)

    def test_active_experiment_cannot_escape_results(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);(root/'results').mkdir()
            with patch.object(paths,'ROOT',root):
                self.assertEqual(paths.current_results(),root/'results/current')
                (root/'results/active.txt').write_text('../escape')
                with self.assertRaises(ValueError):paths.current_results()

    def test_fixed_volume_allows_missing_trace_counts_but_rejects_nonzero_mismatch(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);path=root/'fixture.sgy';ns=800;stride=240+4*ns;raw=bytearray(3600+4*stride)
            for offset,value in [(3216,2000),(3220,ns),(3224,5)]:raw[offset:offset+2]=value.to_bytes(2,'big')
            for i in range(4):
                start=3600+i*stride
                raw[start+8:start+12]=(i//2+1).to_bytes(4,'big')
                raw[start+20:start+24]=(i%2+1).to_bytes(4,'big')
                raw[start+114:start+116]=(ns if i else 0).to_bytes(2,'big')
                raw[start+116:start+118]=(2000).to_bytes(2,'big')
            path.write_bytes(raw);metadata=path.with_suffix('.source.json')
            metadata.write_text(json.dumps(dict(sha256=digest(path))))
            spec=dict(id='fixture',inline_byte=9,crossline_byte=21)
            mapped,info=volume_headers(spec,root)
            self.assertEqual(info['missing_trace_sample_declarations'],1)
            del mapped
            raw[3600+114:3600+116]=(ns-1).to_bytes(2,'big');path.write_bytes(raw)
            metadata.write_text(json.dumps(dict(sha256=digest(path))))
            with self.assertRaisesRegex(ValueError,'Variable sample'):volume_headers(spec,root)
