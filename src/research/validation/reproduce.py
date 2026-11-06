"""Recompute a selected workflow scope from a verified kit in a new directory."""
import argparse
from datetime import datetime,timezone
import json,os
from pathlib import Path
import subprocess,sys,zipfile

from research.common.hashing import digest


def extract(archive,destination):
    archive,destination=Path(archive),Path(destination).resolve()
    destination.mkdir(parents=True,exist_ok=False)
    with zipfile.ZipFile(archive) as zipped:
        for name in zipped.namelist():
            if not (destination/name).resolve().is_relative_to(destination):raise ValueError('Archive escapes reproduction directory')
        manifest=json.loads(zipped.read('KIT_MANIFEST.json'));zipped.extractall(destination)
    for name,info in manifest['files'].items():
        if digest(destination/name)!=info['sha256']:raise ValueError('Reproduction file differs: '+name)
    return manifest


def run(archive,out,scope='closure',python=None,create_env=False):
    out=Path(out).resolve();manifest=extract(archive,out)
    interpreter=Path(python or sys.executable).resolve()
    if create_env:
        subprocess.run([str(interpreter),'-m','venv',str(out/'environment')],check=True)
        interpreter=out/'environment/Scripts/python.exe'
        command=[str(interpreter),'-m','pip','install','--disable-pip-version-check','-r',str(out/'requirements.txt')]
        lock=out/'evidence/closure/environment.lock.txt'
        if lock.is_file():command+=['-c',str(lock)]
        with (out/'dependencies.log').open('wb') as log:subprocess.run(command,stdout=log,stderr=subprocess.STDOUT,check=True)
    env=os.environ.copy()
    for key in ('IRR_PROJECT_ROOT','IRR_WORKSPACE','QTDIR','PYTHONPATH'):env.pop(key,None)
    env['PATH']=str(Path(os.environ['SystemRoot'])/'System32');env['IRR_DISABLE_FIGURES']='1'
    command=[str(interpreter),'-B','pipeline.py','thesis','--scope',scope,'--modes','Release',
             '--workers','8','--out','results/reproduction']
    started=datetime.now(timezone.utc).isoformat()
    with (out/'workflow.log').open('wb') as log:
        process=subprocess.run(command,cwd=out,env=env,stdout=log,stderr=subprocess.STDOUT,
            creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    if process.returncode:raise RuntimeError('Reproduction failed; see '+str(out/'workflow.log'))
    state=dict(status='passed',started_utc=started,completed_utc=datetime.now(timezone.utc).isoformat(),
        scope=scope,archive_sha256=digest(archive),verified_files=len(manifest['files']),python=str(interpreter),
        workflow_sha256=digest(out/'results/reproduction/workflow.json'),
        scope_note='Selected workflow recomputed on the same host; no fresh-machine claim')
    (out/'reproduction_verification.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8');return state


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--kit',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True);parser.add_argument('--scope',choices=['all','legacy','closure'],default='closure')
    parser.add_argument('--python',type=Path);parser.add_argument('--create-env',action='store_true');args=parser.parse_args()
    print(json.dumps(run(args.kit,args.out,args.scope,args.python,args.create_env),indent=2))
