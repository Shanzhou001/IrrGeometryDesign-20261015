"""Create or verify a compact source/input/result-summary ZIP, without bulk predictions."""
import argparse
import datetime
import hashlib
import json
from pathlib import Path, PurePosixPath
import platform
import subprocess
import zipfile

from research.common.hashing import digest
from research.common.paths import ROOT

MANIFEST='reproduction_manifest.json'
BINARY_SUFFIXES={'.dll','.lib','.exe','.obj','.pdb','.idb','.ilk','.exp','.pyc','.pyo','.ncb','.suo'}


def relative_name(name):
    path=PurePosixPath(name)
    if not name or '\\' in name or path.is_absolute() or '..' in path.parts or ':' in name:
        raise ValueError('Unsafe package member: '+name)
    return path.as_posix()


def verify(path):
    with zipfile.ZipFile(path) as archive:
        names=archive.namelist()
        if len(names)!=len(set(names)):raise ValueError('Duplicate archive members')
        for name in names:relative_name(name)
        state=json.loads(archive.read(MANIFEST))
        expected=state['files']
        if set(names)!=set(expected)|{MANIFEST}:raise ValueError('Archive members differ from manifest')
        for name,record in expected.items():
            relative_name(name)
            info=archive.getinfo(name)
            if info.file_size!=record['bytes']:raise ValueError('Package size mismatch: '+name)
            value=hashlib.sha256()
            with archive.open(name) as stream:
                for block in iter(lambda:stream.read(1024*1024),b''):value.update(block)
            if value.hexdigest()!=record['sha256']:raise ValueError('Package hash mismatch: '+name)
    return dict(status='passed',files=len(expected),uncompressed_bytes=sum(r['bytes'] for r in expected.values()),
                scope=state['scope'],path=str(Path(path).resolve()))


def create(out,run=None,support=None,figure_group=None,project=ROOT):
    project=Path(project).resolve();out=Path(out).resolve()
    if out.exists():raise FileExistsError('Use a new bundle path: '+str(out))
    files={}
    def add(path,category):
        path=Path(path).resolve()
        if not path.is_relative_to(project):raise ValueError('Inputs must stay within project: '+str(path))
        if not path.is_file():raise FileNotFoundError(path)
        name=relative_name(path.relative_to(project).as_posix())
        files[name]=dict(path=path,category=category)
    # Include editable sources and their original notices, excluding compiled dependencies.
    for path in (project/'src').rglob('*'):
        if path.is_file() and path.suffix.lower() not in BINARY_SUFFIXES and not any(
                part in {'__pycache__','.git','unit_test_sandbox'} for part in path.parts):add(path,'source')
    for pattern in ('*.md','*.py','*.bat','*.sln','requirements.txt','.gitattributes','.gitignore'):
        for path in project.glob(pattern):
            if path.is_file():add(path,'project')
    for path in (project/'.github/workflows').glob('*'):
        if path.is_file():add(path,'ci')
    registry_path=project/'src/research/data/viking_line12_subset.json'
    registered=json.loads(registry_path.read_text(encoding='utf-8'))
    dataset=project/'data/public/viking_line12_small'
    names={'source_docs/'+name:info for name,info in registered['source_docs'].items()}
    for shot in registered['shots']:names.update(shot['files'])
    for name,info in names.items():
        path=dataset/relative_name(name)
        if not path.is_file() or digest(path)!=info['sha256']:raise ValueError('Registered input missing or changed: '+name)
        add(path,'registered_input')
    for name in ('README.md','manifest.json','prepare_subset.py'):
        if (dataset/name).is_file():add(dataset/name,'input_documentation')
    for path in (project/'data/sps').glob('*'):
        if path.is_file():add(path,'sps_input_unconfirmed_field_provenance')
    if (project/'data/sps_recovery_20261026.json').is_file():add(project/'data/sps_recovery_20261026.json','input_documentation')
    frozen=None
    if run is not None:
        run=Path(run).resolve()
        if not run.is_relative_to(project):raise ValueError('Run must stay inside project')
        manifest=run/'manifest.json'
        frozen=json.loads(manifest.read_text(encoding='utf-8'))
        if frozen.get('status')!='passed' or not frozen.get('completed'):raise ValueError('Only a completed passed run can be bundled')
        for pattern in ('*.json','*.csv','*.md'):
            for path in run.glob(pattern):add(path,'run_summary')
            for path in (run/'analysis').glob(pattern):add(path,'analysis')
        figure_group=figure_group or 'research/'+run.name
    if figure_group:
        folder=(project/'figures'/relative_name(figure_group)).resolve()
        if not folder.is_relative_to(project/'figures'):raise ValueError('Figure group must stay inside figures')
        for path in folder.glob('*'):
            if path.is_file() and path.suffix.lower() in {'.png','.svg','.json','.html','.md'}:add(path,'figure')
    if support is not None:
        support=Path(support).resolve()
        for name in ('manifest.json','implementation_source.zip','before_source.zip'):
            add(support/name,'frozen_source_support')
        support_state=json.loads((support/'manifest.json').read_text(encoding='utf-8'))
        for key,name in [('source_archive_sha256','implementation_source.zip'),('before_source_archive_sha256','before_source.zip')]:
            if digest(support/name)!=support_state[key]:raise ValueError('Frozen support identity changed: '+name)
        if frozen is not None and support_state['runtime_sha256']['IrrGeometryDesign.exe']!=frozen.get('executable_sha256'):
            raise ValueError('Frozen support does not match selected run')
    try:
        commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=project,text=True).strip()
        dirty=bool(subprocess.check_output(['git','status','--porcelain'],cwd=project,text=True).strip())
    except (OSError,subprocess.CalledProcessError):commit=None;dirty=None
    state=dict(format='irr_compact_reproduction_v1',created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        scope='Editable source, registered local inputs, protocols and selected result summaries; bulk predictions and compiled dependencies excluded',
        git_commit=commit,working_tree_dirty=dirty,python=platform.python_version(),
        frozen_run_executable_sha256=None if frozen is None else frozen.get('executable_sha256'),
        availability=dict(bulk_predictions_included=False,compiled_runtime_included=False,
            legacy_RRAW_included=False,full_historical_archives_included=False),
        rights_note='This package does not assign a new license. Original notices and dataset source/terms are preserved. '
                   'Viking terms describe public-domain workshop data; cite the original source. SPS field provenance and redistribution rights '
                   'are not independently established. Review original dependency licenses and data permissions before redistribution.',
        reproduction='Use Python 3.12 and requirements.txt. Build Windows x64 native code with VS2008/Qt4 and the documented external '
                     'libraries. Run pipeline.py check --full; run the selected frozen protocol into a new directory. '
                     'Current working source may differ from a historical frozen run; use the matching archived source and program hash.',
        files={name:dict(bytes=entry['path'].stat().st_size,sha256=digest(entry['path']),category=entry['category'])
               for name,entry in sorted(files.items())})
    out.parent.mkdir(parents=True,exist_ok=True)
    with zipfile.ZipFile(out,'x',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as archive:
        for name,entry in sorted(files.items()):archive.write(entry['path'],name)
        archive.writestr(MANIFEST,json.dumps(state,indent=2,ensure_ascii=False)+'\n')
    report=verify(out)
    out.with_suffix(out.suffix+'.sha256').write_text(digest(out)+'  '+out.name+'\n',encoding='ascii')
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    commands=parser.add_subparsers(dest='command',required=True)
    make=commands.add_parser('create');make.add_argument('--out',type=Path,required=True)
    make.add_argument('--run',type=Path);make.add_argument('--support',type=Path)
    make.add_argument('--figure-group')
    check=commands.add_parser('verify');check.add_argument('bundle',type=Path)
    args=parser.parse_args()
    report=verify(args.bundle) if args.command=='verify' else create(args.out,args.run,args.support,args.figure_group)
    print(json.dumps(report,indent=2))
