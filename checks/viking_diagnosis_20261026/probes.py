"""Small post-hoc controls on two preselected masks; H scores cannot select production settings."""
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import subprocess
import time

import numpy as np

from research.common.hashing import digest
from research.common.paths import ROOT, executable
from research.common.processes import native_environment
from research.common.tabular import rows, write_rows
from research.data.viking import load_dataset
from research.experiments.audit_joint_studies import write_gather, write_mask
from research.experiments.review_studies import snr
from research.experiments.viking_study import hidden_mask

OUT = ROOT/'results/viking_diagnosis_20261026'
SOURCE = ROOT/'results/viking_20261026'


def main():
    shot = load_dataset(shot_ids=[53])[0][0]
    source_rows = rows(SOURCE/'recovery.csv')
    hidden = hidden_mask(120)
    configurations = [('warm40',40,.0265,.0005,False), ('cold40',40,.0265,.0005,True),
                      ('warm80',80,.0265,.0005,False), ('cold80',80,.0265,.0005,True),
                      ('warm160',160,.0265,.0005,False), ('cold160',160,.0265,.0005,True),
                      ('warm_strong40',40,.08,.002,False), ('cold_strong40',40,.08,.002,True),
                      ('warm_weak40',40,.008,.0002,False), ('cold_weak40',40,.008,.0002,True),
                      ('warm_no_shrink1',1,0,0,False), ('cold_no_shrink1',1,0,0,True)]
    folder = OUT/'probes'; folder.mkdir(exist_ok=False)
    exe = executable(); signature = digest(exe)
    tasks = []
    for method in ['Jitter', 'Adaptive']:
        row = next(r for r in source_rows if r['ffid']=='53' and r['m']=='60' and r['seed']=='20261024' and r['method']==method)
        mask = np.array(list(row['mask']), int).astype(bool)
        observed = np.zeros_like(shot.data); observed[:, mask] = shot.data[:, mask]
        gather = folder/(method+'_observed.csv'); maskfile = folder/(method+'_mask.txt')
        write_gather(gather, observed, shot.x, 'POST-HOC DIAGNOSTIC: hidden amplitudes zero; no model selection')
        write_mask(maskfile, mask)
        tasks += [(method, row, config, gather, maskfile) for config in configurations]
    def invoke(task):
        method, original, config, gather, maskfile = task
        name, iterations, start, end, cold = config
        key = method+'_'+name
        cached = SOURCE/'native'/(original['case']+('_selected_cold' if cold else '_fixed'))
        reuse = name in ['warm40', 'cold40'] and cached.is_dir()
        target = cached if reuse else folder/key
        command = [str(exe),'--review-reconstruct',str(gather),'--mask',str(maskfile),'--out',str(target),
                   '--iter',str(iterations),'--threshold-start',str(start),'--threshold-end',str(end)]
        if cold:
            command += ['--cold']
        started = time.perf_counter()
        if not reuse:
            with (folder/(key+'.log')).open('wb') as log:
                result = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, cwd=ROOT,
                                        env=native_environment(), creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0),timeout=120)
            if result.returncode:
                raise RuntimeError(key+' failed')
        prediction = np.loadtxt(target/'prediction.csv', delimiter=',')
        solver = rows(target/'solver.csv')[0]
        result = dict(method=method, configuration=name, iterations=iterations, threshold_start=start, threshold_end=end,
                      cold_start=cold, reused_frozen_prediction=reuse, hidden_snr=float(snr(shot.data,prediction,hidden)),
                      final_step=float(rows(target/'convergence.csv')[-1]['relative_step']),
                      known_scale=float(solver['known_scale']), consistency_error=float(solver['consistency_error']))
        audit = dict(command=command, reused_frozen_prediction=reuse, seconds=time.perf_counter()-started,
                     input_sha256={str(p):digest(p) for p in [gather,maskfile]},
                     output_sha256={str(p):digest(p) for p in target.glob('*.csv')})
        return result, audit
    with ThreadPoolExecutor(max_workers=2) as pool:
        output = list(pool.map(invoke, tasks))
    if digest(exe) != signature:
        raise RuntimeError('Executable changed')
    write_rows(OUT/'parameter_probes.csv', [r[0] for r in output])
    manifest = dict(scope='post-hoc two-mask diagnostic on FFID53/m60/seed20261024; not independent validation; do not select production settings from H',
                    executable_sha256=signature, script_sha256=digest(__file__),
                    source_manifest_sha256=digest(SOURCE/'manifest.json'), runs={str(i):a for i,(_,a) in enumerate(output)})
    (folder/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf8')
    for record,_ in output:
        print(record['method'],record['configuration'],round(record['hidden_snr'],5),flush=True)


if __name__=='__main__':
    main()
