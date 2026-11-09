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
    import shutil
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
    catalog_path=ROOT/'figures/current.catalog.json'
    catalog_bytes=catalog_path.read_bytes()
    catalog=json.loads(catalog_bytes)
    if (workflow['status']!='passed' or workflow['scope']!=scope or not gui.get('workflow_executed') or
            gui['images_loaded']!=catalog['image_count'] or digest(program)!=program_hash):
        raise ValueError('GUI process and scientific evidence do not match')
    sealed_catalog=out/'gui_catalog.json'
    sealed_catalog.write_bytes(catalog_bytes)
    evidence=out/'qt_evidence'
    evidence.mkdir()
    files={'workflow.json':out/'workflow.json','source_identity.json':out/'source_identity.json',
           'gui_verification.json':out/'gui_verification.json','catalog.json':sealed_catalog}
    if scope=='all':
        files.update(primary_verification=out/'field_recovery_v2/verification.json',
                     primary_scores=out/'field_recovery_v2/recovery.csv',
                     application_verification=out/'field_applications/verification.json')
    for name,path in files.items():shutil.copyfile(path,evidence/name)
    state=dict(status='passed',scope=scope,started_utc=started,completed_utc=datetime.now(timezone.utc).isoformat(),
        exit_code=process.returncode,program_sha256=program_hash,workflow_sha256=digest(out/'workflow.json'),
        images_loaded=gui['images_loaded'],study='.',execution_directory=str(out),
        evidence_directory='qt_evidence',evidence_sha256={name:digest(evidence/name) for name in files},
        catalog_sha256=digest(sealed_catalog),
        gui_verification_sha256=digest(out/'gui_verification.json'),
        source_identity_sha256=digest(out/'source_identity.json'),gallery_report_count=len(catalog['reports']))
    if scope=='all':
        if any(name not in workflow['steps'] for name in ('field_recovery_v2','field_applications')):
            raise ValueError('Qt all scope skipped a required field application stage')
        state['field_recovery_sha256']=digest(out/'field_recovery_v2/verification.json')
        state['application_scores_sha256']={
            name:digest(out/'field_applications'/name/'recovery.csv')
            for name in ('soda_followup','viking_transfer','gvda','gvda_confirmation')}
    if retain_study:
        state['calculation_compatibility_sha256']=digest(out/'calculation_compatibility.json')
        state['calculation_source_archives_sha256']={p.name:digest(p) for p in out.glob('calculation_source*.zip')}
    (out/'gui_full_run.json').write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8');return state


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--scope',choices=['all','legacy','closure'],default='all');parser.add_argument('--python',type=Path)
    parser.add_argument('--retain-study',type=Path,help='Reuse archived calculations only across checked acceptance-only changes')
    args=parser.parse_args();print(json.dumps(run(args.out,args.scope,args.python,args.retain_study),indent=2))
