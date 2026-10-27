"""Freeze the x64 batch runtime and Qt/VC90 dependencies for local reproduction."""
import argparse
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess

from research.common.hashing import digest
from research.common.processes import qt_binary_directory


def imports(path):
    data=Path(path).read_bytes();pe=struct.unpack_from('<L',data,60)[0]
    if data[pe:pe+4]!=b'PE\0\0':raise ValueError('Not a PE file: '+str(path))
    machine,sections=struct.unpack_from('<HH',data,pe+4);size=struct.unpack_from('<H',data,pe+20)[0];opt=pe+24
    if machine!=0x8664 or struct.unpack_from('<H',data,opt)[0]!=0x20b:raise ValueError('Expected x64 PE runtime')
    table=opt+size
    def offset(rva):
        for i in range(sections):
            virtual_size,virtual,raw_size,raw=struct.unpack_from('<4L',data,table+i*40+8)
            if virtual<=rva<virtual+max(virtual_size,raw_size):return raw+rva-virtual
        raise ValueError('Unmapped PE address')
    rva=struct.unpack_from('<L',data,opt+120)[0]
    if not rva:return []
    cursor=offset(rva);names=[]
    while any(data[cursor:cursor+20]):
        name=offset(struct.unpack_from('<L',data,cursor+12)[0]);end=data.index(0,name)
        names.append(data[name:end].decode('ascii'));cursor+=20
    return names


def create(program,out):
    program,out=Path(program).resolve(),Path(out).resolve()
    if out.exists():raise FileExistsError(out)
    out.mkdir(parents=True);qt=qt_binary_directory();queue=[program];copied={};system=[]
    # Include only application/Qt dependencies; OS libraries stay with Windows.
    while queue:
        path=queue.pop(0)
        if path.name.lower() in copied:continue
        target=out/path.name;shutil.copy2(path,target);copied[path.name.lower()]=target
        for name in imports(path):
            candidates=[path.parent/name,program.parent/name,qt/name]
            found=next((p for p in candidates if p.is_file()),None)
            if found is not None:queue.append(found)
            elif name.lower().startswith(('qt','fdct','fftw')):raise FileNotFoundError('Required runtime dependency: '+name)
            else:system.append(name)
    redist=Path(os.environ.get('ProgramFiles(x86)','C:/Program Files (x86)'))/'Microsoft Visual Studio 9.0/VC/redist/amd64/Microsoft.VC90.CRT'
    if not redist.is_dir():raise FileNotFoundError('VC90 x64 redistributable assembly is unavailable')
    shutil.copytree(redist,out/'Microsoft.VC90.CRT')
    notices=out/'licenses/qt4';notices.mkdir(parents=True)
    for name in ('LICENSE','LICENSE.LGPL','LICENSE.GPL3'):
        shutil.copy2(qt.parent/name,notices/name)
    (out/'RUNTIME_NOTES.txt').write_text('Local x64 batch runtime. Qt4 notices retained; original Qt source: https://download.qt.io/archive/qt/4.7/qt-everywhere-opensource-src-4.7.4.tar.gz\nVC90 CRT copied from the installed Visual Studio redistributable assembly. Windows system DLLs are not bundled. No external publication or new license is asserted.\n',encoding='utf8')
    files={p.relative_to(out).as_posix():dict(sha256=digest(p),bytes=p.stat().st_size) for p in out.rglob('*') if p.is_file()}
    env=os.environ.copy();env.pop('QTDIR',None);env['PATH']=str(Path(os.environ['SystemRoot'])/'System32')
    env['IRR_DISABLE_FIGURES']='1'
    result=subprocess.run([str(out/program.name),'--test-core'],cwd=out,env=env,capture_output=True,
                          creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
    (out/'runtime_test.log').write_bytes(result.stdout+result.stderr)
    state=dict(status='passed' if result.returncode==0 else 'failed',program=program.name,files=files,
               system_dependencies=sorted(set(system)),qt_removed_from_path=True,core_exit_code=result.returncode,
               scope='Controlled PATH test on this Windows host, not a fresh physical Windows installation',script_sha256=digest(__file__))
    (out/'runtime_manifest.json').write_text(json.dumps(state,indent=2)+'\n')
    if result.returncode:raise RuntimeError('Bundled runtime core test failed; inspect runtime_test.log')
    print(json.dumps(dict(status='passed',files=len(files),out=str(out))));return state


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--exe',type=Path,required=True);parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args();create(args.exe,args.out)
