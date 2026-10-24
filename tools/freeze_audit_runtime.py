"""Bind completed v5 records to an exact local source/binary snapshot before subsequent repairs."""
import argparse
import json
from pathlib import Path
import shutil

from run_audit_checks import digest

ROOT=Path(__file__).resolve().parents[1]


def freeze(root):
    manifest=json.loads((root/'search/manifest.json').read_text(encoding='utf-8'))
    out=root/'runtimes/v5';out.mkdir(parents=True,exist_ok=False)
    mapping={}
    for name,expected in manifest['source_sha256'].items():
        source=ROOT/name;assert digest(source)==expected,name
        target=out/'sources'/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,target);mapping[name]=expected
    binary=Path(manifest['program']);assert digest(binary)==manifest['executable_sha256']
    (out/'bin').mkdir();shutil.copy2(binary,out/'bin'/binary.name)
    dependencies=[binary.parent/'fftw.dll']
    dependencies += [Path('D:/Qt/4.7.4/bin')/name for name in ['QtCore4.dll','QtGui4.dll','QtOpenGL4.dll','Qt3Support4.dll']]
    available={str(p):digest(p) for p in dependencies if p.exists()}
    for p in dependencies:
        if p.exists():shutil.copy2(p,out/'bin'/p.name)
    state=dict(original_program=str(binary),executable_sha256=manifest['executable_sha256'],
               source_sha256=mapping,dependency_sha256=available,experiment_git_commit=manifest['git_commit'],
               purpose='Exact completed audit_joint_v5 runtime; later legacy codec fixes do not rewrite this experiment.')
    (out/'snapshot.json').write_text(json.dumps(state,indent=2),encoding='utf-8');print(out)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);freeze(p.parse_args().root.resolve())
