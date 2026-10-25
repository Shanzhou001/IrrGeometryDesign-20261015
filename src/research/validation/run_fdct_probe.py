"""Run the retained FDCT sources/FFTW with VS2008, including non-power-of-two real-record dimensions."""
import argparse
import csv
import json
import os
from pathlib import Path
import shutil
import subprocess

from research.common.hashing import digest

from research.common.paths import ROOT


def run(out):
    out=out.resolve();out.mkdir(parents=True,exist_ok=False)
    vc=Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio 9.0/VC/vcvarsall.bat'
    sources=[ROOT/'src/tests/native/fdct_roundtrip_probe.cpp',ROOT/'src/third_party/curvelet/Curvelet2d/fdct_wrapping.cpp',ROOT/'src/third_party/curvelet/Curvelet2d/ifdct_wrapping.cpp']
    records=[]
    for mode in ['Release','Debug']:
        work=out/mode;work.mkdir();args=['cl','/nologo','/EHsc','/O2' if mode=='Release' else '/Od','/MD' if mode=='Release' else '/MDd',
            '/I'+str(ROOT/'src/third_party/curvelet/Curvelet2d'),*map(str,sources),'/Fefdct_probe.exe','/link',str(ROOT/'src/third_party/legacy-libs/fftw.lib')]
        build=work/'build.bat';build.write_text('@echo off\ncall "'+str(vc)+'" x86_amd64\nif errorlevel 1 exit /b 1\n'+subprocess.list2cmdline(args)+'\n')
        with (work/'build.log').open('wb') as f:r=subprocess.run(['cmd','/c',str(build)],cwd=work,stdout=f,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        if r.returncode:raise RuntimeError('FDCT probe build failed')
        shutil.copy2(ROOT/'src/third_party/legacy-libs/fftw.dll',work/'fftw.dll')
        command=[str(work/'fdct_probe.exe')]
        with (work/'roundtrip.csv').open('wb') as f:r=subprocess.run(command,cwd=work,stdout=f,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
        if r.returncode:raise RuntimeError('FDCT roundtrip failed')
        with (work/'roundtrip.csv').open() as f:data=list(csv.DictReader(f))
        assert len(data)==20
        records.append(dict(build=mode,command=command,exit_code=r.returncode,cases=len(data),program_sha256=digest(command[0]),
                            max_relative_l2=max(float(v['relative_l2']) for v in data),max_abs=max(float(v['max_abs']) for v in data),
                            max_energy_ratio_deviation=max(abs(float(v['coefficient_energy_ratio'])-1) for v in data)))
    state=dict(passed=True,source_sha256={str(p):digest(p) for p in sources},fftw_sha256=digest(ROOT/'src/third_party/legacy-libs/fftw.dll'),records=records)
    (out/'verification.json').write_text(json.dumps(state,indent=2));print(json.dumps(records))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True);run(p.parse_args().out)
