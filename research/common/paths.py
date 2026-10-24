"""Project paths shared by CLI, experiments, validation and plotting."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[2]
NATIVE = ROOT / 'src/native'
FIGURES = ROOT / 'figures'
BINARY_ROOT = ROOT / 'build/bin'


def executable(mode='Release'):
    return BINARY_ROOT / mode / 'IrrGeometryDesign.exe'


def source_files():
    roots = [NATIVE, ROOT/'third_party/curvelet', ROOT/'third_party/qcustomplot']
    return sorted(p for base in roots for p in base.rglob('*')
                  if p.suffix in {'.cpp', '.h', '.hpp', '.inc'})


def central_figures(group):
    """Return a contained directory. Absolute and parent-traversing groups are invalid."""
    target = (FIGURES / group).resolve()
    if not target.is_relative_to(FIGURES.resolve()):
        raise ValueError('figure output must stay inside ' + str(FIGURES))
    target.mkdir(parents=True, exist_ok=True)
    return target


def migrated_path(path):
    """Resolve recorded old source/image paths without editing scientific manifests."""
    path = Path(path)
    if path.is_absolute():
        try:
            key = path.relative_to(ROOT).as_posix()
        except ValueError:
            return path
    else:
        key = path.as_posix()
    record = ROOT / 'config/path_migrations.json'
    mappings = json.loads(record.read_text(encoding='utf-8'))['files']
    return ROOT / mappings.get(key, key)
