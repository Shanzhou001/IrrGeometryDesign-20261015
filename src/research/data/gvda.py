"""Registered Garner Valley receiver counts and source/station geometry."""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import csv
import json
from pathlib import Path
import re
import time
from urllib.parse import quote, urlencode, urlparse
from urllib.request import urlopen

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT


DATA = ROOT / 'data/public/gvda_2019_field'
REGISTRY = Path(__file__).with_name('gvda_inputs.json')
DOI = '10.17603/ds2-ngdh-af80'
DT = .0025


def fetch(record, directory=DATA):
    """Fetch one publisher-listed file, retaining verified completed downloads."""
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    target = directory / record['name']
    if target.is_file():
        if target.stat().st_size != record['bytes'] or (
                record.get('sha256') and digest(target) != record['sha256']):
            raise ValueError('Existing GVDA input differs from registration: ' + target.name)
        return
    prefix = 'https://www.designsafe-ci.org/api/datafiles/tapis/public/preview/designsafe.storage.published/'
    for attempt in range(7):
        endpoint = prefix + quote(record['source_path'], safe='/' if attempt % 2 else '')
        endpoint += '?' + urlencode({'doi': DOI})
        try:
            with urlopen(endpoint, timeout=15) as stream:
                url = json.load(stream)['href']
            if urlparse(url).hostname != 'designsafe.tapis.io':
                raise ValueError('Unexpected public download host')
            with urlopen(url, timeout=25) as stream:
                payload = stream.read(record['bytes'] + 1)
            if len(payload) != record['bytes'] or b',' not in payload[:350]:
                raise ValueError('Publisher file size or CSV framing differs')
            temporary = target.with_suffix(target.suffix + '.part')
            temporary.write_bytes(payload)
            if record.get('sha256') and digest(temporary) != record['sha256']:
                raise ValueError('Publisher file hash differs from registration')
            temporary.replace(target)
            receipt = dict(doi=DOI, api=endpoint, source_path=record['source_path'],
                           bytes=len(payload), sha256=digest(target))
            target.with_suffix('.json').write_text(json.dumps(receipt, indent=2) + '\n')
            print('downloaded', target.name, len(payload), flush=True)
            return
        except OSError:
            if attempt == 6:
                raise
            time.sleep(min(4, attempt + 1))


def prepare():
    registry = json.loads(REGISTRY.read_text(encoding='utf-8'))
    failures = []
    with ThreadPoolExecutor(max_workers=6) as pool:
        jobs = {pool.submit(fetch, record): record['name'] for record in registry['files']}
        for job in as_completed(jobs):
            try:
                job.result()
            except (OSError, ValueError) as error:
                failures.append(dict(file=jobs[job], error=str(error)))
    if failures:
        raise ValueError('Incomplete GVDA acquisition: ' + json.dumps(failures))
    return {r['name']: r['sha256'] for r in registry['files']}


def coordinates(path, key):
    with path.open(newline='', encoding='utf-8-sig') as stream:
        result = {}
        for row in csv.DictReader(stream):
            identity = int(row[key])
            xy = float(row['Rel. X (m)']), float(row['Rel. Y (m)'])
            if identity in result or not np.isfinite(xy).all():
                raise ValueError('Duplicate or invalid published station identity')
            result[identity] = xy
    return result


def read(item, plan):
    registry = {r['name']: r for r in json.loads(REGISTRY.read_text())['files']}
    names = [r['name'] for r in item['files'].values()]
    names += ['metadata_source_hammer.csv', 'metadata_station.csv']
    inputs = {REGISTRY.relative_to(ROOT).as_posix(): digest(REGISTRY)}
    for name in names:
        record = registry[name]
        path = DATA / name
        if path.stat().st_size != record['bytes'] or digest(path) != record['sha256']:
            raise ValueError('GVDA input changed: ' + name)
        inputs[path.relative_to(ROOT).as_posix()] = record['sha256']
    sources = coordinates(DATA / 'metadata_source_hammer.csv', 'Source Number')
    stations = coordinates(DATA / 'metadata_station.csv', 'Station#')
    paths = {k: DATA / value['name'] for k, value in item['files'].items()}
    with paths['rec_z'].open(newline='') as stream:
        header = next(csv.reader(stream))
        matches = [re.fullmatch(r'Station (\d+) \(counts\)', name) for name in header[1:]]
        if header[0] != 'Time (s)' or not all(matches):
            raise ValueError('Invalid published receiver columns')
        receiver_ids = [int(match.group(1)) for match in matches]
        if len(receiver_ids) != 196 or len(set(receiver_ids)) != 196 or set(receiver_ids) != set(stations):
            raise ValueError('Waveform columns differ from the published 196-station array')
        recorded = np.loadtxt(stream, delimiter=',')
    with paths['src'].open(newline='') as stream:
        if next(csv.reader(stream)) != ['Time (s)', 'Hammerout (counts)']:
            raise ValueError('Unexpected source signature columns')
        force = np.loadtxt(stream, delimiter=',')
    if (not np.isfinite(recorded).all() or not np.isfinite(force).all() or
            not np.allclose(np.diff(recorded[:, 0]), DT, rtol=0, atol=1e-9) or
            not np.all(np.diff(force[:, 0]) > 0) or
            np.max(abs(force[:, 1])) == 0):
        raise ValueError('Invalid published time axis or force signature')
    onset = float(force[np.argmax(abs(force[:, 1])), 0])
    windows = []
    for lo, hi in plan['windows_seconds']:
        start = int(np.searchsorted(recorded[:, 0], onset + lo - 1e-10))
        stop = start + round((hi - lo) / DT)
        if not 0 <= start < stop <= len(recorded):
            raise ValueError('Fixed score window falls outside the recorded trace')
        windows.append((start, stop))
    xy = np.array([stations[i] for i in receiver_ids])
    return xy, np.array(sources[item['source']]), recorded[:, 1:], windows, onset, inputs


if __name__ == '__main__':
    argparse.ArgumentParser(description=__doc__).parse_args()
    print(json.dumps(dict(status='passed', input_sha256=prepare()), indent=2))
