"""Keep one successful result edition, without following directory junctions."""
import json
from pathlib import Path
import zipfile

from research.common.paths import ROOT


def remove_entry(path,base):
    path,base=Path(path).absolute(),Path(base).resolve()
    # Check the parent before resolving the entry: a junction may point outside.
    if path.name in ('.','..') or path==base or not path.parent.resolve().is_relative_to(base):
        raise ValueError('Deletion escapes results: '+str(path))
    if path.is_symlink():path.unlink()
    elif path.is_junction():path.rmdir()
    elif path.is_dir():
        if not path.resolve().is_relative_to(base):raise ValueError('Directory escapes results')
        for child in path.iterdir():remove_entry(child,base)
        path.rmdir()
    else:path.unlink()


def resolve_artifact(path):
    """Resolve registered historical paths after materializing the latest edition."""
    path=Path(path).absolute()
    if path.is_file():return path
    pointer=ROOT/'results/active.txt'
    if not pointer.is_file():return path
    latest=(ROOT/pointer.read_text(encoding='utf-8').strip()).resolve()
    base=(ROOT/'results').resolve()
    if latest==base or not latest.is_relative_to(base):raise ValueError('Invalid active results')
    record=latest/'calculation_compatibility.json'
    if not record.is_file():return path
    state=json.loads(record.read_text(encoding='utf-8'))
    for name in state.get('relocated_roots',[]):
        old=(ROOT/name).absolute()
        if old==base or not old.is_relative_to(base):raise ValueError('Invalid relocated root')
        if path.is_relative_to(old):
            target=latest/path.relative_to(old)
            if not target.resolve().is_relative_to(latest):raise ValueError('Relocated artifact escapes edition')
            return target
    return path


def keep_latest(out):
    from research.experiments.thesis_workflow import source_identity
    from research.validation.run_audit_checks import tracked_files
    from research.validation.artifacts import read_record
    out=Path(out).resolve();base=(ROOT/'results').resolve()
    if out==base or not out.is_relative_to(base):raise ValueError('Latest edition must stay inside results')
    if read_record(out/'workflow.json').get('status')!='passed':raise ValueError('Only successful editions may prune results')
    # Retained calculations must already be materialized before their backing
    # directories can be removed. Never delete across a live junction dependency.
    if any(p.is_symlink() or p.is_junction() for p in out.rglob('*')):
        raise ValueError('Materialize retained result links before pruning')
    archive=out/'accepted_source_snapshot.zip'
    if not archive.exists():
        with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED) as zipped:
            for p in tracked_files():zipped.write(p,p.relative_to(ROOT).as_posix())
            zipped.writestr('SOURCE_IDENTITY.json',json.dumps(source_identity(),indent=2)+'\n')
    (base/'active.txt').write_text(out.relative_to(ROOT).as_posix()+'\n',encoding='utf-8')
    removed=[]
    parent=base
    while parent!=out:
        for child in list(parent.iterdir()):
            if child==out or child in out.parents:continue
            if parent==base and child.name in ('active.txt','workflow.lock'):continue
            removed.append(child.relative_to(base).as_posix());remove_entry(child,base)
        parent=next(p for p in out.parents if p.parent==parent) if out.parent!=parent else out
    record=dict(status='passed',policy='Keep only the latest successful edition',
        latest=out.relative_to(ROOT).as_posix(),removed=removed)
    (out/'retention.json').write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    return record
