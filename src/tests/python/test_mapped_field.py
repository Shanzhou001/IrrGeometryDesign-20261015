"""Reject broken field joins, hidden-value leakage and false application closure."""
import csv
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.data.field_segy import FieldSegy,mapping
from research.experiments.mapped_field import predict,score,dictionary,coherence,hidden_mask
from research.validation.project_closure import status,evaluate,independent_record
from research.analysis.mapped_field_report import summarize
from tests.python.test_original_site import fixture


class MappedFieldTests(unittest.TestCase):
    def test_ieee_decoder_checks_both_orders(self):
        with tempfile.TemporaryDirectory() as folder:
            for order in ('>','<'):
                path=fixture(Path(folder)/('big.sgy' if order=='>' else 'little.sgy'),order=order)
                data=FieldSegy(path).decode([0,1],0,4)
                np.testing.assert_array_equal(data,np.array([[1,2,3,4],[1,2,3,4]]).T)

    def test_ibm_decoder(self):
        with tempfile.TemporaryDirectory() as folder:
            path=fixture(Path(folder)/'ibm.sgy');raw=bytearray(path.read_bytes())
            struct.pack_into('>H',raw,3224,1)
            for offset in (3840,3840+256):struct.pack_into('>4I',raw,offset,0x41100000,0xc1100000,0,0x41200000)
            path.write_bytes(raw)
            np.testing.assert_array_equal(FieldSegy(path).decode([0],0,4).ravel(),[1,-1,0,2])

    def test_partial_trace_and_sampling_conflicts_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            path=fixture(Path(folder)/'bad.sgy');raw=bytearray(path.read_bytes())
            path.write_bytes(raw[:-1])
            with self.assertRaises(ValueError):FieldSegy(path)
            struct.pack_into('>H',raw,3600+116,4000);path.write_bytes(raw)
            with self.assertRaises(ValueError):FieldSegy(path)

    def test_decoder_rejects_duplicate_and_outside_indices(self):
        with tempfile.TemporaryDirectory() as folder:
            segy=FieldSegy(fixture(Path(folder)/'test.sgy'))
            for indices in ([0,0],[-1],[2],[0.0]):
                with self.assertRaises(ValueError):segy.decode(indices,0,4)
            segy.close()

    def test_mapping_rejects_false_field_number_duplicate_and_missing_trace(self):
        with tempfile.TemporaryDirectory() as folder:
            folder=Path(folder);segy=FieldSegy(fixture(folder/'test.sgy'))
            base=[dict(trace_index_zero_based=i,trace_header_byte_offset=3600+i*256,field_record=350,
                channel=i+1,ns=4,dt_us=2000,source_x=1,source_y=2,receiver_x=i+3,receiver_y=4) for i in range(2)]
            def save(data):
                path=folder/'mapping.csv'
                with path.open('w',newline='') as f:
                    writer=csv.DictWriter(f,fieldnames=base[0].keys());writer.writeheader();writer.writerows(data)
                return path
            self.assertEqual(len(mapping(save(base),segy)),2)
            bad=[dict(r) for r in base];bad[0]['field_record']=35011
            for data in (bad,[base[0],base[0]],base[:1]):
                with self.assertRaises(ValueError):mapping(save(data),segy)
            segy.close()

    def test_all_recoverers_ignore_every_unobserved_value(self):
        rng=np.random.default_rng(4);plan=dict(iterations=5,fraction=.01)
        for shape in ((16,12),(16,3,4)):
            data=rng.normal(size=shape);known=np.arange(np.prod(shape[1:])).reshape(shape[1:])%3!=0
            altered=data.copy();altered[:,~known]=np.nan
            for kind in ('zero','linear','fourier'):
                result=predict(data,known,kind,plan)
                np.testing.assert_array_equal(result,predict(altered,known,kind,plan))
                np.testing.assert_array_equal(result[:,known],data[:,known])

    def test_nonfinite_observation_cannot_be_scored_as_success(self):
        data=np.ones((8,6));known=np.array([1,1,0,0,1,1],bool);data[0,0]=np.nan
        with self.assertRaises(ValueError):predict(data,known,'zero',{})
        with self.assertRaises(ValueError):score(data,np.ones_like(data),~known)

    def test_hidden_energy_zero_and_nonboolean_masks_rejected(self):
        for truth,mask in [(np.zeros((4,3)),np.ones(3,bool)),(np.ones((4,3)),np.ones(3,int))]:
            with self.assertRaises(ValueError):score(truth,np.ones((4,3)),mask)

    def test_geometry_is_translation_invariant_and_rejects_duplicate_points(self):
        xy=np.array([(x,y) for y in range(3) for x in range(4)],float);mask=np.arange(12)%2==0
        self.assertAlmostEqual(coherence(dictionary(xy),mask),coherence(dictionary(xy+[600000,4000000]),mask),places=12)
        with self.assertRaises(ValueError):dictionary(np.array([[0.,0.],[0.,0.]]))

    def test_fixed_hidden_mask_has_no_global_rng_dependency(self):
        first=hidden_mask(128,1907);np.random.seed(901);np.random.random(100)
        np.testing.assert_array_equal(first,hidden_mask(128,1907));self.assertEqual(first.sum(),12)

    def test_missing_records_cannot_complete_project(self):
        with tempfile.TemporaryDirectory() as folder:
            result=status(Path(folder))
            self.assertEqual(result['status'],'incomplete');self.assertFalse(result['public_recovery_scope_passed'])
            self.assertFalse(result['original_site_application_passed'])

    def test_passed_status_strings_without_evidence_cannot_complete_public_or_original_scope(self):
        with tempfile.TemporaryDirectory() as folder:
            folder=Path(folder);(folder/'delivery_verification.json').write_text(json.dumps(dict(record_sha256={})))
            (folder/'completion_verification.json').write_text('{"status":"passed"}')
            (folder/'mapped_field').mkdir();(folder/'mapped_field/verification.json').write_text('{"status":"passed"}')
            (folder/'project_checks').mkdir();(folder/'project_checks/checks.json').write_text(json.dumps(dict(status='passed',source_sha256={})))
            with patch('research.validation.project_closure.tracked_files',return_value=[]):result=status(folder)
            self.assertFalse(result['public_recovery_scope_passed']);self.assertEqual(result['status'],'failed')
            self.assertEqual(result['components']['mapped_field_recovery']['status'],'failed')

    def test_optional_original_site_does_not_block_verified_public_application(self):
        scope=dict(required=['geometry_waveform','recovery','reproduction'],optional=['original_site'])
        components={k:dict(status='passed') for k in scope['required']}
        components['original_site']=dict(status='incomplete')
        self.assertEqual(evaluate(components,scope),'passed')
        components['recovery']['status']='failed'
        self.assertEqual(evaluate(components,scope),'failed')

    def test_missing_required_reproduction_cannot_close_scope(self):
        self.assertEqual(evaluate({'recovery':dict(status='passed')},
                                 dict(required=['recovery','reproduction'],optional=[])),'incomplete')

    def test_empty_or_overlapping_required_scope_is_rejected(self):
        for scope in (dict(required=[],optional=[]),dict(required=['x'],optional=['x'])):
            with self.assertRaises(ValueError):evaluate({},scope)

    def test_independent_record_must_match_actual_recovery_bytes(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);(root/'mapped_field').mkdir();(root/'mapped_field/recovery.csv').write_text('changed')
            (root/'mapped_field_independent_reproduction.json').write_text(json.dumps(dict(status='passed',cases=1728,
                columns=25,exact_fields=43200,numeric_max_difference=0,recovery_csv_sha256='0'*64)))
            self.assertEqual(independent_record(root)['status'],'failed')

    def test_stratified_report_keeps_negative_effect_and_requires_paired_controls(self):
        records=[dict(geometry='g',component='0',start='0',h='1',m='4',seed='1',recoverer='fourier',
                      method=method,survey='field',retained_fraction='.5',snr=str(value))
                 for method,value in [('Hybrid',1),('Jitter',2),('SA',3),('RandomBudget',4)]]
        result=summarize(records)
        self.assertEqual(len(result),3);self.assertTrue(all(r['mean_delta']<0 for r in result))
        with self.assertRaises(ValueError):summarize(records[:-1])


if __name__=='__main__':unittest.main()
