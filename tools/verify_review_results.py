"""Independent NumPy audit of v4 masks, objectives, predictions and nested selection."""  # add by wushanzhou in 2026/10/24
import argparse  # add by wushanzhou in 2026/10/24
import ast # add by wushanzhou in 2026/10/24
import csv  # add by wushanzhou in 2026/10/24
import hashlib  # add by wushanzhou in 2026/10/24
import json  # add by wushanzhou in 2026/10/24
from pathlib import Path  # add by wushanzhou in 2026/10/24

import numpy as np  # add by wushanzhou in 2026/10/24

ROOT = Path(__file__).resolve().parents[1]  # add by wushanzhou in 2026/10/24


def digest(path, algorithm="sha256"):  # add by wushanzhou in 2026/10/24
    h = hashlib.new(algorithm)  # add by wushanzhou in 2026/10/24
    with Path(path).open("rb") as stream:  # add by wushanzhou in 2026/10/24
        for block in iter(lambda: stream.read(1024*1024), b""):  # add by wushanzhou in 2026/10/24
            h.update(block)  # add by wushanzhou in 2026/10/24
    return h.hexdigest()  # add by wushanzhou in 2026/10/24


def table(path):  # add by wushanzhou in 2026/10/24
    with Path(path).open(encoding="utf-8-sig") as stream:  # add by wushanzhou in 2026/10/24
        return list(csv.DictReader(stream))  # add by wushanzhou in 2026/10/24


def gather(path):  # add by wushanzhou in 2026/10/24
    lines = Path(path).read_text(encoding="utf-8-sig").splitlines()  # add by wushanzhou in 2026/10/24
    coordinate_lines = [line for line in lines if line.startswith("# offsets_m=")]  # add by wushanzhou in 2026/10/24
    assert len(coordinate_lines) == 1, path  # add by wushanzhou in 2026/10/24
    x = np.array([float(value) for value in coordinate_lines[0].split("=",1)[1].split(",")])  # add by wushanzhou in 2026/10/24
    data = np.loadtxt(path, comments="#", delimiter=",")  # add by wushanzhou in 2026/10/24
    assert data.ndim == 2 and data.shape[1] == len(x) and np.isfinite(data).all() and np.all(np.diff(x)>0)  # add by wushanzhou in 2026/10/24
    return data, x  # add by wushanzhou in 2026/10/24


def bits(path):  # add by wushanzhou in 2026/10/24
    result = np.loadtxt(path, comments="#")  # add by wushanzhou in 2026/10/24
    assert result.ndim == 1 and np.isin(result, [0,1]).all(), path  # add by wushanzhou in 2026/10/24
    identity = hashlib.sha1("".join(str(int(v)) for v in result).encode("ascii")).hexdigest()  # add by wushanzhou in 2026/10/24
    return result.astype(bool), identity  # add by wushanzhou in 2026/10/24


def score(data, prediction, hidden):  # add by wushanzhou in 2026/10/24
    reference = data[:, hidden]  # add by wushanzhou in 2026/10/24
    error = np.square(prediction[:, hidden]-reference).sum()  # add by wushanzhou in 2026/10/24
    return float(10*np.log10(np.square(reference).sum()/max(error,1e-30)))  # add by wushanzhou in 2026/10/24


def interpolate(data, x, mask):  # add by wushanzhou in 2026/10/24
    return np.array([np.interp(x, x[mask], values[mask]) for values in data])  # add by wushanzhou in 2026/10/24


def verify(output, archives=False):  # add by wushanzhou in 2026/10/24
    output = Path(output).resolve()  # add by wushanzhou in 2026/10/24
    manifest = json.loads((output / "run_manifest.json").read_text())  # add by wushanzhou in 2026/10/24
    fingerprint = manifest["fingerprint"]  # add by wushanzhou in 2026/10/24
    assert fingerprint["protocol"] == "adaptive_hybrid_v4"  # add by wushanzhou in 2026/10/24
    assert fingerprint["executable_sha256"] == digest(ROOT/"x64/Release/IrrGeometryDesign.exe")  # add by wushanzhou in 2026/10/24
    audited_tool_changes=[] # add by wushanzhou in 2026/10/24
    for group in ("source_sha256","tool_sha256"):  # add by wushanzhou in 2026/10/24
        for name, expected in fingerprint[group].items():  # add by wushanzhou in 2026/10/24
            if digest(ROOT/name)==expected:continue # add by wushanzhou in 2026/10/24
            # One explicit post-run CLI wording correction; scientific readers must remain AST-identical. # add by wushanzhou in 2026/10/24
            assert group=='tool_sha256' and Path(name).as_posix()=='tools/convert_rraw_to_engine_csv.py',name # add by wushanzhou in 2026/10/24
            audit=json.loads((output.parent/'post_launch_tool_audit.json').read_text()) # add by wushanzhou in 2026/10/24
            snapshot=output.parent/audit['snapshot'] # add by wushanzhou in 2026/10/24
            assert audit['launch_sha256']==expected==digest(snapshot) # add by wushanzhou in 2026/10/24
            assert audit['current_sha256']==digest(ROOT/name) # add by wushanzhou in 2026/10/24
            def scientific_ast(path): # add by wushanzhou in 2026/10/24
                tree=ast.parse(Path(path).read_text(encoding='utf-8-sig')) # add by wushanzhou in 2026/10/24
                tree.body=[node for node in tree.body if not (isinstance(node,ast.FunctionDef) and node.name in ('check_encoding','main')) # add by wushanzhou in 2026/10/24
                           and not (isinstance(node,ast.Expr) and isinstance(node.value,ast.Constant) and isinstance(node.value.value,str))] # add by wushanzhou in 2026/10/24
                return ast.dump(tree,include_attributes=False) # add by wushanzhou in 2026/10/24
            assert scientific_ast(snapshot)==scientific_ast(ROOT/name),'scientific conversion behavior changed' # add by wushanzhou in 2026/10/24
            audited_tool_changes.append(name) # add by wushanzhou in 2026/10/24
    assert digest(ROOT/"data/real_seismic/real_gather_59x256.csv") == fingerprint["real_input_sha256"]  # add by wushanzhou in 2026/10/24
    output_files = 0  # add by wushanzhou in 2026/10/24
    for name, step in manifest["steps"].items():  # add by wushanzhou in 2026/10/24
        assert step["exit_code"] == 0, name  # add by wushanzhou in 2026/10/24
        for path, expected in step["inputs"].items():  # add by wushanzhou in 2026/10/24
            assert digest(path) == expected, path  # add by wushanzhou in 2026/10/24
        for path, expected in step["outputs"].items():  # add by wushanzhou in 2026/10/24
            assert digest(output/path) == expected, path  # add by wushanzhou in 2026/10/24
            output_files += 1  # add by wushanzhou in 2026/10/24
    _, real_x = gather(ROOT/"data/real_seismic/real_gather_59x256.csv")  # add by wushanzhou in 2026/10/24
    counts = dict(search_rows=0,synthetic_rows=0,real_mask_rows=0,nested_rows=0,archived_files=0)  # add by wushanzhou in 2026/10/24
    expected_search_rows = 0  # add by wushanzhou in 2026/10/24
    for path in sorted(output.glob("search_*/search.csv")):  # add by wushanzhou in 2026/10/24
        rows = table(path)  # add by wushanzhou in 2026/10/24
        geometry = real_x if rows[0]["domain"] == "physical" else np.arange(128,dtype=float)  # add by wushanzhou in 2026/10/24
        n = len(geometry)  # add by wushanzhou in 2026/10/24
        period = n*(geometry[-1]-geometry[0])/(n-1)  # add by wushanzhou in 2026/10/24
        atoms = np.exp(-2j*np.pi*np.outer(np.arange(1,n),geometry-geometry[0])/period)  # add by wushanzhou in 2026/10/24
        for index,row in enumerate(rows):  # add by wushanzhou in 2026/10/24
            mask,identity = bits(path.parent/row["mask_path"])  # add by wushanzhou in 2026/10/24
            m = int(mask.sum())  # add by wushanzhou in 2026/10/24
            assert len(mask)==n and m==int(row["m"]) and identity==row["mask_sha1"]  # add by wushanzhou in 2026/10/24
            mu = float(np.abs(atoms[:,mask].sum(axis=1)).max()/m)  # add by wushanzhou in 2026/10/24
            if index == 0:  # add by wushanzhou in 2026/10/24
                dictionary = np.exp(2j*np.pi*np.outer(geometry[mask],np.arange(n))/period)/np.sqrt(m)  # add by wushanzhou in 2026/10/24
                gram = dictionary.conj().T @ dictionary  # add by wushanzhou in 2026/10/24
                np.fill_diagonal(gram,0)  # add by wushanzhou in 2026/10/24
                assert abs(mu-np.abs(gram).max())<1e-10  # add by wushanzhou in 2026/10/24
            kept = geometry[mask]  # add by wushanzhou in 2026/10/24
            gaps = np.diff(kept)  # add by wushanzhou in 2026/10/24
            max_gap = max(kept[0]-geometry[0],geometry[-1]-kept[-1],float(gaps.max()))  # add by wushanzhou in 2026/10/24
            nearest = np.minimum(np.r_[gaps[0],gaps],np.r_[gaps,gaps[-1]])  # add by wushanzhou in 2026/10/24
            cv = float(nearest.std()/nearest.mean())  # add by wushanzhou in 2026/10/24
            goal = mu+float(row["lambda_gap"])*max(0,max_gap/(period/m)-1)+float(row["lambda_nn"])*cv  # add by wushanzhou in 2026/10/24
            assert abs(mu-float(row["mu"]))<2e-6, row  # add by wushanzhou in 2026/10/24
            assert abs(goal-float(row["objective"]))<3e-6, row  # add by wushanzhou in 2026/10/24
            assert abs(cv-float(row["nn_cv"]))<2e-6  # add by wushanzhou in 2026/10/24
            reported_gap = float(row["max_gap_m"] if row["domain"]=="physical" else row["max_gap"])  # add by wushanzhou in 2026/10/24
            assert abs(max_gap-reported_gap)<1e-6  # add by wushanzhou in 2026/10/24
            if float(row["hard_gap_m"]):  # add by wushanzhou in 2026/10/24
                assert max_gap<=float(row["hard_gap_m"])+1e-6  # add by wushanzhou in 2026/10/24
            if row["method"]=="HardGap" and row["domain"]=="uniform":  # add by wushanzhou in 2026/10/24
                assert max_gap<=np.ceil(2*n/m)  # add by wushanzhou in 2026/10/24
            trace = table(str(path.parent/row["mask_path"])+".trace.csv")  # add by wushanzhou in 2026/10/24
            incumbent = np.array([float(item["best_objective"]) for item in trace])  # add by wushanzhou in 2026/10/24
            assert len(trace)==int(row["evals"])<=int(row["budget_cap"])  # add by wushanzhou in 2026/10/24
            assert np.all(np.diff(incumbent)<=1e-7) and abs(incumbent[-1]-goal)<3e-6  # add by wushanzhou in 2026/10/24
            if row["method"] not in ("Regular","Random","Jitter","SAOnly","Local","Hybrid","Restart"):  # add by wushanzhou in 2026/10/24
                phases = [int(item["phase"]) for item in trace]  # add by wushanzhou in 2026/10/24
                assert phases[0]==0  # add by wushanzhou in 2026/10/24
                assert phases.count(1)==int(row["sa_evals"]) and phases.count(2)==int(row["local_evals"])  # add by wushanzhou in 2026/10/24
                assert phases.count(3)==int(row["restart_evals"])  # add by wushanzhou in 2026/10/24
                assert sum(int(row[c]) for c in ("sa_evals","local_evals","restart_evals"))==len(trace)-1  # add by wushanzhou in 2026/10/24
            counts["search_rows"]+=1  # add by wushanzhou in 2026/10/24
        methods = 21 if path.parent.name.endswith("_200") else 12  # add by wushanzhou in 2026/10/24
        expected_search_rows += 3*methods*fingerprint["configuration"]["search_seeds"]  # add by wushanzhou in 2026/10/24
    assert counts["search_rows"]==expected_search_rows  # add by wushanzhou in 2026/10/24
    real_data,_ = gather(ROOT/"data/real_seismic/real_gather_59x256.csv")  # add by wushanzhou in 2026/10/24
    reconstruction_steps={} # add by wushanzhou in 2026/10/24
    for step in manifest['steps'].values(): # add by wushanzhou in 2026/10/24
        command=step['command'] # add by wushanzhou in 2026/10/24
        if len(command)>2 and command[1]=='--review-reconstruct': # add by wushanzhou in 2026/10/24
            reconstruction_steps[Path(command[command.index('--out')+1]).resolve()]=command # add by wushanzhou in 2026/10/24
    for filename,kind in [("synthetic_reconstruction.csv","synthetic_rows"),("real_mask_reconstruction.csv","real_mask_rows")]:  # add by wushanzhou in 2026/10/24
        if not (output/filename).exists():  # add by wushanzhou in 2026/10/24
            continue  # add by wushanzhou in 2026/10/24
        for row in table(output/filename):  # add by wushanzhou in 2026/10/24
            data,x = gather(output/row["input"]) if kind=="synthetic_rows" else (real_data,real_x)  # add by wushanzhou in 2026/10/24
            if kind=="synthetic_rows":assert digest(output/row["input"])==row["input_sha256"]  # add by wushanzhou in 2026/10/24
            mask,identity = bits(output/row["mask"])  # add by wushanzhou in 2026/10/24
            assert identity==row["mask_sha1"]  # add by wushanzhou in 2026/10/24
            prediction = np.loadtxt(output/row["prediction"],delimiter=",")  # add by wushanzhou in 2026/10/24
            prediction_dir=(output/row['prediction']).parent.resolve();command=reconstruction_steps[prediction_dir] # add by wushanzhou in 2026/10/24
            input_path=output/row['input'] if kind=='synthetic_rows' else ROOT/'data/real_seismic/real_gather_59x256.csv' # add by wushanzhou in 2026/10/24
            assert Path(command[2]).resolve()==input_path.resolve() # add by wushanzhou in 2026/10/24
            native_mask,native_identity=bits(command[command.index('--mask')+1]);np.testing.assert_array_equal(native_mask,mask) # add by wushanzhou in 2026/10/24
            solver=table(prediction_dir/'solver.csv')[0] # add by wushanzhou in 2026/10/24
            assert solver['mask_sha1']==identity==native_identity and int(solver['iterations'])==40 and int(solver['scales'])==4 and int(solver['angles'])==16 and solver['warm_start']=='1' # add by wushanzhou in 2026/10/24
            assert abs(float(solver['threshold_start'])-.0265)<1e-12 and abs(float(solver['threshold_end'])-.0005)<1e-12 # add by wushanzhou in 2026/10/24
            observed_scale=float(np.abs(data[:,mask].astype(np.float32)).max());observed_scale=observed_scale if observed_scale>1e-20 else 1. # add by wushanzhou in 2026/10/24
            assert abs(float(solver['known_scale'])-observed_scale)<1e-8 # add by wushanzhou in 2026/10/24
            assert prediction.shape==data.shape and np.isfinite(prediction).all()  # add by wushanzhou in 2026/10/24
            assert np.max(np.abs(prediction[:,mask]-data[:,mask]))<1e-5  # add by wushanzhou in 2026/10/24
            assert abs(score(data,prediction,~mask)-float(row["snr_missing"]))<1e-7  # add by wushanzhou in 2026/10/24
            assert abs(score(data,interpolate(data,x,mask),~mask)-float(row["snr_linear"]))<1e-7  # add by wushanzhou in 2026/10/24
            counts[kind]+=1  # add by wushanzhou in 2026/10/24
    for filename in ("real_nested.csv","cdp_nested.csv"):  # add by wushanzhou in 2026/10/24
        if not (output/filename).exists():  # add by wushanzhou in 2026/10/24
            continue  # add by wushanzhou in 2026/10/24
        for row in table(output/filename):  # add by wushanzhou in 2026/10/24
            data,x = gather(ROOT/row["input"])  # add by wushanzhou in 2026/10/24
            assert digest(ROOT/row["input"])==row["input_sha256"]  # add by wushanzhou in 2026/10/24
            outer,identity = bits(output/row["outer_mask"])  # add by wushanzhou in 2026/10/24
            selection = json.loads((output/"selection"/(row["case"]+".json")).read_text())  # add by wushanzhou in 2026/10/24
            training = np.zeros(len(x),dtype=bool);training[selection["inner_train"]]=True  # add by wushanzhou in 2026/10/24
            validation = np.array(selection["validation"])  # add by wushanzhou in 2026/10/24
            assert not training[validation].any() and outer[validation].all() and not training[~outer].any()  # add by wushanzhou in 2026/10/24
            assert identity==row["outer_mask_sha1"]  # add by wushanzhou in 2026/10/24
            native_mask,_ = bits(output/"masks"/(row["case"]+"_inner.txt"))  # add by wushanzhou in 2026/10/24
            np.testing.assert_array_equal(training,native_mask)  # add by wushanzhou in 2026/10/24
            target = data[:,validation].ravel()  # add by wushanzhou in 2026/10/24
            checked_candidates = []  # add by wushanzhou in 2026/10/24
            for candidate in selection["candidates"]:  # add by wushanzhou in 2026/10/24
                name=candidate["candidate"]  # add by wushanzhou in 2026/10/24
                if name=="zero":values=np.zeros_like(target)  # add by wushanzhou in 2026/10/24
                elif name=="linear":values=interpolate(data,x,training)[:,validation].ravel()  # add by wushanzhou in 2026/10/24
                else:values=np.loadtxt(output/"reconstruction"/(row["case"]+"_inner_"+name)/"prediction.csv",delimiter=",")[:,validation].ravel()  # add by wushanzhou in 2026/10/24
                denominator=float(np.sum(values**2))  # add by wushanzhou in 2026/10/24
                alpha=min(1.,max(0.,float(np.sum(values*target))/denominator)) if denominator>1e-30 else 0.  # add by wushanzhou in 2026/10/24
                mse=float(np.mean((target-alpha*values)**2))  # add by wushanzhou in 2026/10/24
                assert abs(alpha-candidate["gain"])<1e-10 and abs(mse-candidate["validation_mse"])<1e-10  # add by wushanzhou in 2026/10/24
                checked_candidates.append((mse,name,alpha))  # add by wushanzhou in 2026/10/24
            selected_index=min(range(len(checked_candidates)),key=lambda i:(checked_candidates[i][0],i))  # add by wushanzhou in 2026/10/24
            _,name,alpha=checked_candidates[selected_index]  # add by wushanzhou in 2026/10/24
            assert name==row["selected"] and abs(alpha-float(row["gain"]))<1e-10  # add by wushanzhou in 2026/10/24
            predicted=np.loadtxt(output/row["selected_prediction"],delimiter=",")  # add by wushanzhou in 2026/10/24
            if name=="zero":expected=np.zeros_like(data)  # add by wushanzhou in 2026/10/24
            elif name=="linear":expected=interpolate(data,x,outer)*alpha  # add by wushanzhou in 2026/10/24
            else:expected=np.loadtxt(output/"reconstruction"/(row["case"]+"_outer_selected")/"prediction.csv",delimiter=",")*alpha  # add by wushanzhou in 2026/10/24
            expected[:,outer]=data[:,outer]  # add by wushanzhou in 2026/10/24
            np.testing.assert_allclose(predicted,expected,rtol=1e-9,atol=1e-10)  # add by wushanzhou in 2026/10/24
            fixed=np.loadtxt(output/row["fixed_prediction"],delimiter=",")  # add by wushanzhou in 2026/10/24
            assert abs(score(data,predicted,~outer)-float(row["snr_selected"]))<1e-7  # add by wushanzhou in 2026/10/24
            assert abs(score(data,fixed,~outer)-float(row["snr_fixed"]))<1e-7  # add by wushanzhou in 2026/10/24
            assert abs(score(data,interpolate(data,x,outer),~outer)-float(row["snr_linear"]))<1e-7  # add by wushanzhou in 2026/10/24
            counts["nested_rows"]+=1  # add by wushanzhou in 2026/10/24
    if (output/"leakage_check.json").exists():  # add by wushanzhou in 2026/10/24
        leakage=json.loads((output/"leakage_check.json").read_text())  # add by wushanzhou in 2026/10/24
        assert leakage["passed"] and leakage["selected_unchanged"] and leakage["max_prediction_difference"]<2e-6  # add by wushanzhou in 2026/10/24
    if manifest.get("completed"):  # add by wushanzhou in 2026/10/24
        config=fingerprint["configuration"]  # add by wushanzhou in 2026/10/24
        expected_synthetic=config["synthetic_realizations"]*3*(14*config["reconstruction_seeds"]+7*config["weight_reconstruction_seeds"])  # add by wushanzhou in 2026/10/24
        assert counts["synthetic_rows"]==expected_synthetic,counts  # add by wushanzhou in 2026/10/24
        assert counts["real_mask_rows"]==3*14*config["reconstruction_seeds"]  # add by wushanzhou in 2026/10/24
        assert counts["nested_rows"]==3*config["outer_real_seeds"]+5  # add by wushanzhou in 2026/10/24
    if archives:  # add by wushanzhou in 2026/10/24
        baseline=json.loads((ROOT/"results/review_20261024_v4/baseline_manifest.json").read_text())  # add by wushanzhou in 2026/10/24
        for name,expected in baseline["old_result_files"].items():  # add by wushanzhou in 2026/10/24
            assert digest(ROOT/name)==expected,name  # add by wushanzhou in 2026/10/24
            counts["archived_files"]+=1  # add by wushanzhou in 2026/10/24
        assert digest(ROOT/"data/real_seismic/Data/RRAW.SGY")==baseline["original_data_sha256"]  # add by wushanzhou in 2026/10/24
    result=dict(passed=True,counts=counts,native_steps=len(manifest["steps"]),hashed_output_files=output_files,post_launch_cli_only_tool_changes=audited_tool_changes, # add by wushanzhou in 2026/10/24
                independent_real_sources=1)  # add by wushanzhou in 2026/10/24
    (output/"independent_verification.json").write_text(json.dumps(result,indent=2),encoding="utf-8")  # add by wushanzhou in 2026/10/24
    print(json.dumps(result),flush=True)  # add by wushanzhou in 2026/10/24
    return result  # add by wushanzhou in 2026/10/24


if __name__=="__main__":  # add by wushanzhou in 2026/10/24
    parser=argparse.ArgumentParser(description=__doc__)  # add by wushanzhou in 2026/10/24
    parser.add_argument("output",type=Path)  # add by wushanzhou in 2026/10/24
    parser.add_argument("--check-archives",action="store_true")  # add by wushanzhou in 2026/10/24
    args=parser.parse_args()  # add by wushanzhou in 2026/10/24
    verify(args.output,args.check_archives)  # add by wushanzhou in 2026/10/24
