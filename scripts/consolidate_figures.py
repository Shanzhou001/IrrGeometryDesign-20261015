"""One-time centralization of images; previous editions stay in Git c191e6c5."""
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import shutil
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]


def consolidate():
    baseline = json.loads((ROOT/'results/refactor_20261024/baseline.json').read_text(encoding='utf-8'))
    config = ROOT/'config/path_migrations.json'
    record = json.loads(config.read_text(encoding='utf-8'))
    assert not record.get('images'), 'already consolidated'
    groups = defaultdict(list)
    aliases = {}
    for old in baseline['images']:
        path = Path(old)
        if old.startswith('IrrGeometryDesign/Resources/'):
            target = record['files'][old]
        elif old.startswith('results/audit_20261024_v5/figures'):
            target = 'figures/research/v5/' + path.stem + '.png'
        elif old.startswith('results/review_20261024_v4/'):
            target = 'figures/research/v4/' + path.stem + '.png'
        elif old.startswith('figures/enhanced_20261023/'):
            target = old.replace('figures/enhanced_20261023/', 'figures/legacy/v3/')
        elif old.startswith('figures/revised_20261023/'):
            target = old.replace('figures/revised_20261023/', 'figures/legacy/v2/')
        elif old.startswith('figures/gui/'):
            target = old
        elif old.startswith('figures/real/'):
            target = old.replace('figures/real/', 'figures/legacy/real/')
        else:
            target = 'figures/legacy/baseline/' + path.name
        aliases[old] = target
        if path.suffix.lower() == '.png':
            groups[target].append(old)
    # Final thesis/final experiment editions take precedence over drafts/formal rehearsals.
    owners = {}
    for target, candidates in groups.items():
        owners[target] = min(candidates, key=lambda p: (
            0 if '/figures_thesis/' in p or '/final/figures/' in p else 1,
            0 if '/figures_thesis_revision1/' in p else 1, p))
    # Merge exact copies, including duplicate validation figures and resource icons.
    hashes = {}
    redirects = {}
    for target, old in sorted(owners.items(), key=lambda x: ('/validation/' in x[0], x[0])):
        value = baseline['images'][old]['sha256']
        if value in hashes:
            redirects[target] = hashes[value]
        else:
            hashes[value] = target
    aliases = {old: redirects.get(target, target) for old, target in aliases.items()}
    blobs = {}
    for target, old in owners.items():
        actual = ROOT / record['files'].get(old, old)
        assert actual.is_file(), str(actual)
        if target not in redirects:
            blobs[target] = actual.read_bytes()
    for target, data in blobs.items():
        destination = ROOT/target
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
    final_paths = set(blobs)
    for old in baseline['images']:
        current = record['files'].get(old, old)
        path = (ROOT/current).resolve()
        assert path.is_relative_to(ROOT)
        if current not in final_paths and path.exists():
            path.unlink()
    # Retain the canonical figure's numerical companion in the same dedicated directory.
    source = ROOT/'results/audit_20261024_v5/figures_thesis'
    for path in source.glob('*.data.json'):
        shutil.copy2(path, ROOT/'figures/research/v5'/path.name)
    for path in ROOT.glob('figures/assets/*'):
        old = 'IrrGeometryDesign/Resources/' + path.name
        if old in aliases:
            record['files'][old] = aliases[old]
    record['files'].update(aliases)
    record['images'] = dict(old_paths=len(aliases),canonical_images=len(blobs),
                            policy='PNG only for generated figures; latest v5 thesis and v4 final editions supersede image drafts; exact byte duplicates have one owner; static icons are source assets.',
                            original_revision='c191e6c5',aliases=aliases,owners={p:owners[p] for p in blobs})
    config.write_text(json.dumps(record,indent=2),encoding='utf-8')
    qrc = ROOT/'resources/qt/datareconstruction.qrc'
    tree = ET.parse(qrc)
    import os
    for element in tree.iter('file'):
        old = 'IrrGeometryDesign/' + element.get('alias')
        element.text = os.path.relpath(ROOT/aliases[old], qrc.parent).replace('\\','/')
    ET.indent(tree);tree.write(qrc,encoding='utf-8',xml_declaration=True)
    print(record['images']['old_paths'], 'old images ->', len(blobs), 'canonical PNGs')


if __name__ == '__main__':
    consolidate()
