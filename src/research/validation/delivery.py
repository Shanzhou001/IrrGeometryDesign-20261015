"""Seal and check a delivery using explicit paths, including the verifier source."""
import argparse
from datetime import datetime,timezone
import json
from pathlib import Path
import subprocess
import zipfile

from research.common.hashing import digest
from research.common.paths import ROOT,executable
from research.experiments.thesis_workflow import validate_retained,require_identity
from research.validation.artifacts import read_record,require_success,check_hash
from research.validation.run_audit_checks import tracked_files


def runtime_identity(runtime):
    runtime=Path(runtime);state=read_record(runtime/'runtime_manifest.json')
    if state['status']!='passed':raise ValueError('Runtime not verified')
    for name,info in state['files'].items():check_hash(runtime/name,info['sha256'])
    if digest(runtime/state['program'])!=digest(executable()):raise ValueError('Runtime differs from the checked program')
    return state


def seal(study,runtime,require_gui=False):
    study=Path(study).resolve();runtime=Path(runtime).resolve()
    workflow=read_record(study/'workflow.json')
    if workflow['status']!='passed':raise ValueError('Workflow incomplete')
    require_identity(read_record(study/'source_identity.json'))
    validate_retained(study/'checks/checks.json');runtime_state=runtime_identity(runtime)
    if require_gui:
        gui=read_record(study/'gui_verification.json')
        if gui.get('status')!='passed' or gui.get('workflow_executed') is not True:raise ValueError('Actual GUI workflow proof required')
        process=read_record(study/'gui_full_run.json')
        if (process.get('status')!='passed' or process['program_sha256']!=digest(executable()) or
                process['workflow_sha256']!=digest(study/'workflow.json')):raise ValueError('GUI process version mismatch')
    names=['workflow.json','acceptance.json','source_identity.json','checks/checks.json']
    for name in ('gui_verification.json','gui_full_run.json','figure_verification.json','closure/verification.json','closure/report/manifest.json'):
        if (study/name).exists():names.append(name)
    if (study/'closure/verification.json').exists():validate_retained(study/'closure/verification.json')
    if (study/'closure/report/manifest.json').exists():validate_retained(study/'closure/report/manifest.json')
    paths=tracked_files();archive=study/'source_snapshot.zip'
    with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED) as zipped:
        for p in paths:zipped.write(p,p.relative_to(ROOT).as_posix())
    checks=read_record(study/'checks/checks.json')
    result=dict(status='passed',completed_utc=datetime.now(timezone.utc).isoformat(),scope=workflow['scope'],
        source_sha256={p.relative_to(ROOT).as_posix():digest(p) for p in paths},
        source_archive_sha256=digest(archive),executable_sha256=digest(executable()),
        runtime_files=len(runtime_state['files']),record_sha256={n:digest(study/n) for n in names},
        python_tests=next(r['tests'] for r in checks['records'] if r['name']=='python'),
        git_head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),git_commit_created=False)
    target=study/'delivery_verification.json'
    if target.exists():raise FileExistsError(target)
    target.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');return result


def verify(study,runtime,kit,presentation=None):
    study,runtime,kit=Path(study).resolve(),Path(runtime).resolve(),Path(kit).resolve()
    delivery=read_record(study/'delivery_verification.json')
    for name,value in delivery['source_sha256'].items():check_hash(ROOT/name,value)
    actual={p.relative_to(ROOT).as_posix() for p in tracked_files()}
    if actual!=set(delivery['source_sha256']):raise ValueError('Source inventory changed')
    for name,value in delivery['record_sha256'].items():check_hash(study/name,value)
    check_hash(study/'source_snapshot.zip',delivery['source_archive_sha256'])
    runtime_identity(runtime)
    package=read_record(kit/'verification.json')
    if package['status']!='passed':raise ValueError('Kit not verified')
    check_hash(kit/'reproduction_kit.zip',package['archive_sha256'])
    with zipfile.ZipFile(kit/'reproduction_kit.zip') as zipped:
        manifest=json.loads(zipped.read('KIT_MANIFEST.json'))
        for name,value in delivery['source_sha256'].items():
            if manifest['files'][name]['sha256']!=value:raise ValueError('Packaged source differs: '+name)
    result=dict(status='passed',completed_utc=datetime.now(timezone.utc).isoformat(),scope=delivery['scope'],
        source_files=len(actual),python_tests=delivery['python_tests'],
        delivery_sha256=digest(study/'delivery_verification.json'),kit_sha256=package['archive_sha256'],
        kit_files=package['verified_files'],kit_bytes=package['archive_bytes'],git_commit_created=False)
    if presentation:result['presentation_sha256']=digest(presentation)
    (study/'handoff_verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');return result


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('operation',choices=['seal','verify'])
    parser.add_argument('--study',type=Path,required=True);parser.add_argument('--runtime',type=Path,required=True)
    parser.add_argument('--kit',type=Path);parser.add_argument('--presentation',type=Path)
    parser.add_argument('--require-gui',action='store_true');args=parser.parse_args()
    if args.operation=='seal':result=seal(args.study,args.runtime,args.require_gui)
    else:
        if not args.kit:parser.error('--kit is required to verify delivery')
        result=verify(args.study,args.runtime,args.kit,args.presentation)
    print(json.dumps({k:v for k,v in result.items() if k!='source_sha256'},indent=2))
