"""Extract a delivery outside Git and replay it with a caller-supplied clean Python."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import tempfile
from zipfile import ZipFile

from package_delivery import verify


def run(archive, python, out, manuscripts=True):
    archive,python,out=map(lambda p:Path(p).resolve(),(archive,python,out))
    if out.exists():raise FileExistsError(out)
    identity=verify(archive);out.mkdir(parents=True)
    report=dict(status='running',started_utc=datetime.now(timezone.utc).isoformat(),
                archive=identity,python=str(python),steps=[],
                scope='Fresh Python virtual environment; temporary non-Git extraction; PATH limited to Windows System32. Same Windows host, not a fresh OS installation.')
    try:
        with tempfile.TemporaryDirectory(prefix='irr_delivery_replay_') as temporary:
            root=Path(temporary)
            with ZipFile(archive) as source:source.extractall(root)
            project=root/'IrrGeometryDesign';exe=project/'output/runtime_20261027/IrrGeometryDesign.exe'
            env=os.environ.copy()
            for key in ('QTDIR','QT_PLUGIN_PATH','PYTHONPATH','PYTHONHOME','IRR_PROJECT_ROOT','IRR_WORKSPACE'):
                env.pop(key,None)
            env.update(PATH=str(Path(env.get('SystemRoot','C:/Windows'))/'System32'),
                       PYTHONNOUSERSITE='1',PYTHONDONTWRITEBYTECODE='1',MPLBACKEND='Agg',MPLCONFIGDIR=str(root/'mpl'))
            def call(name,command,cwd=project):
                result=subprocess.run(list(map(str,command)),cwd=cwd,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,
                                      creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
                (out/(name+'.log')).write_bytes(result.stdout)
                report['steps'].append(dict(name=name,exit_code=result.returncode))
                if result.returncode:raise RuntimeError(name+' failed; see '+str(out/(name+'.log')))
            call('environment',[python,'-B','-c','import json,sys,numpy,matplotlib; print(json.dumps(dict(prefix=sys.prefix,base_prefix=sys.base_prefix,numpy=numpy.__version__,matplotlib=matplotlib.__version__))); assert sys.prefix!=sys.base_prefix'])
            call('native_core',[exe,'--test-core'])
            call('viking_replay',[python,'-B','pipeline.py','viking-v2','--out','results/zip_replay','--exe',exe,
                '--shots','53','--windows','512','--counts','60','--seeds','84011','--hidden-sets','r271828',
                '--budget','80','--inner-folds','2','--workers','4'])
            state=json.loads((project/'results/zip_replay/manifest.json').read_text())
            assert state['completed'] and state['status']=='passed' and state['isolation']['passed']
            assert state['git_commit'] is None and state['git_metadata_status'] in ('git_not_installed','not_available_for_this_source_copy')
            assert state['source_sha256'] and state['executable_sha256']
            call('independent_prediction_check',[python,'-B','pipeline.py','module','research.validation.verify_viking_batch',
                '--root','results/zip_replay','--out','checks/zip_replay'])
            audit=json.loads((project/'checks/zip_replay/verification.json').read_text())
            assert audit['status']=='passed'
            report['replay']=dict(cases=state['recovery_cases'],native_calls=state['native_calls'],
                git_commit=state['git_commit'],git_metadata_status=state['git_metadata_status'],
                executable_sha256=state['executable_sha256'],source_files=len(state['source_sha256']),
                isolation=state['isolation'],independent_verification=audit)
            # Import and actually load all registered arrays, rather than treating import as a data check.
            call('f3_data_load',[python,'-B','-c',"import sys;sys.path.insert(0,'src');from research.data.f3 import load;w,p=load();assert len(w[0])==3;print('registered F3 input hashes and dimensions passed')"])
            if manuscripts:
                call('manuscript_build',[python,'-B','paper-tgrs/build_manuscripts.py'],root)
                call('manuscript_qa',[python,'-B','paper-tgrs/check_manuscripts.py','--final'],root)
                report['rebuilt_manuscripts']=json.loads((root/'paper-tgrs/build_manifest.json').read_text())['outputs']
            report['status']='passed'
    except Exception as error:
        report.update(status='failed',error=str(error));raise
    finally:
        report['completed_utc']=datetime.now(timezone.utc).isoformat()
        (out/'verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
    print(json.dumps(dict(status=report['status'],archive_sha256=identity['archive_sha256'],out=str(out))))


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive',type=Path);parser.add_argument('--python',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True);parser.add_argument('--skip-manuscripts',action='store_true')
    args=parser.parse_args();run(args.archive,args.python,args.out,not args.skip_manuscripts)
