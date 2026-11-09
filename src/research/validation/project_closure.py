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
    integrated=base/'project_current_checks/checks.json'
    if integrated.is_file():return integrated
    current=base/'recovery_project_checks/checks.json'
    if current.is_file():return current
    dedicated=base/'project_checks/checks.json'
    return dedicated if dedicated.is_file() else base/'checks/checks.json'


def delivery_record(base):
    updated=base/'project_current_delivery.json'
    if not updated.is_file():updated=base/'recovery_project_delivery.json'
    result=record(updated if updated.is_file() else base/'project_delivery.json')
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


def gui_record(base):
    """Verify the sealed Qt evidence using paths contained in the current study."""
    from research.experiments.thesis_workflow import source_identity
    proof_path=base/'project_current_gui.json'
    result=record(proof_path if proof_path.is_file() else base/'field_recovery_gui.json')
    if result['status']!='passed':return result
    try:
        proof=json.loads(Path(result['record']).read_text())
        required={'workflow.json','source_identity.json','gui_verification.json','catalog.json',
                  'primary_verification','primary_scores','application_verification'}
        if set(proof['evidence_sha256'])!=required:
            raise ValueError('Incomplete portable Qt evidence inventory')
        if set(proof['application_scores_sha256'])!={'soda_followup','viking_transfer','gvda','gvda_confirmation'}:
            raise ValueError('Incomplete Qt application score inventory')
        folder=(base/proof['evidence_directory']).resolve()
        if not folder.is_relative_to(base.resolve()):
            raise ValueError('Qt evidence directory escapes the current study')
        for name,value in proof['evidence_sha256'].items():
            path=(folder/name).resolve()
            if not path.is_relative_to(folder) or digest(path)!=value:
                raise ValueError('Qt evidence changed')
        workflow=json.loads((folder/'workflow.json').read_text())
        identity=json.loads((folder/'source_identity.json').read_text())
        gui=json.loads((folder/'gui_verification.json').read_text())
        catalog=json.loads((folder/'catalog.json').read_text(encoding='utf-8'))
        if (proof['scope']!='all' or proof['exit_code']!=0 or workflow['status']!='passed' or
                workflow['scope']!='all' or 'field_applications' not in workflow['steps'] or
                proof['images_loaded']<19 or proof['gallery_report_count']<7 or
                gui['workflow_executed'] is not True or gui['images_loaded']!=proof['images_loaded'] or
                catalog['image_count']!=proof['images_loaded'] or
                len(catalog['reports'])!=proof['gallery_report_count'] or identity!=source_identity() or
                proof['program_sha256']!=identity['executable_sha256'] or
                digest(folder/'workflow.json')!=proof['workflow_sha256'] or
                digest(folder/'source_identity.json')!=proof['source_identity_sha256'] or
                digest(folder/'primary_scores')!=digest(base/'field_recovery_v2/recovery.csv')):
            raise ValueError('Qt evidence does not cover the current integrated workflow')
        for name,value in proof['application_scores_sha256'].items():
            if digest(base/'field_applications'/name/'recovery.csv')!=value:
                raise ValueError('Qt and current application scores differ')
    except (OSError,ValueError,KeyError) as error:result.update(status='failed',reason=str(error))
    return result


def application_reproduction(base):
    from research.experiments.thesis_workflow import source_identity
    result=record(base/'application_reproduction.json')
    if result['status']!='passed':return result
    try:
        proof=json.loads(Path(result['record']).read_text())
        if set(proof['score_sha256'])!={'soda_followup','viking_transfer','gvda','gvda_confirmation'}:
            raise ValueError('Independent application inventory is incomplete')
        folder=(base/'reproduction_evidence').resolve()
        for name,value in proof['files_sha256'].items():
            path=(folder/name).resolve()
            if not path.is_relative_to(folder) or digest(path)!=value:
                raise ValueError('Independent calculation evidence changed')
        identity=json.loads((folder/'source_identity.json').read_text())
        if identity!=source_identity():raise ValueError('Independent source identity differs')
        for name,value in proof['score_sha256'].items():
            if (digest(base/'field_applications'/name/'recovery.csv')!=value or
                    digest(folder/name/'recovery.csv')!=value):
                raise ValueError('Independent application score table differs')
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
    from research.experiments.field_recovery import verify as verify_field_recovery
    recovery=record(base/'field_recovery_v2/verification.json')
    if recovery['status']=='passed':
        try:
            result=verify_field_recovery(base/'field_recovery_v2')
            if result!=json.loads(Path(recovery['record']).read_text()):
                raise ValueError('Field performance differs from its verification record')
            recovery.update(status='passed',historical_effect_status=result['performance_status'],surveys=result['surveys'])
        except (OSError,ValueError,KeyError) as error:recovery.update(status='failed',reason=str(error))
    applications=record(base/'field_applications/verification.json')
    reporting=record(base/'field_applications/report/manifest.json')
    if applications['status']=='passed':
        try:
            from research.experiments.application_suite import verify as verify_applications
            result=verify_applications(base/'field_applications')
            if result!=json.loads(Path(applications['record']).read_text()):
                raise ValueError('Integrated application result differs from verification')
            applications.update(status=result['application_effect_status'],boundary=result['soda_boundary'])
        except (OSError,ValueError,KeyError) as error:
            applications.update(status='failed',reason=str(error))
            reporting.update(status='failed',reason=str(error))
    if recovery['status']=='failed':
        applications.update(status='failed',reason='Historical matched-field calculation audit failed')
    elif recovery['status']!='passed' and applications['status']=='passed':
        applications.update(status=recovery['status'],reason='Historical matched-field calculation audit is incomplete')
    components.update(field_recovery_effect=applications,applicability_reporting=reporting,
                      application_reproduction=application_reproduction(base),qt_workflow=gui_record(base),
                      historical_v2_effect=recovery)
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
