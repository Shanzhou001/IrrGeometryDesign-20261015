"""Generate v4 result tables and standalone figures from actually archived records."""  # add by wushanzhou in 2026/10/24
import argparse  # add by wushanzhou in 2026/10/24
from collections import defaultdict  # add by wushanzhou in 2026/10/24
import hashlib  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24

import matplotlib  # add by wushanzhou in 2026/10/24
matplotlib.use("Agg")  # add by wushanzhou in 2026/10/24
import matplotlib.pyplot as plt  # add by wushanzhou in 2026/10/24
import numpy as np  # add by wushanzhou in 2026/10/24

from ablation_stats import holm, wilcoxon_signed_rank  # add by wushanzhou in 2026/10/24
from review_studies import read_rows, sha, write_rows  # add by wushanzhou in 2026/10/24
from review_diagnostics import diagnose_real # add by wushanzhou in 2026/10/24


def groups(rows, keys):  # add by wushanzhou in 2026/10/24
    result=defaultdict(list)  # add by wushanzhou in 2026/10/24
    for row in rows:result[tuple(row[key] for key in keys)].append(row)  # add by wushanzhou in 2026/10/24
    return result  # add by wushanzhou in 2026/10/24


def average(rows, field):  # add by wushanzhou in 2026/10/24
    return float(np.mean([float(row[field]) for row in rows]))  # add by wushanzhou in 2026/10/24


def spread(rows, field):  # add by wushanzhou in 2026/10/24
    return float(np.std([float(row[field]) for row in rows],ddof=1)) if len(rows)>1 else 0.  # add by wushanzhou in 2026/10/24


def paired(a,b):  # add by wushanzhou in 2026/10/24
    plus,minus,p,n=wilcoxon_signed_rank(a,b)  # add by wushanzhou in 2026/10/24
    return dict(mean_difference=float(np.mean(np.asarray(a)-b)),p_raw=p,n_effective=n,  # add by wushanzhou in 2026/10/24
                paired_rank_biserial=(plus-minus)/(plus+minus) if plus+minus else 0.)  # add by wushanzhou in 2026/10/24


def correct(records):  # add by wushanzhou in 2026/10/24
    for row,p in zip(records,holm([row["p_raw"] for row in records])):  # add by wushanzhou in 2026/10/24
        row["p_holm"]=p  # add by wushanzhou in 2026/10/24
        row["family_size"]=len(records)  # add by wushanzhou in 2026/10/24
        row["significant_005"]=p<.05  # add by wushanzhou in 2026/10/24
    return records  # add by wushanzhou in 2026/10/24


def ranks(values):  # add by wushanzhou in 2026/10/24
    values=np.asarray(values)  # add by wushanzhou in 2026/10/24
    order=np.argsort(values,kind="stable");result=np.empty(len(values),dtype=float)  # add by wushanzhou in 2026/10/24
    start=0  # add by wushanzhou in 2026/10/24
    while start<len(values):  # add by wushanzhou in 2026/10/24
        end=start+1  # add by wushanzhou in 2026/10/24
        while end<len(values) and values[order[end]]==values[order[start]]:end+=1  # add by wushanzhou in 2026/10/24
        result[order[start:end]]=(start+end-1)/2  # add by wushanzhou in 2026/10/24
        start=end  # add by wushanzhou in 2026/10/24
    return result  # add by wushanzhou in 2026/10/24


def correlation(a,b):  # add by wushanzhou in 2026/10/24
    a,b=np.asarray(a,dtype=float),np.asarray(b,dtype=float)  # add by wushanzhou in 2026/10/24
    return float(np.corrcoef(a,b)[0,1]) if np.std(a)>1e-14 and np.std(b)>1e-14 else None  # add by wushanzhou in 2026/10/24


def markdown_table(rows,keys):  # add by wushanzhou in 2026/10/24
    lines=["|"+"|".join(keys)+"|","|"+"|".join("---" for _ in keys)+"|"]  # add by wushanzhou in 2026/10/24
    for row in rows:  # add by wushanzhou in 2026/10/24
        lines.append("|"+"|".join(f"{row[key]:.6g}" if isinstance(row[key],float) else str(row[key]) for key in keys)+"|")  # add by wushanzhou in 2026/10/24
    return lines  # add by wushanzhou in 2026/10/24


def summarize(output):  # add by wushanzhou in 2026/10/24
    output=Path(output).resolve()  # add by wushanzhou in 2026/10/24
    manifest=json.loads((output/"run_manifest.json").read_text())  # add by wushanzhou in 2026/10/24
    assert manifest.get("completed"),"Only summarize a complete batch"  # add by wushanzhou in 2026/10/24
    search=[]  # add by wushanzhou in 2026/10/24
    for path in sorted(output.glob("search_*/search.csv")):  # add by wushanzhou in 2026/10/24
        for row in read_rows(path):row["table_dir"]=str(path.parent.relative_to(output));search.append(row)  # add by wushanzhou in 2026/10/24
    search_summary=[]  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(search,["domain","budget_cap","rate_index","method"]).items()):  # add by wushanzhou in 2026/10/24
        domain,budget,rate,method=key  # add by wushanzhou in 2026/10/24
        search_summary.append(dict(domain=domain,budget=int(budget),rate=int(rate),method=method,n=len(rows),  # add by wushanzhou in 2026/10/24
                                   unique_masks=len({r["mask_sha1"] for r in rows}),mu=average(rows,"mu"),sd=spread(rows,"mu"),  # add by wushanzhou in 2026/10/24
                                   actual_evals=average(rows,"evals"),native_clock_ms=average(rows,"runtime_ms"),  # add by wushanzhou in 2026/10/24
                                   switches=average(rows,"switches")))  # add by wushanzhou in 2026/10/24
    write_rows(output/"search_summary.csv",search_summary)  # add by wushanzhou in 2026/10/24
    comparisons=[]  # add by wushanzhou in 2026/10/24
    controls=("Hybrid","SAOnly","SA_Budget","FixedSwitch","NoRecycle","AdjacentOnly")  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(search,["domain","budget_cap","rate_index"]).items()):  # add by wushanzhou in 2026/10/24
        methods=groups(rows,["method"])  # add by wushanzhou in 2026/10/24
        adaptive={r["seed"]:float(r["mu"]) for r in methods[("Adaptive",)]}  # add by wushanzhou in 2026/10/24
        for control in controls:  # add by wushanzhou in 2026/10/24
            baseline={r["seed"]:float(r["mu"]) for r in methods[(control,)]}  # add by wushanzhou in 2026/10/24
            seeds=sorted(set(adaptive)&set(baseline))  # add by wushanzhou in 2026/10/24
            stats=paired([adaptive[s] for s in seeds],[baseline[s] for s in seeds])  # add by wushanzhou in 2026/10/24
            comparisons.append(dict(domain=key[0],budget=int(key[1]),rate=int(key[2]),A="Adaptive",B=control,n=len(seeds),**stats))  # add by wushanzhou in 2026/10/24
    correct(comparisons);write_rows(output/"search_statistics.csv",comparisons)  # add by wushanzhou in 2026/10/24
    common_evaluations=[];trace_cache={} # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(search,["domain","budget_cap","rate_index"]).items()): # add by wushanzhou in 2026/10/24
        by_method=groups(rows,["method"]) # add by wushanzhou in 2026/10/24
        for control in controls: # add by wushanzhou in 2026/10/24
            a={r['seed']:r for r in by_method[('Adaptive',)]};b={r['seed']:r for r in by_method[(control,)]} # add by wushanzhou in 2026/10/24
            ks,av,bv=[],[],[] # add by wushanzhou in 2026/10/24
            for seed in sorted(a): # add by wushanzhou in 2026/10/24
                k=min(int(a[seed]['evals']),int(b[seed]['evals']));ks.append(k) # add by wushanzhou in 2026/10/24
                for row,values in [(a[seed],av),(b[seed],bv)]: # add by wushanzhou in 2026/10/24
                    path=output/row['table_dir']/(row['mask_path']+'.trace.csv') # add by wushanzhou in 2026/10/24
                    if path not in trace_cache:trace_cache[path]=[float(r['best_objective']) for r in read_rows(path)] # add by wushanzhou in 2026/10/24
                    values.append(trace_cache[path][k-1]) # add by wushanzhou in 2026/10/24
            common_evaluations.append(dict(domain=key[0],budget=int(key[1]),rate=int(key[2]),control=control,n=len(ks), # add by wushanzhou in 2026/10/24
                                           mean_common_evaluations=float(np.mean(ks)),adaptive_mu=float(np.mean(av)), # add by wushanzhou in 2026/10/24
                                           control_mu=float(np.mean(bv)),difference=float(np.mean(np.asarray(av)-bv)))) # add by wushanzhou in 2026/10/24
    write_rows(output/'common_evaluation_comparisons.csv',common_evaluations) # add by wushanzhou in 2026/10/24
    pareto=[]  # add by wushanzhou in 2026/10/24
    for key,rows in groups(search,["domain","budget_cap","rate_index","seed"]).items():  # add by wushanzhou in 2026/10/24
        values=np.array([[float(r["mu"]),float(r["max_gap_m"] if key[0]=="physical" else r["max_gap"]),float(r["nn_cv"])] for r in rows])  # add by wushanzhou in 2026/10/24
        for i,row in enumerate(rows):  # add by wushanzhou in 2026/10/24
            dominated=any(np.all(values[j]<=values[i]+1e-8) and np.any(values[j]<values[i]-1e-8) for j in range(len(rows)) if j!=i)  # add by wushanzhou in 2026/10/24
            pareto.append(dict(domain=key[0],budget=int(key[1]),rate=int(key[2]),seed=key[3],method=row["method"],  # add by wushanzhou in 2026/10/24
                               mu=float(row["mu"]),max_gap=float(values[i,1]),nn_cv=float(row["nn_cv"]),nondominated=not dominated))  # add by wushanzhou in 2026/10/24
    write_rows(output/"pareto.csv",pareto)  # add by wushanzhou in 2026/10/24
    synthetic=read_rows(output/"synthetic_reconstruction.csv")  # add by wushanzhou in 2026/10/24
    synthetic_summary=[]  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(synthetic,["model","family","rate_index","method"]).items()):  # add by wushanzhou in 2026/10/24
        synthetic_summary.append(dict(model=key[0],family=key[1],rate=int(key[2]),method=key[3],n=len(rows),  # add by wushanzhou in 2026/10/24
                                      snr=average(rows,"snr_missing"),sd=spread(rows,"snr_missing"),linear=average(rows,"snr_linear")))  # add by wushanzhou in 2026/10/24
    write_rows(output/"synthetic_summary.csv",synthetic_summary)  # add by wushanzhou in 2026/10/24
    associations=[]  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(synthetic,["model","rate_index"]).items()):  # add by wushanzhou in 2026/10/24
        unique={r["mask_sha1"]:r for r in rows}  # add by wushanzhou in 2026/10/24
        rows=list(unique.values())  # add by wushanzhou in 2026/10/24
        response=np.array([float(r["snr_missing"]) for r in rows])  # add by wushanzhou in 2026/10/24
        features=np.array([[float(r["mu"]),float(r["max_gap"]),float(r["nn_cv"])] for r in rows])  # add by wushanzhou in 2026/10/24
        centered=features-features.mean(axis=0);scale=features.std(axis=0);scale[scale==0]=1  # add by wushanzhou in 2026/10/24
        design=np.c_[np.ones(len(rows)),centered/scale]  # add by wushanzhou in 2026/10/24
        beta=np.linalg.lstsq(design,response,rcond=None)[0]  # add by wushanzhou in 2026/10/24
        total=np.sum((response-response.mean())**2)  # add by wushanzhou in 2026/10/24
        r2=1-float(np.sum((response-design@beta)**2)/total) if total>0 else 0.  # add by wushanzhou in 2026/10/24
        associations.append(dict(model=key[0],rate=int(key[1]),unique_masks=len(rows),  # add by wushanzhou in 2026/10/24
                                 pearson_mu=correlation(features[:,0],response),spearman_mu=correlation(ranks(features[:,0]),ranks(response)),  # add by wushanzhou in 2026/10/24
                                 pearson_gap=correlation(features[:,1],response),pearson_nn_cv=correlation(features[:,2],response),  # add by wushanzhou in 2026/10/24
                                 standardized_mu_coefficient=float(beta[1]),standardized_gap_coefficient=float(beta[2]),  # add by wushanzhou in 2026/10/24
                                 standardized_nn_coefficient=float(beta[3]),descriptive_in_sample_r2=r2))  # add by wushanzhou in 2026/10/24
    write_rows(output/"geometry_quality_associations.csv",associations)  # add by wushanzhou in 2026/10/24
    cluster_comparisons=[]  # add by wushanzhou in 2026/10/24
    models=sorted({r["model"] for r in synthetic_summary})  # add by wushanzhou in 2026/10/24
    for rate in range(3):  # add by wushanzhou in 2026/10/24
        for control in ("Hybrid","SAOnly","Jitter","Regular","W_03_03"):  # add by wushanzhou in 2026/10/24
            a,b=[],[]  # add by wushanzhou in 2026/10/24
            for model in models:  # add by wushanzhou in 2026/10/24
                group={r["method"]:r["snr"] for r in synthetic_summary if r["model"]==model and r["rate"]==rate}  # add by wushanzhou in 2026/10/24
                a.append(group["Adaptive"]);b.append(group[control])  # add by wushanzhou in 2026/10/24
            cluster_comparisons.append(dict(rate=rate,A="Adaptive",B=control,model_realizations=len(models),**paired(a,b)))  # add by wushanzhou in 2026/10/24
    correct(cluster_comparisons);write_rows(output/"synthetic_cluster_statistics.csv",cluster_comparisons)  # add by wushanzhou in 2026/10/24
    real_masks=read_rows(output/"real_mask_reconstruction.csv")  # add by wushanzhou in 2026/10/24
    real_mask_summary=[]  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(real_masks,["keep","method"]).items()):  # add by wushanzhou in 2026/10/24
        real_mask_summary.append(dict(keep=int(key[0]),method=key[1],n=len(rows),snr=average(rows,"snr_missing"),  # add by wushanzhou in 2026/10/24
                                      sd=spread(rows,"snr_missing"),linear=average(rows,"snr_linear")))  # add by wushanzhou in 2026/10/24
    write_rows(output/"real_mask_summary.csv",real_mask_summary)  # add by wushanzhou in 2026/10/24
    nested=read_rows(output/"real_nested.csv")  # add by wushanzhou in 2026/10/24
    nested_summary=[]  # add by wushanzhou in 2026/10/24
    nested_stats=[]  # add by wushanzhou in 2026/10/24
    for key,rows in sorted(groups(nested,["keep"]).items()):  # add by wushanzhou in 2026/10/24
        fixed=[float(r["snr_fixed"]) for r in rows];selected=[float(r["snr_selected"]) for r in rows]  # add by wushanzhou in 2026/10/24
        gains=np.asarray(selected)-fixed  # add by wushanzhou in 2026/10/24
        nested_summary.append(dict(keep=int(key[0]),n=len(rows),linear=average(rows,"snr_linear"),fixed=float(np.mean(fixed)),  # add by wushanzhou in 2026/10/24
                                   selected=float(np.mean(selected)),mean_gain=float(np.mean(gains)),  # add by wushanzhou in 2026/10/24
                                   positive_selected=sum(v>0 for v in selected),zero_selected=sum(abs(v)<1e-12 for v in selected),  # add by wushanzhou in 2026/10/24
                                   positive_gain=int(np.sum(gains>0))))  # add by wushanzhou in 2026/10/24
        nested_stats.append(dict(keep=int(key[0]),scope="conditional_random_splits_one_fixed_source",**paired(selected,fixed)))  # add by wushanzhou in 2026/10/24
    correct(nested_stats)  # add by wushanzhou in 2026/10/24
    write_rows(output/"real_nested_summary.csv",nested_summary)  # add by wushanzhou in 2026/10/24
    write_rows(output/"real_nested_exploratory_statistics.csv",nested_stats)  # add by wushanzhou in 2026/10/24
    summary=dict(protocol="adaptive_hybrid_v4",independent_real_sources=1,search=search_summary,search_comparisons=comparisons,common_evaluation_comparisons=common_evaluations, # add by wushanzhou in 2026/10/24
                 synthetic=synthetic_summary,synthetic_cluster_comparisons=cluster_comparisons,  # add by wushanzhou in 2026/10/24
                 geometry_quality_associations=associations,real_masks=real_mask_summary,real_nested=nested_summary,  # add by wushanzhou in 2026/10/24
                 real_nested_exploratory_statistics=nested_stats,configuration=manifest["fingerprint"]["configuration"],  # add by wushanzhou in 2026/10/24
                 native_steps=len(manifest["steps"]),summed_step_seconds=sum(v["seconds"] for v in manifest["steps"].values()))  # add by wushanzhou in 2026/10/24
    summary["real_diagnosis"]=diagnose_real(output) # add by wushanzhou in 2026/10/24
    for row in nested_summary: # add by wushanzhou in 2026/10/24
        cases=[r for r in summary['real_diagnosis']['error_decomposition'] if r['keep']==row['keep']] # add by wushanzhou in 2026/10/24
        row['fixed_gain_only']=average(cases,'fixed_gain_only_snr');row['selected_before_gain']=average(cases,'selected_before_gain_snr') # add by wushanzhou in 2026/10/24
    write_rows(output/'real_nested_summary.csv',nested_summary) # add by wushanzhou in 2026/10/24
    (output/"summary.json").write_text(json.dumps(summary,indent=2,allow_nan=False),encoding="utf-8") # add by wushanzhou in 2026/10/24
    lines=["# adaptive_hybrid_v4: measured results","","One independent real waveform source. No historical results or original data were overwritten.","",  # add by wushanzhou in 2026/10/24
           "## Search means (smaller mu is better)",""]  # add by wushanzhou in 2026/10/24
    lines+=markdown_table([r for r in search_summary if r["method"] in ("SAOnly","Hybrid","SA_Budget","Adaptive","NoRecycle")],  # add by wushanzhou in 2026/10/24
                          ["domain","budget","rate","method","n","mu","sd","actual_evals","native_clock_ms"])  # add by wushanzhou in 2026/10/24
    lines += ["","All search comparisons form one prespecified Holm family. Methods with spatial penalties optimize a different objective and are excluded.",  # add by wushanzhou in 2026/10/24
              "Legacy SAOnly/Hybrid use the preserved legacy cooling schedule and backend; SA_Budget isolates the new cooling schedule and cached backend. Reconstruction steps run concurrently; their elapsed times are not isolated speed comparisons. Early-stopped traces retain their final incumbent for plotting only.", # add by wushanzhou in 2026/10/24
              "","## Nested real reconstruction (dB; hidden test traces only)",""]  # add by wushanzhou in 2026/10/24
    lines+=markdown_table(nested_summary,["keep","n","linear","fixed","fixed_gain_only","selected_before_gain","selected","mean_gain","positive_selected","positive_gain"]) # add by wushanzhou in 2026/10/24
    lines += ["","Parameters and prediction gain are selected using only inner validation traces drawn from outer-known traces. Zero/linear candidates are retained; selection of zero is a limitation, not curvelet recovery success. Fixed-gain-only applies the legacy candidate's validation coefficient. Selected-before-gain is an output decomposition, not a separately tuned parameter-only method.", # add by wushanzhou in 2026/10/24
              "","## Synthetic model-level comparisons",""]  # add by wushanzhou in 2026/10/24
    lines+=markdown_table(cluster_comparisons,["rate","A","B","model_realizations","mean_difference","p_holm"])  # add by wushanzhou in 2026/10/24
    lines += ["","Masks on the same realization are not pooled as independent geological models. Model-level tests remain exploratory on six parameter/noise realizations from three families.",  # add by wushanzhou in 2026/10/24
              "","## Interpretation limits","","- Lower Fourier coherence, better spatial coverage and higher curvelet SNR are distinct outcomes.",  # add by wushanzhou in 2026/10/24
              "- Association regressions are descriptive in-sample fits; no causal or validated prediction claim is made.",  # add by wushanzhou in 2026/10/24
              "- Real split statistics describe one fixed source and do not establish cross-survey generalization.",  # add by wushanzhou in 2026/10/24
              "- Regular is deterministic: repeated seeds do not create independent masks.",  # add by wushanzhou in 2026/10/24
              "- Candidate validation and all negative outcomes are retained.","","<!-- add by wushanzhou in 2026/10/24 -->"]
    (output/"RESULTS.md").write_text("\n".join(lines)+"\n",encoding="utf-8")  # add by wushanzhou in 2026/10/24
    figures(output,search,search_summary,nested,synthetic_summary)  # add by wushanzhou in 2026/10/24
    artifacts=[p for p in output.glob("*.csv")]+[output/"summary.json",output/"real_diagnosis.json",output/"RESULTS.md"]+list((output/"figures").glob("*")) # add by wushanzhou in 2026/10/24
    post=dict(generator_sha256=sha(__file__),diagnostic_tool_sha256=sha(Path(__file__).with_name('review_diagnostics.py')),matplotlib=matplotlib.__version__,numpy=np.__version__, # add by wushanzhou in 2026/10/24
              input_tables_sha256={str(p.relative_to(output)):sha(p) for p in output.glob("search_*/search.csv")},  # add by wushanzhou in 2026/10/24
              artifact_sha256={str(p.relative_to(output)):sha(p) for p in artifacts if p.is_file()})  # add by wushanzhou in 2026/10/24
    (output/"postprocessing_manifest.json").write_text(json.dumps(post,indent=2),encoding="utf-8")  # add by wushanzhou in 2026/10/24
    print(json.dumps(dict(search_rows=len(search),synthetic_rows=len(synthetic),real_mask_rows=len(real_masks),nested_rows=len(nested),  # add by wushanzhou in 2026/10/24
                          search_significant=sum(r["significant_005"] for r in comparisons))),flush=True)  # add by wushanzhou in 2026/10/24


def figures(output,search,search_summary,nested,synthetic_summary):  # add by wushanzhou in 2026/10/24
    target=output/"figures";target.mkdir(exist_ok=True)  # add by wushanzhou in 2026/10/24
    plt.rcParams.update({"font.size":10,"axes.spines.top":False,"axes.spines.right":False,"svg.hashsalt":"adaptive_hybrid_v4"})  # add by wushanzhou in 2026/10/24
    names=[]  # add by wushanzhou in 2026/10/24
    def save(fig,name):  # add by wushanzhou in 2026/10/24
        fig.savefig(target/(name+".png"),dpi=180,bbox_inches="tight")  # add by wushanzhou in 2026/10/24
        fig.savefig(target/(name+".svg"),bbox_inches="tight")  # add by wushanzhou in 2026/10/24
        plt.close(fig);names.append(name)  # add by wushanzhou in 2026/10/24
    budgets=sorted({int(r["budget_cap"]) for r in search})  # add by wushanzhou in 2026/10/24
    fig,axes=plt.subplots(2,len(budgets),figsize=(6*len(budgets),8),squeeze=False)  # add by wushanzhou in 2026/10/24
    for di,domain in enumerate(("uniform","physical")):  # add by wushanzhou in 2026/10/24
        for bi,budget in enumerate(budgets):  # add by wushanzhou in 2026/10/24
            ax=axes[di,bi]  # add by wushanzhou in 2026/10/24
            for method in ("SAOnly","Hybrid","SA_Budget","Adaptive"):  # add by wushanzhou in 2026/10/24
                rows=sorted([r for r in search_summary if r["domain"]==domain and r["budget"]==budget and r["method"]==method],key=lambda r:r["rate"])  # add by wushanzhou in 2026/10/24
                ax.errorbar([r["rate"] for r in rows],[r["mu"] for r in rows],yerr=[r["sd"] for r in rows],marker="o",capsize=3,label=method)  # add by wushanzhou in 2026/10/24
            ax.set(title=f"{domain}, cap={budget} (mean +/- SD)",xlabel="sampling level",ylabel="Fourier coherence")  # add by wushanzhou in 2026/10/24
            ax.set_xticks([0,1,2],["low","medium","high"]);ax.legend(fontsize=8)  # add by wushanzhou in 2026/10/24
    fig.tight_layout();save(fig,"01_search_comparison")  # add by wushanzhou in 2026/10/24
    fig,axes=plt.subplots(1,2,figsize=(12,4))  # add by wushanzhou in 2026/10/24
    for ax,domain in zip(axes,("uniform","physical")):  # add by wushanzhou in 2026/10/24
        for method in ("SAOnly","Hybrid","Adaptive","NoRecycle"):  # add by wushanzhou in 2026/10/24
            rows=[r for r in search if r["domain"]==domain and r["budget_cap"]=="200" and r["rate_index"]=="1" and r["method"]==method]  # add by wushanzhou in 2026/10/24
            traces=[]  # add by wushanzhou in 2026/10/24
            for row in rows:  # add by wushanzhou in 2026/10/24
                trace=read_rows(output/row["table_dir"]/(row["mask_path"]+".trace.csv"))  # add by wushanzhou in 2026/10/24
                values=np.array([float(r["best_objective"]) for r in trace]);traces.append(np.pad(values,(0,200-len(values)),mode="edge"))  # add by wushanzhou in 2026/10/24
            ax.plot(np.arange(1,201),np.mean(traces,axis=0),label=method)  # add by wushanzhou in 2026/10/24
        ax.set(title=f"{domain}: medium sampling, all seed means",xlabel="objective evaluations",ylabel="best coherence")  # add by wushanzhou in 2026/10/24
        ax.legend(fontsize=8)  # add by wushanzhou in 2026/10/24
    fig.tight_layout();save(fig,"02_convergence")  # add by wushanzhou in 2026/10/24
    fig,axes=plt.subplots(1,3,figsize=(13,4),sharey=True)  # add by wushanzhou in 2026/10/24
    for ax,keep in zip(axes,(20,30,40)):  # add by wushanzhou in 2026/10/24
        rows=[r for r in nested if int(r["keep"])==keep]  # add by wushanzhou in 2026/10/24
        for row in rows:  # add by wushanzhou in 2026/10/24
            ax.plot([0,1],[float(row["snr_fixed"]),float(row["snr_selected"])],"o-",alpha=.55,lw=1)  # add by wushanzhou in 2026/10/24
        ax.axhline(0,color="black",ls="--",lw=.8)  # add by wushanzhou in 2026/10/24
        ax.set(title=f"keep {keep}/59",ylabel="test SNR (dB)")  # add by wushanzhou in 2026/10/24
        ax.set_xticks([0,1],["Fixed POCS","Inner selected"])  # add by wushanzhou in 2026/10/24
    fig.suptitle("Same outer test masks; one real source; all splits shown")  # add by wushanzhou in 2026/10/24
    fig.tight_layout();save(fig,"03_nested_real")  # add by wushanzhou in 2026/10/24
    selected_methods=("Regular","Jitter","SAOnly","Local","Hybrid","Adaptive","W_03_03")  # add by wushanzhou in 2026/10/24
    fig,ax=plt.subplots(figsize=(9,4))  # add by wushanzhou in 2026/10/24
    values=[]  # add by wushanzhou in 2026/10/24
    for method in selected_methods:  # add by wushanzhou in 2026/10/24
        values.append([np.mean([r["snr"] for r in synthetic_summary if r["method"]==method and r["rate"]==rate]) for rate in range(3)])  # add by wushanzhou in 2026/10/24
    image=ax.imshow(values,aspect="auto",cmap="viridis")  # add by wushanzhou in 2026/10/24
    ax.set_yticks(range(len(selected_methods)),selected_methods)  # add by wushanzhou in 2026/10/24
    ax.set_xticks([0,1,2],["low","medium","high"])  # add by wushanzhou in 2026/10/24
    for i,row in enumerate(values):  # add by wushanzhou in 2026/10/24
        for j,value in enumerate(row):ax.text(j,i,f"{value:.2f}",ha="center",va="center",color="white" if value<np.mean(values) else "black")  # add by wushanzhou in 2026/10/24
    ax.set_title("Synthetic missing-trace SNR: mean across archived realizations")  # add by wushanzhou in 2026/10/24
    fig.colorbar(image,ax=ax,label="dB");fig.tight_layout();save(fig,"04_synthetic_snr")  # add by wushanzhou in 2026/10/24
    raw=read_rows(output/"synthetic_reconstruction.csv")  # add by wushanzhou in 2026/10/24
    weight_count=json.loads((output/"run_manifest.json").read_text())["fingerprint"]["configuration"]["weight_reconstruction_seeds"]  # add by wushanzhou in 2026/10/24
    common_seeds=set(sorted({int(r["seed"]) for r in raw})[:weight_count])  # add by wushanzhou in 2026/10/24
    grids=[]  # add by wushanzhou in 2026/10/24
    for rate in range(3):  # add by wushanzhou in 2026/10/24
        by_weight=groups([r for r in raw if int(r["rate_index"])==rate and int(r["seed"]) in common_seeds and  # add by wushanzhou in 2026/10/24
                          (r["method"].startswith("W_") or r["method"]=="Adaptive")],["lambda_gap","lambda_nn"])  # add by wushanzhou in 2026/10/24
        grid={tuple(float(v) for v in key):rows for key,rows in by_weight.items()}  # add by wushanzhou in 2026/10/24
        grids.append(grid)  # add by wushanzhou in 2026/10/24
    color_values=[average(rows,"snr_missing") for grid in grids for rows in grid.values()]  # add by wushanzhou in 2026/10/24
    low,high=min(color_values),max(color_values)  # add by wushanzhou in 2026/10/24
    fig,axes=plt.subplots(1,3,figsize=(14,4.7),layout="constrained")  # add by wushanzhou in 2026/10/24
    weights=[0,.1,.3]  # add by wushanzhou in 2026/10/24
    for rate,(ax,grid) in enumerate(zip(axes,grids)):  # add by wushanzhou in 2026/10/24
        values=np.array([[average(grid[(g,n)],"snr_missing") for n in weights] for g in weights])  # add by wushanzhou in 2026/10/24
        image=ax.imshow(values,cmap="viridis",vmin=low,vmax=high)  # add by wushanzhou in 2026/10/24
        ax.set_xticks(range(3),["0","0.1","0.3"]);ax.set_yticks(range(3),["0","0.1","0.3"])  # add by wushanzhou in 2026/10/24
        ax.set(title=f"Uniform sampling: {[38,64,90][rate]}/128",xlabel="nearest-neighbor weight",ylabel="maximum-gap weight")  # add by wushanzhou in 2026/10/24
        for i,g in enumerate(weights):  # add by wushanzhou in 2026/10/24
            for j,n in enumerate(weights):  # add by wushanzhou in 2026/10/24
                rows=grid[(g,n)];v=values[i,j]  # add by wushanzhou in 2026/10/24
                label=f"SNR {v:.2f} dB\nmu {average(rows,'mu'):.3f}\ngap {average(rows,'max_gap'):.1f}, CV {average(rows,'nn_cv'):.2f}"  # add by wushanzhou in 2026/10/24
                ax.text(j,i,label,ha="center",va="center",fontsize=8,color="white" if v<(low+high)/2 else "black")  # add by wushanzhou in 2026/10/24
    fig.colorbar(image,ax=axes.ravel().tolist(),label="Mean missing-trace SNR (dB)",shrink=.82)  # add by wushanzhou in 2026/10/24
    fig.suptitle("Independent weight tradeoffs; shared mask seeds; fixed reconstruction; exploratory")  # add by wushanzhou in 2026/10/24
    save(fig,"05_weight_tradeoff")  # add by wushanzhou in 2026/10/24
    (target/"index.html").write_text("<!doctype html><meta charset='utf-8'><title>adaptive_hybrid_v4 figures</title>"  # add by wushanzhou in 2026/10/24
                                   "<style>body{font:16px sans-serif;max-width:1200px;margin:auto;padding:24px}img{width:100%}</style>"  # add by wushanzhou in 2026/10/24
                                   "<h1>adaptive_hybrid_v4</h1><p>All plotted numbers are generated from archived results.</p>"+  # add by wushanzhou in 2026/10/24
                                   "".join(f"<h2>{p.stem}</h2><img src='{p.name}'>" for p in sorted(target.glob('*.png'))),encoding="utf-8") # add by wushanzhou in 2026/10/24


if __name__=="__main__":  # add by wushanzhou in 2026/10/24
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument("output",type=Path)  # add by wushanzhou in 2026/10/24
    summarize(parser.parse_args().output)  # add by wushanzhou in 2026/10/24
