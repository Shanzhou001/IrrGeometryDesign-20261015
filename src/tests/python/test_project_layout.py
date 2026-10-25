"""Regression contracts for the directory and image-output refactor."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

from research.common.figures import FigureWriter, generation_pass
from research.common.paths import ROOT, central_figures


class LayoutTests(unittest.TestCase):
    def test_image_path_cannot_escape_central_directory(self):
        with self.assertRaises(ValueError):central_figures('../outside')

    def test_two_producers_cannot_save_the_same_image_in_one_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch('research.common.figures.central_figures',return_value=Path(directory)):
                with generation_pass() as counts:
                    first=FigureWriter('test');first.save(plt.figure(),'same')
                    second=FigureWriter('test');figure=plt.figure()
                    try:
                        with self.assertRaises(ValueError):second.save(figure,'same')
                    finally:plt.close(figure)
                    self.assertEqual(list(counts.values()),[1])
                    self.assertEqual(len(list(Path(directory).glob('*.png'))),1)

    def test_launcher_works_outside_the_repository(self):
        with tempfile.TemporaryDirectory() as directory:
            result=subprocess.run([sys.executable,str(ROOT/'pipeline.py'),'--help'],cwd=directory,capture_output=True)
            self.assertEqual(result.returncode,0,result.stderr.decode(errors='replace'))


    def test_legacy_batch_refuses_existing_output_before_preprocessing(self):
        from research.experiments import run_revised_studies as study
        with tempfile.TemporaryDirectory() as directory:
            folder=Path(directory)/'existing';folder.mkdir();sentinel=folder/'keep.txt';sentinel.write_bytes(b'original')
            with patch.object(sys,'argv',['study','--quick','--output',str(folder)]),patch.object(study,'prepare_models') as prepare:
                with self.assertRaises(FileExistsError):study.main()
                prepare.assert_not_called()
            self.assertEqual(sentinel.read_bytes(),b'original')


if __name__=='__main__':unittest.main()
