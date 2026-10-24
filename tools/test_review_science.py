"""Regression contracts for v4 validation isolation and reproducibility."""  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24
import struct  # add by wushanzhou in 2026/10/24
import sys  # add by wushanzhou in 2026/10/24
import tempfile  # add by wushanzhou in 2026/10/24
import unittest  # add by wushanzhou in 2026/10/24

import numpy as np  # add by wushanzhou in 2026/10/24

import convert_rraw_to_engine_csv as segy  # add by wushanzhou in 2026/10/24
import review_studies as studies  # add by wushanzhou in 2026/10/24


def fixture():  # add by wushanzhou in 2026/10/24
    raw=bytearray(3600+2*(240+8))  # add by wushanzhou in 2026/10/24
    struct.pack_into("<H",raw,3220,2)  # add by wushanzhou in 2026/10/24
    for index in range(2):  # add by wushanzhou in 2026/10/24
        start=3600+index*248  # add by wushanzhou in 2026/10/24
        struct.pack_into("<H",raw,start+114,2)  # add by wushanzhou in 2026/10/24
        struct.pack_into("<H",raw,start+116,8000)  # add by wushanzhou in 2026/10/24
        struct.pack_into("<2f",raw,start+240,index+1,index+2)  # add by wushanzhou in 2026/10/24
    return raw  # add by wushanzhou in 2026/10/24


class ReviewScienceTests(unittest.TestCase):  # add by wushanzhou in 2026/10/24
    def test_binary_change_aborts_before_launch(self): # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder: # add by wushanzhou in 2026/10/24
            output=Path(folder);program=output/'program.exe';program.write_bytes(b'original') # add by wushanzhou in 2026/10/24
            recorder=studies.Recorder(output/'batch',program,{'test':True},False) # add by wushanzhou in 2026/10/24
            program.write_bytes(b'changed-executable') # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(RuntimeError,'Executable changed'): # add by wushanzhou in 2026/10/24
                recorder.call('unreachable',[],output/'unused') # add by wushanzhou in 2026/10/24

    def test_validation_selection_is_scale_equivariant(self): # add by wushanzhou in 2026/10/24
        target=np.array([1.,2.,-3.]);candidates=[('zero',np.zeros(3)),('good',2*target),('bad',-target)] # add by wushanzhou in 2026/10/24
        a,_=studies.validation_choice(target,candidates) # add by wushanzhou in 2026/10/24
        b,_=studies.validation_choice(7*target,[(name,7*value) for name,value in candidates]) # add by wushanzhou in 2026/10/24
        self.assertEqual(a['candidate'],b['candidate']);self.assertAlmostEqual(a['gain'],b['gain']) # add by wushanzhou in 2026/10/24

    def test_trace_headers_are_authoritative(self):  # add by wushanzhou in 2026/10/24
        _,_,_,meta=segy.read_segy(studies.ROOT/"data/real_seismic/Data/RRAW.SGY")  # add by wushanzhou in 2026/10/24
        self.assertEqual(meta["dt_trace_field"],8000)  # add by wushanzhou in 2026/10/24
        self.assertEqual(meta["trace_headers_checked"],59)  # add by wushanzhou in 2026/10/24
        self.assertEqual(meta["dt_raw_field"],meta["dt_trace_field"])  # add by wushanzhou in 2026/10/24

    def test_reject_inconsistent_and_unsupported_sampling_headers(self):  # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder:  # add by wushanzhou in 2026/10/24
            path=Path(folder)/"fixture.sgy"  # add by wushanzhou in 2026/10/24
            raw=fixture();path.write_bytes(raw)  # add by wushanzhou in 2026/10/24
            values,_,_,meta=segy.read_segy(path)  # add by wushanzhou in 2026/10/24
            np.testing.assert_array_equal(values,[[1,2],[2,3]])  # add by wushanzhou in 2026/10/24
            self.assertEqual(meta["dt_us"],8000)  # add by wushanzhou in 2026/10/24
            struct.pack_into("<H",raw,3600+248+114,3);path.write_bytes(raw)  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(ValueError,"inconsistent"):  # add by wushanzhou in 2026/10/24
                segy.read_segy(path)  # add by wushanzhou in 2026/10/24
            raw=fixture();struct.pack_into("<H",raw,3600+248+116,4000);path.write_bytes(raw)  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(ValueError,"inconsistent"):  # add by wushanzhou in 2026/10/24
                segy.read_segy(path)  # add by wushanzhou in 2026/10/24
            for start in (3600,3848):struct.pack_into("<H",raw,start+116,4000)  # add by wushanzhou in 2026/10/24
            path.write_bytes(raw)  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(ValueError,"requires trace-header"):  # add by wushanzhou in 2026/10/24
                segy.read_segy(path)  # add by wushanzhou in 2026/10/24

    def test_reject_nonfinite_payload_after_valid_headers(self):  # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder:  # add by wushanzhou in 2026/10/24
            path=Path(folder)/"fixture.sgy";raw=fixture()  # add by wushanzhou in 2026/10/24
            struct.pack_into("<f",raw,3840,float("nan"));path.write_bytes(raw)  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(ValueError,"non-finite"):  # add by wushanzhou in 2026/10/24
                segy.read_segy(path)  # add by wushanzhou in 2026/10/24

    def test_calibration_uses_only_validation_targets(self):  # add by wushanzhou in 2026/10/24
        target=np.array([[1.,2.],[-1.,-2.]])  # add by wushanzhou in 2026/10/24
        choice,records=studies.validation_choice(target,[("zero",np.zeros_like(target)),("scaled",2*target)])  # add by wushanzhou in 2026/10/24
        self.assertEqual(choice["candidate"],"scaled")  # add by wushanzhou in 2026/10/24
        self.assertAlmostEqual(choice["gain"],.5)  # add by wushanzhou in 2026/10/24
        self.assertEqual(choice["validation_mse"],0)  # add by wushanzhou in 2026/10/24
        self.assertGreater(records[0]["validation_mse"],0)  # add by wushanzhou in 2026/10/24
        outer=np.zeros((2,8));outer[:,[1,3]]=target  # add by wushanzhou in 2026/10/24
        altered=outer.copy();altered[:,[0,2,4,5,6,7]]=1000  # add by wushanzhou in 2026/10/24
        a=studies.validation_choice(outer[:,[1,3]],[("scaled",2*target)])  # add by wushanzhou in 2026/10/24
        b=studies.validation_choice(altered[:,[1,3]],[("scaled",2*target)])  # add by wushanzhou in 2026/10/24
        self.assertEqual(a,b)  # add by wushanzhou in 2026/10/24

    def test_zero_tie_breaker_and_negative_correlation(self):  # add by wushanzhou in 2026/10/24
        target=np.array([1.,-1.])  # add by wushanzhou in 2026/10/24
        choice,_=studies.validation_choice(target,[("zero",np.zeros(2)),("inverted",-target)])  # add by wushanzhou in 2026/10/24
        self.assertEqual(choice["candidate"],"zero")  # add by wushanzhou in 2026/10/24
        with self.assertRaises(ValueError):studies.validation_choice(target,[("nan",np.array([np.nan,1]))])  # add by wushanzhou in 2026/10/24

    def test_physical_interpolation_reads_only_known_columns(self):  # add by wushanzhou in 2026/10/24
        x=np.array([0.,1.,3.,10.]);mask=np.array([True,False,False,True])  # add by wushanzhou in 2026/10/24
        values=np.array([[2.,999.,-999.,22.]])  # add by wushanzhou in 2026/10/24
        prediction=studies.linear_known(values,x,mask)  # add by wushanzhou in 2026/10/24
        np.testing.assert_allclose(prediction,[[2,4,8,22]])  # add by wushanzhou in 2026/10/24
        changed=values.copy();changed[:,~mask]=1e20  # add by wushanzhou in 2026/10/24
        np.testing.assert_array_equal(prediction,studies.linear_known(changed,x,mask))  # add by wushanzhou in 2026/10/24

    def test_fractional_masks_are_rejected(self):  # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder:  # add by wushanzhou in 2026/10/24
            path=Path(folder)/"mask.txt";path.write_text("1\n0.5\n0\n")  # add by wushanzhou in 2026/10/24
            with self.assertRaises(ValueError):studies.read_mask(path)  # add by wushanzhou in 2026/10/24

    def test_snr_is_missing_only_and_keeps_negative_values(self):  # add by wushanzhou in 2026/10/24
        values=np.array([[1.,2.],[2.,4.]])  # add by wushanzhou in 2026/10/24
        hidden=np.array([False,True]);predicted=np.zeros_like(values)  # add by wushanzhou in 2026/10/24
        self.assertEqual(studies.snr(values,predicted,hidden),0)  # add by wushanzhou in 2026/10/24
        predicted[:,hidden]=3*values[:,hidden]  # add by wushanzhou in 2026/10/24
        self.assertAlmostEqual(studies.snr(values,predicted,hidden),-10*np.log10(4))  # add by wushanzhou in 2026/10/24
        predicted[:,~hidden]=1e20  # add by wushanzhou in 2026/10/24
        self.assertAlmostEqual(studies.snr(values,predicted,hidden),-10*np.log10(4))  # add by wushanzhou in 2026/10/24

    def test_synthetic_realizations_are_deterministic_and_distinct(self):  # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder:  # add by wushanzhou in 2026/10/24
            output=Path(folder)  # add by wushanzhou in 2026/10/24
            first=studies.generate_synthetic(output,6)  # add by wushanzhou in 2026/10/24
            hashes=[studies.sha(path) for path,_ in first]  # add by wushanzhou in 2026/10/24
            second=studies.generate_synthetic(output,6)  # add by wushanzhou in 2026/10/24
            self.assertEqual(hashes,[studies.sha(path) for path,_ in second])  # add by wushanzhou in 2026/10/24
            self.assertEqual(len(set(hashes)),6)  # add by wushanzhou in 2026/10/24
            self.assertEqual(len({metadata["family"] for _,metadata in first}),3)  # add by wushanzhou in 2026/10/24
            for path,metadata in first:  # add by wushanzhou in 2026/10/24
                data,x=studies.read_gather(path)  # add by wushanzhou in 2026/10/24
                self.assertEqual(data.shape,(256,128))  # add by wushanzhou in 2026/10/24
                self.assertEqual(len(metadata["events"]),3)  # add by wushanzhou in 2026/10/24
                self.assertTrue(all("fault_position" in e for e in metadata["events"]))  # add by wushanzhou in 2026/10/24

    def test_cache_reuse_requires_intact_outputs(self):  # add by wushanzhou in 2026/10/24
        with tempfile.TemporaryDirectory() as folder:  # add by wushanzhou in 2026/10/24
            output=Path(folder);recorder=studies.Recorder(output,sys.executable,{"test":True},False)  # add by wushanzhou in 2026/10/24
            destination=output/"tiny"  # add by wushanzhou in 2026/10/24
            code="from pathlib import Path;import sys;p=Path(sys.argv[1]);p.mkdir();(p/'value.txt').write_text('verified')"  # add by wushanzhou in 2026/10/24
            args=["-c",code,str(destination)]  # add by wushanzhou in 2026/10/24
            recorder.call("tiny",args,destination)  # add by wushanzhou in 2026/10/24
            recorder.call("tiny",args,destination)  # add by wushanzhou in 2026/10/24
            self.assertEqual(len(recorder.state["steps"]),1)  # add by wushanzhou in 2026/10/24
            resumed=studies.Recorder(output,sys.executable,{"test":True},True)  # add by wushanzhou in 2026/10/24
            self.assertEqual(len(resumed.state["steps"]),1)  # add by wushanzhou in 2026/10/24
            (destination/"value.txt").write_text("corrupted")  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(RuntimeError,"Unverified"):  # add by wushanzhou in 2026/10/24
                recorder.call("tiny",args,destination)  # add by wushanzhou in 2026/10/24
            with self.assertRaisesRegex(RuntimeError,"differs"):  # add by wushanzhou in 2026/10/24
                studies.Recorder(output,sys.executable,{"test":False},True)  # add by wushanzhou in 2026/10/24


if __name__=="__main__":  # add by wushanzhou in 2026/10/24
    unittest.main(verbosity=2)  # add by wushanzhou in 2026/10/24
