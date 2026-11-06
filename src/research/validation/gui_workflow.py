"""Run the actual Qt workflow with a named scope and preserve its process proof."""
import argparse
from datetime import datetime,timezone
import json
from pathlib import Path
import subprocess,sys

from research.common.hashing import digest
from research.common.paths import ROOT,executable
from research.common.processes import native_environment


def run(out,scope='all',python=None,retain_study=None):
    out=Path(out).resolve()
    if not out.is_relative_to(ROOT/'results'):raise ValueError('GUI study must stay inside results/')
    if out.exists():raise FileExistsError(out)
    out.mkdir(parents=True)
    if retain_study:
        from research.validation.source_compatibility import prepare
        prepare(retain_study,out,scope)
    pointer=ROOT/'results/active.txt'
    if pointer.exists():(out/'previous_active.txt').write_bytes(pointer.read_bytes())
    pointer.write_text(out.relative_to(ROOT).as_posix()+'\n',encoding='utf-8')
    env=native_environment(render=True);env['IRR_PYTHON']=str(Path(python or sys.executable).resolve())
    env['IRR_WORKFLOW_SCOPE']=scope;program=executable();program_hash=digest(program)
    started=datetime.now(timezone.utc).isoformat()
    with (out/'gui_process.log').open('wb') as log:
        process=subprocess.run([str(program),'--gui-workflow-test'],cwd=ROOT,env=env,
            stdout=log,stderr=subprocess.STDOUT,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    if process.returncode:raise RuntimeError('Actual GUI workflow failed; see '+str(out/'gui_process.log'))
    workflow=json.loads((out/'workflow.json').read_text(encoding='utf-8'))
    gui=json.loads((out/'gui_verification.json').read_text(encoding='utf-8'))
    catalog=json.loads((ROOT/'figures/current.catalog.json').read_text(encoding='utf-8'))
    if (workflow['status']!='passed' or workflow['scope']!=scope or not gui.get('workflow_executed') or
            gui['images_loaded']!=catalog['image_count'] or digest(program)!=program_hash):
        raise ValueError('GUI process and scientific evidence do not match')
    state=dict(status='passed',scope=scope,started_utc=started,completed_utc=datetime.now(timezone.utc).isoformat(),
        exit_code=process.returncode,program_sha256=program_hash,workflow_sha256=digest(out/'workflow.json'),
        images_loaded=gui['images_loaded'])
    if retain_study:
        state['calculation_compatibility_sha256']=digest(out/'calculation_compatibility.json')
        state['calculation_source_archives_sha256']={p.name:digest(p) for p in out.glob('calculation_source*.zip')}
    (out/'gui_full_run.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8');return state


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--scope',choices=['all','legacy','closure'],default='all');parser.add_argument('--python',type=Path)
    parser.add_argument('--retain-study',type=Path,help='Reuse archived calculations only across checked acceptance-only changes')
    args=parser.parse_args();print(json.dumps(run(args.out,args.scope,args.python,args.retain_study),indent=2))
