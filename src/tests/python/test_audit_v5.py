"""Targeted independent data-format and isolation contracts for the v5 audit."""
import json
from pathlib import Path
import struct
import tempfile
import unittest
import csv
import subprocess

import numpy as np

from research.data.audit_segy_bytes import ibm_float, audit
from research.validation.audit_numerics import query
from research.experiments.audit_joint_studies import real_raw, write_gather, write_mask, initial_mask

from research.common.paths import ROOT, WORKSPACE


class ByteAuditTests(unittest.TestCase):
    def test_ibm_analytic_words_and_endian(self):
        words=np.array([0,0x41100000,0xc1200000,0x40800000,0x42100000,0x3f100000], dtype=np.uint32)
        np.testing.assert_array_equal(ibm_float(words), [0,1,-2,.5,16,1/256])
        for endian in ['<','>']:
            raw=b''.join(struct.pack(endian+'I',int(v)) for v in words)
            np.testing.assert_array_equal(ibm_float(np.frombuffer(raw,dtype=endian+'u4')),ibm_float(words))

    @unittest.skipUnless((WORKSPACE/'data/real_seismic/Data/RRAW.SGY').is_file(), 'external RRAW.SGY dataset is not installed')
    def test_complete_layout_and_all_headers(self):
        with tempfile.TemporaryDirectory() as folder:
            r=audit(WORKSPACE / 'data/real_seismic/Data/RRAW.SGY',Path(folder)/'audit')
            self.assertEqual(r['layout']['traces'],59);self.assertEqual(r['layout']['stride'],1240)
            self.assertEqual(r['trace_LE_unique']['ns'],[250]);self.assertEqual(r['trace_LE_unique']['dt_us'],[8000])
            self.assertEqual(r['binary']['LE']['format'],1)
            self.assertEqual(r['cdp_transitions_in_offset_order'],58)
            self.assertIn('UNRESOLVED',r['identification'])

    @unittest.skipUnless((WORKSPACE/'data/real_seismic/Data/RRAW.SGY').is_file(), 'external RRAW.SGY dataset is not installed')
    def test_truncated_layout_fails_and_does_not_overwrite(self):
        raw=(WORKSPACE / 'data/real_seismic/Data/RRAW.SGY').read_bytes()
        with tempfile.TemporaryDirectory() as folder:
            f=Path(folder)/'bad.sgy';f.write_bytes(raw[:-1])
            with self.assertRaises(ValueError):audit(f,Path(folder)/'out')
            f.write_bytes(raw);out=Path(folder)/'exists';out.mkdir()
            with self.assertRaises(FileExistsError):audit(f,out)


@unittest.skipUnless((WORKSPACE/'build/bin/Release/IrrGeometryDesign.exe').is_file(), 'build the Release application first')
class NativeAuditTests(unittest.TestCase):
    def invoke(self, lines, expected=0):
        with tempfile.TemporaryDirectory() as folder:
            folder=Path(folder);path=folder/'queries.txt';path.write_text('\n'.join(lines)+'\n')
            command=[str(WORKSPACE / 'build/bin/Release/IrrGeometryDesign.exe'),'--audit-queries',str(path),str(folder/'out')]
            r=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
            self.assertEqual(r.returncode,expected,r.stdout)
            if expected:return [],{}
            with (folder/'out/queries.csv').open() as f:rows=list(csv.DictReader(f))
            traces={}
            for row in rows:
                with (folder/'out'/(row['id']+'.trace.csv')).open() as f:traces[row['id']]=list(csv.DictReader(f))
            return rows,traces

    def test_candidate_current_and_count_semantics(self):
        mask=np.arange(32)%3==0
        rows,traces=self.invoke([query('chain',mask,method='SA_Budget',budget=200)])
        t=traces['chain'];self.assertEqual(len(t),200)
        self.assertTrue(any(abs(float(r['candidate_or_legacy_current'])-float(r['accepted_current']))>1e-5 for r in t))
        self.assertTrue(all(float(r['best'])<=float(r['accepted_current'])+1e-7 for r in t))
        self.assertGreaterEqual(int(rows[0]['attempts']),199)

    def test_all_stages_exclude_hidden_and_reproduce(self):
        eligible=np.ones(32,dtype=int);eligible[3::4]=0
        mask=np.zeros(32,dtype=int);mask[np.flatnonzero(eligible)[::2]]=1
        methods=['Adaptive','SA_Budget','FixedSwitch','NoRecycle','AdjacentOnly','NoPerturb','SAOnly','Hybrid']
        lines=[query(name+str(repeat),mask,method=name,budget=200,eligible=eligible,hard=6,minimum=8,window=3)
               for repeat in range(2) for name in methods]
        rows,traces=self.invoke(lines)
        for a,b in zip(rows[:len(methods)],rows[len(methods):]):
            self.assertEqual(a['mask'],b['mask']);self.assertEqual(a['mu'],b['mu'])
            self.assertEqual(sum(map(int,a['mask'])),int(mask.sum()))
            self.assertTrue(all(int(v)<=e for v,e in zip(a['mask'],eligible)))
            self.assertLessEqual(int(a['max_gap']),6)
            self.assertEqual([r['best'] for r in traces[a['id']]],[r['best'] for r in traces[b['id']]])

    def test_invalid_queries_fail_nonzero(self):
        mask=np.arange(8)%2==0
        self.invoke([query('bad',mask,method='Adaptive',window=0)],expected=1)
        eligible=np.ones(8,dtype=int);eligible[0]=0
        self.invoke([query('hidden',mask,method='Adaptive',eligible=eligible)],expected=1)
        self.invoke([query('nan',mask,gap=float('nan'))],expected=1)

    def test_wall_stop_is_reported(self):
        rows,_=self.invoke([query('clock',np.arange(32)%3==0,method='Adaptive',budget=100000,wall=1)])
        self.assertEqual(rows[0]['status'],'wall_limit')
        self.assertLess(int(rows[0]['evals']),100000);self.assertGreaterEqual(float(rows[0]['wall_ms']),1)

    def test_only_one_eligible_mask_stops_without_sampling_empty_set(self):
        mask=np.arange(8)%2==0
        rows,_=self.invoke([query('unique',mask,method='Adaptive',eligible=mask,budget=200)])
        self.assertEqual(rows[0]['status'],'single_feasible_mask');self.assertEqual(int(rows[0]['evals']),1)

    @unittest.skipUnless((WORKSPACE/'data/real_seismic/Data/RRAW.SGY').is_file(), 'external RRAW.SGY dataset is not installed')
    def test_raw_amplitude_consistency_is_scale_relative(self):
        data,x,_,_=real_raw();mask=initial_mask(59,30,20261024);data[:,~mask]=0
        with tempfile.TemporaryDirectory() as folder:
            folder=Path(folder);write_mask(folder/'mask.txt',mask);predictions=[]
            peak=float(abs(data).max())
            for name,scale in [('unit',peak),('raw',1.)]:
                path=folder/(name+'.csv');write_gather(path,data/scale,x,'raw scale regression')
                command=[str(WORKSPACE / 'build/bin/Release/IrrGeometryDesign.exe'),'--review-reconstruct',str(path),
                         '--mask',str(folder/'mask.txt'),'--out',str(folder/name),'--iter','40']
                r=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
                self.assertEqual(r.returncode,0,r.stdout)
                predictions.append(np.loadtxt(folder/name/'prediction.csv',delimiter=',')*scale)
            np.testing.assert_allclose(predictions[0],predictions[1],atol=.003,rtol=2e-5)


if __name__=='__main__':unittest.main()
