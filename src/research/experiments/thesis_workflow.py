"""Single current, fully retained experiment suite used by the Qt experiment window."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import sys

from research.common.paths import ROOT, FIGURES, executable
from research.common.hashing import digest
from research.common.processes import native_environment


def validate_retained(marker):
    """Resume only if every registered input, prediction and raw output still matches."""
    marker=Path(marker);state=json.loads(marker.read_text(encoding='utf-8-sig'));base=marker.parent
    expected={}
    provenance=state.get('configuration',{}).get('dataset_provenance',{})
    for name,value in provenance.get('input_sha256',{}).items():expected[Path(name)]=value
    for section in ('output_tables_sha256','derived_artifacts_sha256','table_sha256'):
        for name,value in state.get(section,{}).items():expected[base/name]=value
    for step in state.get('steps',{}).values():
        if step.get('exit_code')!=0:raise ValueError('Failed retained step')
        for name,value in step.get('inputs',{}).items():expected[Path(name)]=value
        for name,value in step.get('outputs',{}).items():expected[base/name]=value
    for path,value in expected.items():
        if not path.is_file() or digest(path)!=value:
            raise ValueError('Missing or modified retained artifact: '+str(path))
    if 'source_manifest_sha256' in state:
        parent=base.parent/'manifest.json'
        if digest(parent)!=state['source_manifest_sha256']:raise ValueError('Analysis belongs to another batch')
        validate_retained(parent)


def run(workers=16):
    if not 1 <= workers <= 16:
        raise ValueError('Workers must be between 1 and 16')
    out = ROOT/'results/current'
    out.mkdir(parents=True, exist_ok=True)
    # A system-wide exclusive file lock also protects the CLI from a second GUI run.
    import msvcrt
    lock = (ROOT/'results/workflow.lock').open('a+b')
    if lock.tell() == 0:
        lock.write(b'0'); lock.flush()
    lock.seek(0)
    try:
        msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
    except OSError:
        lock.close()
        raise RuntimeError('Another experiment workflow is already running')
    state = dict(status='running', started_utc=datetime.now(timezone.utc).isoformat(),
                 executable_sha256=digest(executable()), workers=workers, steps={})
    status = out/'workflow.json'
    def save():
        status.write_text(json.dumps(state, indent=2)+'\n', encoding='utf8')
    save()
    def step(name, arguments, marker):
        marker = Path(marker)
        if marker.exists():
            # A suite may resume only with the identical executable and source.
            old = json.loads(marker.read_text(encoding='utf-8-sig'))
            if old.get('status') in ('failed','incomplete','running') or old.get('completed') is False:
                raise RuntimeError('Incomplete stage requires inspection before retry: '+str(marker.parent))
            validate_retained(marker)
            print('REUSE '+name, flush=True)
            state['steps'][name] = dict(status='retained', marker_sha256=digest(marker)); save()
            return
        print('START '+name, flush=True)
        log = out/(name+'.log')
        env = native_environment(); env['PYTHONUNBUFFERED'] = '1'
        with log.open('w', encoding='utf8') as stream:
            process = subprocess.Popen([sys.executable, '-B', *map(str, arguments)], cwd=ROOT,
                env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                encoding='utf8', errors='replace', creationflags=subprocess.CREATE_NO_WINDOW)
            for line in process.stdout:
                stream.write(line); stream.flush(); print(line.rstrip(), flush=True)
            rc = process.wait()
        state['steps'][name] = dict(exit_code=rc, marker=str(marker.relative_to(ROOT)))
        save()
        if rc or not marker.exists():
            raise RuntimeError('Stage failed: '+name+'; see '+str(log))
    try:
        # Source identity is frozen for all stages, including resumed suites.
        from research.common.paths import source_files
        sources = source_files()+list((ROOT/'src/research').rglob('*.py'))+list((ROOT/'src/research').rglob('*.json'))
        identity = {p.relative_to(ROOT).as_posix(): digest(p) for p in sources}
        identity_path = out/'source_identity.json'
        current = dict(executable_sha256=digest(executable()), source_sha256=identity)
        if identity_path.exists() and json.loads(identity_path.read_text()) != current:
            raise RuntimeError('Current results belong to different source/program; use a clean results/current after preserving needed data')
        identity_path.write_text(json.dumps(current, indent=2)+'\n')
        step('project_check', ['pipeline.py','check','--full','--mode','both','--out',out/'checks'], out/'checks/checks.json')
        for name,module,extra,marker in [
            ('synthetic_search','review_studies',['--stage','search','--synthetic-only'],'run_manifest.json'),
            ('synthetic_joint','audit_joint_studies',['--stage','joint'],'manifest.json')]:
            step(name,['pipeline.py','module','research.experiments.'+module,'--out',out/name,
                 '--exe',executable(),'--workers',min(workers,8),*extra],out/name/marker)
        step('synthetic_audit',['pipeline.py','module','research.validation.verify_synthetic','--root',out],out/'synthetic_verification.json')
        step('synthetic_analysis',['pipeline.py','module','research.analysis.synthetic_results',
            '--v4-root',out/'synthetic_search','--joint-root',out/'synthetic_joint',
            '--out',out/'synthetic_analysis','--group','research/current/synthetic'],out/'synthetic_analysis/manifest.json')
        step('viking',['pipeline.py','viking-v2','--out',out/'viking','--seeds',84011,94217,105953,
            '--inner-folds',2,'--workers',workers,'--group','research/current/viking'],out/'viking/analysis/manifest.json')
        step('viking_audit',['pipeline.py','module','research.validation.verify_viking_batch',
            '--root',out/'viking','--out',out/'viking_audit'],out/'viking_audit/verification.json')
        step('viking_quality',['pipeline.py','module','research.analysis.waveform_quality',
            '--root',out/'viking','--out',out/'viking_quality'],out/'viking_quality/manifest.json')
        # F3 uses the fixed historical protocol; this rerun is not a new unseen validation.
        from research.experiments.f3_transfer import run as f3
        if not (out/'f3/manifest.json').exists():
            print('START f3',flush=True)
            f3(out/'f3',executable(),workers,group='research/current/f3')
        else:
            validate_retained(out/'f3/manifest.json')
        step('f3_audit',['tools/verify_f3_transfer.py','--root',out/'f3','--out',out/'f3_audit'],out/'f3_audit/verification.json')
        from research.analysis.thesis_figures import run as figures
        figures(out)
        state.update(status='passed', completed_utc=datetime.now(timezone.utc).isoformat())
        save(); print('ALL EXPERIMENTS AND INDEPENDENT CHECKS PASSED',flush=True)
    except BaseException as error:
        state.update(status='failed',error=str(error)); save(); raise
    finally:
        lock.close()
    return state


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--workers',type=int,default=16)
    args=parser.parse_args();run(args.workers)
