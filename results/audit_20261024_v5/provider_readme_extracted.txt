Data: RRAW.SGY (same data ﬁle used in the training session for tesƟng the FK ﬁlter) 
Coordinates: Common Shot gather recorded with a 59 channels oﬀ-end spread. The group interval is about 
12.3m and the ﬁrst group is 33m far from the source while the last group is at 747m from the source. 
ObjecƟve: to process these data by applying the following processing sequence 
 
 Data import 
 Resampling at 1 ms 
 Passband ﬁlter to remove the low frequency components (lower than 10Hz, mostly due to Ground 
Roll) 
 FK ﬁlter to remove the residual Ground Roll and to aƩenuate ﬁrst arrivals (paying aƩenƟon to 
preserve the near-surface reﬂecƟons at long oﬀsets) 
 MuƟng of the remaining energy associated to noise and linear events arriving earlier than the ﬁrst 
reﬂecƟons 
 Velocity analysis. AcƟvaƟng the CMP opƟon but sorƟng the data with shot as sorƟng opƟon and 
oﬀset as second axis. Then, following the suggesƟons of the training session. For the construcƟon of 
the Velocity Spectrum try to opƟmize the Ɵme window (by default 1ms) exploring the eﬀect of 
increasing this parameter to values comparable to the wavelet duraƟon. Conclude the analysis by 
generaƟng and saving a 2D velocity model (although consisƟng of ﬂat layers) as suggested in the 
training session. 
 NMO correcƟons and Stack, saving the single trace stack. 
 DeconvoluƟon with the Spectral Whitening funcƟon to improve the resoluƟon (compare either the 
stack trace or its amplitude spectrum before and aŌer deconvoluƟon) 
 
Prepare the PowerPoint report to illustrate the intermediate results and parameter windows of any 
processing step. 
 
 
 
 
 
 
 
 
 