### 5.6. Frozen transfer to an additional F3 survey

The additional source is the Netherlands F3 poststack benchmark [@f3], distinct from the Viking prestack line. Before accessing waveform values, we froze the source member, section indices 0, 2 and 4, first 120 traces and all 255 samples, three common H sets, counts 36/60/84, two seeds, budget 300, paid 12-trace pilot, two inner folds and the existing eight recovery candidates. These three neighboring sections are dependent conditions from one additional survey. No candidate, threshold or selection rule was retuned using F3 outer scores. The source benchmark's test-once principle is retained for method development: future F3-based changes would invalidate this batch as untouched confirmation.

The subset was extracted using approximately 6.36 MB of exact HTTP byte ranges; the three registered sections and source documents total about 0.70 MB. Original physical coordinates and time origin are not inferred from the NPY arrays. Results use trace/sample indices; the pilot band is 0.02-0.24 cycles/sample, matching the normalized Viking band. The nominal 25-unit search spacing only preserves the existing dimensionless uniform-grid calculation. Full-archive MD5 and complete-member CRC were not locally verified; registered local bytes and fetched ranges have SHA-256 records.

**Table 7. F3 external transfer: mean common-H SNR by layout and observed-only recovery selection. One additional poststack survey, two paired seeds.**

{{external_methods}}

**Table 8. F3 aggregate fixed and internally selected recovery SNR. Absolute scores cannot be subtracted from Viking scores as an algorithm gain.**

{{external_recovery}}

The batch completes 270 recovery cases, 126 unique searches and 2496 successful native calls. Selected aggregate SNR is 9.7933, 12.5366 and 14.3428 dB, versus 9.6650, 12.5503 and 14.3912 dB for fixed linear interpolation. Jitter has the highest selected mean at all three counts. Internal selection chooses linear interpolation in 215/270 cases and warm curvelet in the remaining 55. Thus the frozen transfer does not confirm hybrid superiority; the strong interpolation baseline also exposes dependence on the poststack waveform/processing domain. Independent known-trace projection error is zero.

This is an additional-domain transfer check, with a different acquisition/processing domain from the prestack study. One new survey cannot establish universal cross-survey superiority. Section and mask repeats do not supply independent geological replication. Negative method contrasts and any advantage of fixed recovery are retained in the tables.

![Figure 7. F3 frozen external transfer, one additional poststack survey; all preset conditions retained.]({{figure:f3_recovery}})

### 5.7. Measured-waveform amplitude and phase diagnostics

All 4860 saved Viking robustness prediction arrays were checked against their frozen hashes. Diagnostics are evaluated on the same outer H and were not used for recovery selection. RMS ratio is predicted/target RMS, with ideal value 1; correlation is the centered waveform correlation. Phase MAE is the target-power-weighted absolute cross-spectrum phase in 0.02-0.24 cycles/sample after temporal demeaning and Hann tapering. Target-energy coverage of nonzero predicted bins is reported in the underlying tables; it is 1 for this Viking batch. A zero prediction receives no phase score, rather than a false zero error.

**Table 9. Viking measured-waveform diagnostics, averaged over all five layouts and the frozen robustness conditions.**

{{waveform_quality}}

Internal selection improves average correlation relative to fixed cold curvelet, but the selected RMS ratio is approximately 0.497, 0.662 and 0.764 at increasing counts. Phase errors remain approximately 40.06, 25.85 and 18.68 degrees. These results directly expose amplitude attenuation and residual phase error despite improved squared-error scores. They describe fidelity to measured noisy traces, without physical amplitude calibration, moveout or downstream imaging validation. Corresponding F3 diagnostics are saved separately with the external batch.
