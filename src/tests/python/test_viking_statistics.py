"""Aggregation keeps mask repeats separate from geological sample counts."""
import unittest

from research.analysis.viking_v2_results import statistics


class VikingAggregationTests(unittest.TestCase):
    def test_seed_variation_is_reported_without_creating_new_surveys(self):
        recovery=[]
        for shot,base in [('53',1.),('453',3.)]:
            for seed,delta in [('84011',0.),('94217',2.)]:
                record=dict(ffid=shot,window_start='512',hidden_set='periodic',m='60',seed=seed,method='Jitter')
                for name in ('zero','linear','cold','fk','selected'):
                    record[name+'_snr']=base+delta;record[name+'_nmse']=.5
                recovery.append(record)
        search=[dict(m='60',method='Jitter',mu='.2',max_gap_m='100',evals='1')]
        # The real reader enforces all five methods; provide identical controls
        # here to test only the aggregation contract on a small example.
        for method in ('SA_Mu','Adaptive_Mu','Adaptive_Gap','Adaptive_Signal'):
            recovery += [dict(r,method=method) for r in list(recovery) if r['method']=='Jitter']
            search.append(dict(search[0],method=method))
        tables=statistics(search,recovery)
        overall=next(r for r in tables['overall_means'] if r['method']=='Jitter' and r['recovery']=='selected')
        self.assertEqual(overall['mean_snr'],3.)
        self.assertEqual(overall['independent_surveys'],1)
        seeds={r['seed']:r for r in tables['mask_seed_means'] if r['method']=='Jitter' and r['recovery']=='selected'}
        self.assertEqual(seeds['84011']['mean_snr'],2.)
        self.assertEqual(seeds['94217']['mean_snr'],4.)
        self.assertTrue(all(r['mask_seed_count']==2 for r in tables['cell_means']))
        self.assertTrue(all(r['independent_surveys']==1 for r in tables['mask_seed_means']))


if __name__=='__main__':unittest.main()
