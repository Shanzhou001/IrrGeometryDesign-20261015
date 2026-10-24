"""Finalize a completed unified pipeline run without replacing an existing inventory."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

from research.common.hashing import digest
from research.common.paths import ROOT
from research.validation.layout import inspect_code,inspect_images


def finalize(root):
    report=root/'delivery_manifest.json'
    if report.exists():raise FileExistsError('delivery manifest exists; preserve the completed archive')
    state=json.loads((root/'pipeline.json').read_text(encoding='utf-8'))
    if not state.get('completed'):raise RuntimeError('only a completed pipeline run can be finalized')
    for name,value in state['source_sha256'].items():
        if digest(ROOT/name)!=value:raise RuntimeError('source changed after validation: '+name)
    code=inspect_code();images=inspect_images()
    inventory={str(p.relative_to(root)):dict(bytes=p.stat().st_size,sha256=digest(p)) for p in sorted(root.rglob('*')) if p.is_file() and p!=report and p.suffix not in {'.obj','.pdb','.ilk','.idb','.exp'}}
    result=dict(command=[sys.executable]+sys.argv,passed=True,
                git_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT).decode().strip(),
                tested_source_commit=state['git_commit'],pipeline_sha256=digest(root/'pipeline.json'),
                figure_manifest_sha256=digest(ROOT/'figures/manifest.json'),
                imported_modules=code['imported_modules'],images=images,files=inventory,
                scope='Completed pipeline artifacts; this inventory excludes itself and disposable compiler intermediates. Earlier audit inventories stay at their original revisions.')
    report.write_text(json.dumps(result,indent=2),encoding='utf-8')
    print(json.dumps(dict(passed=True,files=len(inventory),images=images['total_images'],manifest_sha256=digest(report))))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('root',type=Path)
    finalize(parser.parse_args().root.resolve())
