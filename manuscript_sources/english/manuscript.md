# Budget-Aware Seismic Trace Selection with Coverage Constraints and Observed-Only Recovery Selection

**Wu Shanzhou**

Research manuscript draft · 27 October 2026 · Not submitted

## Abstract

Seismic trace selection under a fixed retained count requires separate tests of geometric search efficiency and reconstruction quality. We implement a budget-aware hybrid of simulated annealing and discrete local exchanges, with elite retention, stagnation switching and recycling of unused evaluations. Physical-coordinate Fourier coherence and boundary-aware hard gaps define the geometric task; common hidden traces and observed-only recovery selection define the prediction test. A reproducible synthetic search improves a legacy early-terminating hybrid in four of six Holm-corrected contrasts, while stable superiority over SA-only is not established. Equal-count prefixes identify continued budget use as the clearest mechanism. Public Viking Graben development data are then evaluated using fixed curvelet recovery, an independently implemented f-k/FISTA baseline and eight-candidate internal selection. A supplementary grid of 1215 cases, three paired mask seeds and two disjoint inner-validation groups yields selected mean SNR of 2.353, 3.989 and 5.487 dB at 30%, 50% and 70% retention. Selection improves the average relative to fixed cold curvelet, but fixed f-k remains slightly better at the largest count. Identical-layout comparisons show that a second validation group helps at lower counts without a material high-density gain. Hard gaps reliably enforce coverage; pilot-spectrum weighting remains condition dependent. Independent numerical and information-isolation checks support the implementation, with additional measured-waveform diagnostics and a frozen transfer to one new F3 poststack survey. Two surveys and dependent section repeats still limit external inference. The contribution is a traceable framework for distinguishing budget utilization, geometric feasibility and recovery behavior, without a universal superiority or imaging-quality claim.

**Keywords:** seismic acquisition; fixed-cardinality selection; simulated annealing; Fourier coherence; coverage; curvelet; nested validation.

## 1. Introduction

Missing traces arise from deliberate acquisition reduction, access restrictions and incomplete recording. A sampling plan affects both observation cost and the information available to a recovery procedure. Jittered undersampling combines irregularity with gap control [@jitter]. Curvelet-frame recovery uses directional, multiscale structure [@curvelet], and the fast discrete curvelet transform provides practical wrapping-based transforms [@fdct]. These motivations do not identify a single geometric score that predicts reconstruction for every waveform and solver.

Sparse recovery theory is tied to a specified sensing operator and model [@stable]. In this project the inexpensive selection score is the coherence of a sampled Fourier dictionary, whereas the nonlinear recovery procedure uses redundant curvelets or a Fourier sparsity penalty. Lower Fourier coherence therefore cannot by itself establish a guarantee for the actual recovery algorithm. A coverage constraint can also improve spatial distribution while restricting the attainable coherence. The empirical question is how these choices interact under a controlled evaluation protocol.

Finite computation adds a second issue. Simulated annealing (SA) permits some worsening exchanges [@sa], while a greedy local phase may stop before using its allowed evaluations. Comparing only the permitted maximum can attribute an advantage to hybridization when the difference is continued computation. We record consumed evaluations and compare common trajectory prefixes. We also separate recovery configuration from acquisition design: a reconstruction is selected using acquired traces, with final scores computed only at a common external set that has been excluded from both search and fitting.

The contribution is an implemented and audited workflow: (i) exact-cardinality, budget-accounted hybrid search; (ii) explicit coordinate, coverage and pilot-information contracts; and (iii) a trace-selection-to-recovery evaluation with negative outcomes retained. The current paper replaces unconfirmed historical 59-trace examples as its primary field evidence with the publicly documented Viking source. Unavailable historical real-data tables are not reused as current numeric evidence. No state-of-the-art or cross-survey generalization claim is made.

## 2. Selection objective and feasible layouts

### 2.1. Fixed cardinality and Fourier geometry

For N sorted candidate coordinates x_j, a binary mask φ_j retains the set K. The basic constraint is ∑φ_j=m. A move exchanges one selected and one unselected candidate, preserving m exactly. When an external hidden set H is used, K is a subset of its complement. A required pilot set P must be included in K; its acquisition cost is included in m.

The coordinate period and normalized sampled dictionary are

```text
L = N (x_(N-1) - x_0) / (N-1)
D_(j,k) = exp[-2π i k (x_j-x_0)/L] / sqrt(m), j in K
μ(K) = max_(d=1,...,N-1) |Σ_(j in K) exp[-2π i d (x_j-x_0)/L]| / m
```

Each sampled dictionary column has norm one. Its off-diagonal Gram entries depend only on the difference of atom indices, giving the expression for μ. For equally spaced candidates, L=NΔx and the expression reduces to the normalized nonzero discrete Fourier coefficients of the mask, or the corresponding point-spread-function sidelobe. Translation and consistent coordinate-unit changes leave it invariant. For irregular candidates, a fully selected dictionary need not be orthogonal. Using the endpoint aperture directly as the period while keeping N atoms can produce duplicate uniform-grid columns; the period and frequency convention are part of the model.

### 2.2. Coverage and exact feasibility

For selected coordinates z_1<...<z_m, the largest gap includes the candidate-domain edges. The local spacing descriptor is the population coefficient of variation of each selected point's nearest selected-neighbor distance. They measure different features: uniform local distances alone do not exclude an unobserved edge interval.

```text
g_max = max(z_1-x_0, z_2-z_1, ..., z_m-z_(m-1), x_(N-1)-z_m)
J = μ + λ_g max(g_max/(L/m)-1, 0) + λ_n CV_NN
```

An optional hard limit imposes g_max≤G. An exact-count dynamic program stores whether a feasible path with k selected points can end at candidate j. A transition from i to j is allowed if their distance is at most G; edge conditions check the first and last selected points. Backtracking constructs m points. The necessary length condition (m+1)G≥x_(N-1)-x_0 is insufficient when the candidate set has holes. Eligibility and required-position conditions are separately validated. In Viking v2 all methods share an initial layout that already satisfies the relevant gap bound; unconstrained methods may subsequently leave it.

### 2.3. Pilot-spectrum weighting

The signal-assisted variant uses only 12 required pilot traces to estimate a spatial power profile after temporal demeaning, a Hann window and restriction to 5–60 Hz. Let p be that profile normalized by its maximum. Difference-frequency weights and the score are

```text
raw_w[d] = 0.25 + 0.75 max_q sqrt(p[q] p[q+d])
w[d] = raw_w[d] / max_r raw_w[r]
J_signal = max_d w[d] |DFT(φ)[d]| / m
```

This is a pilot-informed weighted PSF proxy, not the coherence of the curvelet synthesis-observation operator. The pilot traces remain in every inner training subset because they have informed search. The protocol evaluates two-stage acquisition with pilot observations; it does not provide signal-adaptive design without measured signal. Full-frequency μ is retained as a diagnostic so different objectives are not silently ranked as the same quantity.

## 3. Budget-aware hybrid search

### 3.1. Controller and accounting

The controller maintains a current state and a separate best-ever elite. An initial score consumes one evaluation. SA exchanges are accepted with probability min(1,exp(-ΔJ/T)); a worsening accepted state never replaces the better elite. After at least 32 SA candidate evaluations within a global phase, 12 consecutive evaluations without elite improvement trigger local search. Local proposals start from the elite and alternate nearest-empty moves in a random direction with full-range exchanges. Twelve local failures return the remaining budget to SA. The current state resets to the elite and an optional, explicitly evaluated two-exchange perturbation is requested.

```text
Input: initial feasible K, retained count m, budget B, seed, eligibility, P
Score K; current <- K; elite <- K; evaluations <- 1
while evaluations < B and proposal attempts < 100 B:
    propose a cardinality-preserving exchange for the current phase
    retain all required pilot positions; exclude ineligible positions
    reject invalid or gap-infeasible proposals before objective evaluation
    score candidate; count this complete score as one evaluation
    apply SA acceptance, or strict local improvement from elite
    update elite if improvement exceeds 1e-8; record all states and counts
    switch SA -> local after the minimum phase and stagnation window
    on local stagnation: terminate (NoRecycle), or return budget to SA
return elite, candidate/accepted/elite trajectories, phases and counts
```

At candidate evaluation e, T=0.1(10^-4)^((e-2)/max(1,B-2)). A perturbation uses a floor 0.01(1-progress). Repeated complete objective evaluations count again. Invalid proposals consume attempts but not objective evaluations. This distinction prevents both unlimited infeasible proposal loops and a claim that different proposal counts are equivalent computational work.

### 3.2. Comparators and complexity

The historical Hybrid spends a 64-evaluation prefix on SA and then uses an index-ordered local procedure that can terminate early. Its historical “CG” label denotes coordinate-greedy discrete movement and is not a standard conjugate-gradient method. SAOnly is the historical pure-SA baseline. SA_Budget disables local search while sharing the adaptive score backend and full-budget temperature schedule. NoRecycle stops at local stagnation. These distinctions are necessary to interpret which behavior changes.

Uniform-grid Fourier scoring costs O(N log N), with O(N) coverage scanning. The physical-coordinate or weighted-atom backend caches O(N²) phase factors and currently scans the full mask at each difference frequency, giving O(N²) worst-case scoring time and storage. It is incorrect to describe this implementation as only O(Nm). The elementary feasibility dynamic program has O(mN²) time and O(mN) storage bounds. Cache construction is included in recorded search time. Equal evaluation budgets do not establish equal wall-clock cost, especially when pilot feature extraction is present.

## 4. Recovery and evaluation protocol

### 4.1. Curvelet and f-k baselines

The curvelet procedure alternates forward FDCT, nonlinear coefficient shrinkage, inverse FDCT and exact restoration of acquired samples. Its default has four scales, 16 coarse angular divisions, a finest-scale wavelet and 40 iterations. Outside the coarsest scale the map is c|c|²/(|c|²+λ_s²), with a cosine threshold schedule from 0.0265 to 0.0005 in observed-data-normalized units. A stronger candidate uses 0.08 to 0.002; a longer cold-start candidate uses 80 iterations. This shrinkage is not an orthogonal projection onto a fixed convex set, so classical convex-projection convergence is not claimed.

An independently implemented Fourier baseline minimizes 0.5||P_K x-y_K||²+λ||F x||_1, with unitary two-dimensional FFT F. It uses the FISTA update [@fista], unit gradient step, complex soft thresholding and 80 iterations. The regularization parameter is 0.01 or 0.05 times the largest Fourier coefficient of zero-filled observed data after scaling using K alone. The latent objective trajectory is recorded; acquired values are restored for the final prediction. This restoration changes the penalized latent solution and is reported separately. Eighty steps are a fixed computation budget, not evidence of arbitrary-accuracy convergence.

### 4.2. Observed-only selection and common hidden traces

The eight Viking v2 candidates are zero prediction, coordinate-linear interpolation, weak and strong f-k, warm-start curvelet 40, cold-start curvelet 40, stronger-threshold cold-start curvelet 40 and cold-start curvelet 80. From K excluding the pilot P, approximately m/3 traces are hidden for internal validation. For each candidate, the gain α=clip_[0,1](<p_V,y_V>/<p_V,p_V>) and validation error are computed. Ties follow the frozen candidate order. The winning configuration is refit using all K, and the frozen gain applies only to unknown traces; observations are preserved.

External H is never used for scaling, search, recovery fitting or parameter selection. Native recovery input serializes unknown columns as zero. The score is SNR_H=10 log10(||y_H||²/||prediction_H-y_H||²), retaining negative values. Zero prediction gives 0 dB for a nonzero target. An SNR over the entire gather would reward exact fits on acquired samples and is not substituted. The reference is a measured noisy trace, not an independently known clean wavefield.

### 4.3. Data and statistical units

The current synthetic batch uses N=128, m=38/64/90, budgets 200/600 and 20 paired computational seeds. It contains 1980 search rows including sensitivity configurations. The joint batch has 1080 recovery records from six clean analytic structural templates, two noise settings, three mask seeds, five methods, three counts and two external evaluation designs. The fields contain dipping, curved or faulted Ricker events on 256×128 arrays with 4 ms samples and 12.5 m trace spacing; they are not wave-equation simulations. Noise and mask repeats are averaged before template-level comparisons. A 24-test family uses Holm correction [@holm]. Common-H and mask-complement tasks have different prediction targets and are not subtracted as an algorithmic gain.

Viking Line 12 is a public marine seismic source [@viking]. The local registration fixes 30 data files and two source documents; independent big-endian IBM decoding, raw-to-cropped waveform equality and per-file hashes are checked. Ten shots are available, each with 120 traces; the main arrays contain 256 native 4 ms samples. The original coordinate-units code conflicts with the distance interpretation. Relative geometry and documented 25 m receiver spacing support the distance scale, but this does not establish absolute physical amplitude calibration. Only the local subset is hash-verified; the full original large SEG-Y file has not been locally verified.

Viking v1 covers ten shots, three mask seeds and one periodic H. Its 450 recovery cases use the earlier candidate pool and warm-start default. Viking v2 was designed after observing v1 diagnostics and is explicitly a development study. It uses FFIDs 53/453/953, time windows [1.024,2.048), [2.048,3.072) and [3.072,4.096) s, three H sets of 15 traces, counts 36/60/84 and seed 84011. The 12 pilots are included in m, and gap limits are 175/100/75 m. Its balanced grid has 405 cases but only 117 distinct search queries because purely geometric masks are reused. All optimized methods use B=300, while Jitter uses one initial score. Shots, windows, masks and hidden splits from one line are not independent surveys; no cross-survey confidence interval or significance test is reported.

## 5. Results

### 5.1. Search improvement and budget attribution

Table 1 is regenerated from the current synthetic search tables. Adaptive improves the mean over legacy Hybrid in all six budget-density groups, with four contrasts passing the stated 24-test Holm family. Stable superiority over SAOnly or SA_Budget is not established. At B=600,m=64, SAOnly's lower mean than Adaptive is retained. Relative to NoRecycle, all six final-score contrasts improve, but their common-evaluation-prefix differences are exactly zero. Recycling therefore prevents loss of usable evaluations; it does not improve an otherwise identical prefix.

**Table 1. Mean Fourier coherence, N=128, 20 paired seeds per row; lower is better.**

{{table:search}}

![Figure 1. Current synthetic search comparisons. Error bars describe computational seed variation on one candidate geometry; they do not represent variation across geological surveys.]({{figure:search}})

![Figure 2. Final-score and common-count differences. Identical Adaptive–NoRecycle prefixes separate continued budget use from an improvement in shared search decisions.]({{figure:budget}})

### 5.2. Synthetic hidden-trace recovery

**Table 2. Common-H synthetic SNR in dB, after averaging noise settings and masks within six clean structural templates. Fixed and selected configurations are distinct.**

{{table:synthetic}}

At m=38 the fixed Adaptive mean is about 0.987 dB below Jitter; at m=90 it is about 0.447 dB above. Internal selection is also not uniformly beneficial: Adaptive's m=64 mean decreases. No member of the 24-comparison template-level family passes Holm correction; the minimum adjusted p is 0.75. Six nonzero pairs have a minimum exact two-sided sign probability of 2/2^6, so the first Holm threshold is unattainable for this family. This is a limitation of inference, not proof that every effect is zero.

![Figure 3. Common-H synthetic recovery with six structural analysis units. Marginal intervals and multiplicity-adjusted decisions answer different uncertainty questions.]({{figure:synthetic}})

### 5.3. Viking v1 diagnosis and v2 recovery selection

**Table 3. Viking v1 SNR in dB; first average three masks within each shot, then ten shots. These descriptive within-survey results use the older recovery pool and are not directly comparable as gains against v2.**

{{table:viking_v1}}

The first field batch exposes substantial solver sensitivity and some negative hidden-trace scores. A geometric improvement does not consistently transfer across densities. The subsequent development therefore adds cold starts, threshold and iteration controls, an independent f-k baseline and broader preset conditions.

**Table 4. Viking v2 SNR in dB, averaged equally over the five selection methods and the preset shot/window/H grid. “Cold curvelet” and “Weak f-k” are fixed candidates, not selected winners.**

{{table:viking_v2_recovery}}

Observed-only selection improves on fixed cold curvelet in 100/135, 126/135 and 131/135 cases across the three counts. At m=84 its aggregate 5.5648 dB remains slightly below fixed weak f-k's 5.5744 dB. Thus validation does not identify the external winner in every condition. Strong-threshold cold curvelet is selected 164 times, strong f-k 113, cold curvelet 80 iterations 53, warm curvelet 50, default cold curvelet 16, linear interpolation five and weak f-k four. Warm starts remain useful in some cases; early failure diagnosis cannot be generalized to permanent inferiority.

![Figure 4. Viking v2 fixed and internally selected recovery. All windows and hidden sets are retained; the displayed conditions are dependent observations from one survey.]({{figure:viking_v2}})

### 5.4. Coverage, pilot weighting and method ranking

**Table 5. Viking v2 selected-recovery SNR in dB by selection method. No method wins at all three counts.**

{{table:viking_v2_methods}}

**Table 6. Coverage tradeoffs. Gap columns are mean maximum intervals in metres; difference columns compare selected-recovery SNR. Positive differences favor the first named method.**

{{table:viking_v2_gap}}

All constrained results satisfy their frozen gap limits. At m=36 the constraint reduces mean maximum gap from 275 to 175 m but slightly reduces selected-recovery SNR. At m=84 it improves the selected mean by about 0.353 dB. The constraint ensures an engineering property, but its value as a prediction aid depends on density and recovery model.

Pilot weighting gives small fixed-f-k improvements in all three mean comparisons, yet its internally selected mean is slightly below Adaptive_Gap at every count. The realized weights span approximately 0.722–1, and a 12-trace pilot spectrum is aliased by its own sparse geometry. These are plausible contributors to a weak distinction, not individually established causal explanations. The objective is still a spectral proxy rather than direct recovery error.

Across methods and counts, window means are approximately 2.490, 3.369 and 5.912 dB; different signal content and target energy prevent interpreting their subtraction as a same-task algorithm gain. Three H means are approximately 3.861, 4.024 and 3.887 dB. These sensitivities reinforce the need to preserve preset conditions instead of selecting the best window for reporting.

![Figure 5. Viking v2 full-frequency geometry, hard gaps and weighted-objective behavior. Different objective definitions are labeled separately.]({{figure:viking_geometry}})

![Figure 6. Condition-level Viking v2 recovery differences. A window, shot or hidden split is a dependent development condition, not an independent field validation.]({{figure:viking_conditions}})

### 5.5. Supplementary within-survey robustness

{{robustness}}

{{external_and_quality}}

## 6. Numerical checks and two-dimensional integration

Independent explicit dictionaries and small-mask enumeration verify 588 coordinate/objective/coverage cases per native build, including 84 exact-feasibility cases. Two physical-coordinate consistency cases agree with dense references. Twenty FDCT size/parameter combinations per Release and Debug build have maximum round-trip relative error about 2.3×10^-15. These checks establish implementation agreement on tested inputs, not hidden-waveform accuracy.

The separate SpatialFourier2D_v1 extension scores actual SPS (x,y) receiver coordinates with a finite two-dimensional Fourier dictionary. At the default two nondegenerate axes there are 64 atoms and 112 conjugacy-reduced nonzero differences. Cached phases support incremental updates on swaps. The recovered SPS source contains 9072 receivers; processing yields 3,162,602 valid source-receiver relationships and 29,719 active CMP bins. One 50%-retention, 200-evaluation integration case gives Table 10.

**Table 10. Separate SPS integration example, one geometry and seed. IndexRegular and IndexJitter operate on sorted indices and are not spatially regular/jittered two-dimensional designs.**

{{table:sps}}

Five explicit NumPy Gram reconstructions agree within approximately 6.6×10^-16. This validates the finite 2D score and exported masks, not waveform reconstruction or multi-survey performance. The two-dimensional extension remains separate from the one-dimensional statistical experiments.

## 7. Discussion and limitations

The clearest supported search mechanism is continued use of an allowed budget after local stagnation. Current evidence does not require assigning an independent benefit to every switch, neighborhood or perturbation. SA-only remains a necessary comparator. A lower geometric objective can coexist with lower recovery SNR because a finite Fourier dictionary, a coverage requirement and a nonlinear recovery prior describe different properties.

Viking v2 provides a more interpretable recovery comparison than the historical unconfirmed source, but it remains one development line. All ten locally available v1 shots have already informed the project. Additional masks, inner folds or selected subsets cannot create an untouched external survey. Sampling-rate, window and H sensitivities are useful diagnostics, with no population-wide significance inference. The pilot variant also changes the information pattern: it presupposes paid pilot acquisition and cannot be described as signal-free survey planning.

The error metric targets measured noisy traces. Supplementary waveform diagnostics quantify relative amplitude attenuation and residual phase error; physical amplitude calibration, moveout consistency and downstream imaging quality remain unverified. Gain shrinkage may reduce squared error while attenuating signal amplitudes. The coordinate-units conflict, subset-only hash verification and unconfirmed absolute amplitude calibration remain explicit. No normal-moveout correction, denoising ground truth or geological interpretation is assumed without evidence.

The experiment grid was frozen before its own outer scoring after earlier results were inspected. It is a transparent development protocol rather than preregistered external confirmation. Further changes should be developed with permitted training information, then assessed on another frozen source or genuinely unused segment. The added F3 transfer is one such untuned check, with its poststack-domain limitation retained. The current algorithm and baseline set is limited; modern learned and signal-adaptive acquisition methods are not benchmarked. No universal or state-of-the-art claim follows.

## 8. Conclusion

A fixed-cardinality hybrid selector, boundary-aware coverage model and observed-only recovery workflow have been implemented and checked. The current synthetic search improves a legacy early-terminating hybrid in several corrected comparisons; stable superiority over SA-only remains unsupported. Common-count trajectories attribute the clearest benefit to preventing unused evaluations. Synthetic and Viking recovery results show that lower Fourier coherence does not guarantee higher hidden-trace SNR. On the studied Viking development grid, recovery selection improves the mean relative to fixed cold curvelet, hard gaps reliably enforce coverage and pilot-spectrum weighting remains condition dependent. The practical outcome is a reproducible framework for measuring these distinct effects with explicit data and information limits.

## Data, code and manuscript availability

Canonical editable manuscript sources and build/verification tools now reside in the project manuscript_sources directory. The neighboring manuscript folders are regenerated compatibility outputs. The local project contains native/Python source, frozen protocols, experiment manifests, table CSVs, masks, saved predictions and audit records for the current repair, Viking v1 and Viking v2 batches. This manuscript is built from editable Markdown; its tables are regenerated from the cited CSVs, and a build manifest records their SHA-256 identities. The PDF build uses ReportLab. A TeX export and bibliography are provided; XeLaTeX compilation has not been executed in this environment because a TeX engine is absent. The English and Chinese research drafts are not submitted manuscripts or school-approved final theses.

Viking is cited through its public source record; local subset identity and source documents are registered separately. Large input and runtime directories are not bundled automatically with a Git checkout. A local delivery package includes registered small inputs and the frozen Windows command-line runtime, with an isolated extraction replay; external publication is not asserted. Historical unconfirmed RRAW inputs and full original v4/v5 real-data archives remain outside the reproducible scope of this paper. The workspace retains the current manuscript version; experiment records cited by this version remain separate evidence batches.

## Acknowledgment

OpenAI Codex assisted with code, analysis workflows and preparation of the editable research drafts. The author remains responsible for scientific verification, research integrity, institutional formatting and any future submission. No review outcome, publication acceptance, patent authorization or degree milestone is inferred from project files.

## References

{{references}}
