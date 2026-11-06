"""Verify the agreed public-field recovery scope and report optional extensions."""
import argparse
import json
from pathlib import Path

from research.common.hashing import digest
from research.common.paths import ROOT,current_results
from research.validation.run_audit_checks import tracked_files
from research.validation.artifacts import validate_native_steps

SCOPE=ROOT/'src/research/experiments/project_scope.json'


def record(path):
    if not path.is_file():return dict(status='missing',record=str(path))
    state=json.loads(path.read_text(encoding='utf-8-sig'))
    return dict(status=state.get('status','unknown'),record=str(path),sha256=digest(path))


def field_record(base):
    result=record(base/'mapped_field/verification.json')
    if result['status']!='passed':return result
    try:
        verification=json.loads(Path(result['record']).read_text())
        folder=base/'mapped_field';manifest=folder/'manifest.json'
        if (verification.get('cases')!=1728 or verification.get('manifest_sha256')!=digest(manifest) or
                verification.get('score_max_error')!=0 or verification.get('projection_max_error')!=0):
            raise ValueError('Mapped-field verification is not bound to its calculation')
        state=json.loads(manifest.read_text())
        if state.get('status')!='passed' or state.get('completed') is not True or not state.get('derived_artifacts_sha256'):
            raise ValueError('Incomplete mapped-field calculation evidence')
        validate_native_steps(state,folder)
        for name,value in state['input_sha256'].items():
            if digest(name)!=value:raise ValueError('Mapped-field input changed')
        for name,value in state['derived_artifacts_sha256'].items():
            if digest(folder/name)!=value:raise ValueError('Mapped-field artifact changed: '+name)
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def independent_record(base):
    result=record(base/'mapped_field_independent_reproduction.json')
    if result['status']!='passed':return result
    try:
        state=json.loads(Path(result['record']).read_text())
        if (state.get('cases')!=1728 or state.get('columns')!=25 or state.get('exact_fields')!=43200 or
                state.get('numeric_max_difference')!=0 or
                state.get('recovery_csv_sha256')!=digest(base/'mapped_field/recovery.csv')):
            raise ValueError('Independent reproduction is not bound to the current recovery table')
        verification=state.get('independent_verification',{})
        if verification.get('status')!='passed' or verification.get('cases')!=1728:
            raise ValueError('Missing completed independent calculation')
        directory=Path(state['directory'])
        if (digest(directory/'recovery.csv')!=state['recovery_csv_sha256'] or
                digest(directory/'manifest.json')!=verification['manifest_sha256']):
            raise ValueError('Independent raw evidence changed')
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def reporting_record(base):
    folder=base/'mapped_field/report';result=record(folder/'manifest.json')
    if result['status']!='passed':return result
    try:
        state=json.loads(Path(result['record']).read_text())
        if state.get('cases')!=1728 or state.get('comparisons')!=54 or not state.get('output_sha256'):
            raise ValueError('Incomplete stratified outcome report')
        for name,value in state['input_sha256'].items():
            if digest(name)!=value:raise ValueError('Reported experiment changed')
        for name,value in state['output_sha256'].items():
            if digest(folder/name)!=value:raise ValueError('Outcome report changed')
        for name,value in state['source_sha256'].items():
            if digest(ROOT/name)!=value:raise ValueError('Outcome analysis source changed')
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def checks_path(base):
    dedicated=base/'project_checks/checks.json'
    return dedicated if dedicated.is_file() else base/'checks/checks.json'


def delivery_record(base):
    result=record(base/'project_delivery.json')
    if result['status']!='passed':return result
    try:
        state=json.loads(Path(result['record']).read_text())
        actual={p.relative_to(ROOT).as_posix():digest(p) for p in tracked_files()}
        if (state.get('source_sha256')!=actual or state.get('archive_sha256')!=digest(state['archive']) or
                state.get('checks_sha256')!=digest(checks_path(base)) or
                state.get('scope_sha256')!=digest(SCOPE)):
            raise ValueError('Delivery differs from accepted code, package, checks or scope')
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def baseline_record(base):
    path=base/'completion_verification.json'
    if not path.is_file():
        result=record(base/'workflow.json')
        if result['status']!='passed':return result
        try:
            from research.experiments.thesis_workflow import validate_retained
            workflow=json.loads(Path(result['record']).read_text())
            required=['synthetic_joint','viking','f3_audit','synthetic_confirmation','stratton_confirmation',
                      'volve_confirmation','spatial_confirmation','spatial_replication','closure_validation','mapped_field_validation']
            if workflow.get('scope')!='all' or any(k not in workflow.get('steps',{}) for k in required):
                raise ValueError('Incomplete all-scope workflow')
            for folder,marker in [('synthetic_joint','manifest.json'),('viking','run_manifest.json'),
                                  ('f3_audit','verification.json'),('closure','verification.json')]:
                if folder=='viking':marker='manifest.json'
                validate_retained(base/folder/marker)
        except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
        return result
    result=record(path)
    if result['status']!='passed':return result
    try:
        completed=json.loads(path.read_text());sealed=json.loads((base/'delivery_verification.json').read_text())
        if completed.get('recovery_cases')!=7740 or not sealed.get('record_sha256'):
            raise ValueError('Missing archived experiment evidence')
        for name,value in sealed['record_sha256'].items():
            if digest(base/name)!=value:raise ValueError('Archived experiment record changed')
        if digest(base/'source_snapshot.zip')!=sealed['source_archive_sha256']:
            raise ValueError('Archived calculation source changed')
        result['scope']='Archived 7740-case suite with its original sealed calculation identity'
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def evaluate(components,scope):
    required=scope.get('required',[])
    if (not required or len(set(required))!=len(required) or
            set(required)&set(scope.get('optional',[]))):raise ValueError('Invalid empty or overlapping acceptance scope')
    values=[components.get(name,{}).get('status','missing') for name in required]
    return 'passed' if all(v=='passed' for v in values) else 'failed' if 'failed' in values else 'incomplete'


def status(base):
    base=Path(base);scope=json.loads(SCOPE.read_text(encoding='utf-8'));baseline=baseline_record(base)
    field=field_record(base);original=record(base/'original_site_application/registered/verification.json')
    checks=record(checks_path(base))
    if checks['status']=='passed':
        state=json.loads(Path(checks['record']).read_text())
        actual={p.relative_to(ROOT).as_posix():digest(p) for p in tracked_files()}
        if not actual or actual!=state['source_sha256']:checks.update(status='stale',reason='Sources changed after checks or no source evidence')
        if (state.get('scope')!='project' or not state.get('records') or
                any(r.get('status')!='passed' or r.get('exit_code')!=0 for r in state.get('records',[]))):
            checks.update(status='failed',reason='Incomplete project checks')
    components=dict(retained_public_experiments=baseline,mapped_field_recovery=field,current_code_checks=checks,
        outcome_reporting=reporting_record(base),independent_reproduction=independent_record(base),project_delivery=delivery_record(base),
        original_site_application=original,physical_amplitude=dict(status='missing_calibration'),
        downstream_imaging=dict(status='missing_same_site_velocity_and_waveforms'),
        actual_field_cost=dict(status='missing_operational_records'),new_physical_machine=dict(status='not_verified'))
    for name,component in components.items():component['required']=name in scope['required']
    verdict=evaluate(components,scope)
    return dict(status=verdict,profile=scope['profile'],scope_sha256=digest(SCOPE),scope=scope,
                public_recovery_scope_passed=verdict=='passed',
                original_site_application_passed=original['status']=='passed',
                components=components,git_commit_created=False)


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--results',type=Path,default=current_results())
    parser.add_argument('--out',type=Path);args=parser.parse_args();state=status(args.results)
    payload=json.dumps(state,indent=2,ensure_ascii=False,allow_nan=False)+'\n'
    if args.out:args.out.write_text(payload,encoding='utf-8')
    print(payload);raise SystemExit(0 if state['status']=='passed' else 2)
