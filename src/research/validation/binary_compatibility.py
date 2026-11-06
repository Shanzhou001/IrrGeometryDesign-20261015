"""Compare complete spatial query outputs from explicitly selected executables."""
import argparse
import json
from pathlib import Path
import subprocess

from research.common.hashing import digest
from research.common.processes import native_environment


def run(old,new,queries,out):
    out=Path(out).resolve();out.mkdir(parents=True,exist_ok=False)
    for label,program in [('old',old),('new',new)]:
        with (out/(label+'.log')).open('wb') as log:
            process=subprocess.run([str(Path(program).resolve()),'--spatial-queries',str(Path(queries).resolve()),str(out/label)],
                env=native_environment(),stdout=log,stderr=subprocess.STDOUT,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
        if process.returncode:raise RuntimeError('Native comparison failed: '+label)
    old_files={p.relative_to(out/'old').as_posix():digest(p) for p in (out/'old').rglob('*') if p.is_file()}
    new_files={p.relative_to(out/'new').as_posix():digest(p) for p in (out/'new').rglob('*') if p.is_file()}
    if old_files!=new_files:raise ValueError('Native outputs or traces differ')
    result=dict(status='passed',files=len(old_files),old_sha256=digest(old),new_sha256=digest(new),queries_sha256=digest(queries))
    (out/'verification.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');return result


if __name__=='__main__':
    parser=argparse.ArgumentParser()
    for name in ('old','new','queries','out'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();print(json.dumps(run(args.old,args.new,args.queries,args.out),indent=2))
