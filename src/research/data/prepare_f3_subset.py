"""Extract three frozen F3 sections using a bounded prefix of one deflated ZIP member."""
import argparse
import ast
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import time
from urllib.error import URLError,HTTPError
from urllib.request import Request, urlopen
import zlib

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT

PLAN=ROOT/'src/research/experiments/f3_transfer_protocol.json'
REGISTRY=Path(__file__).with_name('f3_transfer_subset.json')
DEFAULT=ROOT/'data/public/f3_transfer_small'
CACHE=None


def fetch(url,start,end):
    cached=CACHE/f'{start}_{end}.bin' if CACHE else None
    metadata=cached.with_suffix('.json') if cached else None
    if cached and cached.is_file() and metadata.is_file():
        record=json.loads(metadata.read_text());data=cached.read_bytes()
        if len(data)!=end-start+1 or hashlib.sha256(data).hexdigest()!=record['sha256']:
            raise ValueError('Cached source range changed')
        return data,record
    request=Request(url,headers={'Range':f'bytes={start}-{end}','User-Agent':'IrrGeometryDesign bounded research subset'})
    curl=shutil.which('curl.exe') or shutil.which('curl')
    if curl and cached:
        body=cached.with_suffix('.download');headers=cached.with_suffix('.headers')
        command=[curl,'--silent','--show-error','--location','--fail','--retry','4','--retry-all-errors','--retry-delay','1','--max-time','40',
                 '--max-filesize',str(end-start+1),'--range',f'{start}-{end}',
                 '--dump-header',str(headers),'--output',str(body),url]
        result=subprocess.run(command,capture_output=True,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if result.returncode:raise OSError('Bounded curl transfer failed: '+result.stderr.decode(errors='replace')[-800:])
        values=re.findall(r'^content-range:\s*(.+)$',headers.read_text(errors='replace'),flags=re.I|re.M)
        received=values[-1].strip() if values else ''
        data=body.read_bytes()
        if not received.startswith(f'bytes {start}-{end}/') or len(data)!=end-start+1:
            raise ValueError('curl response did not preserve exact requested range')
        record=dict(start=start,end=end,sha256=hashlib.sha256(data).hexdigest(),content_range=received,client='curl with certificate verification')
        body.replace(cached);headers.unlink();metadata.write_text(json.dumps(record)+'\n')
        print('verified source range',start,end,flush=True);return data,record
    for attempt in range(4):
        try:
            with urlopen(request,timeout=30) as response:
                received=response.headers.get('Content-Range','')
                if response.status!=206 or not received.startswith(f'bytes {start}-{end}/'):
                    raise ValueError('Server did not honor the bounded byte range: '+received)
                data=response.read(end-start+2)
                if len(data)!=end-start+1:raise ValueError('Truncated or oversized range')
                record=dict(start=start,end=end,sha256=hashlib.sha256(data).hexdigest(),content_range=received)
                if cached:cached.write_bytes(data);metadata.write_text(json.dumps(record)+'\n')
                print('verified source range',start,end,flush=True)
                return data,record
        except (URLError,TimeoutError,ConnectionError):
            if attempt==3:raise
            time.sleep(1)


def npy_header(data):
    if data[:6]!=b'\x93NUMPY':raise ValueError('ZIP member is not an NPY array')
    version=tuple(data[6:8]);size_bytes=2 if version==(1,0) else 4 if version in ((2,0),(3,0)) else 0
    if not size_bytes:raise ValueError('Unsupported NPY version')
    length=int.from_bytes(data[8:8+size_bytes],'little');end=8+size_bytes+length
    if end>65536 or len(data)<end:raise ValueError('Invalid NPY header length')
    header=ast.literal_eval(data[8+size_bytes:end].decode('utf8' if version==(3,0) else 'latin1'))
    dtype=np.dtype(header['descr'])
    if header['fortran_order'] or len(header['shape'])!=3 or dtype.kind!='f' or dtype.itemsize not in (4,8):
        raise ValueError('Expected a C-order floating time-last 3D array')
    return header,end,dtype


def prepare(out=DEFAULT):
    global CACHE
    out=Path(out).resolve()
    if REGISTRY.exists():raise FileExistsError('Preserve registered inputs; preparation is a one-time operation')
    marker=out/'preparation.json'
    if out.exists() and any(out.iterdir()) and (not marker.is_file() or json.loads(marker.read_text())['plan_sha256']!=digest(PLAN)):
        raise ValueError('Existing preparation does not match the frozen plan')
    plan=json.loads(PLAN.read_text());url=plan['source']['data_zip'];out.mkdir(parents=True,exist_ok=True);ranges=[]
    marker.write_text(json.dumps(dict(plan_sha256=digest(PLAN)))+'\n')
    CACHE=out/'source_ranges';CACHE.mkdir(exist_ok=True)
    # The immutable record currently reports this size. Fetch its EOCD and
    # confirm the ZIP member before accessing any waveform values.
    total=1051449986;tail,record=fetch(url,total-65536,total-1);ranges.append(record)
    marker=tail.rfind(b'PK\x05\x06')
    if marker<0:raise ValueError('ZIP end-of-central-directory not found')
    eocd=struct.unpack_from('<4s4H2LH',tail,marker);central=eocd[6]-(total-len(tail));position=central;member=None
    while tail[position:position+4]==b'PK\x01\x02':
        header=struct.unpack_from('<4s6H3L5H2L',tail,position);n,x,c=header[10:13]
        name=tail[position+46:position+46+n].decode('utf8')
        if name==plan['selection']['zip_member']:
            member=dict(name=name,compression=header[4],crc32_publisher_zip=header[7],compressed_bytes=header[8],uncompressed_bytes=header[9],local_offset=header[16])
        position+=46+n+x+c
    if not member or member['compression']!=8:raise ValueError('Frozen deflated test member is missing')
    local,record=fetch(url,member['local_offset'],member['local_offset']+29);ranges.append(record)
    if local[:4]!=b'PK\x03\x04':raise ValueError('Invalid ZIP local header')
    n,x=struct.unpack_from('<HH',local,26);start=member['local_offset']+30+n+x
    decoder=zlib.decompressobj(-15);buffer=bytearray();header=None;needed=None;consumed=0
    while needed is None or len(buffer)<needed:
        if consumed>=16*1024*1024:raise ValueError('Refusing to fetch more than 16 MiB of the source member')
        chunk,record=fetch(url,start+consumed,start+consumed+1024*1024-1);ranges.append(record);consumed+=len(chunk)
        buffer.extend(decoder.decompress(chunk,64*1024*1024-len(buffer)))
        if len(buffer)>=64*1024*1024:raise ValueError('Uncompressed prefix exceeded its bound')
        if header is None:
            header,offset,dtype=npy_header(buffer);ni,nx,nt=header['shape']
            if ni<=max(plan['ffids']) or nx<120 or nt!=plan['samples']:raise ValueError('Source dimensions differ from frozen selection')
            needed=offset+(max(plan['ffids'])*nx+120)*nt*dtype.itemsize
    files={}
    for section in plan['ffids']:
        first=offset+section*nx*nt*dtype.itemsize
        data=np.frombuffer(buffer,dtype=dtype,count=120*nt,offset=first).reshape(120,nt).T.astype(float)
        if not np.isfinite(data).all():raise ValueError('Nonfinite source data')
        path=out/f'section_{section:04d}.npz';np.savez_compressed(path,data=data)
        files[path.name]=dict(sha256=digest(path),bytes=path.stat().st_size,section_index=section,shape=list(data.shape))
    docs=out/'source_docs';docs.mkdir(exist_ok=True)
    for name,address in [('README.md','https://raw.githubusercontent.com/yalaudah/facies_classification_benchmark/main/README.md'),
                         ('SOFTWARE_LICENSE.txt','https://raw.githubusercontent.com/yalaudah/facies_classification_benchmark/main/LICENSE')]:
        path=docs/name;curl=shutil.which('curl.exe') or shutil.which('curl')
        if path.is_file() and path.stat().st_size:
            pass  # An interrupted preparation may already have fetched these documents.
        elif curl:
            subprocess.run([curl,'--silent','--show-error','--location','--fail','--retry','4','--retry-all-errors','--retry-delay','1','--max-time','40',
                            '--max-filesize','1048576','--output',str(path),address],check=True,
                           creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        else:
            with urlopen(Request(address,headers={'User-Agent':'IrrGeometryDesign source documentation'}),timeout=30) as response:
                path.write_bytes(response.read(1024*1024))
        files['source_docs/'+name]=dict(sha256=digest(path),bytes=path.stat().st_size)
    state=dict(protocol='f3_transfer_subset_v1',created_utc=datetime.now(timezone.utc).isoformat(),
               plan_sha256=digest(PLAN),reference_record=plan['source']['record'],member=member,npy_header=header,
               source_ranges=ranges,compressed_prefix_bytes=consumed,full_zip_md5_locally_verified=False,
               full_member_crc_locally_verified=False,files=files,
               scope='Three small poststack F3 sections from one previously unused survey; original physical time origin and amplitude calibration not asserted',
               coordinate_unit='trace intervals; nominal coordinates are only a scale-invariant uniform-grid convention',
               rights_note='Primary benchmark requests citation doi:10.1190/INT-2018-0249.1. Original F3 data terms and benchmark/software terms are distinct; source documents preserved. Local research use, no external publication of this package performed.')
    REGISTRY.write_text(json.dumps(state,indent=2)+'\n',encoding='utf8')
    (out/'manifest.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(status='prepared',source_bytes_fetched=sum(r['end']-r['start']+1 for r in ranges),subset_bytes=sum(f['bytes'] for f in files.values()),shape=header['shape']),indent=2))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,default=DEFAULT)
    prepare(parser.parse_args().out)
