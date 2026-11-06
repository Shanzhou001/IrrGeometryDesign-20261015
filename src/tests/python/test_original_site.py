"""Scientific identity-gate regression tests; fixtures are never field evidence."""
import json
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest.mock import patch

from research.validation import original_site as AUDIT
from research.common.paths import ROOT as PROJECT


def minimal():
    key=(5151,1001,1)
    return dict(sources={key:(679062.6,4146708.5)},shots={350:dict(source_key=key,groups=2)})

def fixture(path,records=(350,350),channels=(1,2),order='>',zero=False):
    ns=4;raw=bytearray(3600)
    for offset,value in ((3216,2000),(3220,ns),(3224,5)):struct.pack_into(order+'H',raw,offset,value)
    for record,channel in zip(records,channels):
        header=bytearray(240)
        struct.pack_into(order+'i',header,8,record);struct.pack_into(order+'i',header,12,channel)
        struct.pack_into(order+'h',header,28,1);struct.pack_into(order+'h',header,70,-10)
        struct.pack_into(order+'h',header,88,1)
        if not zero:
            struct.pack_into(order+'i',header,72,6790626);struct.pack_into(order+'i',header,76,41467085)
        struct.pack_into(order+'H',header,114,ns);struct.pack_into(order+'H',header,116,2000)
        raw.extend(header);raw.extend(struct.pack(order+'4f',1,2,3,4))
    path.write_bytes(raw)
    return path

class OriginalSiteTests(unittest.TestCase):
    def test_original_geometry_uses_standard_ffid_and_actual_spread_counts(self):
        result=AUDIT.target(PROJECT)
        self.assertEqual(set(result['shots']),set(range(2,351)))
        self.assertNotIn(35011,result['shots'])
        self.assertEqual(result['pairs'],3162602)
        self.assertEqual(min(r['groups'] for r in result['shots'].values()),9029)
        self.assertEqual(max(r['groups'] for r in result['shots'].values()),9072)

    def test_modified_original_sps_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);base=root/'data/sps';base.mkdir(parents=True)
            for ext in AUDIT.EXPECTED:shutil.copy2(PROJECT/'data/sps'/('5151-5171.'+ext),base)
            (base/'5151-5171.X').write_text('modified geometry')
            with self.assertRaisesRegex(ValueError,'identity changed'):AUDIT.target(root)

    def test_foreign_ffid_rejected_even_with_matching_coordinates(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'foreign.sgy',records=(7733,7733))
            state=AUDIT.inspect_waveform(p,minimal())
            self.assertEqual(state['status'],'incompatible')
            self.assertEqual(state['outside_original_ffids'],[7733])
            self.assertFalse(state['eligible_for_original_recovery'])

    def test_compound_legacy_number_cannot_pass_as_ffid(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'legacy.sgy',records=(35011,35011))
            self.assertEqual(AUDIT.inspect_waveform(p,minimal())['status'],'incompatible')

    def test_ffid_overlap_alone_does_not_verify_zero_coordinate_data(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'zero.sgy',zero=True)
            result=AUDIT.inspect_waveform(p,minimal())
            self.assertEqual(result['status'],'unverified')
            self.assertEqual(result['zero_source_coordinate_live_traces'],2)
            self.assertFalse(result['eligible_for_original_recovery'])

    def test_matching_source_header_does_not_imply_full_site_validation(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'candidate.sgy')
            result=AUDIT.inspect_waveform(p,minimal())
            self.assertEqual(result['source_coordinate_mismatches'],0)
            self.assertEqual(result['status'],'unverified')
            self.assertFalse(result['eligible_for_original_recovery'])

    def test_wrong_source_coordinates_rejected_for_matching_ffid(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'shifted.sgy');raw=bytearray(p.read_bytes())
            struct.pack_into('>i',raw,3600+72,6791626);p.write_bytes(raw)
            self.assertEqual(AUDIT.inspect_waveform(p,minimal())['status'],'incompatible')

    def test_duplicate_field_record_channel_is_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'duplicate.sgy',channels=(1,1))
            result=AUDIT.inspect_waveform(p,minimal())
            self.assertEqual(result['duplicate_ffid_channel_pairs'],1)
            self.assertEqual(result['status'],'incompatible')

    def test_too_few_live_traces_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'short_spread.sgy',records=(350,),channels=(1,))
            self.assertEqual(AUDIT.inspect_waveform(p,minimal())['records_with_fewer_traces_than_receiver_positions'],[350])

    def test_inconsistent_trace_sampling_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'sampling.sgy');raw=bytearray(p.read_bytes())
            struct.pack_into('>H',raw,3600+116,4000);p.write_bytes(raw)
            with self.assertRaisesRegex(ValueError,'Inconsistent'):AUDIT.inspect_waveform(p,minimal())

    def test_zero_trace_sampling_is_recorded_without_overriding_nonzero_conflicts(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'binary_defaults.sgy');raw=bytearray(p.read_bytes())
            struct.pack_into('>H',raw,3600+114,0);struct.pack_into('>H',raw,3600+116,0);p.write_bytes(raw)
            result=AUDIT.inspect_waveform(p,minimal())
            self.assertEqual(result['zero_trace_sample_declarations'],1)
            self.assertEqual(result['zero_trace_interval_declarations'],1)
            self.assertFalse(result['eligible_for_original_recovery'])

    def test_partial_trace_rejected(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'partial.sgy');p.write_bytes(p.read_bytes()[:-1])
            with self.assertRaisesRegex(ValueError,'framing'):AUDIT.inspect_waveform(p,minimal())

    def test_little_endian_is_checked_without_guessing_big_endian(self):
        with tempfile.TemporaryDirectory() as folder:
            p=fixture(Path(folder)/'little.sgy',order='<')
            self.assertEqual(AUDIT.inspect_waveform(p,minimal())['byte_order'],'little')

    def test_absent_waveforms_never_generate_recovery_or_application_success(self):
        with tempfile.TemporaryDirectory() as folder:
            original=dict(minimal(),input_sha256={},segments=[])
            with patch.object(AUDIT,'target',return_value=original),patch.object(AUDIT,'geometry_check',return_value={'status':'passed'}):
                result=AUDIT.run(PROJECT,Path(folder)/'audit',[])
            self.assertEqual(result['status'],'incomplete')
            self.assertFalse(result['original_application_passed'])
            self.assertEqual(result['original_waveform_recovery_cases'],0)
            self.assertEqual(result['stages']['downstream_imaging'],'not_run')

if __name__=='__main__':unittest.main()
