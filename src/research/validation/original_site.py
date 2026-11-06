"""Original 5151-5171 site preflight; missing evidence never means passed."""
import argparse
import collections
import csv
import hashlib
import json
import math
from pathlib import Path
import re
import struct

import numpy as np

EXPECTED={'S':'f5384518ae3669de10ff7352685fbf7b9604afc98b38e0b2f68d91c703594288',
          'R':'26529a1435ad2c49b340b33d4fc4efb23d9bfb833c9daa3c47d4941dd064e6ed',
          'X':'b27c482396b987216f783e7ccc3f87671c06f22a3b22243e0ec7fcd37d6fb9a7'}

def digest(path):
    h=hashlib.sha256()
    with Path(path).open('rb') as stream:
        for chunk in iter(lambda:stream.read(1024*1024),b''):h.update(chunk)
    return h.hexdigest()

def points(path,kind):
    found={}
    for line in Path(path).read_text(encoding='ascii').splitlines():
        if not line.startswith(kind):continue
        key=(int(float(line[1:11])),int(float(line[11:21])),int(line[23]))
        xy=tuple(float(line[a:b]) for a,b in ((46,55),(55,65)))
        if not all(map(math.isfinite,xy)) or key in found:raise ValueError('Invalid or duplicate SPS station')
        found[key]=xy
    if not found:raise ValueError('Empty station file')
    return found

def target(root):
    base=Path(root)/'data/sps';paths={ext:base/('5151-5171.'+ext) for ext in EXPECTED}
    for ext,path in paths.items():
        if digest(path)!=EXPECTED[ext]:raise ValueError('Original target SPS identity changed: '+ext)
    sources=points(paths['S'],'S');receivers=points(paths['R'],'R')
    shots={};segments=[];global_pairs=0
    for line in paths['X'].read_text(encoding='ascii').splitlines():
        if not line.startswith('X'):continue
        record=int(line[7:15]);skey=(int(float(line[17:27])),int(float(line[27:37])),int(line[37]))
        if skey not in sources:raise ValueError('Missing source station')
        rline=int(float(line[49:59]));first,last=int(float(line[59:69])),int(float(line[69:79]));index=int(line[79])
        if last<first:raise ValueError('Unexpected receiver ordering')
        station_keys=[(rline,k,index) for k in range(first,last+1)]
        if any(k not in receivers for k in station_keys):raise ValueError('Missing receiver station in X range')
        start,end,inc=int(line[38:43]),int(line[43:48]),int(line[48])
        if inc<=0 or start<1 or end<start or end-start+1!=len(station_keys)*inc:
            raise ValueError('Unresolved channel-group extent')
        info=shots.setdefault(record,dict(source_key=skey,receiver_keys=set(),intervals=[],groups=0))
        if info['source_key']!=skey or info['receiver_keys'].intersection(station_keys):
            raise ValueError('Conflicting FFID or repeated receiver')
        if any(not(end<a or start>b) for a,b in info['intervals']):raise ValueError('Overlapping channel envelopes')
        info['receiver_keys'].update(station_keys);info['intervals'].append((start,end));info['groups']+=len(station_keys)
        segments.append(dict(ffid=record,source_key=skey,receiver_line=rline,first_receiver=first,last_receiver=last,
            first_channel=start,last_channel=end,channel_increment=inc,receiver_groups=len(station_keys)))
        global_pairs+=len(station_keys)
    if len(sources)!=349 or len(receivers)!=9072 or set(shots)!=set(range(2,351)) or len(segments)!=6282:
        raise ValueError('Original target inventory differs')
    return dict(sources=sources,receivers=receivers,shots=shots,segments=segments,pairs=global_pairs,
        input_sha256={str(p.resolve()):EXPECTED[e] for e,p in paths.items()})

def inspect_waveform(path,original):
    """Identity screen only; a matching FFID alone cannot pass site validation."""
    path=Path(path);size=path.stat().st_size
    report=dict(file=str(path.resolve()),file_bytes=size,sha256=digest(path),status='unverified',eligible_for_original_recovery=False)
    if size<3840:raise ValueError('Truncated SEG-Y')
    with path.open('rb') as stream:head=stream.read(3600)
    candidates=[];widths={1:4,2:4,3:2,5:4,6:8,8:1}
    for order in ('>','<'):
        ns=struct.unpack_from(order+'H',head,3220)[0];dt=struct.unpack_from(order+'H',head,3216)[0]
        fmt=struct.unpack_from(order+'H',head,3224)[0]
        if ns and dt and fmt in widths:
            stride=240+ns*widths[fmt]
            if (size-3600)%stride==0:candidates.append((order,ns,dt,fmt,stride))
    if len(candidates)!=1:raise ValueError('Unsupported or ambiguous fixed-length SEG-Y framing')
    order,ns,dt,fmt,stride=candidates[0];n=(size-3600)//stride
    mm=np.memmap(path,dtype=np.uint8,mode='r')
    field=lambda offset,kind:np.ndarray((n,),dtype=order+kind,buffer=mm,offset=3600+offset,strides=(stride,))
    trace_ns,trace_dt=field(114,'u2'),field(116,'u2')
    if np.any((trace_ns!=0)&(trace_ns!=ns)) or np.any((trace_dt!=0)&(trace_dt!=dt)):
        raise ValueError('Inconsistent nonzero trace sampling')
    report['zero_trace_sample_declarations']=int(np.sum(trace_ns==0))
    report['zero_trace_interval_declarations']=int(np.sum(trace_dt==0))
    ffid,ch,trid=field(8,'i4'),field(12,'i4'),field(28,'i2');live=np.flatnonzero(trid==1)
    records=set(map(int,ffid[live]));expected=set(original['shots']);outside=records-expected
    report.update(byte_order='big' if order=='>' else 'little',samples_per_trace=ns,sample_interval_us=dt,
        format=fmt,total_traces=n,live_traces=len(live),auxiliary_or_unclassified_traces=n-len(live),
        field_records=len(records),outside_original_ffids=sorted(outside),missing_original_ffids=sorted(expected-records))
    if not len(live):report.update(status='incompatible',reason='No ordinary live traces');return report
    counts=collections.Counter(map(int,ffid[live]));duplicates=len(live)-len(set((int(ffid[i]),int(ch[i])) for i in live))
    source_mismatches=0;source_zero=0;unresolved_units=0
    # Header coordinates cannot be silently assigned a CRS or searched scale.
    sx,sy,scale,units=(field(k,t) for k,t in ((72,'i4'),(76,'i4'),(70,'i2'),(88,'i2')))
    for record in sorted(records&expected):
        indices=live[ffid[live]==record];station=original['sources'][original['shots'][record]['source_key']]
        for i in indices:
            if not sx[i] and not sy[i]:source_zero+=1;continue
            if int(units[i])!=1:unresolved_units+=1;continue
            factor=1.0 if not scale[i] else float(scale[i]) if scale[i]>0 else 1.0/abs(int(scale[i]))
            error=max(abs(float(sx[i])*factor-station[0]),abs(float(sy[i])*factor-station[1]))
            if error>1.:source_mismatches+=1
    short=[record for record in records&expected if counts[record]<original['shots'][record]['groups']]
    report.update(duplicate_ffid_channel_pairs=duplicates,source_coordinate_mismatches=source_mismatches,
        zero_source_coordinate_live_traces=source_zero,unresolved_coordinate_unit_live_traces=unresolved_units,
        records_with_fewer_traces_than_receiver_positions=sorted(short),
        component_order_confirmed=False,original_coordinate_reference_confirmed=False)
    reasons=[]
    if outside:reasons.append('Foreign FFIDs')
    if source_mismatches:reasons.append('Source coordinates disagree with original SPS')
    if short:reasons.append('Per-shot live trace count cannot cover the original receiver groups')
    if duplicates:reasons.append('Duplicate live FFID/channel pairs')
    if reasons:report.update(status='incompatible',reason='; '.join(reasons))
    else:report.update(status='unverified',reason='Source provenance, coordinate reference, components, receiver-by-trace mapping and complete coverage still require original field evidence')
    return report

def geometry_check(root,original):
    base=Path(root)/'results/latest/checks/real_sps/attribute_evaluation'
    with (base/'attribute_thinning_comparison.csv').open(encoding='utf-8-sig') as stream:
        native=next(r for r in csv.DictReader(line for line in stream if not line.startswith('#')) if r['Configuration']=='Full_Coverage')
    if int(native['Traces'])!=original['pairs']:raise ValueError('Native source/receiver pair count differs')
    with (base/'sps_spatial_coordinates.csv').open() as stream:data=list(csv.DictReader(stream))
    xy=np.array([[float(r['x']),float(r['y'])] for r in data]);definition=json.loads((base/'sps_spatial_definition.json').read_text())
    with (base/'sps_spatial_atoms.csv').open() as stream:atoms=list(csv.DictReader(stream))
    frequencies=np.array([[float(r['cycles_per_x_unit']),float(r['cycles_per_y_unit'])] for r in atoms])
    dictionary=np.exp(2j*np.pi*((xy-[definition['origin_x'],definition['origin_y']])@frequencies.T))
    with (base/'sps_spatial_sampling_comparison.csv').open() as stream:comparisons=list(csv.DictReader(stream))
    maximum=0.
    for row in comparisons:
        mask=np.loadtxt(base/'masks'/('sps_'+row['method']+'.txt'),comments='#',dtype=int)
        if set(mask)-{0,1} or mask.sum()!=int(row['receivers']):raise ValueError('Invalid geometric selection')
        a=dictionary[mask.astype(bool)];gram=a.conj().T@a/mask.sum();np.fill_diagonal(gram,0)
        maximum=max(maximum,abs(float(abs(gram).max())-float(row['spatial_fourier_mu'])))
    if maximum>1e-11:raise ValueError('Independent geometric Gram check failed')
    return dict(status='passed',physical_source_receiver_pairs=original['pairs'],native_count_matches=True,
        source_points=len(original['sources']),receiver_positions=len(original['receivers']),
        min_receiver_groups_per_shot=min(v['groups'] for v in original['shots'].values()),
        max_receiver_groups_per_shot=max(v['groups'] for v in original['shots'].values()),
        explicit_gram_max_error=maximum,geometry_layouts=len(comparisons),
        coordinate_unit='As supplied; physical units/CRS require field documentation',
        input_sha256={str(p.resolve()):digest(p) for p in base.rglob('*') if p.is_file()})

def run(project,out,waveforms):
    root=Path(project).resolve();out=Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    original=target(root);geometry=geometry_check(root,original);candidates=[]
    for path in waveforms:
        try:candidates.append(inspect_waveform(path,original))
        except (ValueError,OSError,struct.error) as error:
            candidates.append(dict(file=str(Path(path).resolve()),status='unverified',eligible_for_original_recovery=False,error=str(error)))
    # This is a gate, not a recovery/imaging implementation. No metadata-only
    # success can stand in for a real, same-site held-out waveform experiment.
    state=dict(status='incomplete',scope='Original 5151-5171 application validation readiness; geometry only is verified',
        original_input_sha256=original['input_sha256'],geometry=geometry,candidate_waveforms=candidates,
        original_waveform_identity='missing_verified_inputs',original_waveform_recovery_cases=0,
        original_application_passed=False,
        stages=dict(geometry='passed',same_site_waveforms='missing_verified_inputs',trace_to_station_mapping='not_verified',
            held_out_recovery='not_run',calibrated_amplitude='not_run',downstream_imaging='not_run',actual_field_cost='not_run'),
        required_inputs=['Same-site SEG-Y/SEG-D with source authority, FFID and trace-to-station mapping',
            'Coordinate system/units and instrument/component ordering; channel increment 2 is not a confirmed single-component mapping',
            'Sampling metadata and processing history; freeze recovery/hidden sets before scoring',
            'Calibration/response for physical amplitude; same-site velocity model for imaging; actual operational costs for cost claims'],
        audit_source_sha256=digest(__file__))
    out.mkdir(parents=True)
    (out/'verification.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    with (out/'per_ffid_geometry.csv').open('w',encoding='utf-8-sig',newline='') as stream:
        writer=csv.DictWriter(stream,fieldnames=['ffid','source_line','source_point','source_index','receiver_groups']);writer.writeheader()
        for record,info in sorted(original['shots'].items()):
            writer.writerow(dict(zip(writer.fieldnames,[record,*info['source_key'],info['groups']])))
    return state

if __name__=='__main__':
    from research.common.paths import ROOT
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project',type=Path,default=ROOT);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--waveforms',type=Path,nargs='*',default=[]);args=parser.parse_args()
    result=run(args.project,args.out,args.waveforms)
    print(json.dumps(dict(status=result['status'],geometry=result['geometry']['status'],
        source_receiver_pairs=result['geometry']['physical_source_receiver_pairs'],
        candidates=len(result['candidate_waveforms']),original_waveform_recovery_cases=0)))
    raise SystemExit(2)
