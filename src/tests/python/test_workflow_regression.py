"""The default experiment must retain the inputs required by its final audit."""
import argparse
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from research.common.paths import executable
from research.data.viking import DEFAULT_DATASET
from research.experiments import viking_v2
from research.validation.verify_viking_batch import verify


@unittest.skipUnless(executable().is_file() and DEFAULT_DATASET.is_dir(), 'registered Viking/native runtime required')
class WorkflowRegressionTests(unittest.TestCase):
    def test_default_run_finishes_and_remains_independently_verifiable(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp)
            args=argparse.Namespace(out=root/'batch', group='smoke', dataset=DEFAULT_DATASET,
                exe=executable(), shots=[53], windows=[512], counts=[60], seeds=[84011],
                hidden_sets=['r271828'],budget=8,inner_folds=1,workers=4,keep_intermediates=False)
            with patch.object(viking_v2,'FIGURES',root/'figures'), patch('research.common.paths.FIGURES',root/'figures'):
                state=viking_v2.run(args)
            self.assertTrue(state['completed'])
            self.assertEqual(state['recovery_cases'],5)
            verify(root/'batch',root/'verification')
            self.assertTrue((root/'verification/verification.json').is_file())
