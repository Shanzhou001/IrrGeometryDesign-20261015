import unittest
import numpy as np
from research.analysis.waveform_quality import quality


class WaveformQualityTests(unittest.TestCase):
    def test_perfect_and_scaled_waveforms_separate_amplitude_from_phase(self):
        t=np.arange(128)[:,None];data=np.cos(2*np.pi*t*np.array([[.0625,.125]]))
        same=quality(data,data);scaled=quality(data,.5*data)
        self.assertAlmostEqual(same['rms_ratio'],1);self.assertAlmostEqual(same['centered_waveform_correlation'],1)
        self.assertAlmostEqual(same['band_phase_mae_degrees'],0);self.assertEqual(same['nmse'],0)
        self.assertAlmostEqual(scaled['rms_ratio'],.5);self.assertAlmostEqual(scaled['band_phase_mae_degrees'],0)

    def test_polarity_and_zero_predictions_do_not_get_false_phase_success(self):
        t=np.arange(128)[:,None];data=np.cos(2*np.pi*t/16)
        reverse=quality(data,-data);zero=quality(data,np.zeros_like(data))
        self.assertAlmostEqual(reverse['centered_waveform_correlation'],-1)
        self.assertAlmostEqual(reverse['band_phase_mae_degrees'],180)
        self.assertIsNone(zero['band_phase_mae_degrees']);self.assertEqual(zero['phase_target_energy_coverage'],0)
        self.assertIsNone(zero['centered_waveform_correlation']);self.assertEqual(zero['rms_ratio'],0)

    def test_invalid_or_zero_target_fails(self):
        with self.assertRaises(ValueError):quality(np.zeros((32,2)),np.ones((32,2)))
        with self.assertRaises(ValueError):quality(np.ones((32,2)),np.ones((31,2)))


if __name__=='__main__':unittest.main()
