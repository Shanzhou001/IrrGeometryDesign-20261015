"""Native subprocess environment; batch experiments record data without redrawing images."""
import os
from pathlib import Path
import shutil
from research.common.paths import ROOT, WORKSPACE


def qt_binary_directory():
    configured=os.environ.get('QTDIR')
    if configured and (Path(configured)/'bin/qmake.exe').is_file():return Path(configured)/'bin'
    qmake=shutil.which('qmake.exe')
    if qmake:return Path(qmake).parent
    raise RuntimeError('Qt4 qmake.exe was not found; set QTDIR or PATH to the existing Qt4 installation')


def native_environment(render=False):
    env=os.environ.copy()
    env['IRR_PROJECT_ROOT']=str(ROOT)
    env['IRR_WORKSPACE']=str(WORKSPACE)
    env['PYTHONPATH']=str(ROOT/'src')+os.pathsep+env.get('PYTHONPATH','')
    env['PYTHONDONTWRITEBYTECODE']='1'
    try:qt=qt_binary_directory()
    except RuntimeError:qt=None
    if qt:env['PATH']=str(qt)+os.pathsep+env.get('PATH','')
    env['IRR_DISABLE_FIGURES']='0' if render else '1'
    return env
