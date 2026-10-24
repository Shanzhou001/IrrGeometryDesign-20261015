"""Update maintained documentation and gallery links after the recorded relocation."""
from pathlib import Path
import hashlib
import json
import os
import re

ROOT=Path(__file__).resolve().parents[1]


def run():
    config=ROOT/'config/path_migrations.json';record=json.loads(config.read_text(encoding='utf-8'))
    mapping=record['files'];changed=record.setdefault('rewritten_references',{})
    paths=[ROOT/'README.md']+list((ROOT/'docs').rglob('*.md'))+list((ROOT/'results').rglob('*.html'))+list((ROOT/'results').rglob('*.md'))
    for path in paths:
        if 'runtimes' in path.parts or path.is_relative_to(ROOT/'results/refactor_20261024'):continue
        raw=path.read_bytes()
        try:text=raw.decode('utf-8-sig')
        except UnicodeDecodeError:continue
        before=text
        def link(match):
            target=match[1];anchor=''
            if '://' in target or target.startswith('#'):return match[0]
            if '#' in target:target,anchor=target.split('#',1);anchor='#'+anchor
            absolute=(path.parent/target.replace('\\','/')).resolve()
            if not absolute.is_relative_to(ROOT):return match[0]
            key=absolute.relative_to(ROOT).as_posix()
            if key not in mapping:return match[0]
            return ']('+os.path.relpath(ROOT/mapping[key],path.parent).replace('\\','/')+anchor+')'
        text=re.sub(r'\]\(([^)]+)\)',link,text)
        def html_link(match):
            name=match[2]
            if '://' in name or name.startswith('data:'):return match[0]
            absolute=(path.parent/name).resolve()
            if not absolute.is_relative_to(ROOT):return match[0]
            key=absolute.relative_to(ROOT).as_posix()
            if key not in mapping:return match[0]
            return match[1]+os.path.relpath(ROOT/mapping[key],path.parent).replace('\\','/')+match[3]
        text=re.sub(r'((?:src|href)=[\x27\"])([^\x27\"]+)([\x27\"])',html_link,text)
        if path.is_relative_to(ROOT/'docs') or path.name=='README.md':
            for name,module in record['python_modules'].items():
                text=text.replace('python tools/'+name+'.py','python -m '+module)
            for old,new in sorted(mapping.items(),key=lambda item:-len(item[0])):
                text=text.replace(old,new)
            text=text.replace('x64/Release/IrrGeometryDesign.exe','build/bin/Release/IrrGeometryDesign.exe').replace('x64\\Release\\IrrGeometryDesign.exe','build\\bin\\Release\\IrrGeometryDesign.exe')
        if text!=before:
            name=path.relative_to(ROOT).as_posix()
            previous=changed.get(name,{})
            path.write_bytes(text.encode('utf-8'))
            changed[name]=dict(before_sha256=previous.get('before_sha256',hashlib.sha256(raw).hexdigest()),
                               after_sha256=hashlib.sha256(path.read_bytes()).hexdigest(),reason='source/module/image reference relocation only')
    config.write_text(json.dumps(record,indent=2),encoding='utf-8')
    print('Updated references in',len(changed),'documents/galleries')


if __name__=='__main__':run()
