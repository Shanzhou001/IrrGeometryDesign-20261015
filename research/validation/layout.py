"""Check imports, dependency cycles, native include paths, resources and image policy."""
import argparse
import ast
import hashlib
import importlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

from research.common.hashing import digest
from research.common.paths import ROOT, FIGURES

IMAGE_SUFFIXES={'.png','.svg','.jpg','.jpeg','.bmp','.gif','.ico','.webp','.tif','.tiff'}


def scientific_source_equivalence():
    record=json.loads((ROOT/'config/path_migrations.json').read_text(encoding='utf-8'))
    names=['Sampling.cpp','sample.h','CurveletMiu.cpp','CalculateMiu.h','OptimizeMiu.cpp','OptimizeMiu.h',
           'adaptivesearch.inc','ExpRng.cpp','ExpRng.h','spsmaskbuilder.cpp','spsmaskbuilder.h',
           'Diagmul.cpp','spssegy/segyio.cpp','spssegy/segyio.h',
           'Curvelet2d/fdct_wrapping.cpp','Curvelet2d/ifdct_wrapping.cpp']
    def normalize(raw):
        raw=raw.removeprefix(b'\xef\xbb\xbf').replace(b'\r\n',b'\n')
        return re.sub(rb'(?m)^\s*#include[^\n]*\n',b'',raw)
    for name in names:
        old='IrrGeometryDesign/'+name
        original=subprocess.check_output(['git','show','c191e6c5:'+old],cwd=ROOT)
        assert normalize(original)==normalize((ROOT/record['files'][old]).read_bytes()),old
    return dict(compared_files=len(names),same_except_include_paths_and_line_endings=True)


def inspect_code():
    paths=sorted((ROOT/'research').rglob('*.py'))
    modules={p.relative_to(ROOT).with_suffix('').as_posix().replace('/','.') for p in paths if p.name!='__init__.py'}
    graph={m:set() for m in modules};failures=[]
    for path in paths:
        module=path.relative_to(ROOT).with_suffix('').as_posix().replace('/','.')
        if module not in modules:continue
        importlib.import_module(module)
        for node in ast.walk(ast.parse(path.read_text(encoding='utf-8-sig'))):
            candidates=[]
            if isinstance(node,ast.Import):candidates=[x.name for x in node.names]
            elif isinstance(node,ast.ImportFrom):candidates=[node.module]+[(node.module or '')+'.'+x.name for x in node.names]
            graph[module].update(m for m in candidates if m in modules)
    visited=set();active=[]
    def visit(module):
        if module in active:raise AssertionError('module cycle: '+' -> '.join(active+[module]))
        if module in visited:return
        active.append(module)
        for dependency in sorted(graph[module]):visit(dependency)
        active.pop();visited.add(module)
    for module in sorted(graph):visit(module)
    project=ROOT/'build/legacy/IrrGeometryDesign.vcproj';tree=ET.parse(project)
    include_dirs=sorted({p.parent for base in [ROOT/'src/native',ROOT/'third_party/curvelet',ROOT/'third_party/qcustomplot'] for p in base.rglob('*.h')})
    include_dirs += [ROOT/'build/generated',ROOT/'build/generated/Release',ROOT/'build/generated/Debug']
    qmake=shutil.which('qmake.exe')
    if qmake:
        qt=Path(qmake).parent.parent/'include'
        include_dirs += [qt]+[qt/name for name in ['QtCore','QtGui','QtOpenGL','Qt3Support']]
    if os.name=='nt':include_dirs.append(Path(os.environ['ProgramFiles(x86)'])/'Microsoft Visual Studio 9.0/VC/include')
    checked=set()
    for element in tree.iter('File'):
        path=(project.parent/element.get('RelativePath').replace('\\','/')).resolve()
        if path.is_relative_to(ROOT/'build/generated'):continue
        assert path.is_file(),str(path)
        if path.suffix not in {'.cpp','.h','.hpp','.inc'}:continue
        checked.add(path)
        raw=re.sub(rb'/\*.*?\*/',b'',path.read_bytes(),flags=re.S)
        raw=re.sub(rb'//[^\r\n]*',b'',raw)
        for name in re.findall(rb'(?m)^\s*#include\s*"([^"\r\n]+)"',raw):
            name=name.decode('ascii').replace('\\','/')
            if not any((base/name).is_file() for base in [path.parent]+include_dirs):failures.append([str(path.relative_to(ROOT)),name])
    assert not failures,failures
    qrc=ROOT/'resources/qt/datareconstruction.qrc';resources=[]
    for element in ET.parse(qrc).iter('file'):
        path=(qrc.parent/element.text).resolve();assert path.is_file() and path.is_relative_to(FIGURES)
        resources.append(element.get('alias'))
    return dict(imported_modules=len(modules),dependency_graph={m:sorted(v) for m,v in sorted(graph.items())},
                acyclic=True,native_include_files=len(checked),qt_resource_aliases=resources)


def inspect_images():
    images=[];outside=[]
    for base,dirs,names in os.walk(ROOT):
        dirs[:]=[d for d in dirs if d!='.git']
        for name in names:
            path=Path(base)/name
            if path.suffix.lower() in IMAGE_SUFFIXES:
                (images if path.is_relative_to(FIGURES) else outside).append(path)
    assert not outside,[str(p.relative_to(ROOT)) for p in outside]
    manifest=json.loads((FIGURES/'manifest.json').read_text(encoding='utf-8'))
    actual={p.relative_to(ROOT).as_posix() for p in images};expected=set(manifest['images'])
    assert actual==expected,(sorted(actual-expected),sorted(expected-actual))
    hashes={};duplicates=[]
    from PIL import Image
    for path in images:
        assert path.suffix.lower()=='.png'
        with Image.open(path) as image:image.verify()
        value=digest(path);name=path.relative_to(ROOT).as_posix();entry=manifest['images'][name]
        assert value==entry['sha256']
        assert entry['generated_this_pass']==(0 if entry['kind']=='source_asset' else 1)
        if value in hashes:duplicates.append([hashes[value],name])
        hashes[value]=name
    assert not duplicates,duplicates
    thesis=json.loads((FIGURES/'research/v5/render_manifest.json').read_text(encoding='utf-8'))
    for name,value in thesis['output_sha256'].items():assert digest(ROOT/name)==value,name
    return dict(total_images=len(images),source_assets=sum(r['kind']=='source_asset' for r in manifest['images'].values()),
                generated_once=sum(r['generated_this_pass']==1 for r in manifest['images'].values()),
                images_outside_figures=0,duplicate_hashes=0)


def inspect_cli():
    records=[]
    for path in sorted((ROOT/'research').rglob('*.py')):
        text=path.read_text(encoding='utf-8-sig')
        if '__main__' not in text or 'ArgumentParser' not in text:continue
        module=path.relative_to(ROOT).with_suffix('').as_posix().replace('/','.')
        command=[sys.executable,'-m',module,'--help']
        result=subprocess.run(command,cwd=ROOT,capture_output=True,creationflags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0)
        records.append(dict(module=module,command=command,exit_code=result.returncode))
        assert result.returncode==0,(module,result.stderr.decode(errors='replace'))
    return records


def inspect_references():
    count=0;missing=[]
    for path in [ROOT/'README.md']+sorted((ROOT/'docs').rglob('*.md')):
        for value in re.findall(r'\]\(([^)]+)\)',path.read_text(encoding='utf-8-sig')):
            if '://' in value or value.startswith('#'):continue
            value=value.split('#',1)[0].strip('<>')
            if not value:continue
            target=(path.parent/value).resolve();count+=1
            if not target.exists():missing.append([path.relative_to(ROOT).as_posix(),value])
    assert not missing,missing
    return dict(local_links=count,missing=0)


def run(report,figures=False,cli=False):
    assert not report.exists(),'report already exists'
    result=dict(passed=True,code=inspect_code())
    result['scientific_source_equivalence']=scientific_source_equivalence()
    result['references']=inspect_references()
    if cli:result['cli_help']=inspect_cli()
    if figures:result['figures']=inspect_images()
    report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(result,indent=2),encoding='utf-8')
    brief={k:v for k,v in result.items() if k not in ['code','cli_help']}
    if cli:brief['cli_help_passed']=len(result['cli_help'])
    print(json.dumps(brief),result['code']['imported_modules'],'modules')
    return result


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',type=Path,required=True);parser.add_argument('--figures',action='store_true');parser.add_argument('--cli',action='store_true')
    args=parser.parse_args();run(args.report.resolve(),args.figures,args.cli)
