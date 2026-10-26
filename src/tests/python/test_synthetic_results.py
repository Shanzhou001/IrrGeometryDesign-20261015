"""Self-contained contracts for the synthetic analysis evidence chain."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from research.analysis import synthetic_results as analysis
from research.analysis import summarize_review
from research.common.hashing import digest
from research.common.tabular import rows, write_rows


def fixture(root):
    """Artificial CSV fixtures; they never replace measured experiment results."""
    v4=root/'v4';joint=root/'joint';v4.mkdir();joint.mkdir();search=v4/'search_uniform_200';search.mkdir()
    (search/'masks').mkdir();data=[];outputs={}
    for rate,m in enumerate(analysis.COUNTS):
        for index,method in enumerate((*analysis.METHODS,'NoRecycle')):
            for seed in [11,22]:
                mu=.2+index*.01+rate*.001+seed*.0001
                name=f'masks/{method}_{m}_{seed}.txt'
                trace=search/(name+'.trace.csv')
                trace_rows=[dict(best_objective=.6),dict(best_objective=mu)] if method!='NoRecycle' else [dict(best_objective=mu)]
                write_rows(trace,trace_rows);outputs[trace.relative_to(v4).as_posix()]=digest(trace)
                data.append(dict(domain='uniform',n=128,rate_index=rate,m=m,seed=seed,method=method,budget_cap=200,
                                 lambda_gap=0,lambda_nn=0,mu=mu,evals=len(trace_rows),mask_path=name))
    write_rows(search/'search.csv',data);outputs['search_uniform_200/search.csv']=digest(search/'search.csv')
    provenance=dict(status='not_used',field_claims_eligible=False,independent_real_sources=0)
    config=dict(synthetic_only=True,real_source_provenance=provenance,budgets=[200])
    program='f'*64
    manifest=dict(fingerprint=dict(configuration=config,executable_sha256=program),steps=dict(search=dict(exit_code=0,outputs=outputs)))
    (v4/'run_manifest.json').write_text(json.dumps(manifest))
    common=[]
    eligible=[j for j in range(128) if j%8!=4]
    hidden=''.join('1' if j%8==4 else '0' for j in range(128))
    for m in analysis.COUNTS:
        mask=''.join('1' if j in eligible[:m] else '0' for j in range(128))
        for method_index,method in enumerate(analysis.METHODS):
            for template in range(6):
                for noise in range(2):
                    for seed_index,seed in enumerate([11,22,33]):
                        fixed=-template-1+noise*.1+seed_index*.01+method_index*.1
                        common.append(dict(design='common_H',m=m,method=method,model=f'model_{template*2+noise:02d}',
                                           family=['dipping','curved','faulted'][template//2],structure=template%2,noise=noise,
                                           seed=seed,mask=mask,evaluation_bits=hidden,fixed_snr=fixed,selected_snr=fixed+.4))
    write_rows(joint/'joint.csv',common)
    (joint/'manifest.json').write_text(json.dumps(dict(completed=True,configuration=config,executable_sha256=program,
                                                     output_tables_sha256={'joint.csv':digest(joint/'joint.csv')},steps={})))
    return v4,joint


class SyntheticResultTests(unittest.TestCase):
    def test_measured_tables_produce_template_statistics_and_three_figures(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);v4,joint=fixture(root);images=root/'images';images.mkdir()
            with patch('research.common.figures.central_figures',return_value=images):
                manifest=analysis.run(v4,joint,root/'analysis',budgets=(200,))
            means=rows(root/'analysis/joint_means.csv')
            self.assertEqual(len(means),30)
            self.assertTrue(all(r['analysis_units']=='6' for r in means))
            self.assertTrue(all(float(r['mean_snr'])<0 for r in means))
            self.assertEqual(len(rows(root/'analysis/joint_template_means.csv')),180)
            self.assertEqual(len(list(images.glob('*.png'))),3)
            self.assertEqual(manifest['independent_real_sources'],0)
            final=next(r for r in rows(root/'analysis/search_pairs.csv') if r['comparison']=='Adaptive-NoRecycle')
            common=next(r for r in rows(root/'analysis/common_evaluation_pairs.csv') if r['comparison']=='Adaptive-NoRecycle')
            self.assertLess(float(final['difference']),0)
            self.assertGreater(float(common['difference']),0)

    def test_changed_raw_table_is_rejected_before_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);v4,joint=fixture(root)
            with (joint/'joint.csv').open('a') as stream:stream.write('\n')
            with self.assertRaisesRegex(ValueError,'changed raw artifact'):
                analysis.run(v4,joint,root/'analysis',budgets=(200,))
            self.assertFalse((root/'analysis').exists())

    def test_legacy_report_redirects_synthetic_batches_before_writing(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);v4,joint=fixture(root)
            before={p.name for p in v4.iterdir()}
            with self.assertRaisesRegex(ValueError,'synthetic_results'):
                summarize_review.summarize(v4)
            self.assertEqual(before,{p.name for p in v4.iterdir()})

    def test_legacy_missing_confirmation_remains_sensitivity(self):
        scope=summarize_review.source_scope(dict(fingerprint=dict(configuration={})))
        self.assertEqual(scope['status'],'unconfirmed_sensitivity')
        self.assertFalse(scope['field_claims_eligible'])


if __name__=='__main__':unittest.main()
