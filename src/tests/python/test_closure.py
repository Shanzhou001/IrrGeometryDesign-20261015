"""Frozen controls must preserve budgets, pairing and source byte identity."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.analysis.closure_report import summarize,run as report
from research.common.hashing import digest
from research.data import closure_holdout as data
from research.experiments.closure_validation import random_candidates,coherence,gram_coherence


class ClosureTests(unittest.TestCase):
    def test_random_budget_counts_initial_and_preserves_eligibility(self):
        hidden=np.zeros(256,bool);hidden[4::8]=True
        initial=np.zeros(256,bool);initial[np.flatnonzero(~hidden)[:76]]=True
        candidates=list(random_candidates(initial,hidden,11031,76,0,300))
        self.assertEqual(len(candidates),300);np.testing.assert_array_equal(candidates[0],initial)
        self.assertTrue(all(mask.sum()==76 and not np.any(mask&hidden) for mask in candidates))
        self.assertGreater(len({mask.tobytes() for mask in candidates}),290)
        repeated=list(random_candidates(initial,hidden,11031,76,0,300))
        self.assertTrue(all(np.array_equal(a,b) for a,b in zip(candidates,repeated)))
        with self.assertRaises(ValueError):list(random_candidates(initial,hidden,11031,75,0,300))

    def test_fft_objective_matches_explicit_finite_gram(self):
        rng=np.random.default_rng(4)
        for m in (76,128,179):
            mask=np.zeros(256,bool);mask[rng.choice(256,m,replace=False)]=True
            self.assertAlmostEqual(coherence(mask),gram_coherence(mask),places=12)
            self.assertGreaterEqual(coherence(mask,full=True)+1e-14,coherence(mask))

    def test_protocol_cannot_change_after_freeze_or_follow_existing_amplitudes(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);protocol=root/'plan.json';protocol.write_text('{"holdout":{}}')
            with patch.object(data,'PROTOCOL',protocol):
                data.freeze(root/'cache');protocol.write_text('{"holdout":{"changed":true}}')
                with self.assertRaisesRegex(ValueError,'changed after'):data.freeze(root/'cache')
                other=root/'other';other.mkdir();(other/'parihaka.npy').write_bytes(b'existing')
                with self.assertRaisesRegex(ValueError,'already exists'):data.freeze(other)

    def test_range_reader_rejects_ignored_truncated_or_wrong_ranges(self):
        class Response:
            status=206;headers={'Content-Range':'bytes 10-13/100'}
            def __enter__(self):return self
            def __exit__(self,*args):pass
            def read(self,count):return b'abcd'
        response=Response()
        with patch.object(data.urllib.request,'urlopen',return_value=response):
            self.assertEqual(data.range_data('https://example.test/data',10,4,100),b'abcd')
            response.status=200
            with self.assertRaisesRegex(ValueError,'exact frozen'):data.range_data('https://example.test/data',10,4,100)
            response.status=206;response.headers={'Content-Range':'bytes 11-14/100'}
            with self.assertRaises(ValueError):data.range_data('https://example.test/data',10,4,100)
            response.headers={'Content-Range':'bytes 10-14/100'}
            with self.assertRaisesRegex(ValueError,'Truncated'):data.range_data('https://example.test/data',10,5,100)

    def test_block_summary_retains_negative_primary_and_rejects_missing_controls(self):
        plan=dict(time_windows=[[128,256]],hidden_sets=['periodic'],seeds=[1],
                  primary=dict(survey='parihaka',count=128))
        rows=[]
        for block in range(6):
            for method,score in [('Jitter',2),('SA',3),('RandomBudget',4),('Hybrid',1)]:
                rows.append(dict(survey='parihaka',block=str(block),window_start=128,hidden_set='periodic',
                    m=128,seed=1,method=method,snr=score,mu=.1,full_fft_mu=.2))
        _,summary,_=summarize(rows,plan)
        self.assertEqual(len(summary),3)
        self.assertTrue(all(r['mean_delta']<0 and not r['all_blocks_positive'] for r in summary))
        with self.assertRaisesRegex(ValueError,'Missing frozen'):summarize(rows[1:],plan)
        with self.assertRaisesRegex(ValueError,'Duplicate'):summarize(rows+[rows[0]],plan)

    def test_report_rejects_table_mutation_even_with_a_matching_manifest_receipt(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);table=root/'recovery.csv';table.write_text('case,snr\na,1\n')
            manifest=root/'manifest.json'
            manifest.write_text(json.dumps(dict(output_tables_sha256={'recovery.csv':digest(table)})))
            (root/'verification.json').write_text(json.dumps(dict(status='passed',manifest_sha256=digest(manifest))))
            table.write_text('case,snr\na,99\n')
            with self.assertRaisesRegex(ValueError,'table changed'):report(root,root/'report')
            self.assertFalse((root/'report').exists())
