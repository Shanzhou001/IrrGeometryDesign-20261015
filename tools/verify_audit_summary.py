"""Independent raw-record aggregation and exhaustive sign-permutation audit of final thesis tables."""
import argparse
from collections import defaultdict
import itertools
import json
from pathlib import Path

import numpy as np

from audit_joint_studies import rows
from run_audit_checks import digest


def exact_test(differences):
    d=[float(v) for v in differences if abs(v)>1e-12]
    if not d:return 1.,0.
    ranks=[1+sum(abs(x)<abs(v)-1e-12 for x in d)+.5*(sum(abs(abs(x)-abs(v))<1e-12 for x in d)-1) for v in d]
    observed=sum(r*(1 if v>0 else -1) for r,v in zip(ranks,d))
    extreme=sum(abs(sum(r*s for r,s in zip(ranks,signs)))>=abs(observed)-1e-10 for signs in itertools.product([-1,1],repeat=len(d)))
    return extreme/2**len(d),observed/sum(ranks)


def run(root,output=None):
    output=output or root/'summary_verification.json';assert not output.exists()
    raw=rows(root/'joint/joint.csv');templates=defaultdict(list)
    for r in raw:
        if r['design']=='common_H':
            for recovery in ['fixed','selected']:
                templates[r['m'],recovery,r['method'],r['family'],r['structure']].append(float(r[recovery+'_snr']))
    # Balanced 2 noise conditions x 3 masks for each clean structure, independently averaged.
    assert all(len(values)==6 for values in templates.values())
    values={k:float(sum(v)/len(v)) for k,v in templates.items()}
    means=rows(root/'analysis_thesis/joint_means.csv')
    for r in means:
        v=[x for (m,recovery,method,_,_),x in values.items() if (m,recovery,method)==(r['m'],r['recovery'],r['method'])]
        assert len(v)==int(r['analysis_units'])==6
        assert abs(sum(v)/6-float(r['mean_snr']))<1e-12
    comparisons=rows(root/'analysis_thesis/joint_pairs.csv');rawp=[]
    for r in comparisons:
        a,b=r['comparison'].split('-');keys=sorted((family,structure) for m,recovery,method,family,structure in values if (m,recovery,method)==(r['m'],r['recovery'],a))
        av=np.array([values[r['m'],r['recovery'],a,*k] for k in keys]);bv=np.array([values[r['m'],r['recovery'],b,*k] for k in keys]);d=av-bv
        p,effect=exact_test(d);rawp.append(p)
        assert abs(float(r['difference'])-d.mean())<1e-12
        assert abs(float(r['p_raw'])-p)<1e-12 and abs(float(r['rank_biserial'])-effect)<1e-12
        rng=np.random.default_rng(54127);samples=d[rng.integers(6,size=(10000,6))].mean(axis=1)
        low,high=np.quantile(samples,[.025,.975])
        assert abs(float(r['ci_low'])-low)<1e-11 and abs(float(r['ci_high'])-high)<1e-11
    order=sorted(range(len(rawp)),key=lambda i:rawp[i]);previous=0.
    for rank,i in enumerate(order):
        previous=max(previous,min(1.,(len(order)-rank)*rawp[i]));assert abs(previous-float(comparisons[i]['p_holm']))<1e-12
    prefix_pairs=0
    for r in rows(root/'search/search.csv'):
        if r['method']!='NoRecycle':continue
        name=r['id'].replace('_NoRecycle_','_Adaptive_')
        a=rows(root/'search/sensitivity'/(name+'.trace.csv'));b=rows(root/'search/sensitivity'/(r['id']+'.trace.csv'))
        assert len(a)>=len(b)
        for x,y in zip(a,b):
            assert all(x[k]==y[k] for k in x if k!='wall_ms'),(name,x,y)
        prefix_pairs+=1
    # Check frozen v4 plotted scalars and final artifact checksums without regenerating historical files.
    source=json.loads((root/'figures_thesis/01_v4_search.data.json').read_text())
    for r in source:
        directory=Path(__file__).resolve().parents[1]/'results/review_20261024_v4/final'/f"search_{r['domain']}_{r['budget']}"
        v=[float(row['mu']) for row in rows(directory/'search.csv') if row['method']==r['method'] and int(row['rate_index'])==r['rate']]
        assert abs(sum(v)/len(v)-r['mean'])<1e-12 and len(v)==r['n']
    manifest=json.loads((root/'final_figures_manifest.json').read_text())
    for p,h in manifest['output_sha256'].items():assert digest(root/p)==h,p
    statistic=json.loads((root/'analysis_thesis_manifest.json').read_text())
    for p,h in statistic['output_sha256'].items():
        actual=root/p
        if p.replace('\\','/').startswith('figures_thesis/'):actual=root/'figures_thesis_draft'/Path(p).name
        assert digest(actual)==h,p
    result=dict(passed=True,model_conditions=12,clean_structural_units=6,model_means=len(means),paired_tests=len(comparisons),
                exact_sign_permutations_per_test=64,holm_significant=sum(float(r['p_holm'])<.05 for r in comparisons),
                plotted_v4_search_means=len(source),final_figures=len(manifest['plots']),figure_output_hashes=len(manifest['output_sha256']))
    result['identical_adaptive_norecycle_prefix_pairs']=prefix_pairs
    output.write_text(json.dumps(result,indent=2));print(json.dumps(result))


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('root',type=Path);p.add_argument('--report',type=Path)
    a=p.parse_args();run(a.root.resolve(),a.report.resolve() if a.report else None)
