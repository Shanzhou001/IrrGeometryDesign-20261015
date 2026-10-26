"""Validate the registered Viking subset against raw IBM SEG-Y and measured geometry."""
from dataclasses import dataclass
import json
from pathlib import Path

import numpy as np

from research.common.hashing import digest
from research.common.paths import WORKSPACE
from research.common.tabular import rows
from research.data.audit_segy_bytes import ibm_float
from research.experiments.review_studies import read_gather

DEFAULT_DATASET = WORKSPACE/'data/public/viking_line12_small'
REGISTRY = Path(__file__).with_name('viking_line12_subset.json')


@dataclass(frozen=True)
class Shot:
    ffid: int
    data: np.ndarray
    x: np.ndarray
    sample_interval_s: float
    time_origin_s: float


def decode_shot(path, expected_ffid):
    """Read exactly one 120-channel shot; do not guess an encoding from amplitudes."""
    raw = Path(path).read_bytes()
    ns, nx, stride = 1500, 120, 6240
    if len(raw) != 3600 + nx*stride:
        raise ValueError('Viking shot has a truncated or unexpected SEG-Y layout')
    def number(buffer, offset, width=4, signed=True):
        return int.from_bytes(buffer[offset:offset+width], 'big', signed=signed)
    if tuple(number(raw, offset, 2, False) for offset in (3216, 3220, 3224, 3254)) != (4000, ns, 1, 1):
        raise ValueError('Viking requires documented big-endian IBM / 4000 us / metre measurement system')
    headers = [raw[3600+j*stride:3840+j*stride] for j in range(nx)]
    if any(number(h, 8) != expected_ffid or number(h, 114, 2, False) != ns or
           number(h, 116, 2, False) != 4000 for h in headers):
        raise ValueError('Mixed shot identity, sample count or sampling interval')
    scalars = np.array([number(h, 70, 2) for h in headers])
    factors = np.where(scalars > 0, scalars, 1/np.maximum(abs(scalars), 1))
    sx = np.array([number(h, 72) for h in headers])*factors
    gx = np.array([number(h, 80) for h in headers])*factors
    offset = np.array([number(h, 36) for h in headers])
    if (len(set(sx)) != 1 or any(number(h, 76) or number(h, 84) for h in headers) or
            not np.array_equal(abs(offset), abs(gx-sx))):
        raise ValueError('Viking shot is not the documented collinear geometry')
    order = np.argsort(gx, kind='stable')
    if not np.array_equal(np.diff(gx[order]), np.full(nx-1, 25.)):
        raise ValueError('Receiver coordinates do not match the documented 25 m spacing')
    words = np.ndarray((nx, ns), dtype='>u4', buffer=raw, offset=3840, strides=(stride, 4))
    data = ibm_float(words).astype(np.float32).T[:, order]
    if not np.isfinite(data).all() or not np.any(data):
        raise ValueError('Invalid or zero-energy Viking payload')
    geometry = [dict(output_column_zero_based=column, original_channel=number(headers[j], 12),
                     source_x_m=sx[j], receiver_x_m=gx[j], offset_header_m=offset[j],
                     coordinate_scalar_raw=scalars[j], coordinate_units_raw=number(headers[j], 88, 2))
                for column, j in enumerate(order)]
    return data, gx[order], geometry


def load_dataset(directory=DEFAULT_DATASET, shot_ids=None):
    """Pinned hashes plus raw/CSV equivalence; local manifest edits cannot bless changed data."""
    directory = Path(directory).resolve()
    registered = json.loads(REGISTRY.read_text(encoding='utf-8'))
    selected = {entry['ffid'] for entry in registered['shots']} if shot_ids is None else set(shot_ids)
    if not selected or selected - {entry['ffid'] for entry in registered['shots']}:
        raise ValueError('Select one or more registered Viking FFIDs')
    expected = {'source_docs/'+name: info for name, info in registered['source_docs'].items()}
    for entry in registered['shots']:
        expected.update(entry['files'])
    missing = [str(directory/name) for name in expected if not (directory/name).is_file()]
    if missing:
        raise FileNotFoundError('Viking dataset inputs are missing:\n'+'\n'.join(missing))
    verified = {}
    for name, info in expected.items():
        path = directory/name
        if ('bytes' in info and path.stat().st_size != info['bytes']) or digest(path) != info['sha256']:
            raise ValueError('Viking registered file identity changed: '+name)
        verified[str(path)] = info['sha256']
    shots = []
    for entry in registered['shots']:
        raw_path = next(directory/name for name in entry['files'] if name.startswith('raw/'))
        csv_path = next(directory/name for name in entry['files'] if name.startswith('csv_'))
        geometry_path = next(directory/name for name in entry['files'] if name.startswith('geometry/'))
        raw, x, geometry = decode_shot(raw_path, entry['ffid'])
        data, csv_x = read_gather(csv_path)
        if (data.shape != (256, 120) or not np.array_equal(csv_x, x) or
                not np.array_equal(data.astype(np.float32), raw[512:768])):
            raise ValueError('Derived CSV does not preserve the registered raw crop and geometry')
        saved = rows(geometry_path)
        if len(saved) != len(geometry) or any(float(a[key]) != float(b[key])
                for a, b in zip(saved, geometry) for key in b):
            raise ValueError('Derived geometry table does not match SEG-Y trace headers')
        if geometry[0]['source_x_m'] != entry['source_x_m']:
            raise ValueError('Source coordinate differs from registered acquisition')
        if entry['ffid'] in selected:
            shots.append(Shot(entry['ffid'], data, x, .004, 2.048))
    report = dict(protocol=registered['protocol'], status='documented_public_subset_with_header_caveat',
                  dataset_directory=str(directory), registry_sha256=digest(REGISTRY),
                  reference_record=registered['reference_record'], citation=registered['citation'],
                  selected_ffids=[shot.ffid for shot in shots], registered_shots_verified=len(registered['shots']),
                  independent_surveys=1, input_sha256=verified,
                  original_format=registered['original_format'], window_samples=[512, 768],
                  sample_interval_s=.004, time_origin_s=2.048,
                  full_file_sha256_locally_verified=False,
                  coordinate_distance_scale_evidence=registered['coordinate_distance_scale_evidence'],
                  coordinate_units_header_note=registered['coordinate_units_header_note'],
                  amplitude_units='recorded digital amplitudes; absolute calibration is not established',
                  scope='Within-survey measured-trace prediction; shots and mask seeds are not independent surveys')
    return shots, report


def load_windows(starts, directory=DEFAULT_DATASET, shot_ids=None):
    """Reuse the verified raw shots for prespecified 256-sample native-rate windows."""
    starts = list(starts)
    if not starts or len(set(starts)) != len(starts) or any(not isinstance(s, int) or s < 0 or s+256 > 1500 for s in starts):
        raise ValueError('Window starts must be distinct integers inside the 1500-sample raw record')
    selected, provenance = load_dataset(directory, shot_ids)
    registered = json.loads(REGISTRY.read_text(encoding='utf-8'))
    windows = {start: {} for start in starts}
    for shot in selected:
        entry = next(r for r in registered['shots'] if r['ffid'] == shot.ffid)
        path = next(Path(directory)/name for name in entry['files'] if name.startswith('raw/'))
        raw, x, _ = decode_shot(path, shot.ffid)
        for start in starts:
            windows[start][shot.ffid] = Shot(shot.ffid, raw[start:start+256].astype(float), x, .004, start*.004)
    provenance = dict(provenance, study_windows=[dict(start_sample=s, stop_sample=s+256, time_origin_s=s*.004) for s in starts])
    return windows, provenance
