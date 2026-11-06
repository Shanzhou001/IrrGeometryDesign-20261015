"""Acquire one frozen project-unseen numeric volume without accessing labels."""
import argparse
from datetime import datetime, timezone
import hashlib
import io
import json
from pathlib import Path
import subprocess
import urllib.request
import urllib.error
import time
from concurrent.futures import ThreadPoolExecutor

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT

PROTOCOL=ROOT/'src/research/experiments/closure_protocol.json'
DEFAULT=ROOT/'data/public/closure_20261104'
AMENDMENT=PROTOCOL.with_name('closure_transport_amendment.json')


def freeze(directory=DEFAULT):
    directory=Path(directory);marker=directory/'freeze.json'
    plan=json.loads(PROTOCOL.read_text(encoding='utf-8'))
    if marker.exists():
        state=json.loads(marker.read_text(encoding='utf-8'))
        if state['protocol_sha256']!=digest(PROTOCOL):raise ValueError('Closure protocol changed after freezing')
    else:
        if any(directory.glob('*.npy')) or any(directory.glob('*.part')):
            raise ValueError('Holdout waveform already exists before protocol freezing')
        directory.mkdir(parents=True,exist_ok=True)
        state=dict(frozen_utc=datetime.now(timezone.utc).isoformat(),protocol_sha256=digest(PROTOCOL),
                   protocol=plan,waveform_access_started=False,
                   role='Frozen protocol cache preparation; repeated downloads do not create a new unseen survey')
        marker.write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    return state


def publication(directory,spec,refresh=False):
    path=Path(directory)/('publication_after.json' if refresh else 'publication.json')
    if refresh or not path.exists():
        with urllib.request.urlopen(spec['url'].split('/files/')[0],timeout=30) as response:
            content=response.read()
        record=json.loads(content)
    else:
        content=path.read_bytes();record=json.loads(content)
    listed=next(f for f in record['files'] if f['key']=='parihaka_dataset.npy')
    if listed['size']!=spec['bytes'] or listed['checksum']!='md5:'+spec['md5']:
        raise ValueError('Publisher identity differs from the frozen source')
    path.write_bytes(content)
    return listed


def download(directory=DEFAULT):
    directory=Path(directory);state=freeze(directory);spec=state['protocol']['holdout']
    target=directory/'parihaka.npy';metadata=target.with_suffix('.source.json')
    if metadata.exists():
        source=json.loads(metadata.read_text(encoding='utf-8'))
        if source.get('transport')=='verified_byte_ranges':list(load_blocks(directory))
        elif digest(target)!=source['sha256']:raise ValueError('Holdout source changed')
        return source
    if target.exists():raise FileExistsError('Unregistered holdout requires inspection')
    # Retain the publisher's metadata and checksum before downloading amplitudes.
    publication(directory,spec)
    partial=target.with_suffix('.part');http=target.with_suffix('.http');log=target.with_suffix('.download.log')
    if partial.exists():raise FileExistsError('Partial download retained; inspect before an explicit retry')
    with log.open('wb') as stream:
        result=subprocess.run(['curl.exe','--location','--fail','--http1.1','--connect-timeout','25',
            '--max-time','900','--output',str(partial),'--dump-header',str(http),spec['url']],
            stdout=stream,stderr=subprocess.STDOUT,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    if result.returncode:raise RuntimeError('Holdout download failed; evidence retained at '+str(log))
    md5=hashlib.md5()
    with partial.open('rb') as stream:
        for block in iter(lambda:stream.read(4*1024*1024),b''):md5.update(block)
    if partial.stat().st_size!=spec['bytes'] or md5.hexdigest()!=spec['md5']:
        raise ValueError('Holdout size or publisher checksum differs')
    partial.replace(target)
    source=dict(spec,transport='full_volume',sha256=digest(target),publisher_checksum='md5:'+spec['md5'],
                downloaded_utc=datetime.now(timezone.utc).isoformat(),
                freeze_sha256=digest(directory/'freeze.json'),publication_sha256=digest(directory/'publication.json'))
    metadata.write_text(json.dumps(source,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(status='passed',bytes=spec['bytes'],sha256=source['sha256'])),flush=True)
    return source


def canonical_volume(array):
    if array.ndim!=3 or sorted(array.shape)!=[590,782,1006] or array.dtype.kind!='f':
        raise ValueError('Published holdout dimensions or type differ from the frozen axis rule')
    return np.moveaxis(array,array.shape.index(1006),0)


def load(directory=DEFAULT):
    directory=Path(directory);freeze(directory);path=directory/'parihaka.npy'
    source=json.loads(path.with_suffix('.source.json').read_text(encoding='utf-8'))
    if digest(path)!=source['sha256']:raise ValueError('Holdout volume identity changed')
    return canonical_volume(np.load(path,mmap_mode='r',allow_pickle=False)),source


def range_data(url,start,length,total):
    request=urllib.request.Request(url,headers={'Range':f'bytes={start}-{start+length-1}'})
    for attempt in range(3):
        try:
            with urllib.request.urlopen(request,timeout=60) as response:
                expected=f'bytes {start}-{start+length-1}/{total}'
                if response.status!=206 or response.headers.get('Content-Range')!=expected:
                    raise ValueError('Server did not honor the exact frozen byte range')
                data=response.read(length+1)
            break
        except (urllib.error.URLError,TimeoutError,OSError) as error:
            if (isinstance(error,urllib.error.HTTPError) and error.code not in (429,500,502,503,504)) or attempt==2:raise
            time.sleep(attempt+1)
    if len(data)!=length:raise ValueError('Truncated or oversized source range')
    return data


def download_ranges(directory=DEFAULT):
    from research.experiments.spatial_replication import block_indices
    directory=Path(directory);state=freeze(directory);plan=state['protocol'];spec=plan['holdout']
    marker=directory/'transport_freeze.json';target=directory/'parihaka.source.json'
    if target.exists():
        source=json.loads(target.read_text(encoding='utf-8'))
        list(load_blocks(directory))
        return source
    if marker.exists():
        if json.loads(marker.read_text())['amendment_sha256']!=digest(AMENDMENT):raise ValueError('Transport amendment changed')
    else:
        marker.write_text(json.dumps(dict(frozen_utc=datetime.now(timezone.utc).isoformat(),
            amendment_sha256=digest(AMENDMENT),amendment=json.loads(AMENDMENT.read_text()),
            selected_waveforms_decoded=False),indent=2)+'\n',encoding='utf-8')
    published=publication(directory,spec)
    header=range_data(spec['url'],0,128,spec['bytes']);stream=io.BytesIO(header)
    version=np.lib.format.read_magic(stream)
    if version!=(1,0):raise ValueError('Unexpected NPY header version')
    shape,fortran,dtype=np.lib.format.read_array_header_1_0(stream)
    if shape!=(590,782,1006) or fortran or dtype!=np.dtype('<f4') or stream.tell()!=128:
        raise ValueError('Range transport requires the published C-order float32 layout')
    ny,nx,nt=shape;blocks=block_indices(np.repeat(np.arange(ny),nx),np.tile(np.arange(nx),ny),plan)
    cache=directory/'ranges'
    if cache.exists():raise FileExistsError('Partial range cache retained; inspect before an explicit retry')
    cache.mkdir();(cache/'header.bin').write_bytes(header);jobs=[]
    for block in blocks:
        for y in block['inlines']:
            start=128+(y*nx+block['crosslines'][0])*nt*4
            jobs.append((block['block'],y,start,16*nt*4))
    def fetch(job):
        block,y,start,length=job;data=range_data(spec['url'],start,length,spec['bytes'])
        path=cache/f'{block}_y{y}.bin';path.write_bytes(data)
        return dict(block=block,y=y,start=start,length=length,file=path.relative_to(directory).as_posix(),sha256=digest(path))
    with ThreadPoolExecutor(max_workers=6) as pool:received=list(pool.map(fetch,jobs))
    # The publisher record must still identify the same immutable file version.
    listed=publication(directory,spec,refresh=True)
    if listed['checksum']!=published['checksum'] or listed['size']!=published['size']:
        raise ValueError('Publisher identity changed during range retrieval')
    for block in blocks:
        pieces=[np.frombuffer((directory/r['file']).read_bytes(),dtype='<f4').reshape(16,nt)
                for r in received if r['block']==block['block']]
        np.save(cache/(block['block']+'.npy'),np.stack(pieces).transpose(2,0,1),allow_pickle=False)
    index=directory/'parihaka.ranges.json'
    index.write_text(json.dumps(dict(publisher_file=published,ranges=received,blocks=blocks,
        header_sha256=digest(cache/'header.bin'),transport_amendment_sha256=digest(AMENDMENT)),indent=2)+'\n',encoding='utf-8')
    source=dict(spec,transport='verified_byte_ranges',sha256=digest(index),
        publisher_full_checksum_verified=False,publisher_checksum='md5:'+spec['md5'],
        downloaded_utc=datetime.now(timezone.utc).isoformat(),freeze_sha256=digest(directory/'freeze.json'),
        cache_sha256={p.relative_to(directory).as_posix():digest(p) for p in cache.iterdir()},
        transport_amendment_sha256=digest(AMENDMENT),received_bytes=128+sum(r['length'] for r in received))
    target.write_text(json.dumps(source,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(dict(status='passed',transport=source['transport'],received_bytes=source['received_bytes'])),flush=True)
    return source


def load_blocks(directory=DEFAULT):
    from research.experiments.spatial_replication import block_indices
    directory=Path(directory);state=freeze(directory)
    source=json.loads((directory/'parihaka.source.json').read_text(encoding='utf-8'))
    if source.get('transport','full_volume')=='full_volume':
        path=directory/'parihaka.npy'
        if digest(path)!=source['sha256']:raise ValueError('Holdout volume changed')
        volume=canonical_volume(np.load(path,mmap_mode='r',allow_pickle=False));ny,nx=volume.shape[1:]
        blocks=block_indices(np.repeat(np.arange(ny),nx),np.tile(np.arange(nx),ny),state['protocol'])
        for block in blocks:
            y,x=block['inlines'],block['crosslines']
            yield block,volume[:,y[0]:y[-1]+1,x[0]:x[-1]+1],source,path
        return
    if source.get('transport')!='verified_byte_ranges':raise ValueError('Unknown holdout transport')
    amendment=json.loads((directory/'transport_freeze.json').read_text(encoding='utf-8'))
    if amendment['amendment_sha256']!=digest(AMENDMENT) or source['transport_amendment_sha256']!=digest(AMENDMENT):
        raise ValueError('Transport amendment changed after freezing')
    index=directory/'parihaka.ranges.json'
    if digest(index)!=source['sha256']:raise ValueError('Range index changed')
    for name,value in source['cache_sha256'].items():
        if digest(directory/name)!=value:raise ValueError('Range cache changed')
    registered=json.loads(index.read_text(encoding='utf-8'))
    for block in registered['blocks']:
        saved=np.load(directory/'ranges'/(block['block']+'.npy'),allow_pickle=False)
        pieces=[np.frombuffer((directory/r['file']).read_bytes(),dtype='<f4').reshape(16,1006)
                for r in registered['ranges'] if r['block']==block['block']]
        if not np.array_equal(saved,np.stack(pieces).transpose(2,0,1),equal_nan=True):
            raise ValueError('Decoded block differs from retained source bytes')
        yield block,saved,source,index


def prepare(directory=DEFAULT):
    directory=Path(directory).resolve();source=download_ranges(directory)
    paths={directory/'parihaka.source.json':digest(directory/'parihaka.source.json')}
    for name,value in source.get('cache_sha256',{}).items():paths[directory/name]=value
    index=directory/('parihaka.ranges.json' if source.get('transport')=='verified_byte_ranges' else 'parihaka.npy')
    paths[index]=source['sha256']
    for name in ('freeze.json','transport_freeze.json','publication.json','publication_after.json'):
        if (directory/name).exists():paths[directory/name]=digest(directory/name)
    state=dict(status='passed',scope='registered_holdout_cache',protocol_sha256=digest(PROTOCOL),
               input_sha256={str(p):value for p,value in paths.items()})
    (directory/'ready.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    return state


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--download',action='store_true')
    parser.add_argument('--ranges',action='store_true');parser.add_argument('--directory',type=Path,default=DEFAULT)
    parser.add_argument('--prepare',action='store_true')
    args=parser.parse_args()
    if args.prepare:prepare(args.directory)
    elif args.ranges:download_ranges(args.directory)
    elif args.download:download(args.directory)
    else:print(json.dumps(freeze(args.directory),indent=2))
