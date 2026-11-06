"""Single current, fully retained experiment suite used by the Qt experiment window."""
import argparse
from datetime import datetime, timezone
import json
import hashlib
import os
from pathlib import Path
import subprocess
import sys

from research.common.paths import ROOT, FIGURES, executable, current_results, source_files
from research.common.hashing import digest
from research.common.processes import native_environment
from research.validation.run_audit_checks import tracked_files
from research.validation.artifacts import read_record,require_success,check_hash,validate_native_steps


def validate_retained(marker):
    """Resume only if every registered input, prediction and raw output still matches."""
    marker=Path(marker);state=read_record(marker);base=marker.parent
    # Older spatial auditors wrote a compact summary. It is acceptable only
    # after its same-directory calculation manifest and raw evidence validate.
    legacy=(marker.name=='verification.json' and
            set(state)=={'status','score_max_error','scope'} and
            state.get('status')=='passed' and isinstance(state.get('scope'),str) and state['scope'])
    if legacy and (base/'manifest.json').is_file():
        validate_retained(base/'manifest.json')
        require_success(dict(state,manifest_sha256=digest(base/'manifest.json')))
    else:require_success(state)
    expected={}
    from research.validation.source_compatibility import check_retained_source
    for name,value in state.get('source_sha256',{}).items():check_retained_source(ROOT/name,value,marker)
    if state.get('program') and state.get('executable_sha256'):expected[Path(state['program'])]=state['executable_sha256']
    for name,value in state.get('input_sha256',{}).items():expected[Path(name)]=value
    provenance=state.get('configuration',{}).get('dataset_provenance',{})
    for name,value in provenance.get('input_sha256',{}).items():expected[Path(name)]=value
    for section in ('output_tables_sha256','derived_artifacts_sha256','table_sha256','output_sha256'):
        for name,value in state.get(section,{}).items():expected[base/name]=value
    validate_native_steps(state,base)
    for path,value in expected.items():
        check_hash(path,value)
    if 'source_manifest_sha256' in state:
        parent=base.parent/'manifest.json'
        if digest(parent)!=state['source_manifest_sha256']:raise ValueError('Analysis belongs to another batch')
        validate_retained(parent)
    if 'manifest_sha256' in state:
        if digest(base/'manifest.json')!=state['manifest_sha256']:raise ValueError('Verification belongs to another manifest')
    if marker.name!='manifest.json' and (base/'manifest.json').is_file():
        validate_retained(base/'manifest.json')


def source_identity():
    return dict(executable_sha256=digest(executable()),
        source_sha256={p.relative_to(ROOT).as_posix():digest(p) for p in tracked_files()})


def require_identity(expected):
    if source_identity()!=expected:raise RuntimeError('Sources or native program changed during the workflow')


def run(workers=16,out=None,modes=('Release','Debug'),scope='all'):
    if scope not in ('all','legacy','closure'):raise ValueError('Unknown workflow scope')
    if not 1 <= workers <= 16:
        raise ValueError('Workers must be between 1 and 16')
    if not modes or len(set(modes))!=len(modes) or 'Release' not in modes:
        raise ValueError('The scientific Release program must be checked; modes must be distinct')
    # Acceptance includes tests, build files and the top-level launcher.
    current=source_identity()
    identity_digest=hashlib.sha256(json.dumps(dict(identity=current,scope=scope),sort_keys=True).encode()).hexdigest()[:16]
    out=Path(out).resolve() if out is not None else current_results()
    previous=out/'source_identity.json'
    if previous.exists() and (json.loads(previous.read_text())!=current or
            ((out/'workflow.json').is_file() and json.loads((out/'workflow.json').read_text()).get('scope','legacy')!=scope)):
        out=ROOT/'results/runs'/identity_digest
    if not out.resolve().is_relative_to((ROOT/'results').resolve()):
        raise ValueError('Experiment output must remain inside results/')
    figure_group='research/runs/'+identity_digest
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
                 executable_sha256=digest(executable()), workers=workers,scope=scope, steps={})
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
        validate_retained(marker)
    try:
        # A changed source/program creates a new batch; historical results stay intact.
        identity_path = out/'source_identity.json'
        if identity_path.exists() and json.loads(identity_path.read_text()) != current:
            raise RuntimeError('Current results belong to different source/program; use a clean results/current after preserving needed data')
        identity_path.write_text(json.dumps(current, indent=2)+'\n')
        pointer=ROOT/'results/active.txt';temporary=pointer.with_suffix('.tmp')
        temporary.write_text(out.relative_to(ROOT).as_posix()+'\n',encoding='utf-8');temporary.replace(pointer)
        mode='both' if set(modes)=={'Release','Debug'} else modes[0]
        step('project_check', ['pipeline.py','check','--full','--mode',mode,'--out',out/'checks'], out/'checks/checks.json')
        if scope in ('all','legacy'):
            for name,module,extra,marker in [
                ('synthetic_search','review_studies',['--stage','search','--synthetic-only'],'run_manifest.json'),
                ('synthetic_joint','audit_joint_studies',['--stage','joint'],'manifest.json')]:
                step(name,['pipeline.py','module','research.experiments.'+module,'--out',out/name,
                     '--exe',executable(),'--workers',min(workers,8),*extra],out/name/marker)
            step('synthetic_audit',['pipeline.py','module','research.validation.verify_synthetic','--root',out],out/'synthetic_verification.json')
            step('synthetic_analysis',['pipeline.py','module','research.analysis.synthetic_results',
                '--v4-root',out/'synthetic_search','--joint-root',out/'synthetic_joint',
                '--out',out/'synthetic_analysis','--group',figure_group+'/synthetic'],out/'synthetic_analysis/manifest.json')
            step('viking',['pipeline.py','viking-v2','--out',out/'viking','--seeds',84011,94217,105953,
                '--inner-folds',2,'--workers',workers,'--group',figure_group+'/viking'],out/'viking/analysis/manifest.json')
            step('viking_audit',['pipeline.py','module','research.validation.verify_viking_batch',
                '--root',out/'viking','--out',out/'viking_audit'],out/'viking_audit/verification.json')
            step('viking_quality',['pipeline.py','module','research.analysis.waveform_quality',
                '--root',out/'viking','--out',out/'viking_quality'],out/'viking_quality/manifest.json')
            # F3 uses the fixed historical protocol; this rerun is not a new unseen validation.
            from research.experiments.f3_transfer import run as f3
            if not (out/'f3/manifest.json').exists():
                print('START f3',flush=True)
                f3(out/'f3',executable(),workers,group=figure_group+'/f3')
            else:
                validate_retained(out/'f3/manifest.json')
            step('f3_audit',['tools/verify_f3_transfer.py','--root',out/'f3','--out',out/'f3_audit'],out/'f3_audit/verification.json')
            from research.data.confirmation import DEFAULT as confirmation_data,freeze
            step('confirmation_data',['pipeline.py','module','research.data.confirmation','--download'],confirmation_data/'ready.json')
            step('synthetic_confirmation',['pipeline.py','module','research.experiments.confirmation_synthetic',
                 '--out',out/'synthetic_confirmation','--workers',min(workers,8)],out/'synthetic_confirmation/verification.json')
            for survey in [s['id'] for s in freeze()['protocol']['external']['surveys']]:
                step(survey+'_confirmation',['pipeline.py','module','research.experiments.confirmation_external',
                     '--survey',survey,'--out',out/(survey+'_confirmation'),'--workers',workers],out/(survey+'_confirmation/verification.json'))
            step('spatial_confirmation',['pipeline.py','module','research.experiments.confirmation_spatial',
                 '--out',out/'spatial_confirmation'],out/'spatial_confirmation/verification.json')
            from research.analysis.thesis_figures import run as figures
            step('spatial_replication',['pipeline.py','module','research.experiments.spatial_replication',
                 '--out',out/'spatial_replication','--workers',min(workers,4)],out/'spatial_replication/verification.json')
            step('research_evidence',['pipeline.py','module','research.analysis.contribution_report','--root',out,
                 '--out',out/'research_evidence'],out/'research_evidence/manifest.json')
            figures(out,figure_group)
            step('confirmation_report',['pipeline.py','module','research.analysis.confirmation_report',
                 '--root',out,'--out',out/'confirmation_report'],out/'confirmation_report/manifest.json')
            step('applicability_report',['pipeline.py','module','research.analysis.applicability_report',
                 '--root',out,'--out',out/'applicability_report'],out/'applicability_report/manifest.json')
        if scope in ('all','closure'):
            from research.data.closure_holdout import DEFAULT as holdout_data
            step('closure_data',['pipeline.py','module','research.data.closure_holdout','--prepare'],holdout_data/'ready.json')
            step('closure_validation',['pipeline.py','module','research.experiments.closure_validation',
                 '--out',out/'closure','--workers',min(workers,8)],out/'closure/verification.json')
            step('closure_report',['pipeline.py','module','research.analysis.closure_report',
                 '--study',out/'closure','--out',out/'closure/report'],out/'closure/report/manifest.json')
        if scope=='all':
            step('mapped_field_validation',['pipeline.py','field','--out',out/'mapped_field',
                 '--workers',min(workers,8)],out/'mapped_field/verification.json')
            step('mapped_field_report',['pipeline.py','module','research.analysis.mapped_field_report',
                 '--study',out/'mapped_field','--out',out/'mapped_field/report'],out/'mapped_field/report/manifest.json')
        from research.analysis.catalog import publish
        import shutil
        images=json.loads((out/'figure_verification.json').read_text(encoding='utf-8'))['images'] if (out/'figure_verification.json').exists() else []
        reports=[]
        for folder in ('research_evidence','confirmation_report','applicability_report','closure/report','mapped_field/report'):
            path=out/folder/'report.html'
            if not path.exists():path=out/folder/'report.md'
            if path.is_file():reports.append((folder,path))
        if scope in ('all','closure'):
            image=FIGURES/figure_group/'closure/primary_effects.png';image.parent.mkdir(parents=True,exist_ok=True)
            shutil.copyfile(out/'closure/report/primary_effects.png',image)
            images.append(dict(path=image.relative_to(FIGURES).as_posix(),title='Parihaka: fixed spatial Hybrid',
                description='Frozen 128/256 condition; six correlated blocks; no population significance claim.',sha256=digest(image)))
        if scope=='all':
            for name,title in [('paired_effects','公开炮集：分保留率配对效果'),('waveform_example','公开炮集：固定波形恢复示例')]:
                image=FIGURES/figure_group/'mapped_field'/(name+'.png');image.parent.mkdir(parents=True,exist_ok=True)
                shutil.copyfile(out/'mapped_field/report'/(name+'.png'),image)
                images.append(dict(path=image.relative_to(FIGURES).as_posix(),title=title,
                    description='配套实测几何与波形；保留负向结果；重复案例不增加独立工区数。',sha256=digest(image)))
        publish(out,images,reports,scope)
        require_identity(current)
        state.update(status='passed', completed_utc=datetime.now(timezone.utc).isoformat())
        (out/'acceptance.json').write_text(json.dumps(dict(status='passed',
            scope=scope,completed_utc=state['completed_utc'],executable_sha256=current['executable_sha256'],
            source_identity_sha256=digest(identity_path),
            historical_baseline='results/current; source and executable preserved in results/history_20261028',
            evidence={name:value for name,value in state['steps'].items()},
            limitations='No universal-superiority, calibrated-amplitude, migration or actual field-cost claim. Sections/seeds remain dependent; unavailable Teapot replaced before waveform access with recorded amendment.'),indent=2)+'\n')
        save(); print('ALL EXPERIMENTS AND INDEPENDENT CHECKS PASSED',flush=True)
        from research.validation.results_retention import keep_latest
        keep_latest(out)
    except BaseException as error:
        state.update(status='failed',error=str(error)); save(); raise
    finally:
        lock.close()
    return state


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--workers',type=int,default=16)
    parser.add_argument('--out',type=Path)
    parser.add_argument('--modes',nargs='+',choices=['Release','Debug'],default=['Release','Debug'])
    parser.add_argument('--scope',choices=['all','legacy','closure'],default='all')
    args=parser.parse_args();run(args.workers,args.out,args.modes,args.scope)
