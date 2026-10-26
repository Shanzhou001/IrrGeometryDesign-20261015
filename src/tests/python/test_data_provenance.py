"""Source identity, explicit uncertainty and known-only preprocessing contracts."""
import argparse
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.common.hashing import digest
from research.common.provenance import real_source_provenance, require_inputs
from research.data import make_59x256
from research.data.make_59x256 import convert
from research.experiments import review_studies, audit_joint_studies


class ProvenanceTests(unittest.TestCase):
    def declaration(self, source):
        return dict(source_sha256=digest(source), payload_profile='IEEE_LE_FLOAT32',
                    sample_interval_us=8000, offset_units='m', amplitude_units='fixture units',
                    geometry_description='fixture offsets', confirmed_by='fixture author',
                    evidence_reference='test fixture only; not a real-data confirmation')

    def test_unconfirmed_source_requires_explicit_sensitivity_mode(self):
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder)/'fixture.sgy';source.write_bytes(b'fixture')
            with self.assertRaisesRegex(ValueError,'unconfirmed'):
                real_source_provenance(source)
            state=real_source_provenance(source,allow_unconfirmed=True)
            self.assertEqual(state['status'],'unconfirmed_sensitivity')
            self.assertFalse(state['field_claims_eligible'])
            self.assertIn('IBM',state['header_profile_conflict'])

    def test_confirmation_is_bound_to_exact_source_and_supported_profile(self):
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder)/'fixture.sgy';source.write_bytes(b'fixture')
            confirmation=Path(folder)/'declaration.json';declaration=self.declaration(source)
            confirmation.write_text(json.dumps(declaration))
            state=real_source_provenance(source,confirmation)
            self.assertEqual(state['status'],'declared_confirmed')
            self.assertTrue(state['field_claims_eligible'])
            self.assertEqual(state['confirmation_sha256'],digest(confirmation))
            source.write_bytes(b'changed')
            with self.assertRaisesRegex(ValueError,'SHA-256'):
                real_source_provenance(source,confirmation)
            declaration['source_sha256']=digest(source);declaration['payload_profile']='IBM_BE'
            confirmation.write_text(json.dumps(declaration))
            with self.assertRaisesRegex(ValueError,'decoder profile'):
                real_source_provenance(source,confirmation)

    def test_incomplete_confirmation_cannot_upgrade_uncertainty(self):
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder)/'fixture.sgy';source.write_bytes(b'fixture')
            confirmation=Path(folder)/'declaration.json';declaration=self.declaration(source)
            declaration['evidence_reference']=None;confirmation.write_text(json.dumps(declaration))
            with self.assertRaisesRegex(ValueError,'evidence_reference'):
                real_source_provenance(source,confirmation)

    def test_missing_inputs_are_reported_together(self):
        with tempfile.TemporaryDirectory() as folder:
            paths=[Path(folder)/name for name in ['raw.sgy','geometry.R']]
            with self.assertRaises(FileNotFoundError) as error:require_inputs(paths)
            self.assertTrue(all(str(path) in str(error.exception) for path in paths))

    def test_raw_resampling_does_not_use_hidden_amplitudes_for_known_traces(self):
        data=np.sin(np.arange(250)[:,None]*.1)*np.array([[1.,2.,3.,4.]])
        changed=data.copy();changed[:,[1,3]]=13*changed[:,[1,3]]+7
        original,_=convert(data,np.arange(4),normalize=False)
        altered,_=convert(changed,np.arange(4),normalize=False)
        np.testing.assert_array_equal(original[:,[0,2]],altered[:,[0,2]])
        legacy,_=convert(data,np.arange(4));legacy_altered,_=convert(changed,np.arange(4))
        self.assertGreater(np.max(abs(legacy[:,[0,2]]-legacy_altered[:,[0,2]])),.01)

    def test_reconstruction_serializes_only_known_values(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder);gather=output/'truth.csv';maskfile=output/'mask.txt'
            truth=np.arange(64*8,dtype=float).reshape(64,8)
            review_studies.write_gather(gather,truth,np.arange(8),'fixture')
            mask=np.arange(8)%2==0;review_studies.write_mask(maskfile,mask)
            class Recorder:
                def __init__(self):self.output=output
                def call(self,key,args,destination):
                    observed,_=review_studies.read_gather(args[1])
                    np.testing.assert_array_equal(observed[:,mask],truth[:,mask])
                    np.testing.assert_array_equal(observed[:,~mask],0)
                    destination.mkdir(parents=True)
                    np.savetxt(destination/'prediction.csv',observed,delimiter=',')
            review_studies.reconstruct(Recorder(),'fixture',gather,maskfile,review_studies.CONFIGURATIONS[0])

    def test_conversion_cli_defaults_to_raw_amplitude_and_records_uncertainty(self):
        with tempfile.TemporaryDirectory() as folder:
            source=Path(folder)/'fixture.sgy';output=Path(folder)/'output.csv'
            raw=bytearray(3600+2*(240+250*4))
            struct.pack_into('<H',raw,3220,250);struct.pack_into('<H',raw,3224,1)
            for j in range(2):
                start=3600+j*(240+250*4)
                struct.pack_into('<i',raw,start+36,j*10)
                struct.pack_into('<H',raw,start+114,250);struct.pack_into('<H',raw,start+116,8000)
                struct.pack_into('<250f',raw,start+240,*([2.+j]*250))
            source.write_bytes(raw)
            arguments=['convert','--input',str(source),'--output',str(output)]
            with patch.object(sys,'argv',arguments):
                with self.assertRaisesRegex(ValueError,'unconfirmed'):make_59x256.main()
            self.assertFalse(output.exists())
            with patch.object(sys,'argv',arguments+['--allow-unconfirmed-real']):make_59x256.main()
            values=np.loadtxt(output,comments='#',delimiter=',')
            self.assertEqual(float(values.max()),3.)
            provenance=json.loads(Path(str(output)+'.provenance.json').read_text())
            self.assertEqual(provenance['status'],'unconfirmed_sensitivity')
            self.assertTrue(provenance['strict_preprocessing_isolation'])
            self.assertFalse(provenance['field_claims_eligible'])

    def test_missing_real_source_fails_before_experiment_output_is_created(self):
        with tempfile.TemporaryDirectory() as folder:
            executable=Path(folder)/'fixture.exe';executable.write_bytes(b'fixture')
            output=Path(folder)/'out'
            args=argparse.Namespace(out=output,exe=executable,stage='real',workers=1)
            with patch.object(audit_joint_studies,'SOURCE',Path(folder)/'missing.sgy'):
                with self.assertRaises(FileNotFoundError):audit_joint_studies.run(args)
            self.assertFalse(output.exists())

    def test_v4_synthetic_search_does_not_require_real_data(self):
        with tempfile.TemporaryDirectory() as folder:
            output=Path(folder)/'out';executable=Path(folder)/'fixture.exe';executable.write_bytes(b'fixture')
            calls=[]
            class Recorder:
                def __init__(self,out,exe,configuration,resume):
                    self.output=out;out.mkdir();self.state={'configuration':configuration}
                def call(self,key,args,destination):calls.append(args)
                def save(self):pass
            args=argparse.Namespace(out=output,exe=executable,quick=True,workers=1,resume=False,stage='search',synthetic_only=True)
            with patch.object(review_studies,'Recorder',Recorder),patch.object(review_studies,'real_source_provenance',side_effect=AssertionError('real source read')):
                review_studies.run(args)
            self.assertEqual(len(calls),1)
            self.assertNotIn('--input',calls[0])

    def test_v5_joint_does_not_require_real_source(self):
        with tempfile.TemporaryDirectory() as folder:
            executable=Path(folder)/'fixture.exe';executable.write_bytes(b'fixture')
            class Recorder:
                def __init__(self,out,exe,configuration):self.count=0;self.state={'configuration':configuration}
                def finish(self):pass
            args=argparse.Namespace(out=Path(folder)/'out',exe=executable,stage='joint',workers=1)
            with patch.object(audit_joint_studies,'Recorder',Recorder),patch.object(audit_joint_studies,'joint') as joint,patch.object(audit_joint_studies,'real_source_provenance',side_effect=AssertionError('real source read')):
                audit_joint_studies.run(args)
            self.assertTrue(joint.call_args.args[0].state['configuration']['synthetic_only'])


if __name__=='__main__':unittest.main()
