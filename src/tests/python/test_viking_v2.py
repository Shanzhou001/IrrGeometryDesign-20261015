"""Independent solver, mandatory-pilot, weighted-PSF and multi-window contracts."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import numpy as np

from research.common.paths import executable
from research.common.tabular import rows
from research.data import viking
from research.experiments import viking_v2 as study
from research.recovery import fk_fista, select_and_refit
from research.validation.audit_numerics import query
from tests.python.test_viking import dataset_fixture


class FourierRecoveryTests(unittest.TestCase):
    def test_full_observation_solution_matches_analytic_soft_threshold(self):
        rng=np.random.default_rng(45);data=rng.normal(size=(32,16));known=np.ones(16,bool)
        actual,info=fk_fista(data,known,iterations=3,fraction=.05)
        scale=np.max(abs(data));coeff=np.fft.fft2(data/scale,norm='ortho');lam=.05*abs(coeff).max()
        expected=np.fft.ifft2(coeff*np.maximum(1-lam/np.maximum(abs(coeff),1e-30),0),norm='ortho').real*scale
        np.testing.assert_allclose(actual,expected,atol=1e-12)
        self.assertEqual(info['iterations'],3)

    def test_unobserved_amplitudes_and_scale_do_not_enter_the_solver(self):
        rng=np.random.default_rng(51);data=rng.normal(size=(32,16));known=np.arange(16)%2==0
        changed=data.copy();changed[:,~known]=13*changed[:,~known]+700
        a,ia=fk_fista(data,known,iterations=10);b,ib=fk_fista(changed,known,iterations=10)
        np.testing.assert_array_equal(a,b);self.assertEqual(ia,ib)
        normalized,info=fk_fista(data,known,iterations=5,fraction=1)
        self.assertLess(np.max(abs(normalized)),1e-12)

    def test_objective_improves_over_zero_on_missing_data(self):
        t=np.arange(32)[:,None];x=np.arange(16)[None,:]
        data=np.cos(2*np.pi*(t/16+x/8));known=np.arange(16)%3!=0
        _,info=fk_fista(data,known,iterations=30,fraction=.02)
        initial=.5*np.sum((data[:,known]/info['scale'])**2)
        self.assertLess(info['objective_trace'][-1],initial)


class PilotProtocolTests(unittest.TestCase):
    def plan(self):
        return study.make_plan(argparse.Namespace(shots=None,windows=None,counts=None,hidden_sets=None,budget=None,inner_folds=None,workers=2))

    def test_all_prespecified_masks_are_feasible_and_pilots_cost_is_included(self):
        plan=self.plan()
        self.assertEqual(len(plan['hidden_sets']),3);self.assertEqual(plan['window_starts'],[256,512,768])
        for hset in plan['hidden_sets']:
            hidden=np.zeros(120,bool);hidden[hset['indices']]=True
            pilot=study.pilot_mask(~hidden,12)
            for m in plan['counts']:
                initial,_=study.feasible_initial(~hidden,pilot,m,84011,plan['hard_gap_m'][str(m)])
                self.assertEqual(sum(initial),m);self.assertTrue(initial[pilot].all());self.assertFalse(np.any(initial&hidden))

    def test_spectrum_weights_ignore_nonpilot_truth_and_are_scale_invariant(self):
        rng=np.random.default_rng(71);data=rng.normal(size=(256,120));eligible=np.ones(120,bool);eligible[4::8]=False
        pilot=study.pilot_mask(eligible,12);changed=data.copy();changed[:,~pilot]=1e10
        a,_=study.signal_weights(data[:,pilot],np.flatnonzero(pilot),120)
        b,_=study.signal_weights(changed[:,pilot],np.flatnonzero(pilot),120)
        c,_=study.signal_weights(data[:,pilot]*17,np.flatnonzero(pilot),120)
        np.testing.assert_array_equal(a,b);np.testing.assert_allclose(a,c,atol=1e-14)
        np.testing.assert_allclose(a,a[::-1],atol=1e-14)
        self.assertTrue(np.all((a>=.25)&(a<=1)));self.assertAlmostEqual(a.max(),1.)

    def test_zero_pilot_falls_back_to_unweighted_target(self):
        weights,_=study.signal_weights(np.zeros((256,12)),np.arange(12)*10,120)
        np.testing.assert_array_equal(weights,np.ones(119))

    def test_extra_time_windows_use_raw_samples_and_correct_time_origin(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);truth,registry=dataset_fixture(root)
            with patch.object(viking,'REGISTRY',registry):
                windows,_=viking.load_windows([1,768],root,[53])
            np.testing.assert_array_equal(windows[1][53].data,truth[1:257])
            np.testing.assert_array_equal(windows[768][53].data,truth[768:1024])
            self.assertEqual(windows[1][53].time_origin_s,.004)

    def test_two_inner_folds_keep_pilot_observed_and_ignore_outer_hidden_values(self):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder);known=np.zeros(120,bool);known[:36]=True;pilot=np.zeros(120,bool);pilot[:12]=True
            data=np.sin(np.arange(64)[:,None]*.1+np.arange(120)[None,:]*.2);seen=[]
            class Recorder:
                out=root
            def fake_predict(recorder,key,values,x,train,config):
                self.assertTrue(np.all(train[pilot]));self.assertFalse(np.any(train&~known));seen.append(train.copy())
                return np.stack([np.interp(x,x[train],row[train]) for row in values])
            with patch('research.recovery.predict',side_effect=fake_predict):
                a,ca=select_and_refit(Recorder(),'a',data,np.arange(120),known,pilot,7,folds=2)
                changed=data.copy();changed[:,~known]=13*changed[:,~known]+7
                b,cb=select_and_refit(Recorder(),'b',changed,np.arange(120),known,pilot,7,folds=2)
            self.assertEqual(ca,cb)
            for name in a:np.testing.assert_array_equal(a[name],b[name])
            self.assertFalse(set(ca['splits'][0]['validation'])&set(ca['splits'][1]['validation']))


@unittest.skipUnless(executable().is_file(),'build the Release application first')
class NativePilotTests(unittest.TestCase):
    def invoke(self,lines,expected=0):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);path=root/'input.txt';path.write_text('\n'.join(lines)+'\n')
            command=[str(executable()),'--audit-queries',str(path),str(root/'out')]
            result=subprocess.run(command,capture_output=True,creationflags=getattr(subprocess,'CREATE_NO_WINDOW',0))
            self.assertEqual(result.returncode,expected,result.stdout+result.stderr)
            return rows(root/'out/queries.csv') if expected==0 else []

    def test_weighted_objective_matches_explicit_dictionary_with_unit_budget(self):
        x=np.array([0.,1,3,4,8,9,12]);weights=np.linspace(.25,1,6);lines=[];masks=[]
        for code in range(1,32):
            mask=np.array([(code>>j)&1 for j in range(7)])
            if mask.sum()<2:continue
            lines.append(query('c'+str(code),mask,x,'Adaptive',budget=1,frequency_weights=weights));masks.append(mask)
        result=self.invoke(lines)
        for row,mask in zip(result,masks):
            dictionary=np.exp(2j*np.pi*np.outer(x[mask.astype(bool)],np.arange(7))/14)/np.sqrt(mask.sum())
            gram=abs(dictionary.conj().T@dictionary);np.fill_diagonal(gram,0)
            expected=max(float(np.max(np.diag(gram,d)))*weights[d-1] for d in range(1,7))
            self.assertAlmostEqual(float(row['mu']),float(gram.max()),places=6)
            self.assertAlmostEqual(float(row['objective']),expected,places=6)

    def test_required_pilot_is_kept_through_all_search_phases_and_hard_gap(self):
        mask=np.arange(32)%2==0;required=np.zeros(32,int);required[[0,8,16,24]]=1
        eligible=np.ones(32,int);eligible[3::4]=0
        weights=np.linspace(.3,1,31)
        for method in ['SA_Budget','Adaptive']:
            row=self.invoke([query(method,mask,np.arange(32.),method,budget=200,minimum=8,window=3,
                                   hard_m=5,eligible=eligible,required=required,frequency_weights=weights)])[0]
            result=np.array(list(row['mask']),int)
            self.assertTrue(np.all(result[required.astype(bool)]==1));self.assertFalse(np.any(result&~eligible.astype(bool)))
            self.assertEqual(result.sum(),16);self.assertLessEqual(float(row['max_gap_m']),5)

    def test_all_selected_required_has_one_feasible_mask(self):
        mask=np.arange(8)%2==0
        row=self.invoke([query('fixed',mask,method='Adaptive',budget=40,required=mask)])[0]
        self.assertEqual(row['evals'],'1');self.assertEqual(row['status'],'single_feasible_mask')

    def test_invalid_weights_and_unobserved_required_points_fail(self):
        mask=np.arange(8)%2==0
        self.invoke([query('bad',mask,method='Adaptive',frequency_weights=[0]*7)],expected=1)
        self.invoke([query('bad',mask,method='Adaptive',frequency_weights=[1]*6)],expected=1)
        required=np.ones(8,int)
        self.invoke([query('bad',mask,method='Adaptive',required=required)],expected=1)


if __name__=='__main__':unittest.main()
