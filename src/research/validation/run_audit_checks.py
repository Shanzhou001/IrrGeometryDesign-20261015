"""Run requested checks with exact commands, logs, exit codes and executable hashes."""
import argparse
import json
from pathlib import Path
import subprocess
import time
import sys
import shutil

from research.common.paths import ROOT, WORKSPACE


from research.common.hashing import digest
from research.common.processes import native_environment


def checks(out, build=False):
    out = out.resolve()
    out.mkdir(parents=True, exist_ok=False)
    WORKSPACE.mkdir(parents=True, exist_ok=True)
    commands = []
    if build:
        commands += [(mode.lower()+'_build', ['cmd', '/c', str(ROOT/'build.bat'), mode]) for mode in ['Release', 'Debug']]
    commands += [('python', [sys.executable, '-B', str(ROOT/'pipeline.py'), 'test'])]
    commands += [(mode.lower()+'_regression', [str(WORKSPACE / 'build/bin'/mode/'IrrGeometryDesign.exe'), '--test']) for mode in ['Release', 'Debug']]
    records = []
    for name, command in commands:
        start = time.perf_counter()
        with (out/(name+'.log')).open('wb') as log:
            run = subprocess.run(command, cwd=WORKSPACE, env=native_environment(), stdout=log, stderr=subprocess.STDOUT,
                                 creationflags=subprocess.CREATE_NO_WINDOW)
        record = dict(name=name, command=command, exit_code=run.returncode, seconds=time.perf_counter()-start)
        if Path(command[0]).is_file():record['program_sha256'] = digest(command[0])
        records.append(record)
        native_log=WORKSPACE/'unit_tests.log'
        if name.endswith('_regression') and native_log.is_file():
            shutil.move(str(native_log),str(out/(name+'_native.log')))
        (out/'checks.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
        print(record, flush=True)
        if run.returncode:raise RuntimeError('check failed: '+name)
    return records


if __name__ == '__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out', type=Path, required=True)
    p.add_argument('--build', action='store_true');a=p.parse_args();checks(a.out, a.build)
