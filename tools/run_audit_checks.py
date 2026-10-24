"""Run requested checks with exact commands, logs, exit codes and executable hashes."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(1048576), b''):
            h.update(block)
    return h.hexdigest()


def checks(out, build=False):
    out.mkdir(parents=True, exist_ok=False)
    commands = []
    if build:
        commands += [(mode+'_build', ['cmd', '/c', 'build_'+mode+'.bat']) for mode in ['release', 'debug']]
    commands += [('python', ['python', '-m', 'unittest', 'discover', '-s', 'tools', '-p', 'test_*.py', '-v'])]
    commands += [(mode.lower()+'_regression', [str(ROOT/'x64'/mode/'IrrGeometryDesign.exe'), '--test']) for mode in ['Release', 'Debug']]
    records = []
    for name, command in commands:
        start = time.perf_counter()
        with (out/(name+'.log')).open('wb') as log:
            run = subprocess.run(command, cwd=ROOT, stdout=log, stderr=subprocess.STDOUT,
                                 creationflags=subprocess.CREATE_NO_WINDOW)
        record = dict(name=name, command=command, exit_code=run.returncode, seconds=time.perf_counter()-start)
        if Path(command[0]).is_file():record['program_sha256'] = digest(command[0])
        records.append(record)
        (out/'checks.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
        print(record, flush=True)
        if run.returncode:raise RuntimeError('check failed: '+name)
    return records


if __name__ == '__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out', type=Path, required=True)
    p.add_argument('--build', action='store_true');a=p.parse_args();checks(a.out, a.build)
