"""Source and image paths; generated files live outside the repository."""
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
WORKSPACE = Path(os.environ.get('IRR_WORKSPACE', str(ROOT) + '-workspace')).expanduser().resolve()
NATIVE = ROOT / 'src/native'
FIGURES = ROOT / 'figures'
BINARY_ROOT = WORKSPACE / 'build/bin'


def executable(mode='Release'):
    return BINARY_ROOT / mode / 'IrrGeometryDesign.exe'


def source_files():
    roots = [NATIVE, ROOT/'src/third_party/curvelet', ROOT/'src/third_party/qcustomplot']
    return sorted(p for base in roots for p in base.rglob('*')
                  if p.suffix in {'.cpp', '.h', '.hpp', '.inc'})


def central_figures(group):
    target = (FIGURES / group).resolve()
    if not target.is_relative_to(FIGURES.resolve()):
        raise ValueError('figure output must stay inside ' + str(FIGURES))
    target.mkdir(parents=True, exist_ok=True)
    return target
