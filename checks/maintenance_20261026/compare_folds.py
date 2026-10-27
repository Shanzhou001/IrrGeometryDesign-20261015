"""Compare one/two inner folds only on the identical shared 84011 layouts."""
import csv
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]


def read(path):
    with path.open(newline='',encoding='utf8') as stream:return list(csv.DictReader(stream))


def main():
    old_root=ROOT/'results/viking_v2_20261026';new_root=ROOT/'results/viking_robust_20261026'
    manifests=[json.loads((p/'manifest.json').read_text()) for p in (old_root,new_root)]
    assert all(m.get('completed') and m.get('status')=='passed' for m in manifests)
    assert manifests[0]['executable_sha256']==manifests[1]['executable_sha256']
    fields=('ffid','window_start','hidden_set','m','method')
    key=lambda r:tuple(r[k] for k in fields)
    a={key(r):r for r in read(old_root/'recovery.csv') if r['seed']=='84011'}
    b={key(r):r for r in read(new_root/'recovery.csv') if r['seed']=='84011'}
    assert set(a)==set(b) and len(a)==405
    max_fixed=0.;splits_equal=True
    for k in a:
        assert all(a[k][f]==b[k][f] for f in ('mask','pilot_bits','H_bits'))
        for kind in ('cold','fk','linear'):
            max_fixed=max(max_fixed,abs(float(a[k][kind+'_snr'])-float(b[k][kind+'_snr'])))
        ca=json.loads((old_root/'selection'/(a[k]['case']+'.json')).read_text())
        cb=json.loads((new_root/'selection'/(b[k]['case']+'.json')).read_text())
        splits_equal &= ca['splits'][0]==cb['splits'][0]
    assert max_fixed<1e-10 and splits_equal
    rows=[]
    for m in ('36','60','84'):
        keys=[k for k in a if a[k]['m']==m];n=len(keys)
        old=sum(float(a[k]['selected_snr']) for k in keys)/n
        new=sum(float(b[k]['selected_snr']) for k in keys)/n
        rows.append(dict(m=m,cases=n,one_fold_mean_snr=old,two_fold_mean_snr=new,mean_delta_snr=new-old,
                         positive_cases=sum(float(b[k]['selected_snr'])>float(a[k]['selected_snr'])+1e-12 for k in keys)))
    folder=Path(__file__).resolve().parent
    with (folder/'inner_fold_comparison.csv').open('w',newline='',encoding='utf8') as stream:
        writer=csv.DictWriter(stream,fieldnames=list(rows[0]));writer.writeheader();writer.writerows(rows)
    result=dict(status='passed',matched_cases=len(a),mask_seed=84011,masks_identical=True,first_inner_split_identical=splits_equal,
                fixed_recovery_max_snr_difference=max_fixed,independent_surveys=1,
                scope='Descriptive matched-layout comparison; changes only the number of inner validation groups.',records=rows)
    (folder/'inner_fold_comparison.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result,indent=2))


if __name__=='__main__':main()
