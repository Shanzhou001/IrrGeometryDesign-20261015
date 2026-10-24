"""Build exact legacy codec sources with VS2008/Qt4 and record pass/fail/timeout honestly."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]


def run(out,broken=False):
    out=out.resolve();out.mkdir(parents=True,exist_ok=False)
    qt=Path(shutil.which('qmake.exe')).parent.parent
    vc=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio 9.0/VC/vcvarsall.bat'
    source=ROOT/'IrrGeometryDesign/spssegy/segyio.cpp';header=source.with_suffix('.h');test=ROOT/'tools/legacy_codec_regression.cpp'
    results=[]
    for mode in ['Release','Debug']:
        work=out/mode;work.mkdir()
        args=['cl','/nologo','/EHsc','/Od','/MDd' if mode=='Debug' else '/MD','/DQT_DLL','/DWIN32',
              '/I'+str(qt/'include'),'/I'+str(qt/'include/QtCore'),'/I'+str(ROOT/'IrrGeometryDesign')]
        if mode=='Debug':args+=['/D_DEBUG']
        args += [str(test),str(source),str(ROOT/'IrrGeometryDesign/Diagmul.cpp'),'/Felegacy_codec_tests.exe','/link','/LIBPATH:'+str(qt/'lib'),'QtCored4.lib' if mode=='Debug' else 'QtCore4.lib']
        script=work/'build.bat';script.write_text('@echo off\ncall "'+str(vc)+'" x86_amd64\nif errorlevel 1 exit /b 1\n'+subprocess.list2cmdline(args)+'\n',encoding='utf-8')
        with (work/'build.log').open('wb') as log:r=subprocess.run(['cmd','/c',str(script)],cwd=work,stdout=log,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        if r.returncode:raise RuntimeError('codec build failed: '+mode)
        cases=['ibm_known','ibm_zero','ieee_file','matrix_selection'] if broken else ['ibm_known','ibm_zero','ieee_file','ibm_file','bounds','lifetime','matrix_selection']
        for case in cases:
            command=[str(work/'legacy_codec_tests.exe'),case];started=time.perf_counter();entry=dict(build=mode,case=case,command=command)
            with (work/(case+'.log')).open('wb') as log:
                try:
                    r=subprocess.run(command,cwd=work,stdout=log,stderr=subprocess.STDOUT,timeout=5,creationflags=subprocess.CREATE_NO_WINDOW)
                    entry.update(exit_code=r.returncode,status='PASS' if r.returncode==0 else 'FAIL')
                except subprocess.TimeoutExpired:entry.update(exit_code=None,status='TIMEOUT',terminated_by_harness=True)
            entry['seconds']=time.perf_counter()-started;results.append(entry);print(entry,flush=True)
    state=dict(source_sha256={str(p):digest(p) for p in [source,header,ROOT/'IrrGeometryDesign/Diagmul.cpp',test,Path(__file__)]},results=results,
               expected_broken=broken,all_passed=all(r['status']=='PASS' for r in results))
    (out/'checks.json').write_text(json.dumps(state,indent=2),encoding='utf-8')
    if not broken and not state['all_passed']:raise RuntimeError('legacy codec checks failed')


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True)
    p.add_argument('--expect-broken',action='store_true');a=p.parse_args();run(a.out,a.expect_broken)
