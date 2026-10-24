"""Check staged archive byte preservation and write a final non-circular inventory.

Run after staging the audit outputs. Existing reports are never overwritten.
The inventory excludes itself and Git-ignored build intermediates.
"""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import xml.etree.ElementTree as ET

from PIL import Image

from run_audit_checks import digest

ROOT = Path(__file__).resolve().parents[1]


def git(*args, data=None):
    return subprocess.check_output(['git'] + list(args), cwd=ROOT, input=data)


def finalize(root):
    report = root / 'delivery_manifest.json'
    qa = root / 'figure_archive_checks.json'
    assert not report.exists() and not qa.exists(), 'refuse to replace final records'
    relative = root.relative_to(ROOT).as_posix()
    staged = []
    for line in git('ls-files', '--stage', '-z', '--', relative).split(b'\0'):
        if not line:
            continue
        info, name = line.split(b'\t', 1)
        mode, blob, stage = info.decode().split()
        assert stage == '0' and mode == '100644'
        staged.append((name.decode('utf-8'), blob))
    assert staged, 'stage the archive before running this check'
    names = [name for name, _ in staged]
    actual = git('hash-object', '--no-filters', '--stdin-paths',
                 data=('\n'.join(names) + '\n').encode('utf-8')).decode().splitlines()
    assert len(actual) == len(staged)
    for (name, expected), value in zip(staged, actual):
        assert value == expected, 'Git blob differs from working bytes: ' + name

    figures = root / 'figures_thesis'
    comparisons = []
    for path in sorted(figures.glob('*.png')):
        with Image.open(path) as picture:
            picture.verify()
        with Image.open(path) as picture:
            size = list(picture.size)
        ET.parse(path.with_suffix('.svg'))
        json.loads(path.with_suffix('.data.json').read_text(encoding='utf-8'))
        previous = root / 'figures_thesis_revision1' / path.name
        comparisons.append(dict(name=path.stem, pixels=size,
                                unchanged_from_prior=digest(path) == digest(previous)))
    assert len(comparisons) == 10
    assert {r['name'] for r in comparisons if not r['unchanged_from_prior']} == {
        '02_common_budget', '08_v4_convergence'}
    qa.write_text(json.dumps(dict(
        passed=True, command=[sys.executable] + sys.argv,
        staged_archive_blobs_checked=len(staged),
        png_decode_svg_xml_data_json= comparisons,
        visual_review='All ten groups were viewed during the session; final changed groups 02 and 08 were viewed again. This script checks file integrity, not visual quality.',
    ), indent=2), encoding='utf-8')

    paths = git('ls-files', '--cached', '--others', '--exclude-standard', '-z',
                '--', relative, 'docs', 'tools', 'IrrGeometryDesign',
                'README.md', '.gitattributes').split(b'\0')
    records = {}
    for raw in sorted(set(paths)):
        if not raw:
            continue
        name = raw.decode('utf-8')
        path = ROOT / name
        if not path.is_file() or path == report:
            continue
        records[name] = dict(bytes=path.stat().st_size, sha256=digest(path))
    final = dict(
        command=[sys.executable] + sys.argv,
        git_head_before_archive_commit=git('rev-parse', 'HEAD').decode().strip(),
        scope='Audit outputs, maintained docs/tools/native source; Git-ignored build intermediates excluded. This manifest excludes itself. Earlier protocol files retain their original hashes in baseline_manifest.json.',
        protected_baseline_sha256=digest(root / 'baseline_manifest.json'),
        integrity_report_sha256=digest(root / 'final_integrity_verification.json'),
        staged_archive_blobs_verified=len(staged),
        files=records,
    )
    report.write_text(json.dumps(final, indent=2, ensure_ascii=False), encoding='utf-8')
    print(json.dumps(dict(passed=True, staged_blobs=len(staged),
                          inventory_files=len(records),
                          inventoried_bytes=sum(r['bytes'] for r in records.values()),
                          manifest_sha256=digest(report))))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('root', type=Path)
    finalize(parser.parse_args().root.resolve())
