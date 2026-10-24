"""Verify live scientific results and explicitly identified pre-refactor originals."""
import json
from pathlib import Path

from research.common.archives import HistoricalFiles
from research.common.hashing import digest
from research.common.paths import ROOT
from research.validation import verify_audit_v5 as v5
from research.validation import verify_review_results as v4
from research.validation import verify_audit_summary as summary


def run(out):
    out.mkdir(parents=True,exist_ok=False)
    root=ROOT/'results/audit_20261024_v5';runtime=root/'runtimes/v5'
    snapshot=json.loads((runtime/'snapshot.json').read_text(encoding='utf-8'))
    for name,value in snapshot['dependency_sha256'].items():
        assert digest(runtime/'bin'/Path(name).name)==value
    stages=[v5.verify_stage(root/name,runtime) for name in ['search','joint','real','timing']]
    baseline=json.loads((root/'baseline_manifest.json').read_text(encoding='utf-8'))
    archive=HistoricalFiles();counts={'working_bytes':0,'original_git_bytes':0}
    try:
        for name,record in baseline['protected_files'].items():
            counts[archive.check(name,record['sha256'])]+=1
        protocol_status=archive.check('docs/EXPERIMENT_PROTOCOL.md',archive.expected['docs/EXPERIMENT_PROTOCOL.md'])
        # These switches only affect source/retired image provenance. Numerical readers
        # below still consume the actual working CSVs, predictions and original data.
        original=v4.digest;v4.digest=archive.digest
        try:
            older=v4.verify(ROOT/'results/review_20261024_v4/final',False,report=out/'v4.json')
        finally:v4.digest=original
        original=summary.digest;summary.digest=archive.digest
        try:summary.run(root,out/'structural_statistics.json')
        finally:summary.digest=original
        report=dict(passed=True,scientific_stages=stages,protected_files=counts,
                    original_protocol_document=protocol_status,
                    historical_v4_program='results/audit_20261024_v5/baseline_runtime/x64/Release/IrrGeometryDesign.exe',
                    original_source_revision='72240bbf',original_image_revision='c191e6c5',
                    originals_read=sorted(archive.originals),
                    proven_line_ending_reconstructions=sorted(archive.line_ending_reconstructed),
                    exact_frozen_snapshot_sources=sorted(archive.exact_snapshot_sources),
                    scope='Scientific inputs and numerical outputs checked in the working tree; relocated or regenerated historical display artifacts checked against explicitly named Git originals. This is not current-binary cache reuse.')
    finally:archive.close()
    (out/'verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(dict(passed=True,protected_files=counts,stages=len(stages))))
    return report


if __name__=='__main__':
    import argparse
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--out',type=Path,required=True)
    run(parser.parse_args().out.resolve())
