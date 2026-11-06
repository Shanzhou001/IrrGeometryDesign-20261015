"""Strict fixed-length SEG-Y access and published trace/station joins."""
import csv
from pathlib import Path
import struct

import numpy as np
from research.data.confirmation import ibm_float


class FieldSegy:
    def __init__(self,path):
        self.path=Path(path);size=self.path.stat().st_size
        if size<3840:raise ValueError('Truncated SEG-Y')
        with self.path.open('rb') as stream:head=stream.read(3600)
        candidates=[]
        for order in ('>','<'):
            ns,dt,fmt=(struct.unpack_from(order+'H',head,k)[0] for k in (3220,3216,3224))
            if ns and dt and fmt in (1,5):
                stride=240+4*ns
                if (size-3600)%stride==0:candidates.append((order,ns,dt,fmt,stride))
        if len(candidates)!=1:raise ValueError('Unsupported or ambiguous fixed-length SEG-Y framing')
        self.order,self.ns,self.dt,self.fmt,self.stride=candidates[0];self.count=(size-3600)//self.stride
        self.raw=np.memmap(self.path,mode='r',dtype=np.uint8)
        for offset,value in ((114,self.ns),(116,self.dt)):
            declared=self.field(offset,'u2')
            if np.any((declared!=0)&(declared!=value)):
                self.close();raise ValueError('Conflicting trace sampling')

    def close(self):
        self.raw._mmap.close()

    def __enter__(self):return self

    def __exit__(self,*exception):self.close()

    def field(self,offset,kind):
        return np.ndarray((self.count,),dtype=self.order+kind,buffer=self.raw,
                          offset=3600+offset,strides=(self.stride,))

    def decode(self,indices,start,stop):
        indices=np.asarray(indices)
        if (indices.ndim!=1 or indices.dtype.kind not in 'iu' or not len(indices) or
                np.any(indices<0) or np.any(indices>=self.count) or len(set(indices.tolist()))!=len(indices) or
                not 0<=start<stop<=self.ns):raise ValueError('Invalid selected trace/sample indices')
        if np.any(self.field(28,'i2')[indices]!=1):raise ValueError('Auxiliary trace selected')
        words=np.ndarray((self.count,self.ns),dtype=self.order+'u4',buffer=self.raw,
                         offset=3840,strides=(self.stride,4))[indices,start:stop]
        data=ibm_float(words.astype(np.uint32)) if self.fmt==1 else words.copy().view(self.order+'f4')
        data=np.asarray(data,float).T
        if not np.isfinite(data).all() or not np.any(data):raise ValueError('Nonfinite or empty field window; no replacement')
        return data


def mapping(path,segy):
    with Path(path).open(encoding='utf-8-sig',newline='') as stream:rows=list(csv.DictReader(stream))
    indices=set();keys=set();ffid=segy.field(8,'i4');channel=segy.field(12,'i4');trid=segy.field(28,'i2')
    for row in rows:
        index=int(row['trace_index_zero_based']);ch=int(row.get('raw_channel',row.get('channel')))
        key=(int(row['field_record']),ch)
        if (index in indices or key in keys or not 0<=index<segy.count or trid[index]!=1 or
                ffid[index]!=key[0] or channel[index]!=ch or
                int(row['trace_header_byte_offset'])!=3600+index*segy.stride or
                int(row['ns'])!=segy.ns or int(row['dt_us'])!=segy.dt):
            raise ValueError('Trace/station mapping conflicts with SEG-Y')
        coords=[float(row[k]) for k in row if k in ('source_x','source_y','receiver_x','receiver_y',
                'source_x_sps','source_y_sps','receiver_x_sps','receiver_y_sps')]
        if len(coords)!=4 or not np.isfinite(coords).all():raise ValueError('Invalid assigned station coordinates')
        indices.add(index);keys.add(key)
    if indices!=set(np.flatnonzero(trid==1).tolist()):raise ValueError('Live trace mapping coverage differs')
    return rows
