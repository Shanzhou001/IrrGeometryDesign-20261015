"""Shared checks for successful records and their registered native artifacts."""
import json
import math
from pathlib import Path
import re

from research.common.hashing import digest


def finite_number(value,name):
    number=float(value)
    if not math.isfinite(number):raise ValueError('Nonfinite '+name)
    return number


def finite_error(actual,expected):
    return abs(finite_number(actual,'computed score')-finite_number(expected,'recorded score'))


def finite_arrays(reference,prediction):
    import numpy as np
    if reference.shape!=prediction.shape or not np.isfinite(reference).all() or not np.isfinite(prediction).all():
        raise ValueError('Nonfinite or mismatched prediction arrays')


def read_record(path):
    def reject(value):raise ValueError('Nonfinite JSON constant: '+value)
    state=json.loads(Path(path).read_text(encoding='utf-8-sig'),parse_constant=reject)
    if not isinstance(state,dict) or not state:raise ValueError('Empty or invalid acceptance record')
    return state


def require_success(state):
    if not isinstance(state,dict) or not state:raise ValueError('Empty acceptance record')
    if 'status' in state and state['status']!='passed':raise ValueError('Acceptance status is not passed')
    if 'completed' in state and state['completed'] is not True:raise ValueError('Incomplete acceptance record')
    legacy_analysis=(state.get('protocol') and state.get('table_sha256') and
                     (state.get('input_sha256') or state.get('source_manifest_sha256')))
    if state.get('status')!='passed' and state.get('completed') is not True and not legacy_analysis:
        raise ValueError('Acceptance record has no positive completion evidence')
    evidence=any(state.get(k) for k in ('source_sha256','input_sha256','output_tables_sha256',
        'derived_artifacts_sha256','table_sha256','output_sha256','records','steps','manifest_sha256',
        'source_manifest_sha256','experiment_manifest_sha256','prediction_sha256'))
    numeric_proof=any(isinstance(state.get(k),int) and state[k]>0 for k in ('cases','recovery_cases'))
    if not evidence and not numeric_proof:raise ValueError('Acceptance record has no registered evidence')
    for key,value in state.items():
        if key.endswith(('_max_error','_max_difference')):finite_number(value,key)
    if 'records' in state:
        if not state['records'] or not state.get('scope'):raise ValueError('Missing check scope or records')
        for record in state['records']:
            if record.get('status')!='passed' or record.get('exit_code')!=0:raise ValueError('Failed check record')
            if record.get('name')=='python':
                tests=record.get('tests') or {}
                if tests.get('run',0)<1 or any(tests.get(k,0) for k in ('failures','errors','unexpected_successes')):
                    raise ValueError('Invalid Python test acceptance')


def check_hash(path,expected):
    from research.validation.results_retention import resolve_artifact
    path=resolve_artifact(path)
    if not isinstance(expected,str) or not re.fullmatch('[0-9a-f]{64}',expected):
        raise ValueError('Invalid SHA-256 registration: '+str(path))
    if not path.is_file() or digest(path)!=expected:raise ValueError('Missing or modified retained artifact: '+str(path))


def validate_native_steps(state,base):
    base=Path(base)
    if 'steps' not in state:return
    if not isinstance(state['steps'],dict) or not state['steps']:raise ValueError('Empty native steps')
    for name,step in state['steps'].items():
        if step.get('exit_code')!=0:raise ValueError('Failed retained native step: '+name)
        if not step.get('command') or not step.get('outputs'):raise ValueError('Incomplete native step: '+name)
        for path,value in step.get('inputs',{}).items():check_hash(Path(path),value)
        for path,value in step['outputs'].items():check_hash(base/path,value)
        if step.get('log_sha256'):
            if not step.get('log'):raise ValueError('Missing native log path')
            check_hash(Path(step['log']),step['log_sha256'])
