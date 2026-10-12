# Sound Effect Map: Assets/Source/Sound Effects

Scope: 163 unlabeled WAVs in `Assets/Source/Sound Effects` matched against the audio of `Assets/Source/videoplayback (1).mp4` (388.0 s) and `(2).mp4` (378.1 s). Both were decoded to mono 22050 Hz.

## Result summary
- **Resolved (corrected rate confirmed): 0 of 163.** No WAV reached the acceptance rule (NCC >= 0.35, robust z >= 15, >= 2 consistent hits).
- **Speed-factor hypothesis: not supported.** Scanning 70 speed factors per WAV (log grid 0.33-3.0 plus 18518/x ratios and inverses) produced no dominant factor. Best factors are spread across the grid (1.01 x12, 0.40 x8, 3.00 x7, 1.27 x6, ...). The 0.40 and 3.00 picks sit at grid edges, a bias of short templates, not evidence.
- **Only one candidate:** `gen_1E.wav` (header 8000 Hz, f=1.014) hit once in video 1 at 192.8 s (NCC 0.58, z 15.1), with no second hit and no hit in video 2 (z 5.4). Rejected under the >= 2-hit rule. Worth a frame check at 192.8 s if a manual lead is wanted.
- **Labels:** the 5 labels from the previous attempt (gen_02, gen_0A, gen_0E, gen_28, gen_2D) are carried over with their old confidences. Attempt 2 did not reproduce them (their z stays below 15), so treat them as unverified. No new labels were assigned, since no frame check was justified without a supported hit.

## Method (attempt 2)
- Features: 64-band mel spectrogram (150-10000 Hz, NFFT 1024, hop 256 = 11.6 ms), sqrt power, minus a 2 s running mean per band (BGM suppression).
- Template: WAV resampled from header_rate x f to 22050 Hz (duration preserving), trimmed to onset, first 0.6 s, centred per band.
- Score: FFT-based normalised cross-correlation over the whole video (energy floor = 5% of median window energy). Peaks = local maxima, 0.15 s spacing. Robust z = (peak - median) / (1.4826 MAD) over the full lag curve.
- Second run with per-band whitening: worse. Median best NCC rose to 0.56 and max z fell to 10. This run is not used.
- Probable failure reason: NCC over a short mixed-audio template is dominated by generic spectral-envelope overlap, with no clear peak above the lag-curve background. Expect real SFX matches to need a different detector (for example, onset-only features with a template-length-corrected null model).

## Format summary
- 163 files, all mono 16-bit PCM, data at byte offset 44. Header rates: 8000 (117), 16000 (20), 11025 (12), 22050 (3), and odd values 8001-8023 / 16007 (treated as 8000 or 16000 in the table).
- Corrected rate: **not determined** for any file (column left empty). Header rates remain in use. Durations below are at the header rate.

## Per-file table
| WAV | Header SR (Hz) | Dur at header (s) | Best f (not validated) | NCC v1 / v2 | z v1 / v2 | Hits (z>=15, NCC>=0.35) | Corrected SR | Label | Conf | Status |
|---|---|---|---|---|---|---|---|---|---|---|
| gen_00.wav | 8000 | 1.314 | 2.890 | 0.367 / 0.375 | 4.4 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_01.wav | 8000 | 1.803 | 2.784 | 0.517 / 0.540 | 3.5 / 3.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_02.wav | 8000 | 1.319 | 2.397 | 0.506 / 0.549 | 3.5 / 3.5 | 0 | - | large explosion / fireball (boss area) | 0.45 | attempt-1 label, not reproduced |
| gen_03.wav | 8001 | 1.539 | 1.014 | 0.374 / 0.338 | 10.8 / 9.7 | 0 | - | unresolved | 0.00 | unresolved |
| gen_0A.wav | 8000 | 1.928 | 1.014 | 0.473 / 0.480 | 7.7 / 7.4 | 0 | - | explosion / impact (weak) | 0.15 | attempt-1 label, not reproduced |
| gen_0B.wav | 8000 | 0.787 | 0.840 | 0.486 / 0.558 | 4.9 / 5.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_0C.wav | 16000 | 1.449 | 2.397 | 0.402 / 0.412 | 4.3 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_0D.wav | 8000 | 0.931 | 1.530 | 0.477 / 0.500 | 4.3 / 4.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_0E.wav | 8000 | 2.240 | 1.014 | 0.622 / 0.490 | 9.7 / 7.5 | 0 | - | explosion in boss area (weak) | 0.20 | attempt-1 label, not reproduced |
| gen_0F.wav | 8000 | 0.817 | 1.367 | 0.424 / 0.429 | 4.3 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_10.wav | 8000 | 2.940 | 0.330 | 0.275 / 0.351 | 4.7 / 6.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_11.wav | 8000 | 2.899 | 1.014 | 0.461 / 0.572 | 10.9 / 13.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_12.wav | 8000 | 2.941 | 1.014 | 0.586 / 0.526 | 11.3 / 10.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_13.wav | 8000 | 1.471 | 2.682 | 0.403 / 0.421 | 4.2 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| gen_14.wav | 8000 | 1.429 | 2.397 | 0.398 / 0.405 | 4.6 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_15.wav | 11025 | 1.567 | 1.014 | 0.493 / 0.411 | 8.1 / 6.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_16.wav | 8013 | 0.855 | 1.588 | 0.353 / 0.320 | 4.4 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_17.wav | 8000 | 0.556 | 0.413 | 0.329 / 0.431 | 7.0 / 9.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_19.wav | 8000 | 0.743 | 1.317 | 0.352 / 0.376 | 4.2 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_1A.wav | 8000 | 0.265 | 0.977 | 0.478 / 0.461 | 7.0 / 6.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_1C.wav | 11025 | 0.463 | 3.000 | 0.425 / 0.443 | 4.8 / 5.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_1D.wav | 8000 | 0.236 | 0.941 | 0.391 / 0.338 | 5.4 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_1E.wav | 8000 | 0.686 | 1.014 | 0.582 / 0.201 | 15.1 / 5.4 | 1 | - | unresolved | 0.00 | unresolved |
| gen_1F.wav | 8010 | 1.246 | 1.014 | 0.445 / 0.287 | 7.8 / 5.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_20.wav | 8000 | 0.411 | 2.682 | 0.455 / 0.416 | 5.2 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_21.wav | 8000 | 0.700 | 1.191 | 0.484 / 0.460 | 3.7 / 3.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_22.wav | 8000 | 1.171 | 1.134 | 0.396 / 0.339 | 5.0 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| gen_23.wav | 8000 | 0.398 | 3.000 | 0.492 / 0.433 | 6.2 / 5.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_24.wav | 8000 | 0.535 | 0.906 | 0.394 / 0.354 | 4.5 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| gen_25.wav | 8000 | 1.336 | 2.142 | 0.519 / 0.568 | 3.9 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_26.wav | 8000 | 0.692 | 0.977 | 0.487 / 0.377 | 7.8 / 5.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_27.wav | 8000 | 0.751 | 1.988 | 0.437 / 0.408 | 4.5 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_28.wav | 11025 | 1.518 | 2.682 | 0.541 / 0.554 | 3.5 / 3.3 | 0 | - | unresolved (repeats) | 0.10 | attempt-1 label, not reproduced |
| gen_29.wav | 8000 | 1.121 | 1.915 | 0.466 / 0.488 | 3.9 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_2A.wav | 8000 | 1.472 | 2.682 | 0.473 / 0.497 | 4.3 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_2B.wav | 8000 | 0.728 | 1.134 | 0.503 / 0.545 | 3.5 / 3.6 | 0 | - | unresolved | 0.00 | unresolved |
| gen_2C.wav | 8000 | 0.565 | 0.864 | 0.521 / 0.465 | 3.7 / 3.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_2D.wav | 11025 | 0.574 | 0.941 | 0.316 / 0.369 | 3.5 / 3.9 | 0 | - | boss cannon / heavy shot (weak) | 0.25 | attempt-1 label, not reproduced |
| gen_2E.wav | 11025 | 0.288 | 1.014 | 0.588 / 0.589 | 12.8 / 12.7 | 0 | - | unresolved | 0.00 | unresolved |
| gen_2F.wav | 8000 | 0.625 | 1.052 | 0.395 / 0.355 | 4.5 / 3.7 | 0 | - | unresolved | 0.00 | unresolved |
| gen_30.wav | 8000 | 0.488 | 1.157 | 0.534 / 0.557 | 4.5 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| gen_31.wav | 8000 | 0.699 | 1.052 | 0.489 / 0.478 | 3.9 / 3.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_32.wav | 8000 | 0.889 | 1.588 | 0.365 / 0.338 | 4.0 / 3.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_33.wav | 8000 | 0.849 | 1.014 | 0.466 / 0.495 | 6.5 / 6.9 | 0 | - | unresolved | 0.00 | unresolved |
| gen_34.wav | 8000 | 0.751 | 1.222 | 0.438 / 0.388 | 4.1 / 3.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_35.wav | 8000 | 0.879 | 1.420 | 0.406 / 0.390 | 3.9 / 3.4 | 0 | - | unresolved | 0.00 | unresolved |
| gen_36.wav | 8000 | 0.371 | 0.333 | 0.282 / 0.308 | 5.9 / 6.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3A.wav | 8000 | 0.271 | 2.890 | 0.433 / 0.501 | 5.9 / 6.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3B.wav | 11025 | 0.559 | 0.977 | 0.319 / 0.308 | 4.0 / 3.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3C.wav | 11025 | 0.970 | 0.343 | 0.430 / 0.455 | 4.7 / 5.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3D.wav | 8000 | 0.352 | 1.269 | 0.360 / 0.296 | 5.0 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3E.wav | 8003 | 0.853 | 0.595 | 0.215 / 0.208 | 4.4 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_3F.wav | 8003 | 0.828 | 2.224 | 0.221 / 0.234 | 5.8 / 6.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_40.wav | 8000 | 0.658 | 1.191 | 0.507 / 0.532 | 3.9 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_41.wav | 8000 | 2.865 | 1.588 | 0.240 / 0.246 | 4.6 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| gen_42.wav | 8000 | 0.570 | 0.578 | 0.376 / 0.453 | 4.4 / 5.1 | 0 | - | unresolved | 0.00 | unresolved |
| gen_43.wav | 11025 | 0.623 | 0.724 | 0.429 / 0.439 | 4.0 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| gen_44.wav | 8000 | 0.204 | 2.315 | 0.393 / 0.348 | 7.3 / 6.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_45.wav | 8006 | 0.742 | 0.429 | 0.531 / 0.551 | 4.2 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| gen_46.wav | 8000 | 0.245 | 2.682 | 0.472 / 0.345 | 7.2 / 5.3 | 0 | - | unresolved | 0.00 | unresolved |
| gen_47.wav | 8000 | 0.314 | 0.724 | 0.535 / 0.560 | 4.1 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| gen_48.wav | 8000 | 0.912 | 1.777 | 0.367 / 0.384 | 4.1 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st1_00.wav | 8000 | 0.825 | 2.064 | 0.290 / 0.297 | 4.4 / 4.3 | 0 | - | unresolved | 0.00 | unresolved |
| st1_01.wav | 8000 | 0.653 | 1.222 | 0.521 / 0.564 | 4.0 / 4.3 | 0 | - | unresolved | 0.00 | unresolved |
| st1_02.wav | 16000 | 2.955 | 0.445 | 0.309 / 0.343 | 4.7 / 5.3 | 0 | - | unresolved | 0.00 | unresolved |
| st1_03.wav | 16000 | 0.955 | 0.356 | 0.288 / 0.330 | 4.6 / 5.2 | 0 | - | unresolved | 0.00 | unresolved |
| st1_04.wav | 16000 | 0.982 | 0.398 | 0.467 / 0.468 | 5.9 / 6.1 | 0 | - | unresolved | 0.00 | unresolved |
| st1_05.wav | 22050 | 0.078 | 0.398 | 0.576 / 0.440 | 9.6 / 7.5 | 0 | - | unresolved | 0.00 | unresolved |
| st1_06.wav | 22050 | 0.165 | 0.873 | 0.579 / 0.630 | 10.2 / 10.5 | 0 | - | unresolved | 0.00 | unresolved |
| st1_07.wav | 8000 | 4.291 | 2.784 | 0.324 / 0.316 | 4.3 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st1_08.wav | 8004 | 0.982 | 1.269 | 0.214 / 0.274 | 3.9 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st1_09.wav | 8000 | 0.688 | 1.988 | 0.293 / 0.313 | 8.2 / 9.5 | 0 | - | unresolved | 0.00 | unresolved |
| st1_0A.wav | 8000 | 0.980 | 0.578 | 0.376 / 0.358 | 4.5 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st1_0B.wav | 8000 | 0.433 | 0.780 | 0.494 / 0.489 | 4.2 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| st1_0C.wav | 8023 | 0.301 | 1.530 | 0.212 / 0.192 | 6.0 / 5.3 | 0 | - | unresolved | 0.00 | unresolved |
| st1_0D.wav | 8000 | 0.743 | 0.810 | 0.521 / 0.469 | 4.3 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| st1_0E.wav | 8000 | 0.980 | 1.845 | 0.344 / 0.322 | 3.9 / 3.6 | 0 | - | unresolved | 0.00 | unresolved |
| st2_00.wav | 16000 | 1.300 | 2.397 | 0.486 / 0.518 | 4.0 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| st2_01.wav | 16000 | 0.862 | 1.317 | 0.489 / 0.503 | 4.8 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| st2_02.wav | 16007 | 1.387 | 0.343 | 0.303 / 0.314 | 4.7 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st2_03.wav | 8000 | 0.079 | 0.398 | 0.579 / 0.442 | 10.2 / 8.2 | 0 | - | unresolved | 0.00 | unresolved |
| st2_04.wav | 8000 | 0.519 | 0.864 | 0.434 / 0.441 | 3.9 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| st2_05.wav | 8000 | 2.661 | 0.383 | 0.413 / 0.460 | 6.6 / 7.5 | 0 | - | unresolved | 0.00 | unresolved |
| st2_08.wav | 8000 | 0.100 | 0.600 | 0.333 / 0.398 | 5.3 / 6.2 | 0 | - | unresolved | 0.00 | unresolved |
| st2_09.wav | 8000 | 0.102 | 0.906 | 0.387 / 0.385 | 6.2 / 6.0 | 0 | - | unresolved | 0.00 | unresolved |
| st2_0A.wav | 8000 | 0.839 | 1.845 | 0.506 / 0.527 | 4.0 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st2_0C.wav | 11025 | 0.451 | 0.780 | 0.439 / 0.479 | 4.2 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st2_0D.wav | 8000 | 0.404 | 2.583 | 0.547 / 0.431 | 5.5 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st2_0F.wav | 8000 | 0.491 | 1.269 | 0.332 / 0.365 | 4.0 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st2_10.wav | 8000 | 1.735 | 2.682 | 0.436 / 0.460 | 4.0 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| st2_11.wav | 8000 | 0.889 | 1.052 | 0.503 / 0.554 | 3.5 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| st2_13.wav | 8000 | 0.460 | 0.977 | 0.534 / 0.557 | 4.4 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st2_14.wav | 8000 | 0.529 | 1.777 | 0.488 / 0.432 | 5.5 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st3_02.wav | 16000 | 0.448 | 2.890 | 0.407 / 0.502 | 5.2 / 5.6 | 0 | - | unresolved | 0.00 | unresolved |
| st3_03.wav | 16000 | 0.485 | 3.000 | 0.548 / 0.500 | 5.6 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st3_04.wav | 16000 | 1.472 | 0.517 | 0.279 / 0.348 | 3.3 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st3_06.wav | 8000 | 0.439 | 0.462 | 0.448 / 0.489 | 11.7 / 13.7 | 0 | - | unresolved | 0.00 | unresolved |
| st3_07.wav | 8000 | 0.616 | 0.977 | 0.469 / 0.488 | 3.8 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| st3_08.wav | 8000 | 0.912 | 1.915 | 0.565 / 0.580 | 4.6 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st3_0A.wav | 8000 | 0.495 | 1.269 | 0.574 / 0.588 | 4.6 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st3_0B.wav | 11025 | 0.274 | 0.498 | 0.213 / 0.306 | 6.7 / 9.3 | 0 | - | unresolved | 0.00 | unresolved |
| st3_0C.wav | 8000 | 0.567 | 1.014 | 0.529 / 0.488 | 4.4 / 3.7 | 0 | - | unresolved | 0.00 | unresolved |
| st3_0D.wav | 8000 | 0.963 | 1.649 | 0.398 / 0.401 | 4.1 / 3.8 | 0 | - | unresolved | 0.00 | unresolved |
| st3_0E.wav | 8000 | 1.086 | 0.333 | 0.310 / 0.348 | 4.7 / 5.5 | 0 | - | unresolved | 0.00 | unresolved |
| st3_10.wav | 8000 | 0.428 | 0.752 | 0.519 / 0.548 | 3.4 / 3.4 | 0 | - | unresolved | 0.00 | unresolved |
| st3_11.wav | 8000 | 0.730 | 1.777 | 0.460 / 0.455 | 4.8 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| st3_14.wav | 8000 | 0.492 | 0.498 | 0.266 / 0.245 | 4.5 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| st3_15.wav | 8000 | 0.309 | 0.578 | 0.375 / 0.439 | 6.8 / 7.8 | 0 | - | unresolved | 0.00 | unresolved |
| st3_16.wav | 8000 | 1.728 | 2.488 | 0.547 / 0.585 | 4.0 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| st4_00.wav | 16000 | 0.796 | 0.383 | 0.392 / 0.372 | 8.4 / 8.0 | 0 | - | unresolved | 0.00 | unresolved |
| st4_02.wav | 16000 | 0.750 | 1.269 | 0.473 / 0.463 | 4.8 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| st4_03.wav | 16000 | 0.558 | 0.672 | 0.212 / 0.178 | 6.0 / 5.1 | 0 | - | unresolved | 0.00 | unresolved |
| st4_04.wav | 16000 | 1.416 | 1.680 | 0.564 / 0.389 | 7.1 / 5.1 | 0 | - | unresolved | 0.00 | unresolved |
| st4_07.wav | 16000 | 2.260 | 3.000 | 0.460 / 0.477 | 4.0 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| st4_08.wav | 22050 | 0.273 | 0.498 | 0.161 / 0.224 | 6.6 / 9.2 | 0 | - | unresolved | 0.00 | unresolved |
| st4_0F.wav | 8000 | 0.473 | 0.810 | 0.425 / 0.428 | 3.4 / 3.3 | 0 | - | unresolved | 0.00 | unresolved |
| st4_10.wav | 8000 | 0.137 | 0.697 | 0.513 / 0.393 | 6.1 / 4.7 | 0 | - | unresolved | 0.00 | unresolved |
| st4_11.wav | 8000 | 0.918 | 0.600 | 0.430 / 0.523 | 5.2 / 6.1 | 0 | - | unresolved | 0.00 | unresolved |
| st4_12.wav | 8000 | 0.244 | 0.413 | 0.500 / 0.340 | 6.4 / 4.7 | 0 | - | unresolved | 0.00 | unresolved |
| st4_13.wav | 8000 | 1.060 | 1.845 | 0.353 / 0.375 | 4.4 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st4_16.wav | 8000 | 0.079 | 0.333 | 0.496 / 0.404 | 10.1 / 8.4 | 0 | - | unresolved | 0.00 | unresolved |
| st4_17.wav | 8000 | 0.095 | 0.840 | 0.370 / 0.476 | 5.3 / 6.6 | 0 | - | unresolved | 0.00 | unresolved |
| st4_18.wav | 8000 | 0.094 | 0.810 | 0.405 / 0.472 | 6.0 / 6.8 | 0 | - | unresolved | 0.00 | unresolved |
| st4_19.wav | 8000 | 0.222 | 2.309 | 0.515 / 0.463 | 5.7 / 4.9 | 0 | - | unresolved | 0.00 | unresolved |
| st5_01.wav | 8000 | 0.606 | 0.864 | 0.461 / 0.521 | 3.5 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st5_02.wav | 16000 | 1.538 | 1.014 | 0.388 / 0.363 | 11.2 / 10.5 | 0 | - | unresolved | 0.00 | unresolved |
| st5_04.wav | 8000 | 0.919 | 1.777 | 0.421 / 0.441 | 4.2 / 4.2 | 0 | - | unresolved | 0.00 | unresolved |
| st5_05.wav | 8009 | 1.072 | 0.398 | 0.297 / 0.195 | 8.9 / 6.0 | 0 | - | unresolved | 0.00 | unresolved |
| st5_06.wav | 8000 | 1.666 | 0.480 | 0.490 / 0.495 | 7.0 / 7.2 | 0 | - | unresolved | 0.00 | unresolved |
| st5_0B.wav | 11025 | 0.325 | 0.356 | 0.374 / 0.337 | 7.5 / 7.0 | 0 | - | unresolved | 0.00 | unresolved |
| st5_0C.wav | 8000 | 0.657 | 3.000 | 0.471 / 0.401 | 7.9 / 6.2 | 0 | - | unresolved | 0.00 | unresolved |
| st5_0D.wav | 11025 | 2.280 | 3.000 | 0.413 / 0.445 | 4.3 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| st5_0E.wav | 8000 | 0.319 | 0.600 | 0.345 / 0.305 | 3.9 / 3.3 | 0 | - | unresolved | 0.00 | unresolved |
| st5_12.wav | 8000 | 1.457 | 1.777 | 0.444 / 0.346 | 6.1 / 4.4 | 0 | - | unresolved | 0.00 | unresolved |
| st5_14.wav | 8003 | 0.872 | 0.445 | 0.217 / 0.237 | 6.0 / 6.4 | 0 | - | unresolved | 0.00 | unresolved |
| st5_15.wav | 8000 | 0.835 | 0.398 | 0.557 / 0.557 | 8.4 / 8.9 | 0 | - | unresolved | 0.00 | unresolved |
| st5_1A.wav | 8000 | 0.880 | 2.142 | 0.437 / 0.467 | 4.0 / 4.1 | 0 | - | unresolved | 0.00 | unresolved |
| st5_1D.wav | 8000 | 1.275 | 2.064 | 0.581 / 0.578 | 4.2 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| st6_00.wav | 8000 | 0.503 | 0.864 | 0.303 / 0.339 | 3.7 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st6_01.wav | 8000 | 0.599 | 1.014 | 0.397 / 0.446 | 3.3 / 3.6 | 0 | - | unresolved | 0.00 | unresolved |
| st6_03.wav | 16000 | 0.473 | 1.649 | 0.391 / 0.466 | 4.2 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| st6_05.wav | 8000 | 1.586 | 0.383 | 0.482 / 0.409 | 5.1 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| st6_06.wav | 8000 | 2.377 | 1.777 | 0.346 / 0.349 | 4.6 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| st6_07.wav | 8000 | 2.446 | 0.906 | 0.551 / 0.544 | 4.9 / 4.7 | 0 | - | unresolved | 0.00 | unresolved |
| st6_08.wav | 8000 | 1.166 | 1.014 | 0.216 / 0.379 | 5.5 / 9.7 | 0 | - | unresolved | 0.00 | unresolved |
| st6_09.wav | 8000 | 1.191 | 0.647 | 0.371 / 0.433 | 4.1 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st6_0A.wav | 8000 | 0.349 | 0.780 | 0.398 / 0.450 | 4.2 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| st6_0D.wav | 8000 | 1.094 | 0.557 | 0.299 / 0.369 | 6.3 / 7.5 | 0 | - | unresolved | 0.00 | unresolved |
| st6_0E.wav | 8000 | 0.844 | 2.142 | 0.462 / 0.484 | 4.0 / 4.0 | 0 | - | unresolved | 0.00 | unresolved |
| st6_13.wav | 8000 | 0.686 | 0.398 | 0.461 / 0.502 | 4.4 / 4.8 | 0 | - | unresolved | 0.00 | unresolved |
| st6_14.wav | 8000 | 0.653 | 1.988 | 0.556 / 0.556 | 4.8 / 4.7 | 0 | - | unresolved | 0.00 | unresolved |
| st7_01.wav | 16000 | 0.658 | 3.000 | 0.514 / 0.560 | 5.0 / 4.9 | 0 | - | unresolved | 0.00 | unresolved |
| st7_02.wav | 16000 | 0.645 | 0.780 | 0.405 / 0.507 | 3.9 / 5.0 | 0 | - | unresolved | 0.00 | unresolved |
| st7_03.wav | 16000 | 1.212 | 2.315 | 0.468 / 0.492 | 4.2 / 4.3 | 0 | - | unresolved | 0.00 | unresolved |
| st7_04.wav | 16000 | 2.529 | 3.000 | 0.346 / 0.350 | 3.9 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st7_0D.wav | 8000 | 1.197 | 0.647 | 0.319 / 0.353 | 4.0 / 4.5 | 0 | - | unresolved | 0.00 | unresolved |
| st7_0F.wav | 8000 | 0.652 | 0.398 | 0.357 / 0.322 | 7.2 / 7.1 | 0 | - | unresolved | 0.00 | unresolved |
| st7_10.wav | 8000 | 0.419 | 0.941 | 0.449 / 0.453 | 4.6 / 4.6 | 0 | - | unresolved | 0.00 | unresolved |
| st7_11.wav | 8000 | 0.726 | 1.014 | 0.500 / 0.468 | 4.1 / 3.5 | 0 | - | unresolved | 0.00 | unresolved |
| st7_12.wav | 8000 | 3.096 | 1.134 | 0.497 / 0.496 | 3.6 / 3.5 | 0 | - | unresolved | 0.00 | unresolved |
| st7_13.wav | 8000 | 3.034 | 0.697 | 0.457 / 0.505 | 3.3 / 3.7 | 0 | - | unresolved | 0.00 | unresolved |
| st7_14.wav | 8000 | 2.984 | 0.724 | 0.480 / 0.524 | 3.5 / 3.9 | 0 | - | unresolved | 0.00 | unresolved |
| st7_15.wav | 8000 | 2.079 | 0.780 | 0.450 / 0.475 | 3.5 / 3.7 | 0 | - | unresolved | 0.00 | unresolved |
