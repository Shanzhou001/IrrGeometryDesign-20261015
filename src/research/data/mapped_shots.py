"""Read held-out land shots with checked source and receiver coordinates."""
from collections import defaultdict
from pathlib import Path

import numpy as np

from research.common.paths import ROOT
from research.data.field_segy import FieldSegy, mapping


POLAND = ROOT / 'data/public/poland_line001_field'
SODA = ROOT / 'data/public/soda_lake_2010_field'


def soda_waveform(ffid):
    folder = SODA if ffid == 7733 else SODA / 'additional'
    return folder / f'F{ffid}R1.SGY'


def station_coordinates(path, kind):
    stations = {}
    with path.open(encoding='cp1252') as stream:
        for line in stream:
            if not line.startswith(kind):
                continue
            key = line[1:17].strip(), int(line[17:25])
            xy = float(line[46:55]), float(line[55:65])
            if key in stations and stations[key] != xy:
                raise ValueError(f'Ambiguous station {key} in {path}')
            stations[key] = xy
    return stations


def receiver_span(receiver_points, first, last, count):
    """Expand the XPS endpoints across stations actually listed in the RPS."""
    points = [p for p in receiver_points if min(first, last) <= p <= max(first, last)]
    points.sort(reverse=first > last)
    if len(points) != count or not points or points[0] != first or points[-1] != last:
        raise ValueError(f'XPS receiver span {first}..{last} does not match {count} channels')
    return points


def soda_geometry(ffids):
    geometry = SODA / 'geometry'
    sources = station_coordinates(geometry / 'SodaLakeExportedSPS.sps', 'S')
    receivers = station_coordinates(geometry / 'SodaLakeExportedSPS.rps', 'R')
    receiver_points = defaultdict(list)
    for line, point in receivers:
        receiver_points[line].append(point)
    shots = {fid: (set(), {}) for fid in ffids}
    with (geometry / 'SodaLakeExportedSPS.xps').open(encoding='cp1252') as stream:
        for line in stream:
            if not line.startswith('X'):
                continue
            fid = int(line[7:11])
            if fid not in shots:
                continue
            source_keys, channels = shots[fid]
            source_keys.add((line[13:29].strip(), int(line[29:37])))
            first, last, step = int(line[38:42]), int(line[42:46]), int(line[46:47])
            receiver_line = line[47:63].strip()
            a, b = int(line[63:71]), int(line[71:79])
            if step == 0 and (a != b or first != last):
                raise ValueError(f'Invalid singleton channel increment in shot {fid}')
            expected = range(first, last + 1, max(1, step))
            points = receiver_span(receiver_points[receiver_line], a, b, len(expected))
            for channel, point in zip(expected, points):
                if channel in channels:
                    raise ValueError(f'Duplicate channel {channel} in shot {fid}')
                channels[channel] = receiver_line, point
    result = {}
    for fid, (source_keys, channels) in shots.items():
        if len(source_keys) != 1 or not channels or sorted(channels) != list(range(1, len(channels) + 1)):
            raise ValueError(f'Incomplete relations for shot {fid}')
        source = np.asarray(sources[next(iter(source_keys))])
        keys = [channels[k] for k in sorted(channels)]
        n = len(keys) // 3
        if len(keys) != n * 3 or keys != keys[:n] * 3 or len(set(keys[:n])) != n:
            raise ValueError(f'Inconsistent component blocks in shot {fid}')
        result[fid] = source, np.asarray([receivers[key] for key in keys[:n]])
    return result


def read_shots(plan, role):
    """Yield survey, field record, component, coordinates, source, dt and samples."""
    with FieldSegy(POLAND / 'Line_001.sgy') as segy:
        wanted = set(plan[f'poland_{role}'])
        grouped = {fid: {} for fid in wanted}
        for row in mapping(POLAND / 'trace_source_receiver_mapping.csv', segy):
            fid = int(row['field_record'])
            if fid in wanted:
                grouped[fid][int(row['channel'])] = row
        for fid in plan[f'poland_{role}']:
            first = plan['poland_first_channel']
            group = [grouped[fid][ch] for ch in range(first, first + plan['poland_receivers'])]
            xy = np.asarray([[float(r['receiver_x']), float(r['receiver_y'])] for r in group])
            sources = {(float(r['source_x']), float(r['source_y'])) for r in group}
            if len(sources) != 1:
                raise ValueError(f'Inconsistent source in shot {fid}')
            indices = [int(r['trace_index_zero_based']) for r in group]
            if np.any(segy.field(108, 'i2')[indices]):
                raise ValueError(f'Unsupported trace delay in shot {fid}')
            yield 'poland', fid, 0, xy, np.asarray(sources.pop()), segy.dt * 1e-6, segy.decode(indices, 0, segy.ns)
    for fid, (source, xy) in soda_geometry(plan[f'soda_{role}']).items():
        with FieldSegy(soda_waveform(fid)) as segy:
            live = np.flatnonzero(segy.field(28, 'i2') == 1)
            if len(live) != 3 * len(xy) or np.any(segy.field(8, 'i4')[live] != fid):
                raise ValueError(f'Live trace/FFID mismatch in shot {fid}')
            channels = segy.field(12, 'i4')[live]
            if channels[0] not in (1, 2) or not np.array_equal(channels, np.arange(channels[0], channels[0] + len(live))):
                raise ValueError(f'Unexpected trace channel order in shot {fid}')
            actual_xy = np.column_stack([segy.field(k, 'i4')[live] for k in (80, 84)]) * .1
            actual_source = np.column_stack([segy.field(k, 'i4')[live] for k in (72, 76)]) * .1
            if (np.max(abs(actual_xy - np.tile(xy, (3, 1)))) > .051 or
                    np.max(abs(actual_source - source)) > 1 or np.any(segy.field(108, 'i2')[live])):
                raise ValueError(f'Header geometry or delay mismatch in shot {fid}')
            chosen = np.argsort(np.linalg.norm(xy - source, axis=1), kind='stable')[:plan['soda_receivers']]
            if len(chosen) != plan['soda_receivers']:
                raise ValueError(f'Insufficient receivers in shot {fid}')
            chosen.sort()
            for component, block in enumerate(live.reshape(3, -1), 1):
                yield 'soda', fid, component, xy[chosen], source, segy.dt * 1e-6, segy.decode(block[chosen], 0, segy.ns)
