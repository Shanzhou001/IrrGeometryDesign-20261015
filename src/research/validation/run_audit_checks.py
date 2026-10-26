"""Record core checks separately from full external-data integration checks."""
import argparse
import datetime
import json
from pathlib import Path
import platform
import re
import shutil
import subprocess
import sys
import time

from research.common.hashing import digest
from research.common.paths import ROOT, WORKSPACE, executable, source_files
from research.common.processes import native_environment
from research.common.provenance import add_real_data_arguments, real_source_provenance


def external_inputs(workspace):
    return [workspace/'data/real_seismic'/name for name in
            ['Data/RRAW.SGY','real_gather_59x256.csv','real_gather_256x128.csv']] + [
                workspace/'data/sps'/('5151-5171.'+extension) for extension in ['S','R','X']]


def tracked_files():
    candidates=source_files()+list((ROOT/'src/research').rglob('*.py'))+list((ROOT/'src/tests/python').glob('*.py'))
    candidates+=list((ROOT/'src/build').rglob('*'))
    candidates+=[ROOT/name for name in ['pipeline.py','build.bat','requirements.txt','IrrGeometryDesign.sln']]
    return sorted({path for path in candidates if path.is_file()})


def unittest_summary(log):
    """Read unittest's final summary, preserving skipped tests as a separate outcome."""
    if not Path(log).is_file():return None
    text=Path(log).read_text(encoding='utf-8',errors='replace')
    totals=re.findall(r'^Ran (\d+) tests? in [^\n]+',text,re.MULTILINE)
    footers=re.findall(r'^(?:OK|FAILED)(?: \(([^\n]*)\))?\s*$',text,re.MULTILINE)
    if not totals or not footers:return None
    counts={key.strip():int(value) for key,value in re.findall(r'([a-z ]+)=(\d+)',footers[-1])}
    result=dict(run=int(totals[-1]),skipped=counts.get('skipped',0),failures=counts.get('failures',0),
                errors=counts.get('errors',0),expected_failures=counts.get('expected failures',0),
                unexpected_successes=counts.get('unexpected successes',0))
    result['passed']=result['run']-sum(result[key] for key in result if key!='run')
    return result


def checks(out, build=False, full=False, modes=('Release','Debug'),
           source_confirmation=None, allow_unconfirmed_real=False):
    modes=tuple(modes)
    if not modes or len(modes)!=len(set(modes)) or any(mode not in ('Release','Debug') for mode in modes):
        raise ValueError('modes must contain Release and/or Debug')
    out=Path(out).resolve();out.mkdir(parents=True,exist_ok=False)
    WORKSPACE.mkdir(parents=True,exist_ok=True)
    tracked=tracked_files()
    state=dict(scope='full' if full else 'core',status='running',
               started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
               python=platform.python_version(),platform=platform.platform(),modes=list(modes),
               source_sha256={p.relative_to(ROOT).as_posix():digest(p) for p in tracked},
               excluded=[] if full else ['external SPS/real-data integration suite'],records=[])
    def save():
        (out/'checks.json').write_text(json.dumps(state,indent=2),encoding='utf-8')
    def invoke(name,command,native_log=None):
        started=time.perf_counter();record=dict(name=name,command=list(map(str,command)))
        try:
            program=Path(command[0])
            if program.is_file():record['program_sha256']=digest(program)
            with (out/(name+'.log')).open('wb') as log:
                result=subprocess.run(record['command'],cwd=WORKSPACE,env=native_environment(),
                                      stdout=log,stderr=subprocess.STDOUT,
                                      creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            record.update(exit_code=result.returncode,status='passed' if result.returncode==0 else 'failed')
        except OSError as error:
            record.update(exit_code=None,status='failed',error=str(error))
        if name=='python':
            summary=unittest_summary(out/(name+'.log'))
            record['tests']=summary
            if summary is None or not summary['run']:
                record['summary_error']='Python test summary is missing or the suite is empty'
                record['status']='failed'
            elif summary['passed']<0 or any(summary[key] for key in ['failures','errors','unexpected_successes']):
                record.update(status='failed',summary_error='Python summary reports failed tests')
            elif full and summary['skipped'] and record['status']=='passed':
                record.update(status='incomplete',error='Full validation requires zero skipped Python tests')
        record['seconds']=time.perf_counter()-started
        if native_log and (WORKSPACE/native_log).is_file():
            shutil.copy2(WORKSPACE/native_log,out/(name+'_native.log'))
        state['records'].append(record);save();print(json.dumps(record),flush=True)
    save()
    if build:
        for mode in modes:invoke(mode.lower()+'_build',['cmd','/c',ROOT/'build.bat',mode])
    invoke('python',[sys.executable,'-B',ROOT/'pipeline.py','test'])
    for mode in modes:
        label=mode.lower();program=executable(mode)
        invoke(label+'_core',[program,'--test-core'],'science_tests.log')
        invoke(label+'_numerics',[sys.executable,'-B','-m','research.validation.audit_numerics',
                                 '--out',out/(label+'_numerics'),'--exe',program])
        invoke(label+'_projection',[sys.executable,'-B','-m','research.validation.audit_projection',
                                   '--out',out/(label+'_projection'),'--exe',program])
        invoke(label+'_pipeline',[sys.executable,'-B','-m','research.validation.pipeline_smoke',
                                 '--out',out/(label+'_pipeline'),'--exe',program])
    if full:
        missing=[str(path) for path in external_inputs(WORKSPACE) if not path.is_file()]
        state['missing_external_inputs']=missing
        if not missing:
            try:
                state['real_source_provenance']=real_source_provenance(
                    WORKSPACE/'data/real_seismic/Data/RRAW.SGY',source_confirmation,allow_unconfirmed_real)
            except (ValueError,OSError) as error:
                state['external_preflight_error']=str(error)
            else:
                if not state['real_source_provenance']['field_claims_eligible']:
                    state['external_preflight_error']='Source is unconfirmed; sensitivity checks do not complete formal field-data validation'
                for mode in modes:
                    invoke(mode.lower()+'_integration',[executable(mode),'--test'],'unit_tests.log')
    current={path.relative_to(ROOT).as_posix():path for path in tracked_files()}
    changed=[]
    for name in sorted(set(state['source_sha256'])|set(current)):
        try:value=digest(current[name]) if name in current else None
        except OSError:value=None
        if value!=state['source_sha256'].get(name):changed.append(name)
    state['changed_sources']=changed
    failed=changed or any(record['status']=='failed' for record in state['records'])
    incomplete=state.get('missing_external_inputs') or state.get('external_preflight_error') or any(
        record['status']=='incomplete' for record in state['records'])
    state['status']='failed' if failed else 'incomplete' if incomplete else 'passed'
    state['completed_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();save()
    if state['status']!='passed':raise RuntimeError('Checks '+state['status']+'; inspect '+str(out/'checks.json'))
    return state


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--build',action='store_true');parser.add_argument('--full',action='store_true')
    parser.add_argument('--mode',choices=['Release','Debug','both'],default='both')
    add_real_data_arguments(parser);args=parser.parse_args()
    checks(args.out,args.build,args.full,('Release','Debug') if args.mode=='both' else (args.mode,),
           args.source_confirmation,args.allow_unconfirmed_real)
