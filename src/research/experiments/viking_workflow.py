"""Run and verify the Viking-only project from the CLI and Qt experiment window."""
import argparse
from datetime import datetime, timezone
from html import escape
import json
import msvcrt
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

from research.analysis.catalog import publish
from research.common.hashing import digest
from research.common.paths import FIGURES, ROOT, current_results
from research.common.processes import native_environment
from research.common.tabular import rows
from research.experiments.thesis_workflow import source_identity
from research.validation.verify_viking_batch import verify


SCOPE = 'viking'
EVIDENCE = ('source_identity.json', 'checks.json', 'viking/manifest.json',
            'viking/analysis/manifest.json', 'viking/analysis/summary.md',
            'viking_audit/verification.json', 'report.html')
FIGURES_INFO = (
    ('recovery', 'Viking 缺失道恢复', '同一隐藏道上的固定曲波、固定 f-k 与内层选模。'),
    ('geometry', 'Viking 布局与覆盖', '分别显示 Fourier 互干度和最大空档。'),
    ('conditions', 'Viking 分条件结果', '分时间窗和隐藏集合报告同一保留数下的结果。'),
)


def write_json(path, value):
    temporary = path.with_suffix('.tmp')
    temporary.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    temporary.replace(path)


def read_json(path):
    return json.loads(path.read_text(encoding='utf-8'))


def inspect_batch(out):
    state = read_json(out/'workflow.json')
    if state.get('scope') != SCOPE or state.get('status') != 'passed':
        raise ValueError('A completed Viking-only batch is required')
    if read_json(out/'source_identity.json') != source_identity():
        raise ValueError('Viking batch belongs to a different source tree or executable')
    if state.get('evidence_sha256') != {name: digest(out/name) for name in EVIDENCE}:
        raise ValueError('Viking workflow evidence changed')
    analysis = read_json(out/'viking/analysis/manifest.json')
    for name, expected in analysis['table_sha256'].items():
        if digest(out/'viking/analysis'/name) != expected:
            raise ValueError('Viking summary table changed: ' + name)
    return state


def report(out, audit):
    analysis = out/'viking/analysis'
    values = rows(analysis/'overall_means.csv')
    scores = {(r['m'], r['method'], r['recovery']): float(r['mean_snr']) for r in values}
    lines = ''.join(
        f'<tr><td>{int(m)}</td><td>{escape(method)}</td>' + ''.join(
            f'<td>{scores[m, method, kind]:.4f}</td>' for kind in ('cold', 'fk', 'selected')) + '</tr>'
        for m, method in sorted({(r['m'], r['method']) for r in values}, key=lambda x: (int(x[0]), x[1])))
    (out/'report.html').write_text(f'''<!doctype html><meta charset="utf-8">
<title>Viking v2 实测验证</title>
<main style="max-width:1000px;margin:36px auto;font:17px sans-serif;line-height:1.6">
<h1>Viking v2 接收道选择与缺失道重建</h1>
<p>实测输入为 Viking Graben Line 12。独立核验覆盖 {audit['recovery_cases']} 个实验配置和
{audit['checked_prediction_arrays']} 份预测数组。工区数为 1，配置数用于记录重复实验。</p>
<p>下表在同一隐藏道上评分，SNR 单位为 dB，越高越好。先导观测计入保留道数。</p>
<table cellpadding="6"><thead><tr><th>保留道</th><th>布局</th><th>固定曲波</th>
<th>固定 f-k</th><th>内层选模</th></tr></thead><tbody>{lines}</tbody></table>
<p>保留 60 道时，内层选模的 Adaptive_Gap 高于 Jitter。在固定 f-k 恢复下，SA 的同档结果高于 Adaptive_Gap。
这些对照用于区分布局和恢复器的贡献，结论适用于已测炮集和协议。</p>
<p><a href="viking/analysis/summary.md">完整报告</a> · <a href="viking/analysis/paired_effects.csv">配对结果</a> ·
<a href="viking_audit/verification.json">独立核验</a></p></main>''', encoding='utf-8')


def activate(out, state):
    publish(out, state['images'], [('Viking 实测报告', out/'report.html')], SCOPE)
    pointer = ROOT/'results/active.txt'
    temporary = pointer.with_suffix('.tmp')
    temporary.write_text(out.relative_to(ROOT).as_posix() + '\n', encoding='utf-8')
    temporary.replace(pointer)


def run(workers, modes, destination=None):
    results = (ROOT/'results').resolve()
    results.mkdir(exist_ok=True)
    with (results/'workflow.lock').open('a+b') as lock:
        lock.seek(0)
        msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
        if destination is None:
            active = current_results()
            if (active/'workflow.json').is_file() and read_json(active/'workflow.json').get('scope') == SCOPE:
                state = inspect_batch(active)
                activate(active, state)
                print(f'Verified Viking batch: {active}', flush=True)
                return
            destination = results/'viking_only'
            if destination.exists():
                destination = results/f'viking_{datetime.now(timezone.utc):%Y%m%d_%H%M%S}'
        out = Path(destination).resolve()
        if not out.is_relative_to(results) or out == results:
            raise ValueError('Viking batch must be a new directory inside results/')
        out.mkdir(parents=True)
        state = dict(status='running', scope=SCOPE, dataset='viking_line12_small', steps={})
        write_json(out/'workflow.json', state)
        try:
            write_json(out/'source_identity.json', source_identity())
            mode = 'both' if 'Debug' in modes else 'Release'
            group = 'research/viking/' + out.relative_to(results).as_posix()
            with TemporaryDirectory(prefix='irrg_viking_checks_') as temporary:
                checks = Path(temporary)/'checks'
                subprocess.run([sys.executable, '-B', 'pipeline.py', 'check', '--full', '--mode', mode,
                                '--out', str(checks)], cwd=ROOT, env=native_environment(), check=True)
                write_json(out/'checks.json', read_json(checks/'checks.json'))
            state['steps']['project_check'] = 'passed'
            subprocess.run([sys.executable, '-B', 'pipeline.py', 'viking-v2', '--out', str(out/'viking'),
                            '--workers', str(workers), '--group', group, '--seeds', '84011', '94217', '105953',
                            '--inner-folds', '2'], cwd=ROOT, env=native_environment(), check=True)
            state['steps']['viking'] = 'passed'
            audit = verify(out/'viking', out/'viking_audit')
            if audit['recovery_cases'] != 1215 or audit['independent_surveys'] != 1:
                raise ValueError('Viking batch does not match the registered study')
            state['steps']['independent_audit'] = 'passed'
            report(out, audit)
            images = [dict(path=f'{group}/{name}.png', title=title, description=description,
                           sha256=digest(FIGURES/group/f'{name}.png')) for name, title, description in FIGURES_INFO]
            if source_identity() != read_json(out/'source_identity.json'):
                raise ValueError('Source or executable changed during the Viking workflow')
            state.update(status='passed', recovery_cases=audit['recovery_cases'], images=images,
                         evidence_sha256={name: digest(out/name) for name in EVIDENCE})
            write_json(out/'workflow.json', state)
            activate(out, state)
        except Exception as error:
            state.update(status='failed', error=str(error))
            write_json(out/'workflow.json', state)
            raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', nargs='?', choices=['run', 'status'], default='run')
    parser.add_argument('--workers', type=int, choices=range(1, 17), default=8)
    parser.add_argument('--modes', nargs='+', choices=['Release', 'Debug'], default=['Release', 'Debug'])
    parser.add_argument('--scope', choices=['viking', 'all'], default='viking')
    parser.add_argument('--out', type=Path)
    parser.add_argument('--results', type=Path)
    args = parser.parse_args()
    if args.action == 'status':
        base = (args.results or current_results()).resolve()
        state = inspect_batch(base)
        summary = {key: state[key] for key in ('status', 'scope', 'dataset', 'recovery_cases', 'steps')}
        if args.out:
            write_json(args.out, summary)
        print(json.dumps(summary, indent=2))
    else:
        if 'Release' not in args.modes or args.results is not None:
            parser.error('Run requires Release and uses --out to select a batch directory')
        run(args.workers, args.modes, args.out)


if __name__ == '__main__':
    main()
