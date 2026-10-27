"""Registered, bounded F3 poststack sections for a frozen external transfer test."""
import json
from pathlib import Path
import numpy as np
from research.common.hashing import digest
from research.common.paths import ROOT
from research.data.viking import Shot

DEFAULT=ROOT/'data/public/f3_transfer_small'
REGISTRY=Path(__file__).with_name('f3_transfer_subset.json')


def load(directory=DEFAULT):
    directory=Path(directory).resolve();state=json.loads(REGISTRY.read_text());inputs={};shots={}
    for name,info in state['files'].items():
        path=directory/name
        if not path.is_file() or path.stat().st_size!=info['bytes'] or digest(path)!=info['sha256']:
            raise ValueError('Registered F3 input is missing or changed: '+name)
        inputs[str(path)]=info['sha256']
        if 'section_index' in info:
            with np.load(path,allow_pickle=False) as saved:data=saved['data'].copy()
            if data.shape!=(255,120) or not np.isfinite(data).all() or not np.any(data):
                raise ValueError('Invalid F3 section')
            # Legacy numerical carrier: these are sample/trace indices, not a
            # claim that the benchmark provides physical shot/time coordinates.
            shots[info['section_index']]=Shot(info['section_index'],data,np.arange(120,dtype=float),1.,0.)
    return {0:shots},dict(state,input_sha256=inputs,registry_sha256=digest(REGISTRY),independent_surveys=1,
                          dataset_directory=str(directory),time_unit='sample_index',spatial_unit='trace_interval')
