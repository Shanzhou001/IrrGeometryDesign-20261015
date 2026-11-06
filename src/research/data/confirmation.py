"""Download and validate the two prospectively frozen poststack surveys."""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import subprocess
import urllib.request

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT
from research.data.audit_segy_bytes import ibm_float
from research.data.viking import Shot

PROTOCOL=ROOT/'src/research/experiments/confirmation_protocol.json'
AMENDMENT=PROTOCOL.with_name('confirmation_amendment.json')
DEFAULT=ROOT/'data/public/confirmation_20261103'


def freeze(directory=DEFAULT):
    directory=Path(directory);directory.mkdir(parents=True,exist_ok=True)
    marker=directory/'freeze.json'
    if marker.exists():
        state=json.loads(marker.read_text())
        if state['protocol_sha256']!=digest(PROTOCOL):raise ValueError('Confirmation protocol changed after freezing')
    else:
        state=dict(frozen_utc=datetime.now(timezone.utc).isoformat(),protocol_sha256=digest(PROTOCOL),
                   protocol=json.loads(PROTOCOL.read_text()),waveform_access_started=False)
        marker.write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    if AMENDMENT.is_file():
        amendment=json.loads(AMENDMENT.read_text());amendment_marker=directory/'amendment_freeze.json'
        if amendment_marker.exists():
            saved=json.loads(amendment_marker.read_text())
            if saved['amendment_sha256']!=digest(AMENDMENT):raise ValueError('Frozen availability amendment changed')
        else:
            if (directory/(amendment['replacement']['id']+'.sgy')).exists():raise ValueError('Replacement waveform existed before amendment freeze')
            amendment_marker.write_text(json.dumps(dict(frozen_utc=datetime.now(timezone.utc).isoformat(),
                original_protocol_sha256=state['protocol_sha256'],amendment_sha256=digest(AMENDMENT),
                amendment=amendment),indent=2)+'\n')
        state=dict(state,amendment_sha256=digest(AMENDMENT),protocol=json.loads(json.dumps(state['protocol'])))
        state['protocol']['external']['surveys']=[amendment['replacement'] if s['id']==amendment['replace_survey_id'] else s
            for s in state['protocol']['external']['surveys']]
    return state


def download(directory=DEFAULT):
    directory=Path(directory);state=freeze(directory)
    for survey in state['protocol']['external']['surveys']:
        target=directory/(survey['id']+'.sgy');metadata=target.with_suffix('.source.json')
        if metadata.exists():
            old=json.loads(metadata.read_text())
            if not target.is_file() or digest(target)!=old['sha256']:raise ValueError('Downloaded survey changed')
            continue
        if target.exists():raise FileExistsError('Unregistered download requires inspection: '+str(target))
        print('DOWNLOAD',survey['id'],flush=True)
        if survey.get('download_client')=='curl':
            partial=target.with_suffix('.part');http=target.with_suffix('.http');log=target.with_suffix('.download.log')
            with log.open('wb') as stream:
                result=subprocess.run(['curl.exe','--location','--fail','--retry','3','--retry-all-errors',
                    '--http1.1','--connect-timeout','25',
                    '--output',str(partial),'--dump-header',str(http),survey['url']],stdout=stream,stderr=subprocess.STDOUT,
                    creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            if result.returncode:raise RuntimeError('Source download failed; see '+str(log))
            from email.parser import Parser
            headers=dict(Parser().parsestr(http.read_text().rsplit('HTTP/',1)[1].split('\n',1)[1]))
            total=partial.stat().st_size
            if total!=int(headers['Content-Length']):raise ValueError('Truncated source download')
            partial.replace(target)
        else:
            request=urllib.request.Request(survey['url'],headers={'User-Agent':'IrrGeometryDesign research reproducibility'})
            with urllib.request.urlopen(request,timeout=60) as response,target.open('xb') as stream:
                if response.status!=200:raise ValueError('A full source download is required')
                headers=dict(response.headers);total=0
                while True:
                    block=response.read(4*1024*1024)
                    if not block:break
                    stream.write(block);total+=len(block)
                    if total%(64*1024*1024)==0:print(survey['id'],total,'bytes',flush=True)
        if total!=int(headers['Content-Length']):raise ValueError('Truncated source download')
        metadata.write_text(json.dumps(dict(url=survey['url'],source_record=survey['source_record'],
            downloaded_utc=datetime.now(timezone.utc).isoformat(),bytes=total,sha256=digest(target),
            response_headers=headers,protocol_sha256=state['protocol_sha256'],
            attribution=survey.get('attribution'),license=survey.get('license'),amendment_sha256=state.get('amendment_sha256'),
            note='Full local file SHA-256; S3 ETag retained as source identity, not asserted to be a publisher SHA-256'),indent=2)+'\n')
        print('VERIFIED',survey['id'],total,flush=True)
    inputs={str(directory.resolve()/'freeze.json'):digest(directory/'freeze.json')}
    if (directory/'amendment_freeze.json').is_file():inputs[str(directory.resolve()/'amendment_freeze.json')]=digest(directory/'amendment_freeze.json')
    for survey in state['protocol']['external']['surveys']:
        p=directory/(survey['id']+'.sgy');inputs[str(p.resolve())]=digest(p)
    (directory/'ready.json').write_text(json.dumps(dict(status='passed',input_sha256=inputs,
        protocol_sha256=state['protocol_sha256']),indent=2)+'\n')


def volume_headers(survey,directory=DEFAULT):
    directory=Path(directory);freeze(directory)
    path=directory/(survey['id']+'.sgy');metadata=json.loads(path.with_suffix('.source.json').read_text())
    if digest(path)!=metadata['sha256']:raise ValueError('Source volume identity changed')
    raw=np.memmap(path,mode='r',dtype=np.uint8)
    number=lambda offset,width=2:int.from_bytes(raw[offset:offset+width].tobytes(),'big')
    ns,dt,fmt=number(3220),number(3216),number(3224)
    if ns<768 or dt<=0 or fmt not in (1,5):raise ValueError('Unsupported SEG-Y header')
    stride=240+4*ns
    if (len(raw)-3600)%stride:raise ValueError('SEG-Y size does not match its fixed-length trace headers')
    count=(len(raw)-3600)//stride
    integers=np.ndarray((count,60),dtype='>i4',buffer=raw,offset=3600,strides=(stride,4))
    inline=integers[:,(survey['inline_byte']-1)//4].astype(int)
    crossline=integers[:,(survey['crossline_byte']-1)//4].astype(int)
    trace_ns=np.ndarray((count,),dtype='>u2',buffer=raw,offset=3600+114,strides=(stride,))
    trace_dt=np.ndarray((count,),dtype='>u2',buffer=raw,offset=3600+116,strides=(stride,))
    if np.any((trace_ns!=ns)&(trace_ns!=0)) or np.any((trace_dt!=dt)&(trace_dt!=0)):
        raise ValueError('Variable sample counts or intervals are not supported')
    if len(set(zip(inline,crossline)))!=count:raise ValueError('Duplicate inline/crossline coordinates')
    return raw,dict(samples=ns,sample_interval_s=dt/1e6,format=fmt,stride=stride,
                    missing_trace_sample_declarations=int(np.sum(trace_ns==0)),
                    inline=inline,crossline=crossline,source=metadata)


def decode(raw,info,indices,start,stop):
    if not 0<=start<stop<=info['samples']:raise ValueError('Invalid frozen sample window')
    words=np.ndarray((len(info['inline']),info['samples']),dtype='>u4',buffer=raw,
                     offset=3840,strides=(info['stride'],4))[indices,start:stop]
    values=ibm_float(words) if info['format']==1 else words.copy().view('>f4')
    if not np.isfinite(values).all() or not np.any(values):raise ValueError('Invalid or zero-energy frozen section; no score-based replacement')
    return np.asarray(values,float).T


def load(survey_id,directory=DEFAULT):
    state=freeze(directory);spec=next(s for s in state['protocol']['external']['surveys'] if s['id']==survey_id)
    raw,info=volume_headers(spec,directory);lines=np.unique(info['inline']);shots={};selections=[]
    for rank,q in enumerate((.25,.5,.75)):
        line=int(lines[int((len(lines)-1)*q)])
        indices=np.flatnonzero(info['inline']==line)
        indices=indices[np.argsort(info['crossline'][indices],kind='stable')]
        if len(indices)<120:raise ValueError('Frozen inline has fewer than 120 traces')
        start=(len(indices)-120)//2;indices=indices[start:start+120]
        data=decode(raw,info,indices,512,768)
        shots[rank]=Shot(rank,data,np.arange(120,dtype=float),info['sample_interval_s'],512*info['sample_interval_s'])
        selections.append(dict(section=rank,inline=line,crosslines=info['crossline'][indices].tolist(),trace_indices=indices.tolist()))
    root=Path(directory).resolve();inputs={str(root/(survey_id+'.sgy')):info['source']['sha256'],
        str(root/'freeze.json'):digest(root/'freeze.json'),str(PROTOCOL):digest(PROTOCOL)}
    provenance=dict(status='prospectively_frozen_public_poststack_volume',survey=survey_id,independent_surveys=1,
        input_sha256=inputs,selection=selections,source=info['source'],protocol_sha256=digest(PROTOCOL),
        sample_interval_s=info['sample_interval_s'],spatial_unit='trace_rank',amplitude_units='uncalibrated digital amplitude',
        header_caveat=f"{info['missing_trace_sample_declarations']} trace sample-count fields are zero; binary sample count and exact fixed record/file lengths verified; no nonzero mismatch permitted",
        scope='Within-survey held-out poststack trace interpolation; no imaging or acquisition cost claim')
    return {512:shots},provenance


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--download',action='store_true')
    args=parser.parse_args()
    if args.download:download()
    else:print(json.dumps(freeze(),indent=2))
