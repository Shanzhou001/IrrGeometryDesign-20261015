"""Paper rendering must work with external results and preserve unfavorable outcomes."""
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import matplotlib
matplotlib.use('Agg')

from research.analysis import paper_figures as paper
from research.analysis import summarize_audit_v5 as summary
from research.common.figures import FigureWriter
from research.common.hashing import digest
from research.common.tabular import write_rows


def fixture(root):
    """Small artificial result tables for renderer contracts, never research evidence."""
    v4, v5 = root/'external-v4', root/'external-v5'
    for domain, counts in paper.COUNTS.items():
        records = [dict(domain=domain, rate_index=rate, m=count, seed=seed,
                        method=method, budget_cap=600, mu=value)
                   for rate, count in enumerate(counts) for method in paper.SEARCH_METHODS
                   for seed, value in [(11, .2), (22, .4)]]
        path = v4/f'search_{domain}_600/search.csv'; path.parent.mkdir(parents=True)
        write_rows(path, records)
    table = v5/'analysis_thesis'; table.mkdir(parents=True)
    pairs = [dict(domain=domain, rate=rate, comparison=comparison,
                  difference=-.01, ci_low=-.02, ci_high=0.)
             for domain in paper.COUNTS for rate in range(3) for comparison in paper.COMPARISONS]
    for name in ('search_pairs', 'common_evaluation_pairs'):
        write_rows(table/(name+'.csv'), pairs)
    joint = [dict(m=count, recovery=recovery, method=method, mean_snr=mean,
                  ci_low=mean-.5, ci_high=mean+.5, analysis_units=6, unit='clean structural template')
             for count in paper.COUNTS['uniform'] for recovery, mean in [('fixed', -3.), ('selected', -1.)]
             for method in ('Jitter', *paper.SEARCH_METHODS)]
    write_rows(table/'joint_means.csv', joint)
    real = [dict(window=window, m=count, seed=seed, fixed_snr=fixed, selected_snr=selected)
            for window in paper.WINDOWS for count in paper.COUNTS['physical']
            for seed, fixed, selected in [(11, -4., -1.), (22, -2., .3)]]
    path = v5/'real/real.csv'; path.parent.mkdir()
    write_rows(path, real)
    provenance = dict(status='not_used', independent_real_sources=0, field_claims_eligible=False)
    (v4/'run_manifest.json').write_text(json.dumps(dict(fingerprint=dict(configuration=dict(real_source_provenance=provenance)))))
    (v5/'manifest.json').write_text(json.dumps(dict(configuration=dict(real_source_provenance=provenance))))
    (v5/'real/provenance.json').write_text(json.dumps(provenance))
    return v4, v5


class PaperFigureTests(unittest.TestCase):
    def test_analysis_missing_inputs_does_not_create_an_output_directory(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder); output = root/'v5'; output.mkdir()
            with patch.object(summary, 'V4', root/'v4'), patch.object(summary, 'WORKSPACE', root/'workspace'):
                with self.assertRaises(FileNotFoundError) as failure:
                    summary.run(output)
            self.assertIn('search.csv', str(failure.exception))
            self.assertIn('real_gather_59x256.csv', str(failure.exception))
            self.assertEqual(list(output.iterdir()), [])

    def test_external_tables_render_without_sidecars_and_keep_negative_snr(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder); v4, v5 = fixture(root)
            output = root/'images'; output.mkdir(); limits = []
            save = FigureWriter.save
            def capture(writer, figure, name, *args, **kwargs):
                if name == 'recovery':
                    limits.extend(ax.get_ylim() for ax in figure.axes)
                    self.assertIn('zero / linear / curvelet', figure.axes[1].get_title())
                return save(writer, figure, name, *args, **kwargs)
            with patch('research.common.figures.central_figures', return_value=output), \
                    patch.object(FigureWriter, 'save', capture):
                self.assertEqual(paper.render(v4, v5), list(paper.FIGURE_IDS))
            self.assertEqual(len(list(output.glob('*.png'))), 4)
            self.assertLess(limits[0][0], -3.5)
            self.assertLess(limits[1][0], -1.5)
            archive = json.loads((output/'search.data.json').read_text())
            self.assertEqual(len(archive['records']), 24)
            self.assertAlmostEqual(archive['records'][0]['mean'], .3)
            source = 'v4/search_uniform_600/search.csv'
            self.assertEqual(archive['sources_sha256'][source], digest(v4/'search_uniform_600/search.csv'))
            self.assertTrue(all(key.startswith(('v4/', 'v5/')) for key in archive['sources_sha256']))
            self.assertEqual(archive['source_provenance']['status'], 'not_used')

    def test_unconfirmed_archives_are_visibly_labeled_as_sensitivity(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder); v4, v5 = fixture(root)
            (v5/'real/provenance.json').write_text('{}')
            output = root/'images'; output.mkdir(); titles = []
            def capture(writer, figure, name, *args, **kwargs):
                titles.append(figure._suptitle.get_text())
                self.assertEqual(args[0]['source_provenance']['status'], 'unconfirmed_sensitivity')
                import matplotlib.pyplot as plt
                plt.close(figure)
            with patch('research.common.figures.central_figures', return_value=output), \
                    patch.object(FigureWriter, 'save', capture):
                paper.render(v4, v5)
            self.assertEqual(len(titles), 4)
            self.assertTrue(all('Sensitivity diagnostic' in title for title in titles))

    def test_conflicting_source_declarations_do_not_establish_field_evidence(self):
        first = dict(status='declared_confirmed', field_claims_eligible=True, source_sha256='a')
        second = dict(first, source_sha256='b')
        status = paper.source_status(dict(fingerprint=dict(configuration=dict(real_source_provenance=first))),
                                     dict(configuration=dict(real_source_provenance=first)), second)
        self.assertEqual(status['status'], 'unconfirmed_sensitivity')

    def test_missing_inputs_are_reported_together_before_output(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            with patch.object(paper, 'FigureWriter') as writer:
                with self.assertRaises(FileNotFoundError) as failure:
                    paper.render(root/'v4', root/'v5')
                writer.assert_not_called()
            for name in ('search_uniform_600/search.csv', 'search_physical_600/search.csv',
                         'search_pairs.csv', 'common_evaluation_pairs.csv', 'joint_means.csv', 'real/real.csv'):
                self.assertIn(name, str(failure.exception).replace('\\', '/'))
            self.assertEqual(list(root.iterdir()), [])

    def test_incomplete_recovery_table_fails_before_any_image(self):
        with tempfile.TemporaryDirectory() as folder:
            v4, v5 = fixture(Path(folder)); path = v5/'analysis_thesis/joint_means.csv'
            lines = path.read_text().splitlines(); path.write_text('\n'.join(lines[:-1])+'\n')
            with patch.object(paper, 'FigureWriter') as writer:
                with self.assertRaisesRegex(ValueError, 'common-H recovery groups'):
                    paper.render(v4, v5)
                writer.assert_not_called()

    def test_search_methods_must_use_matching_seeds(self):
        with tempfile.TemporaryDirectory() as folder:
            v4, v5 = fixture(Path(folder)); path = v4/'search_uniform_600/search.csv'
            content = path.read_text().replace('0,38,11,Hybrid', '0,38,33,Hybrid')
            path.write_text(content)
            with patch.object(paper, 'FigureWriter') as writer:
                with self.assertRaisesRegex(ValueError, 'share the same seeds'):
                    paper.render(v4, v5)
                writer.assert_not_called()


if __name__ == '__main__':
    unittest.main()
