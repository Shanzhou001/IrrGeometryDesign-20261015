"""Build or verify the single final delivery, excluding cached and bulk experiment outputs."""
import argparse
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path,PurePosixPath
from zipfile import ZipFile,ZIP_DEFLATED

ROOT=Path(__file__).resolve().parents[2]
PROJECT=ROOT/'IrrGeometryDesign'
MANIFEST='DELIVERY_MANIFEST.json'


def sha(data):return hashlib.sha256(data).hexdigest()


def safe(name):
    path=PurePosixPath(name)
    if not name or path.is_absolute() or '..' in path.parts or '\\' in name or ':' in name:
        raise ValueError('Unsafe archive path: '+name)


def verify(path):
    with ZipFile(path) as archive:
        names=archive.namelist()
        if len(names)!=len(set(names)):raise ValueError('Duplicate delivery entries')
        for name in names:safe(name)
        state=json.loads(archive.read(MANIFEST))
        if set(names)!=set(state['files'])|{MANIFEST}:raise ValueError('Delivery member list changed')
        for name,record in state['files'].items():
            data=archive.read(name)
            if len(data)!=record['bytes'] or sha(data)!=record['sha256']:raise ValueError('Delivery file changed: '+name)
    return dict(status='passed',files=len(state['files']),archive=str(Path(path).resolve()),
                archive_sha256=sha(Path(path).read_bytes()),scope=state['scope'])


def create(out):
    out=Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    selected=set()
    def add(path):
        path=Path(path).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file():raise ValueError('Missing or external file: '+str(path))
        selected.add(path)
    def tree(folder):
        for path in folder.rglob('*'):
            if path.is_file() and '__pycache__' not in path.parts and path.suffix.lower() not in {'.pyc','.pyo','.obj','.pdb','.ilk','.exp'}:add(path)
    for folder in ('src','tools','manuscript_sources','.github','figures'):
        tree(PROJECT/folder)
    for pattern in ('*.md','*.py','*.bat','*.sln','requirements.txt','.gitignore','.gitattributes'):
        for path in PROJECT.glob(pattern):add(path)
    for name in ('README.md','package_delivery.py'):add(ROOT/name)
    for folder in ('paper-tgrs','thesis'):tree(ROOT/folder)
    for folder in ('data/public/viking_line12_small','data/public/f3_transfer_small','data/sps','output/runtime_20261027'):
        tree(PROJECT/folder)
    add(PROJECT/'data/sps_recovery_20261026.json')
    for name in ('repair_20261026','viking_20261026','viking_diagnosis_20261026','viking_v2_20261026','viking_robust_20261026','viking_quality_20261027','f3_transfer_20261027'):
        study=PROJECT/'results'/name
        for folder in [study,study/'analysis']+([study/'joint',study/'search'] if name=='repair_20261026' else []):
            for path in folder.glob('*'):
                if path.is_file() and path.suffix in {'.json','.csv','.md'}:add(path)
    for name in ('viking_v2_support_20261026','viking_robust_support_20261026'):
        for path in (PROJECT/'results'/name).glob('*'):
            if path.is_file() and path.suffix in {'.json','.zip'}:add(path)
    tree(PROJECT/'checks/cleanup_final')
    tree(PROJECT/'checks/maintenance_20261026')
    tree(PROJECT/'checks/repair_20261026_sps')
    tree(PROJECT/'checks/repair_20261026_fdct')
    tree(PROJECT/'checks/repair_20261026_codec')
    closure=PROJECT/'checks/closure_20261027'
    for path in closure.glob('*'):
        if path.is_file() and path.name not in {'delivery.json'}:add(path)
    add(closure/'project/checks.json')
    tree(closure/'f3_archive_audit')
    build=json.loads((ROOT/'paper-tgrs/build_manifest.json').read_text(encoding='utf8'))
    for name,record in build['evidence'].items():
        path=ROOT/name
        if sha(path.read_bytes())!=record['sha256']:raise ValueError('Manuscript evidence changed: '+name)
        add(path)
    out.parent.mkdir(parents=True,exist_ok=True);records={}
    with ZipFile(out,'x',ZIP_DEFLATED,compresslevel=6) as archive:
        for path in sorted(selected):
            name=path.relative_to(ROOT).as_posix();safe(name);data=path.read_bytes()
            archive.writestr(name,data);records[name]=dict(bytes=len(data),sha256=sha(data))
        archive.writestr(MANIFEST,json.dumps(dict(created_utc=datetime.now(timezone.utc).isoformat(),
            scope='Current editable research project, registered inputs, runtime, cited study summaries and verification. Bulk experimental predictions and original academic reference documents stay in the workspace.',
            status='research draft; final organized delivery; no external publication or universal performance claim',files=records),ensure_ascii=False,indent=2)+'\n')
    report=verify(out)
    out.with_suffix('.zip.sha256').write_text(report['archive_sha256']+'  '+out.name+'\n',encoding='ascii')
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);commands=parser.add_subparsers(dest='command',required=True)
    make=commands.add_parser('create');make.add_argument('--out',type=Path,required=True)
    check=commands.add_parser('verify');check.add_argument('archive',type=Path)
    args=parser.parse_args();print(json.dumps(create(args.out) if args.command=='create' else verify(args.archive),indent=2))
