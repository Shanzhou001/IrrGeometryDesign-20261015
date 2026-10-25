# Single-source real-data diagnostic

One real source. CDP subsets are dependent descriptive checks; they do not add independent surveys or establish a cause of reconstruction error.

All checks use the fixed seed 20261023, 40 iterations, about two thirds kept, and the same 0-1.020 s window. No parameters were selected using these SNR values.

|Group|Traces|CDP counts|Mean adjacent zero-lag correlation|
|---|---:|---|---:|
|MixedCDP|59|{'237': 8, '238': 15, '239': 15, '240': 14, '241': 7}|0.3249|
|CDP_237|8|{'237': 8}|-0.0656|
|CDP_238|15|{'238': 15}|-0.0132|
|CDP_239|15|{'239': 15}|0.0422|
|CDP_240|14|{'240': 14}|-0.0055|
|CDP_241|7|{'241': 7}|-0.0616|

|Group|Keep|Linear SNR dB|POCS SNR dB|Gain vs linear dB|
|---|---:|---:|---:|---:|
|MixedCDP|39|-0.82|0.19|1.01|
|CDP_237|5|-2.95|-2.33|0.62|
|CDP_238|10|-3.40|-1.92|1.48|
|CDP_239|10|-2.08|-1.50|0.57|
|CDP_240|9|-2.61|-1.77|0.85|
|CDP_241|5|-3.40|-2.44|0.96|
