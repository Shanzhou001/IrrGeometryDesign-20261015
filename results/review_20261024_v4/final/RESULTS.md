# adaptive_hybrid_v4: measured results

One independent real waveform source. No historical results or original data were overwritten.

## Search means (smaller mu is better)

|domain|budget|rate|method|n|mu|sd|actual_evals|native_clock_ms|
|---|---|---|---|---|---|---|---|---|
|physical|200|0|Adaptive|20|0.328112|0.0107248|200|8.2|
|physical|200|0|Hybrid|20|0.339886|0.0154042|120.2|9.4|
|physical|200|0|NoRecycle|20|0.354404|0.0195926|61.1|2.95|
|physical|200|0|SAOnly|20|0.329312|0.0116708|200|16.45|
|physical|200|0|SA_Budget|20|0.326001|0.0157221|200|8.35|
|physical|200|1|Adaptive|20|0.264181|0.00846401|200|9.05|
|physical|200|1|Hybrid|20|0.27799|0.0190774|142.95|14.2|
|physical|200|1|NoRecycle|20|0.299386|0.0204873|71.55|3.55|
|physical|200|1|SAOnly|20|0.264408|0.0116031|200|21.3|
|physical|200|1|SA_Budget|20|0.269132|0.0106779|200|9.05|
|physical|200|2|Adaptive|20|0.267625|0.0104633|200|9.8|
|physical|200|2|Hybrid|20|0.284716|0.0147822|139.65|15.85|
|physical|200|2|NoRecycle|20|0.308253|0.0326177|91.2|4.6|
|physical|200|2|SAOnly|20|0.267664|0.00729426|200|24.05|
|physical|200|2|SA_Budget|20|0.269664|0.01102|200|9.4|
|physical|600|0|Adaptive|20|0.3148|0.00800341|600|24.9|
|physical|600|0|Hybrid|20|0.339886|0.0154042|120.2|9.4|
|physical|600|0|NoRecycle|20|0.35175|0.01651|62.1|2.95|
|physical|600|0|SAOnly|20|0.316342|0.0101899|600|47.9|
|physical|600|0|SA_Budget|20|0.31382|0.00992307|600|24.5|
|physical|600|1|Adaptive|20|0.249775|0.00707506|600|27.2|
|physical|600|1|Hybrid|20|0.277902|0.0189372|145.25|14.7|
|physical|600|1|NoRecycle|20|0.303649|0.0292582|74.6|3.7|
|physical|600|1|SAOnly|20|0.24875|0.00645521|600|62.95|
|physical|600|1|SA_Budget|20|0.252163|0.00770844|600|26.7|
|physical|600|2|Adaptive|20|0.25905|0.00357626|600|28.45|
|physical|600|2|Hybrid|20|0.284716|0.0147822|139.65|16.45|
|physical|600|2|NoRecycle|20|0.310706|0.0299698|79.9|4.05|
|physical|600|2|SAOnly|20|0.257792|0.00272113|600|71.65|
|physical|600|2|SA_Budget|20|0.259303|0.00455899|600|28.65|
|uniform|200|0|Adaptive|20|0.210884|0.005614|200|8.7|
|uniform|200|0|Hybrid|20|0.218608|0.00812898|177.65|6.95|
|uniform|200|0|NoRecycle|20|0.231026|0.0134083|57.3|2.45|
|uniform|200|0|SAOnly|20|0.214528|0.00647506|200|9.35|
|uniform|200|0|SA_Budget|20|0.211348|0.00492515|200|8.65|
|uniform|200|1|Adaptive|20|0.137556|0.00369268|200|10.1|
|uniform|200|1|Hybrid|20|0.139139|0.00394745|193.4|8.4|
|uniform|200|1|NoRecycle|20|0.151494|0.0074471|65.1|3.4|
|uniform|200|1|SAOnly|20|0.136901|0.00382051|200|10.65|
|uniform|200|1|SA_Budget|20|0.13777|0.00392541|200|10.65|
|uniform|200|2|Adaptive|20|0.090337|0.00256997|200|10.5|
|uniform|200|2|Hybrid|20|0.0929325|0.00398249|173.65|8.05|
|uniform|200|2|NoRecycle|20|0.0966255|0.00564722|68|4.05|
|uniform|200|2|SAOnly|20|0.0895319|0.00308429|200|10.85|
|uniform|200|2|SA_Budget|20|0.0904291|0.00372276|200|10.7|
|uniform|600|0|Adaptive|20|0.203456|0.00531699|600|26|
|uniform|600|0|Hybrid|20|0.21851|0.00816454|195.65|7.25|
|uniform|600|0|NoRecycle|20|0.23185|0.0141968|59.15|2.75|
|uniform|600|0|SAOnly|20|0.203757|0.00409253|600|26.25|
|uniform|600|0|SA_Budget|20|0.205808|0.00336175|600|26.95|
|uniform|600|1|Adaptive|20|0.133257|0.00431723|600|30.15|
|uniform|600|1|Hybrid|20|0.138674|0.00409885|236.6|10.4|
|uniform|600|1|NoRecycle|20|0.151502|0.00879943|62.85|3.1|
|uniform|600|1|SAOnly|20|0.130277|0.00329976|600|30.1|
|uniform|600|1|SA_Budget|20|0.131564|0.00356858|600|30.15|
|uniform|600|2|Adaptive|20|0.0867909|0.00231042|600|31.85|
|uniform|600|2|Hybrid|20|0.0929325|0.00398249|191.7|9.05|
|uniform|600|2|NoRecycle|20|0.0977769|0.0046564|66.1|3.5|
|uniform|600|2|SAOnly|20|0.0868215|0.00230004|600|31.85|
|uniform|600|2|SA_Budget|20|0.0861383|0.0021943|600|32.3|

All search comparisons form one prespecified Holm family. Methods with spatial penalties optimize a different objective and are excluded.
Legacy SAOnly/Hybrid use the preserved legacy cooling schedule and backend; SA_Budget isolates the new cooling schedule and cached backend. Reconstruction steps run concurrently; their elapsed times are not isolated speed comparisons. Early-stopped traces retain their final incumbent for plotting only.

## Nested real reconstruction (dB; hidden test traces only)

|keep|n|linear|fixed|fixed_gain_only|selected_before_gain|selected|mean_gain|positive_selected|positive_gain|
|---|---|---|---|---|---|---|---|---|---|
|20|10|-1.89174|-1.21822|0.00939323|0.0365681|0.215146|1.43337|9|10|
|30|10|-1.70983|-0.923261|0.0763039|0.220953|0.353842|1.2771|10|10|
|40|10|-1.72205|-0.678155|0.193366|0.441296|0.627491|1.30565|10|10|

Parameters and prediction gain are selected using only inner validation traces drawn from outer-known traces. Zero/linear candidates are retained; selection of zero is a limitation, not curvelet recovery success. Fixed-gain-only applies the legacy candidate's validation coefficient. Selected-before-gain is an output decomposition, not a separately tuned parameter-only method.

## Synthetic model-level comparisons

|rate|A|B|model_realizations|mean_difference|p_holm|
|---|---|---|---|---|---|
|0|Adaptive|Hybrid|6|-0.356012|0.46875|
|0|Adaptive|SAOnly|6|-0.698777|0.46875|
|0|Adaptive|Jitter|6|-2.97605|0.46875|
|0|Adaptive|Regular|6|-3.45652|0.46875|
|0|Adaptive|W_03_03|6|-3.10542|0.46875|
|1|Adaptive|Hybrid|6|-1.12075|0.46875|
|1|Adaptive|SAOnly|6|-0.833627|0.46875|
|1|Adaptive|Jitter|6|-1.57511|0.46875|
|1|Adaptive|Regular|6|-0.637582|1|
|1|Adaptive|W_03_03|6|-1.28713|0.46875|
|2|Adaptive|Hybrid|6|0.285086|0.46875|
|2|Adaptive|SAOnly|6|0.358732|0.46875|
|2|Adaptive|Jitter|6|0.102692|1|
|2|Adaptive|Regular|6|-0.522294|0.46875|
|2|Adaptive|W_03_03|6|0.297414|0.46875|

Masks on the same realization are not pooled as independent geological models. Model-level tests remain exploratory on six parameter/noise realizations from three families.

## Interpretation limits

- Lower Fourier coherence, better spatial coverage and higher curvelet SNR are distinct outcomes.
- Association regressions are descriptive in-sample fits; no causal or validated prediction claim is made.
- Real split statistics describe one fixed source and do not establish cross-survey generalization.
- Regular is deterministic: repeated seeds do not create independent masks.
- Candidate validation and all negative outcomes are retained.

<!-- add by wushanzhou in 2026/10/24 -->
