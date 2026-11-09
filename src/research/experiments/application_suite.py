"""Maintained matched-field applications and prospective applicability confirmation."""
import argparse
import json
from pathlib import Path
import shutil

from research.common.hashing import digest
from research.common.paths import ROOT, FIGURES, current_results
from research.experiments import gvda_recovery, soda_followup, viking_transfer
from research.experiments.field_recovery import dump, verify as verify_primary
from research.validation.field_artifacts import check_manifest
from research.validation.run_audit_checks import tracked_files


def verify(out, model_study=None):
    out = Path(out)
    manifest = json.loads((out/'manifest.json').read_text())
    if manifest.get('status') != 'passed':
        raise ValueError('Incomplete field application suite')
    for section, base in [('source_sha256', ROOT), ('input_sha256', ROOT), ('output_sha256', out)]:
        for name, value in manifest[section].items():
            if digest(base/name) != value:
                raise ValueError('Application suite evidence changed: ' + name)
    stages = {}
    for name, module in [('soda_followup', soda_followup), ('viking_transfer', viking_transfer),
                         ('gvda', gvda_recovery), ('gvda_confirmation', gvda_recovery)]:
        state = module.verify(out/name)
        if state != json.loads((out/name/'verification.json').read_text()):
            raise ValueError('Application verification differs from recorded evidence: ' + name)
        stages[name] = state
    primary = verify_primary(Path(model_study or out.parent/'field_recovery_v2'))
    summary = json.loads((out/'report/summary.json').read_text())
    report = json.loads((out/'report/manifest.json').read_text())
    for name, value in report['input_sha256'].items():
        if digest(ROOT/name) != value:
            raise ValueError('Application report input changed')
    for name, value in report['output_sha256'].items():
        if digest(out/'report'/name) != value:
            raise ValueError('Application report output changed')
    effect = (primary['surveys']['poland']['passed'] and
              all(stages[k]['performance_status'] == 'passed'
                  for k in ('viking_transfer', 'gvda', 'gvda_confirmation')))
    if summary['application_effect_status'] != ('passed' if effect else 'failed'):
        raise ValueError('Application report effect differs from verified source results')
    if (summary['soda_boundary']['main_passed'] != primary['surveys']['soda']['passed'] or
            summary['soda_boundary']['local_delay_passed'] !=
            (stages['soda_followup']['performance_status'] == 'passed')):
        raise ValueError('Reported Soda Lake boundary differs from measured outcomes')
    protocol = check_manifest(out/'gvda_confirmation')
    if len(protocol['holdout']) != 12 or len(protocol['layout_seeds']) < 20:
        raise ValueError('Prospective source or layout replication is incomplete')
    return dict(status='passed', cases=sum(v['cases'] for v in stages.values()),
                application_effect_status='passed' if effect else 'failed',
                soda_boundary=summary['soda_boundary'], stages=stages,
                manifest_sha256=digest(out/'manifest.json'))


def run(out, model_study, workers=6):
    from research.analysis.application_report import run as analyze
    from research.data.gvda import prepare

    out, model_study = Path(out).resolve(), Path(model_study).resolve()
    out.mkdir(parents=True)
    prepare()
    # Each stage rejects existing directories and retains all scientific outcomes.
    soda_followup.run(out/'soda_followup', workers)
    viking_transfer.run(out/'viking_transfer', model_study, workers)
    gvda_recovery.run(out/'gvda', workers)
    gvda_recovery.run(out/'gvda_confirmation', workers, gvda_recovery.CONFIRMATION, out/'gvda')
    analyze(out.parent, out, model_study)
    outputs = ['report/manifest.json']
    for name in ('soda_followup', 'viking_transfer', 'gvda', 'gvda_confirmation'):
        outputs.extend([name+'/manifest.json', name+'/verification.json'])
    dump(out/'manifest.json', dict(status='passed',
         source_sha256={p.relative_to(ROOT).as_posix(): digest(p) for p in tracked_files()},
         input_sha256={(model_study/'verification.json').relative_to(ROOT).as_posix():
                       digest(model_study/'verification.json')},
         output_sha256={name: digest(out/name) for name in outputs}))
    result = verify(out, model_study)
    dump(out/'verification.json', result)
    print(json.dumps(result, indent=2), flush=True)
    return result


def publish(base, figure_group, scope='all'):
    if scope != 'all':
        return
    from research.analysis.catalog import publish as publish_catalog

    base = Path(base)
    report = base/'field_applications/report'
    state = json.loads((FIGURES/'current.catalog.json').read_text(encoding='utf-8'))
    images = list(state['images'])
    for name, title in [('source_effects', '实测应用：震源统计与适用范围'),
                        ('layout_effects', '布局贡献：震源配对差与不确定性')]:
        destination = FIGURES/figure_group/'applications'/(name+'.png')
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(report/(name+'.png'), destination)
        images.append(dict(path=destination.relative_to(FIGURES).as_posix(), title=title,
                           description='以震源为统计单位，保留Soda Lake不适用结果和全部布局对照。',
                           sha256=digest(destination)))
    reports = [(r['title'], ROOT/r['path']) for r in state['reports']]
    reports.append(('实测应用与适用条件', report/'report.html'))
    return publish_catalog(base, images, reports, scope)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, default=current_results()/'field_applications')
    parser.add_argument('--model-study', type=Path, default=current_results()/'field_recovery_v2')
    parser.add_argument('--workers', type=int, default=6, choices=range(1, 17))
    parser.add_argument('--verify', action='store_true')
    args = parser.parse_args()
    if args.verify:
        print(json.dumps(verify(args.out, args.model_study), indent=2))
    else:
        run(args.out, args.model_study, args.workers)
