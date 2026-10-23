# Corrected experiment results

Protocol: classic_psf_v2. One independent real waveform source.

## Synthetic missing-trace SNR

|Model|Rate|Mask|Runs|Unique masks|Mean dB|SD dB|
|---|---:|---|---:|---:|---:|---:|
|mask_recon_builtin|0.30|Constrained|10|10|7.7918|0.7169|
|mask_recon_builtin|0.30|JitterInit|10|10|8.3480|0.3144|
|mask_recon_builtin|0.30|Random|10|10|5.1481|1.4588|
|mask_recon_builtin|0.30|Regular|10|1|9.1288|0.0000|
|mask_recon_builtin|0.30|Restart|10|10|7.9504|0.4489|
|mask_recon_builtin|0.30|SA|10|10|5.1565|0.6296|
|mask_recon_builtin|0.50|Constrained|10|10|10.4476|0.4953|
|mask_recon_builtin|0.50|JitterInit|10|10|10.7311|0.1329|
|mask_recon_builtin|0.50|Random|10|10|9.1328|1.2870|
|mask_recon_builtin|0.50|Regular|10|1|10.6043|0.0000|
|mask_recon_builtin|0.50|Restart|10|10|10.7079|0.2656|
|mask_recon_builtin|0.50|SA|10|10|9.1345|1.0515|
|mask_recon_builtin|0.70|Constrained|10|10|10.4759|0.4423|
|mask_recon_builtin|0.70|JitterInit|10|10|10.7592|0.2836|
|mask_recon_builtin|0.70|Random|10|10|10.6451|0.6263|
|mask_recon_builtin|0.70|Regular|10|1|10.7279|0.0000|
|mask_recon_builtin|0.70|Restart|10|10|10.7757|0.1763|
|mask_recon_builtin|0.70|SA|10|10|10.1030|1.0254|
|mask_recon_dipping|0.30|Constrained|10|10|14.3410|0.5589|
|mask_recon_dipping|0.30|JitterInit|10|10|14.5511|0.2851|
|mask_recon_dipping|0.30|Random|10|10|12.0186|2.3991|
|mask_recon_dipping|0.30|Regular|10|1|14.5357|0.0000|
|mask_recon_dipping|0.30|Restart|10|10|14.5627|0.3088|
|mask_recon_dipping|0.30|SA|10|10|13.2009|1.4048|
|mask_recon_dipping|0.50|Constrained|10|10|14.7820|0.5267|
|mask_recon_dipping|0.50|JitterInit|10|10|14.7327|0.5180|
|mask_recon_dipping|0.50|Random|10|10|14.8413|0.8553|
|mask_recon_dipping|0.50|Regular|10|1|13.8709|0.0000|
|mask_recon_dipping|0.50|Restart|10|10|14.5948|0.7466|
|mask_recon_dipping|0.50|SA|10|10|14.3341|0.9932|
|mask_recon_dipping|0.70|Constrained|10|10|14.1311|1.0910|
|mask_recon_dipping|0.70|JitterInit|10|10|14.0781|1.0451|
|mask_recon_dipping|0.70|Random|10|10|15.5698|2.6617|
|mask_recon_dipping|0.70|Regular|10|1|15.8292|0.0000|
|mask_recon_dipping|0.70|Restart|10|10|13.9238|0.9028|
|mask_recon_dipping|0.70|SA|10|10|15.4277|1.7870|
|mask_recon_faulted|0.30|Constrained|10|10|6.7543|0.8718|
|mask_recon_faulted|0.30|JitterInit|10|10|7.3851|0.3448|
|mask_recon_faulted|0.30|Random|10|10|4.2424|1.0444|
|mask_recon_faulted|0.30|Regular|10|1|8.3478|0.0000|
|mask_recon_faulted|0.30|Restart|10|10|7.1010|0.6128|
|mask_recon_faulted|0.30|SA|10|10|4.2299|0.4871|
|mask_recon_faulted|0.50|Constrained|10|10|11.6957|0.7778|
|mask_recon_faulted|0.50|JitterInit|10|10|11.9300|0.2415|
|mask_recon_faulted|0.50|Random|10|10|9.0820|1.6246|
|mask_recon_faulted|0.50|Regular|10|1|11.4852|0.0000|
|mask_recon_faulted|0.50|Restart|10|10|11.7918|0.1456|
|mask_recon_faulted|0.50|SA|10|10|9.1049|1.2678|
|mask_recon_faulted|0.70|Constrained|10|10|11.7711|1.5001|
|mask_recon_faulted|0.70|JitterInit|10|10|12.7475|0.5356|
|mask_recon_faulted|0.70|Random|10|10|11.9742|1.6122|
|mask_recon_faulted|0.70|Regular|10|1|11.9146|0.0000|
|mask_recon_faulted|0.70|Restart|10|10|12.4771|0.6319|
|mask_recon_faulted|0.70|SA|10|10|11.5522|1.4722|

## Within-rate exploratory correlation, duplicate masks removed

|Model|Rate|Unique masks|Pearson r|Spearman rho|
|---|---:|---:|---:|---:|
|mask_recon_builtin|0.30|51|0.2622|0.3657|
|mask_recon_builtin|0.50|50|0.1489|0.3539|
|mask_recon_builtin|0.70|50|0.2596|0.3079|
|mask_recon_dipping|0.30|51|-0.0071|0.0198|
|mask_recon_dipping|0.50|50|-0.1343|0.0872|
|mask_recon_dipping|0.70|50|-0.1638|-0.2553|
|mask_recon_faulted|0.30|51|0.2734|0.3172|
|mask_recon_faulted|0.50|50|0.1395|0.3045|
|mask_recon_faulted|0.70|50|0.2268|0.2771|

## Measured-trace holdout with actual offsets

|Keep|Mask|n|POCS dB|Gain vs linear dB|Gain vs zero dB|
|---:|---|---:|---:|---:|---:|
|20|Constrained|10|-1.3469|0.5606|-1.3469|
|20|Hybrid|10|-1.4611|0.6363|-1.4611|
|20|JitterInit|10|-1.1547|0.5660|-1.1547|
|20|Random|10|-1.2280|0.5265|-1.2280|
|20|Regular|10|-1.2690|0.3109|-1.2690|
|20|Restart|10|-1.2043|0.6019|-1.2043|
|30|Constrained|10|-0.7703|0.7173|-0.7703|
|30|Hybrid|10|-0.7661|0.8623|-0.7661|
|30|JitterInit|10|-0.6435|0.7428|-0.6435|
|30|Random|10|-0.7781|0.8020|-0.7781|
|30|Regular|10|-0.7013|0.7526|-0.7013|
|30|Restart|10|-0.4682|0.8298|-0.4682|
|40|Constrained|10|-0.2930|1.1118|-0.2930|
|40|Hybrid|10|-0.6307|0.9998|-0.6307|
|40|JitterInit|10|-0.0499|1.0133|-0.0499|
|40|Random|10|-0.3421|0.9955|-0.3421|
|40|Regular|10|-0.0298|1.1307|-0.0298|
|40|Restart|10|-0.0665|1.0521|-0.0665|

## Interpretation limits

- Selection and reconstruction masks are linked by canonical SHA1.
- Methods share a 200-evaluation maximum; local search may stop early. Actual counts and times are reported.
- The constrained method optimizes a different, explicitly reported penalized objective.
- Real mask selection uses sorted trace indices. Reconstruction and the reported maximum gap use actual offsets.
- Regular masks are deterministic: repeated seeds are not independent observations for that baseline.
- The raw real file contains five CDP values; it is not a single clean CMP gather.
- Synthetic models and different random splits do not add independent real surveys.
