import io
import unittest
import numpy as np
from research.data.prepare_f3_subset import npy_header
from research.experiments.viking_v2 import signal_weights


class ExternalDataTests(unittest.TestCase):
    def test_npy_prefix_interprets_shape_without_pickle_or_full_volume(self):
        data=np.arange(3*120*255,dtype=np.float64).reshape(3,120,255);stream=io.BytesIO();np.save(stream,data)
        value=stream.getvalue();info,offset,dtype=npy_header(value[:1024])
        self.assertEqual(info['shape'],data.shape)
        np.testing.assert_array_equal(np.frombuffer(value,dtype=dtype,count=120*255,offset=offset).reshape(120,255),data[0])
        for invalid in [b'notnumpy',b'\x93NUMPY\xff\xff0000']:
            with self.assertRaises(ValueError):npy_header(invalid)

    def test_frequency_weights_are_invariant_to_explicit_time_unit_conversion(self):
        data=np.random.default_rng(27).normal(size=(255,12));indices=np.arange(12)*10
        physical,_=signal_weights(data,indices,120,band=(5,60),sample_interval_s=.004)
        normalized,_=signal_weights(data,indices,120,band=(.02,.24),sample_interval_s=1.)
        np.testing.assert_allclose(physical,normalized,atol=1e-14)


if __name__=='__main__':unittest.main()
