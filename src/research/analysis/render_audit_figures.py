"""Render the final thesis figure edition from archived tables, preserving statistical provenance."""
import argparse
import json
from pathlib import Path

import research.analysis.summarize_audit_v5 as summary
from research.common.tabular import rows
from research.common.hashing import digest
from research.common.paths import ROOT, central_figures


def run(root):
    summary.TABLES='analysis_thesis';summary.STRUCTURAL=True
    def numeric(path,keys):
        data=rows(path)
        for r in data:
            for k in keys:r[k]=float(r[k]) if r[k]!='' else None
        return data
    matched=numeric(root/'analysis_thesis/common_evaluation_pairs.csv',['difference','ci_low','ci_high'])
    joint=numeric(root/'analysis_thesis/joint_means.csv',['mean_snr','ci_low','ci_high'])
    band=numeric(root/'analysis_thesis/band_rank_sensitivity.csv',['rho_default'])
    figures=central_figures('research/v5')
    names=summary.figures(root,[],matched,joint,band)
    old=json.loads((root/'analysis_thesis_manifest.json').read_text())
    state=dict(statistical_generator_sha256=old['script_sha256'],statistical_source_snapshot='runtimes/analysis_thesis_v1.py',
               figure_generator_sha256=digest(Path(summary.__file__)),render_driver_sha256=digest(__file__),
               image_policy='one PNG per figure; overwrite canonical output; statistical archives read only',
               input_analysis_manifest_sha256=digest(root/'analysis_thesis_manifest.json'),plots=names,
               output_sha256={str(p.relative_to(ROOT)):digest(p) for p in figures.rglob('*') if p.is_file() and p.name!='render_manifest.json'})
    (figures/'render_manifest.json').write_text(json.dumps(state,indent=2),encoding='utf-8');print(names)
    return names


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);run(p.parse_args().root.resolve())
