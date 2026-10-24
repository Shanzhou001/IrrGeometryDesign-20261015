"""Independent, descriptive SEG-Y byte audit; never selects a decoding profile."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
import struct

import numpy as np


def ibm_float(words):
    """IBM hexadecimal floating point, evaluated in float64 (including unnormalised words)."""
    words = np.asarray(words, dtype=np.uint32)
    sign = np.where(words >> 31, -1., 1.)
    return sign * ((words & 0xffffff).astype(float) / 2**24) * np.power(16., ((words >> 24) & 127).astype(int)-64)


TRACE_FIELDS = {
    'sequence_line': (0, 'i'), 'sequence_file': (4, 'i'), 'field_record': (8, 'i'),
    'trace_in_record': (12, 'i'), 'source_point': (16, 'i'), 'cdp': (20, 'i'),
    'trace_in_ensemble': (24, 'i'), 'trace_id': (28, 'h'), 'offset': (36, 'i'),
    'elevation_scalar': (68, 'h'), 'coordinate_scalar': (70, 'h'),
    'source_x': (72, 'i'), 'source_y': (76, 'i'), 'receiver_x': (80, 'i'),
    'receiver_y': (84, 'i'), 'coordinate_units': (88, 'h'),
    'delay_ms': (108, 'h'), 'ns': (114, 'H'), 'dt_us': (116, 'H'),
    'trace_weighting': (168, 'h'), 'cdp_x': (180, 'i'), 'cdp_y': (184, 'i'),
    'amplitude_unit': (202, 'h'), 'time_scalar': (214, 'h')}
BINARY_FIELDS = {'job': (3200, 'i'), 'line': (3204, 'i'), 'reel': (3208, 'i'),
                 'traces_per_ensemble': (3212, 'H'), 'dt_us': (3216, 'H'),
                 'ns': (3220, 'H'), 'format': (3224, 'H'), 'sorting': (3228, 'h'),
                 'measurement_system': (3254, 'h'), 'endian_test': (3296, 'I'),
                 'revision': (3500, 'H'), 'fixed_length': (3502, 'H'), 'extended_text': (3504, 'h')}


def values(raw, fields, endian):
    return {k: struct.unpack_from(endian+fmt, raw, offset)[0] for k, (offset, fmt) in fields.items()}


def describe_payload(payload):
    result = {}
    # Pairwise swapping is included because rev 2 explicitly defines that byte order.
    pair = np.frombuffer(payload, np.uint8).reshape(-1, 4)[:, [1, 0, 3, 2]].copy().tobytes()
    for label, raw, endian in [('LE', payload, '<'), ('BE', payload, '>'), ('pair_swapped_BE', pair, '>')]:
        words = np.frombuffer(raw, dtype=endian+'u4').astype(np.uint32)
        for fmt in ['IEEE', 'IBM']:
            x = np.frombuffer(raw, dtype=endian+'f4').astype(float) if fmt == 'IEEE' else ibm_float(words)
            finite = x[np.isfinite(x)]
            info = {'finite': int(len(finite)), 'total': len(x), 'zeros': int(np.sum(x == 0)),
                    'first_16': [float(v) if np.isfinite(v) else str(v) for v in x[:16]],
                    'abs_quantiles': np.quantile(abs(finite), [0, .25, .5, .75, .99, 1]).tolist() if len(finite) else [],
                    'rms': float(np.sqrt(np.mean(finite**2))) if len(finite) else None}
            if fmt == 'IBM':
                nonzero = (words & 0xffffff) != 0
                info['unnormalized_nonzero_fraction'] = int(np.sum(nonzero & ((words & 0xf00000) == 0)))
                info['noncanonical_zero_words'] = int(np.sum(~nonzero & (words != 0)))
            else:
                info['subnormal_words'] = int(np.sum(((words >> 23) & 255 == 0) & ((words & 0x7fffff) != 0)))
            result[fmt+'_'+label] = info
    return result


def audit(path, output):
    raw = path.read_bytes()
    if len(raw) < 3840:
        raise ValueError('truncated SEG-Y')
    binary = {label: values(raw, BINARY_FIELDS, endian) for label, endian in [('LE', '<'), ('BE', '>')]}
    ns = binary['LE']['ns']
    stride = 240+4*ns
    if ns < 1 or (len(raw)-3600) % stride:
        raise ValueError('not a complete fixed-length four-byte LE layout')
    output.mkdir(parents=True, exist_ok=False)
    text = {}
    for encoding in ['ascii', 'cp500', 'cp037']:
        s = raw[:3200].decode(encoding, errors='replace')
        text[encoding] = {'printable_fraction': sum(c.isprintable() for c in s)/3200,
                          'lines': [s[k:k+80] for k in range(0, 3200, 80)]}
    headers, payload = [], []
    for index, start in enumerate(range(3600, len(raw), stride)):
        header = raw[start:start+240]
        row = {'file_index': index, 'byte_offset_zero_based': start, 'header_hex': header.hex(),
               'header_sha256': hashlib.sha256(header).hexdigest()}
        for label, endian in [('LE', '<'), ('BE', '>')]:
            row.update({label+'_'+k: v for k, v in values(header, TRACE_FIELDS, endian).items()})
        headers.append(row)
        payload.append(raw[start+240:start+stride])
    with (output/'trace_headers.csv').open('w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=headers[0]);writer.writeheader();writer.writerows(headers)
    order = sorted(range(len(headers)), key=lambda i: (headers[i]['LE_offset'], i))
    result = {'source': str(path.resolve()), 'sha256': hashlib.sha256(raw).hexdigest(),
              'file_bytes': len(raw), 'binary_hex': raw[3200:3600].hex(), 'binary': binary,
              'text': text, 'layout': {'start': 3600, 'stride': stride, 'ns': ns, 'traces': len(headers),
                                      'payload_bytes': sum(map(len, payload)), 'trailing_bytes': 0},
              'trace_LE_unique': {k: sorted({h['LE_'+k] for h in headers}) for k in TRACE_FIELDS},
              'sorted_file_indices': order,
              'cdp_counts': {str(c): sum(h['LE_cdp'] == c for h in headers) for c in sorted({h['LE_cdp'] for h in headers})},
              'cdp_transitions_in_offset_order': sum(headers[a]['LE_cdp'] != headers[b]['LE_cdp'] for a, b in zip(order, order[1:])),
              'payload_first_64_bytes_hex': b''.join(payload)[:64].hex(),
              'interpretations': describe_payload(b''.join(payload)),
              'identification': 'UNRESOLVED: descriptive statistics and IBM canonicality do not prove provider intent'}
    (output/'audit.json').write_text(json.dumps(result, indent=2, allow_nan=False), encoding='utf-8')
    return result


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input', type=Path, default=Path('data/real_seismic/Data/RRAW.SGY'))
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args();r = audit(a.input, a.out)
    print(json.dumps({k:r[k] for k in ['sha256', 'layout', 'cdp_counts', 'identification']}))
