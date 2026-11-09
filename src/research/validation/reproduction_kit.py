"""Export source, registered inputs and a Release runtime; test relocation locally."""
import argparse
from datetime import datetime,timezone
import json,os
from pathlib import Path
import subprocess,sys,zipfile

from research.common.hashing import digest
from research.common.paths import ROOT,current_results,executable


def members(root,runtime,study,extension=None):
    result={}
    def add(path,name=None):
        if path.is_file():result[name or path.relative_to(root).as_posix()]=path
    for folder in ('src','data','tools'):
        for p in (root/folder).rglob('*'):
            if (p.is_file() and '__pycache__' not in p.parts and p.suffix not in ('.pyc','.user','.log','.part')
                    and p.name!='ready.json'):add(p)
    for pattern in ('*.md','*.sln','*.bat','*.py','requirements.txt','.gitignore','.gitattributes'):
        for p in root.glob(pattern):add(p)
    for p in (root/'.github').rglob('*'):add(p)
    for p in runtime.rglob('*'):
        if p.is_file():
            relative=p.relative_to(runtime).as_posix();add(p,'output/runtime_current/'+relative);add(p,'build/bin/Release/'+relative)
    # Reference summaries are kept outside results/: incomplete raw evidence must
    # never masquerade as a resumable passed scientific batch after extraction.
    for folder in ('confirmation_report','applicability_report','research_evidence'):
        for p in (study/folder).glob('*'):add(p,'evidence/'+folder+'/'+p.name)
    for folder in ('field_recovery_v2','soda_local_warp_confirmation'):
        for p in (study/folder).glob('*'):
            if p.is_file():add(p,'evidence/'+folder+'/'+p.name)
    for p in (study/'field_applications').rglob('*'):
        if p.is_file() and 'predictions' not in p.relative_to(study).parts:
            add(p,'evidence/'+p.relative_to(study).as_posix())
    for p in (study/'qt_evidence').glob('*'):
        add(p,'evidence/qt_evidence/'+p.name)
    add(study/'field_recovery_gui.json','evidence/field_recovery_gui.json')
    add(study/'project_current_gui.json','evidence/project_current_gui.json')
    add(study/'application_reproduction.json','evidence/application_reproduction.json')
    for p in (study/'reproduction_evidence').rglob('*'):
        if p.is_file():add(p,'evidence/reproduction_evidence/'+p.relative_to(study/'reproduction_evidence').as_posix())
    for name in ('acceptance.json','final_verification.json','source_identity.json','delivery_verification.json','source_snapshot.zip'):
        add(study/name,'evidence/'+name)
    if extension is not None:
        for p in (Path(extension)/'report').glob('*'):add(p,'evidence/closure/'+p.name)
        for name in ('verification.json','source_identity.json','delivery_verification.json'):
            add(Path(extension)/name,'evidence/closure/'+name)
    return result


def create(out,runtime,study=None,extension=None):
    out,runtime=Path(out).resolve(),Path(runtime).resolve();study=Path(study or current_results()).resolve()
    if out.exists():raise FileExistsError(out)
    if not out.is_relative_to((ROOT/'output').resolve()):raise ValueError('Kit output must stay inside output/')
    state=json.loads((runtime/'runtime_manifest.json').read_text(encoding='utf-8'))
    if state['status']!='passed':raise ValueError('A verified runtime is required')
    if digest(runtime/state['program'])!=digest(executable()):raise ValueError('Runtime does not match the current scientific program')
    for name,info in state['files'].items():
        if digest(runtime/name)!=info['sha256']:raise ValueError('Runtime file changed')
    out.mkdir(parents=True);files=members(ROOT,runtime,study,extension)
    manifest=dict(created_utc=datetime.now(timezone.utc).isoformat(),original_root=str(ROOT),
        scope='Source, registered available inputs, Release runtime and reference summaries. Full predictions remain in the original retained study. No completed scientific results batch is activated in this kit.',
        optional_missing_inputs=['data/real_seismic/Data/RRAW.SGY','waveforms corresponding to the 9072-point SPS survey'],
        files={name:dict(sha256=digest(path),bytes=path.stat().st_size) for name,path in files.items()})
    instructions='''# Reproduction kit

This kit includes source, available registered inputs and the verified Release runtime.
Full per-case raw predictions remain in the original retained study. The evidence/
directory contains reference summaries, not an accepted or resumable results batch.
KIT_MANIFEST.json identifies the files in this kit. The reference scientific
source identity belongs to the sealed calculation snapshot; delivery-only
packaging and acceptance-index updates are recorded separately.

Install Python 3.12 and the pinned requirements, then run from this directory:

    python -m pip install -r requirements.txt
    python -B pipeline.py check --full --mode Release --out results/reproduction_checks
    python -B pipeline.py thesis --scope all --workers 16 --modes Release

The all scope includes the frozen equal-budget and external-survey study.
To rerun and verify the geometry-aware recovery study alone:

    python -B pipeline.py field-v2 --out results/field_recovery_v2 --workers 8
    python -B pipeline.py field-v2 --out results/field_recovery_v2 --verify

Compare its scores against evidence/field_recovery_v2. The workflow and raw
holdout performance have separate verdicts. The Soda Lake local-warp follow-up
has its own fixed shots and reference scores.

The maintained application suite includes Soda Lake boundary replication, the
frozen Poland-to-Viking transfer, the original Garner Valley validation, and
the prospective twelve-source, twenty-layout-seed Garner Valley confirmation.
Run it after field-v2:

    python -B pipeline.py applications --model-study results/field_recovery_v2 --out results/field_applications --workers 8
    python -B pipeline.py applications --model-study results/field_recovery_v2 --out results/field_applications --verify

All runners and frozen protocols are under src/. Registered Garner Valley
source signatures, station geometry and receiver counts are bundled under data/.

For the Parihaka study alone, using the included cache:

    python -B pipeline.py thesis --scope closure --modes Release --out results/closure_reproduction --workers 8

The source includes generic delivery seal/check and independent-directory
reproduction commands. Their arguments and scope are documented in RUNNING.md.

The GUI uses Release-only checks when a Debug executable is absent. Building source
still requires the documented Qt4.7.4 / VS2008 toolchain; running the bundled Release
program does not require QTDIR. Preserve source and data notices. This relocation
check runs on the existing Windows host and is not a fresh-machine certification.

No original-SPS waveform, calibrated-amplitude, migration or actual acquisition-cost
validation is supplied. Existing negative research results remain valid.
'''
    archive=out/'reproduction_kit.zip'
    with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=1) as zipped:
        for name,path in files.items():zipped.write(path,name)
        zipped.writestr('KIT_MANIFEST.json',json.dumps(manifest,indent=2)+'\n')
        zipped.writestr('START_HERE.md',instructions)
    destination=out/'relocated'
    with zipfile.ZipFile(archive) as zipped:
        for name in zipped.namelist():
            target=(destination/name).resolve()
            if not target.is_relative_to(destination.resolve()):raise ValueError('Archive member escapes destination')
        zipped.extractall(destination)
    for name,info in manifest['files'].items():
        if digest(destination/name)!=info['sha256']:raise ValueError('Relocated file identity differs')
    env=os.environ.copy();env.pop('QTDIR',None);env.pop('IRR_PROJECT_ROOT',None);env.pop('IRR_WORKSPACE',None)
    env.pop('PYTHONPATH',None);env['PATH']=str(Path(os.environ['SystemRoot'])/'System32')
    env['IRR_DISABLE_FIGURES']='1'
    checks=[]
    for name,command in [('native_core',[str(destination/'output/runtime_current/IrrGeometryDesign.exe'),'--test-core']),
        ('python_and_registered_data',[sys.executable,'-B',str(destination/'pipeline.py'),'check','--full','--mode','Release','--out',str(destination/'results/relocation_checks')])]:
        with (out/(name+'.log')).open('wb') as log:
            process=subprocess.run(command,cwd=destination,env=env,stdout=log,stderr=subprocess.STDOUT,
                creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        checks.append(dict(name=name,exit_code=process.returncode))
        if process.returncode:raise RuntimeError('Relocation check failed: '+name)
    result=dict(status='passed',completed_utc=datetime.now(timezone.utc).isoformat(),archive_sha256=digest(archive),
        archive_bytes=archive.stat().st_size,verified_files=len(files),checks=checks,
        scope='Relocated path and controlled PATH on the same host; full scientific reproduction requires executing the documented thesis command.')
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result,indent=2));return result


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True);parser.add_argument('--runtime',type=Path,required=True)
    parser.add_argument('--study',type=Path);parser.add_argument('--extension',type=Path)
    args=parser.parse_args();create(args.out,args.runtime,args.study,args.extension)
