"""Single entry point for building, checking and rendering the project."""
import argparse
import datetime
import html
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import platform
import shutil

from research.common.hashing import digest
from research.common.paths import ROOT, FIGURES, executable, source_files
from research.common.processes import native_environment
from research.common.figures import generation_pass


class Run:
    def __init__(self,out):
        self.out=out.resolve();self.out.mkdir(parents=True,exist_ok=False)
        self.state=dict(command=[sys.executable]+sys.argv,started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                        git_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT).decode().strip(),
                        source_sha256={p.relative_to(ROOT).as_posix():digest(p) for p in source_files()+sorted((ROOT/'research').rglob('*.py'))},
                        steps=[])
        import numpy,matplotlib,PIL
        self.state['environment']=dict(python=sys.version,platform=platform.platform(),numpy=numpy.__version__,matplotlib=matplotlib.__version__,Pillow=PIL.__version__)
        self.state['source_sha256'].update({p.relative_to(ROOT).as_posix():digest(p) for p in (ROOT/'build').rglob('*') if p.is_file() and p.suffix in {'.bat','.vcproj','.pro'} and 'qt-check' not in p.parts})
        self.state['source_sha256'].update({p.relative_to(ROOT).as_posix():digest(p) for p in
            [ROOT/'pipeline.py',ROOT/'IrrGeometryDesign.sln',ROOT/'build_release.bat',ROOT/'build_debug.bat',*sorted((ROOT/'resources/qt').glob('*'))] if p.is_file()})
        qmake=shutil.which('qmake.exe')
        dependencies=list((ROOT/'third_party/legacy-libs').glob('*'))
        if qmake:
            dependencies += [Path(qmake).parent/name for name in ['QtCore4.dll','QtGui4.dll','QtOpenGL4.dll','Qt3Support4.dll','QtCored4.dll','QtGuid4.dll','QtOpenGLd4.dll','Qt3Supportd4.dll']]
        self.state['runtime_dependencies_sha256']={str(p):digest(p) for p in dependencies if p.is_file()}
        self.save()

    def save(self):
        (self.out/'pipeline.json').write_text(json.dumps(self.state,indent=2),encoding='utf-8')

    def command(self,name,args,render=False):
        command=list(map(str,args));print('START',name,flush=True);start=time.perf_counter()
        with (self.out/(name+'.log')).open('wb') as log:
            result=subprocess.run(command,cwd=ROOT,env=native_environment(render),stdout=log,stderr=subprocess.STDOUT,
                                  creationflags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0)
        entry=dict(name=name,command=command,exit_code=result.returncode,seconds=time.perf_counter()-start)
        entry['input_sha256']={str(Path(value).resolve()):digest(value) for value in command[1:] if Path(value).is_file()}
        if Path(command[0]).is_file():entry['program_sha256']=digest(command[0])
        self.state['steps'].append(entry);self.save();print('END',name,result.returncode,flush=True)
        if result.returncode:raise RuntimeError(name+' failed; see '+str(self.out/(name+'.log')))

    def module(self,name,module,*args):
        self.command(name,[sys.executable,'-m',module,*args])

    def native(self,name,args,render=False):
        self.command(name,[executable(),*args],render)

    def finish(self):
        for name,value in self.state['source_sha256'].items():
            if digest(ROOT/name)!=value:raise RuntimeError('active source changed during pipeline: '+name)
        self.state.update(completed=True,finished_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                          outputs={p.relative_to(self.out).as_posix():digest(p) for p in self.out.rglob('*') if p.is_file() and p.name!='pipeline.json'})
        self.save()


def prepare_image_registry():
    from research.analysis.legacy_figures import plan
    jobs,aliases=plan();path=ROOT/'config/path_migrations.json';record=json.loads(path.read_text(encoding='utf-8'))
    previous=set(record['images']['owners'])
    owners={p:old for p,old in record['images']['owners'].items() if not p.startswith('figures/legacy/')}
    for name in jobs:
        destination='figures/'+name
        owner=next(old for old,target in aliases.items() if target==destination and old in record['images']['aliases'])
        owners[destination]=owner
    record['files'].update(aliases);record['images']['aliases'].update(aliases)
    record['images']['owners']=owners;record['images']['canonical_images']=len(owners)
    record['images']['deduplication']='figure kind and actual source hashes; old equal PNG bytes can be stale copies'
    path.write_text(json.dumps(record,indent=2),encoding='utf-8')
    for name in previous-set(owners):
        p=(ROOT/name).resolve();assert p.is_relative_to(FIGURES.resolve())
        if p.exists():p.unlink()
        companion=p.with_suffix('.data.json')
        if companion.exists():companion.unlink()
    pairs={old.removeprefix('figures/'):new.removeprefix('figures/') for old,new in record['images']['aliases'].items() if old.startswith('figures/')}
    (FIGURES/'aliases.tsv').write_text(''.join(k+'\t'+v+'\n' for k,v in sorted(pairs.items())),encoding='utf-8')
    return record


def render(run):
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    from research.analysis import summarize_review as v4
    from research.analysis.review_diagnostics import plot_real_diagnosis
    from research.analysis.render_audit_figures import run as thesis
    from research.analysis.legacy_figures import render as legacy
    from research.common.tabular import rows
    record=prepare_image_registry()
    plt.rcParams.update({'font.family':['Microsoft YaHei','SimSun','DejaVu Sans'],'axes.unicode_minus':False})
    root=ROOT/'results/review_20261024_v4/final';state=json.loads((root/'summary.json').read_text(encoding='utf-8'))
    with generation_pass() as counts:
        thesis(ROOT/'results/audit_20261024_v5')
        search=[dict(r,table_dir=p.parent.name) for p in sorted(root.glob('search_*/search.csv')) for r in rows(p)]
        v4.figures(root,search,state['search'],rows(root/'real_nested.csv'),state['synthetic'])
        plot_real_diagnosis(json.loads((root/'real_diagnosis.json').read_text(encoding='utf-8')))
        legacy_records,_=legacy(run.out,run.native)
        run.native('gui_screenshots',['--screenshot-all'],render=True)
        run.native('gui_survey',['--screenshot-survey'],render=True)
        for p in (FIGURES/'gui').glob('*.png'):
            if str(p.resolve()) in counts:raise AssertionError('GUI image generated twice')
            counts[str(p.resolve())]=1
        saved={Path(p).relative_to(ROOT).as_posix():n for p,n in counts.items()}
    expected=set(record['images']['owners'])
    generated={p for p in expected if not p.startswith('figures/assets/')}
    assert set(saved)==generated,(set(saved)-generated,generated-set(saved))
    images={p:dict(sha256=digest(ROOT/p),bytes=(ROOT/p).stat().st_size,
                   kind='source_asset' if '/assets/' in p else 'generated',
                   generated_this_pass=saved.get(p,0)) for p in sorted(expected)}
    manifest=dict(command=run.state['command'],image_format='PNG',duplicates=0,
                  images=images,legacy_source_records=legacy_records,
                  note='Static Qt icons are original source assets. Each generated image is saved once in this pass; existing canonical paths are overwritten.')
    (FIGURES/'manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
    for directory in sorted({(ROOT/p).parent for p in expected}):
        if directory==FIGURES/'research/v5':continue  # Thesis writer already owns this gallery and its manifest.
        entries=sorted(p for p in directory.glob('*.png'))
        (directory/'index.html').write_text('<!doctype html><meta charset="utf-8"><style>body{font:16px sans-serif;max-width:1200px;margin:auto}img{max-width:100%}</style>'+''.join('<h2>'+html.escape(p.stem)+'</h2><img src="'+p.name+'" loading="lazy">' for p in entries),encoding='utf-8')
    (FIGURES/'index.html').write_text('<!doctype html><meta charset="utf-8"><title>项目图片总览</title><style>body{font:16px sans-serif;max-width:1100px;margin:auto}img{max-width:100%}</style><h1>项目图片总览</h1>'+''.join('<h2>'+html.escape(p)+'</h2><img src="'+p.removeprefix('figures/')+'" loading="lazy">' for p in sorted(expected) if '/assets/' not in p),encoding='utf-8')
    print('Generated',len(saved),'PNGs once; source assets',len(expected)-len(saved),flush=True)


def all_checks(run):
    run.module('regression','research.validation.run_audit_checks','--build','--out',run.out/'regression')
    run.command('qmake_build',['cmd','/c','build\\qmake_check.bat'])
    run.module('legacy_pipeline','research.experiments.run_revised_studies','--quick','--output',run.out/'legacy_pipeline')
    for name,module in [('numeric','audit_numerics'),('projection','audit_projection'),
                        ('fdct','run_fdct_probe'),('legacy_safety','run_legacy_codec_checks'),
                        ('smoke','pipeline_smoke')]:
        run.module(name,'research.validation.'+module,'--out',run.out/name)
    run.module('replay','research.validation.replay_audit_science',ROOT/'results/audit_20261024_v5','--out',run.out/'replay')
    for name in ['revised_20261023','enhanced_20261023']:
        run.module(name,'research.validation.verify_revised_results',ROOT/'results'/name)
    run.module('archives','research.validation.archives','--out',run.out/'archives')
    render(run)
    run.module('layout','research.validation.layout','--report',run.out/'layout.json','--figures','--cli')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command',choices=['all','build','test','figures','verify','inspect'])
    parser.add_argument('--out',type=Path,help='new validation directory; canonical figures always go to figures/')
    args=parser.parse_args();os.chdir(ROOT)
    out=args.out or ROOT/'results/validation'/datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f')
    run=Run(out)
    try:
        if args.command=='all':all_checks(run)
        elif args.command=='build':
            for mode in ['release','debug']:run.command('build_'+mode,['cmd','/c','build_'+mode+'.bat'])
        elif args.command=='test':run.module('regression','research.validation.run_audit_checks','--out',run.out/'regression')
        elif args.command=='figures':
            render(run);run.module('layout','research.validation.layout','--report',run.out/'layout.json','--figures')
        elif args.command=='verify':
            run.module('archives','research.validation.archives','--out',run.out/'archives')
            run.module('layout','research.validation.layout','--report',run.out/'layout.json','--figures')
        else:run.module('layout','research.validation.layout','--report',run.out/'layout.json')
        run.finish()
    except Exception as error:
        run.state.update(completed=False,error=str(error));run.save();raise
    print('PASS',run.out,flush=True)


if __name__=='__main__':main()
