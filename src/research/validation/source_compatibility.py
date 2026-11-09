"""Retain calculations only across explicitly checked acceptance-only changes.

Calculation manifests are never rewritten. Their original full source archive is
kept separately from the current acceptance source identity. Algorithms, native
programs, protocols, inputs and output hashes remain strict.
"""
import ast
from functools import lru_cache
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import zipfile

from research.common.hashing import digest
from research.common.paths import ROOT,FIGURES
from research.validation.artifacts import read_record,check_hash,require_success
from research.validation.run_audit_checks import tracked_files

THESIS='src/research/experiments/thesis_workflow.py'
GUI='src/research/validation/gui_workflow.py'
TEST='src/tests/python/test_hardening.py'
SELF='src/research/validation/source_compatibility.py'
HOUSEKEEPING='src/research/validation/results_retention.py'
ARTIFACTS='src/research/validation/artifacts.py'
PIPELINE='src/research/pipeline.py'
INTEGRATION_ONLY={
    'src/research/validation/project_closure.py',
    'src/research/validation/reproduction_kit.py',
    'src/research/experiments/project_scope.json',
}
APPLICATION_ADDITIONS={
    'src/research/data/gvda.py', 'src/research/data/gvda_inputs.json',
    'src/research/experiments/gvda_recovery.py', 'src/research/experiments/gvda_protocol.json',
    'src/research/experiments/gvda_confirmation_protocol.json',
    'src/research/experiments/viking_transfer.py', 'src/research/experiments/viking_transfer_protocol.json',
    'src/research/experiments/soda_followup.py', 'src/research/experiments/soda_followup_protocol.json',
    'src/research/experiments/application_suite.py', 'src/research/recovery_local.py',
    'src/research/validation/field_artifacts.py', 'src/research/analysis/application_report.py',
    'src/tests/python/test_applications.py',
}
RETAINED=('synthetic_search','synthetic_joint','synthetic_analysis','viking','viking_audit',
          'viking_quality','f3','f3_audit','synthetic_confirmation','stratton_confirmation','volve_confirmation')
ATTESTATION='calculation_compatibility.json'


def _acceptance_ast(name,raw):
    tree=ast.parse(raw.decode('utf-8-sig'))
    allowed={'validate_retained'} if name==THESIS else {'check_hash'} if name==ARTIFACTS else {'run'}
    if name==THESIS:
        class ApplicationIntegration(ast.NodeTransformer):
            def visit_Expr(self, node):
                call = node.value
                if isinstance(call, ast.Call) and isinstance(call.func, ast.Name):
                    if (call.func.id == 'step' and call.args and isinstance(call.args[0], ast.Constant)
                            and call.args[0].value == 'field_applications'):
                        return None
                    if call.func.id == 'publish_applications':
                        return None
                return self.generic_visit(node)

            def visit_ImportFrom(self, node):
                if node.module == 'research.experiments.application_suite':
                    return None
                return node
        tree = ApplicationIntegration().visit(tree)
        for function in tree.body:
            if isinstance(function,ast.FunctionDef) and function.name=='run':
                for node in function.body:
                    if isinstance(node,ast.Try) and len(node.body)>=2:
                        suffix='\n'.join(ast.unparse(n) for n in node.body[-2:])
                        if suffix=='from research.validation.results_retention import keep_latest\nkeep_latest(out)':
                            node.body=node.body[:-2]
    tree.body=[node for node in tree.body if not (
        isinstance(node,(ast.FunctionDef,ast.AsyncFunctionDef)) and node.name in allowed)]
    if name==GUI:
        tree.body=[node for node in tree.body if not (isinstance(node,ast.If) and
                   ast.unparse(node.test)=="__name__ == '__main__'")]
    if name==PIPELINE:
        # Existing command mappings and parser behavior remain identical.
        for node in ast.walk(tree):
            if isinstance(node, ast.Tuple):
                node.elts = [n for n in node.elts if not (
                    isinstance(n, ast.Constant) and n.value == 'applications')]
            if isinstance(node, ast.Dict):
                keep = [(k, v) for k, v in zip(node.keys, node.values) if not (
                    isinstance(k, ast.Constant) and k.value == 'applications')]
                node.keys, node.values = [k for k, _ in keep], [v for _, v in keep]
    return ast.dump(tree,include_attributes=False)


def compare_sources(before,after):
    """All calculation/data/protocol/analysis sources must remain byte-identical."""
    if set(before)-set(after):raise ValueError('A calculation source was removed')
    added=set(after)-set(before)
    if added-({SELF,HOUSEKEEPING}|APPLICATION_ADDITIONS):raise ValueError('Unexpected added calculation source')
    changed=[]
    for name,raw in before.items():
        if raw==after[name]:continue
        if name in (TEST,SELF,HOUSEKEEPING) or name in INTEGRATION_ONLY:
            changed.append(name);continue
        if name not in (THESIS,GUI,ARTIFACTS,PIPELINE) or _acceptance_ast(name,raw)!=_acceptance_ast(name,after[name]):
            raise ValueError('Calculation source changed: '+name)
        changed.append(name)
    return dict(changed=sorted(changed),added=sorted(added),
                unchanged_calculation_sources=len(before)-len(changed))


def _archive(path,identity):
    with zipfile.ZipFile(path) as zipped:
        names=set(identity['source_sha256'])
        if set(zipped.namelist())!=names|{'SOURCE_IDENTITY.json'}:
            raise ValueError('Calculation archive inventory differs')
        if json.loads(zipped.read('SOURCE_IDENTITY.json'))!=identity:
            raise ValueError('Calculation archive identity differs')
        sources={name:zipped.read(name) for name in names}
    for name,raw in sources.items():
        if hashlib.sha256(raw).hexdigest()!=identity['source_sha256'][name]:
            raise ValueError('Calculation archive source changed: '+name)
    return sources


def _current_sources():
    return {p.relative_to(ROOT).as_posix():p.read_bytes() for p in tracked_files()}


def _junction(target,source):
    # Both are new/retained result directories inside this project. Do not follow
    # arbitrary junctions or compose deletion/move commands.
    base=(ROOT/'results').resolve()
    if not source.resolve().is_relative_to(base) or not target.parent.resolve().is_relative_to(base):
        raise ValueError('Retained result directory escapes project results')
    if target.exists():raise FileExistsError(target)
    if any(c in str(target)+str(source) for c in ('"','%','!','\r','\n')):
        raise ValueError('Unsupported junction path characters')
    command=f'cmd.exe /d /v:off /c mklink /J "{target}" "{source}"'
    result=subprocess.run(command,capture_output=True,
        creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    if result.returncode:
        detail=(result.stdout+result.stderr).decode('mbcs',errors='replace')
        raise RuntimeError('Could not create retained result junction: '+detail)
    if target.resolve()!=source.resolve():raise ValueError('Retained junction target differs')


def prepare(study,out,scope):
    from research.experiments.thesis_workflow import source_identity
    study,out=Path(study).resolve(),Path(out).resolve()
    if scope!='all':raise ValueError('Acceptance-only retention requires the full all scope')
    if (not out.is_relative_to((ROOT/'results').resolve()) or not out.is_dir() or any(out.iterdir())):
        raise ValueError('Use a new empty acceptance directory inside results')
    if study==out or not study.is_relative_to((ROOT/'results').resolve()):
        raise ValueError('Use a distinct retained calculation directory inside results')
    old=read_record(study/'source_identity.json');current=source_identity()
    if old['executable_sha256']!=current['executable_sha256']:
        raise ValueError('Native program changed; calculation retention is forbidden')
    if (study/'workflow.json').is_file() and read_record(study/'workflow.json').get('status')=='passed':
        return _prepare_completed(study,out,scope,old,current)
    archive=study/'calculation_source_snapshot.zip'
    registered=read_record(study/'calculation_source_archive.json')
    check_hash(archive,registered['sha256'])
    check_hash(study/'source_identity.json',registered['source_identity_sha256'])
    comparison=compare_sources(_archive(archive,old),_current_sources())
    for name in RETAINED:
        marker=study/name/('manifest.json' if name not in ('synthetic_search',) else 'run_manifest.json')
        if name in ('viking_audit','f3_audit','synthetic_confirmation','stratton_confirmation','volve_confirmation'):
            marker=study/name/'verification.json'
        if name=='viking':marker=study/name/'analysis/manifest.json'
        require_success(read_record(marker))
    copied=out/'calculation_source_snapshot.zip';shutil.copy2(archive,copied)
    state=dict(schema='acceptance_only_retention_v1',calculation_root=study.relative_to(ROOT).as_posix(),
        calculation_identity=old,acceptance_identity=current,comparison=comparison,
        archive='calculation_source_snapshot.zip',archive_sha256=digest(copied),
        retained_entries=list(RETAINED)+['synthetic_verification.json'],
        scope='Original computation source preserved. Only acceptance functions and tests differ; all algorithms, protocols, data loaders, analyses and native program are unchanged.')
    (out/ATTESTATION).write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    for name in RETAINED:_junction(out/name,study/name)
    shutil.copy2(study/'synthetic_verification.json',out/'synthetic_verification.json')
    def group(identity):
        value=hashlib.sha256(json.dumps(dict(identity=identity,scope=scope),sort_keys=True).encode()).hexdigest()[:16]
        return FIGURES/'research/runs'/value
    before,after=group(old),group(current)
    if after.exists():raise FileExistsError('Use a fresh acceptance figure group')
    shutil.copytree(before,after)
    return state


def _prepare_completed(study,out,scope,old,current):
    """Move one completed edition into its successor with original records intact."""
    origins=[];roots=[study.relative_to(ROOT).as_posix()]
    previous=study/ATTESTATION
    if previous.is_file():
        prior=read_record(previous)
        if prior['schema']=='acceptance_only_retention_v1':
            origins.append(dict(identity=prior['calculation_identity'],archive=prior['archive'],sha256=prior['archive_sha256']))
            roots.append(prior['calculation_root'])
        else:
            origins.extend(prior['origins']);roots.extend(prior['relocated_roots'])
    snapshot=study/'accepted_source_snapshot.zip'
    if not snapshot.is_file():snapshot=study/'acceptance_source_snapshot.zip'
    origins.append(dict(identity=old,archive=snapshot.name,sha256=digest(snapshot)))
    for index,origin in enumerate(origins):
        archive=study/origin['archive'];check_hash(archive,origin['sha256'])
        comparison=compare_sources(_archive(archive,origin['identity']),_current_sources())
        target=out/('calculation_source_'+str(index)+'.zip');shutil.copy2(archive,target)
        origin.update(archive=target.name,comparison=comparison)
    entries=[p for p in study.iterdir() if p.is_dir() and p.name not in ('checks',)]
    state=dict(schema='acceptance_only_retention_v2',acceptance_identity=current,origins=origins,
        relocated_roots=sorted(set(roots)),retained_entries=[p.name for p in entries]+['synthetic_verification.json'],
        scope='Original calculation sources and manifests preserved; only checked validation and result retention changes allowed.')
    (out/ATTESTATION).write_text(json.dumps(state,indent=2)+'\n',encoding='utf-8')
    base=(ROOT/'results').resolve()
    for path in entries:
        source=path.resolve()
        if not source.is_relative_to(base):raise ValueError('Retained directory escapes results')
        if path.is_junction():path.rmdir()
        elif path.is_symlink():raise ValueError('Unsupported retained symbolic link')
        shutil.move(str(source),str(out/path.name))
    for path in study.glob('*.log'):shutil.copy2(path,out/path.name)
    shutil.copy2(study/'synthetic_verification.json',out/'synthetic_verification.json')
    def group(identity):
        value=hashlib.sha256(json.dumps(dict(identity=identity,scope=scope),sort_keys=True).encode()).hexdigest()[:16]
        return FIGURES/'research/runs'/value
    before,after=group(old),group(current)
    for name in ('synthetic','viking','f3'):shutil.copytree(before/name,after/name)
    return state


@lru_cache(maxsize=8)
def _context(path,registration_sha256):
    from research.experiments.thesis_workflow import source_identity
    path=Path(path);check_hash(path,registration_sha256);state=read_record(path)
    if state.get('schema') not in ('acceptance_only_retention_v1','acceptance_only_retention_v2'):
        raise ValueError('Unknown source retention schema')
    if source_identity()!=state['acceptance_identity']:raise ValueError('Acceptance source identity changed')
    if state['schema']=='acceptance_only_retention_v2':
        for origin in state['origins']:
            archive=path.parent/origin['archive'];check_hash(archive,origin['sha256'])
            if compare_sources(_archive(archive,origin['identity']),_current_sources())!=origin['comparison']:
                raise ValueError('Source compatibility declaration differs')
        return state
    archive=path.parent/state['archive'];check_hash(archive,state['archive_sha256'])
    comparison=compare_sources(_archive(archive,state['calculation_identity']),_current_sources())
    if comparison!=state['comparison']:raise ValueError('Source compatibility declaration differs')
    return state


def check_retained_source(path,expected,marker):
    """A changed acceptance file needs a bound archive; every other hash is strict."""
    path,marker=Path(path),Path(marker).absolute()
    actual=digest(path)
    if actual==expected:return
    for parent in marker.parents:
        registration=parent/ATTESTATION
        if not registration.is_file():continue
        state=_context(str(registration),digest(registration))
        relative=marker.relative_to(parent)
        if relative.parts[0] not in state['retained_entries']:
            raise ValueError('Marker is not a declared retained calculation')
        name=path.relative_to(ROOT).as_posix()
        origins=state['origins'] if state['schema']=='acceptance_only_retention_v2' else [
            dict(identity=state['calculation_identity'],comparison=state['comparison'])]
        if (not any(name in origin['comparison']['changed'] and
                origin['identity']['source_sha256'].get(name)==expected for origin in origins) or
                state['acceptance_identity']['source_sha256'].get(name)!=actual):
            raise ValueError('Unregistered source mismatch: '+name)
        return
    raise ValueError('Retained source differs without a calculation archive: '+str(path))
