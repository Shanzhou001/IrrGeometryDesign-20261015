"""Bounded measured-data software check; does not infer cross-survey effectiveness."""
import argparse
import json
from pathlib import Path

from research.common.paths import executable
from research.data.viking import DEFAULT_DATASET, load_dataset
from research.experiments.audit_joint_studies import Recorder
from research.experiments.viking_study import search, evaluate_case, verify_isolation


def run(out, exe=None, dataset=DEFAULT_DATASET):
    program=Path(exe) if exe is not None else executable()
    if not program.is_file():raise FileNotFoundError(program)
    shots,provenance=load_dataset(dataset,[53])
    shot=shots[0]
    recorder=Recorder(Path(out),program,dict(purpose='registered measured-data software check',
        dataset_provenance=provenance,independent_surveys=1,budget=64,iterations=4),
        protocol='registered_viking_software_check_v1')
    layouts=search(recorder,shot.x-shot.x[0],[60],[20261026],64)
    records=[]
    for row in layouts:
        records.append(evaluate_case(recorder,shot,row,4))
    isolation=verify_isolation(recorder,shot,layouts[0],4)
    recorder.state.update(status='passed',recovery_cases=len(records),isolation=isolation)
    recorder.finish()
    report=dict(status='passed',scope='Registered Viking inputs and bounded observed/hidden-data software integration; not a scientific superiority test',
        registered_inputs_verified=len(provenance['input_sha256']),independent_surveys=1,
        source_provenance=provenance,recovery_cases=len(records),records=records,isolation=isolation,
        inferential_tests_performed=False)
    (recorder.out/'verification.json').write_text(json.dumps(report,indent=2,allow_nan=False)+'\n',encoding='utf-8')
    print(json.dumps(dict(status='passed',recovery_cases=len(records),registered_inputs_verified=len(provenance['input_sha256']))))
    return report


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--exe',type=Path,default=executable())
    parser.add_argument('--dataset',type=Path,default=DEFAULT_DATASET)
    args=parser.parse_args();run(args.out,args.exe,args.dataset)
