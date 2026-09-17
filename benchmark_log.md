# Benchmark Log

This document records baseline and optimization runtimes for the CS598 APE HW1 raytracer experiments. These numbers are intended to be referenced later in the paper.

## Reader's Guide

This repository contains a C++ ray tracer. It renders a single still image or an animation from a scene-description file. The experiments use four workloads: Piano Room, Globe animation, a 3,168-triangle Sphere mesh (loaded through `inputs/elephant.ray`), and a 111,748-triangle Real Elephant mesh (defined in `inputs/realelephant.ray`).

Read this log in the following order:

1. **VM Measurements** is the authoritative dataset for the paper: it contains same-machine normal timings, minimum-observed values, and `perf` profiles from the course VM.
2. **Final Capability Results** records the larger final workloads that were practical only after optimization. They demonstrate completion, but are not used for baseline speedup claims when the original workload could not finish.
3. The later **Docker** sections are preliminary evidence: they document discovery and confirmation of the baseline memory-growth problem. Do not combine Docker and VM times in one speedup calculation.

### Final Source Changes

| Optimization | Files | Change | Purpose |
| --- | --- | --- | --- |
| Opt1: nearest-hit scan | `src/shape.cpp` | Replaced per-ray dynamic intersection-array allocation, copying, and sorting with a linear scan retaining only the nearest hit. | Removes the skybox-path leak and avoids work that is discarded after sorting. |
| Opt2: compiler optimization | `src/Makefile` | Changed core source compilation from `-O0` to `-O3`. | Optimizes the remaining numeric kernels identified by `perf`. |
| Opt3: pixel parallelism | `main.cpp`, top-level `Makefile` | Added `#pragma omp parallel for` to `refresh()` and `-fopenmp` to the build flags. | Uses the VM's four CPUs to render independent pixels concurrently. |
| Real Elephant workload | `inputs/realelephant.ray` | Added a scene file that references the supplied 111,748-triangle mesh data. | Exercises the required large-mesh testcase. |

## Environment

- Repository: `PRONTOLab/598APE-HW1`
- Local path: `/Users/zhangyidong/Desktop/CS598_APE/598APE-HW1`
- Preliminary runtime environment: Docker (memory diagnosis and early exploratory timings)
- Final timing and profiling environment: course VM, documented in the VM Measurements section below
- Output format for baseline runs: PPM unless otherwise noted

## VM Measurements

The following measurements are being collected separately on the course VM and will be used for the final, same-machine comparison. They must not be combined with the Docker timings below when computing speedups.

For each final VM benchmark, the reported runtime is the minimum observed value among the collected normal runs, following the instructor's measurement rule. The table reports the number of normal runs (`n`) for every value. `perf` runs are collected separately for profiling and are not included in the timing comparison.

Starting with Optimization 3, each normal timing configuration will be run three times and reported as the minimum observed value (`n=3`). Previously completed Optimization 1 and Optimization 2 rows remain labeled `n=1` rather than being treated as repeated measurements.

- Host: `fa26-cs598a-042.cs.illinois.edu`
- Build tools: GNU Make 4.4.1; g++ 15.2.0
- Baseline checkout: unmodified `PRONTOLab/598APE-HW1` on branch `main`
- Baseline compile behavior: raytracer objects are compiled with `-O0 -g -Werror`; the top-level link command includes `-O3 -lm -g -Werror`

### Real Elephant Measurement Scope

The formal cross-version Real Elephant comparison uses one frame at 1 x 1 for Baseline, Optimization 1, and all later versions. The original baseline 10 x 10 pilot did not finish frame 0 after several minutes, so it cannot provide a fair baseline for a 10 x 10 timing table. Larger optimized-only runs, such as the 10 x 10 Opt1 perf workload, are retained only to profile the post-optimization hotspot or demonstrate practical scalability; they are explicitly not used to calculate speedup versus baseline.

### Raw VM Results

| Version | Scene | Command | Run | Runtime | Notes |
| --- | --- | --- | --- | --- | --- |
| Baseline | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_baseline.ppm -H 500 -W 500` | 1 | 2.398141 s |  |
| Baseline | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_baseline.ppm -H 500 -W 500` | 2 | 2.399512 s |  |
| Baseline | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_baseline.ppm -H 500 -W 500` | 3 | 2.403106 s |  |
| Baseline | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_baseline.ppm -H 500 -W 500` | 4 | 2.394590 s |  |
| Baseline | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_baseline.ppm -H 500 -W 500` | 5 | 2.391404 s | Lowest of five runs; reported VM baseline runtime |
| Baseline | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g0.mp4` | diagnostic | 150.906342 s | All 24 frames rendered; `ffmpeg` was unavailable after the printed render time, so no MP4 was created and this run is excluded from the final repeated-run minimum |
| Baseline | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g0_1.mp4` | 1 | 150.023727 s | All 24 frames rendered and MP4 encoding completed |
| Baseline | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g0_2.mp4` | 2 | 150.013977 s | Lowest of three runs; reported VM baseline runtime |
| Baseline | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g0_3.mp4` | 3 | 150.360199 s | All 24 frames rendered and MP4 encoding completed |
| Baseline | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_baseline_f1.ppm` | 1 | 184.056992 s | Lowest of three runs; reported VM baseline runtime |
| Baseline | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_baseline_f1.ppm` | 2 | 186.358932 s |  |
| Baseline | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_baseline_f1.ppm` | 3 | 185.061874 s |  |
| Baseline pilot | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 10 -H 10 -o output/realelephant_baseline_10x10.ppm` | 1 | Aborted | Still had not completed frame 0 after several minutes; stopped because the unoptimized large-mesh workload was impractical at 10 x 10. Final comparison configuration changed to 1 x 1, 1 frame for every version. |
| Baseline | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_baseline_1x1.ppm` | 1 | 47.799919 s | Single-run timing; 111,748-triangle mesh. Chosen after the 10 x 10 baseline pilot did not complete in several minutes. |
| Optimization 1 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt1.ppm -H 500 -W 500` | 1 | 1.631119 s | Single-run timing after replacing the intersection-array allocation and sorting path with a nearest-hit scan. |
| Optimization 1 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g1.mp4` | 1 | 115.591682 s | Single-run timing; all 24 frames rendered and MP4 encoding completed. |
| Optimization 1 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt1_f1.ppm` | 1 | 2.959240 s | Single-run timing after replacing the intersection-array allocation and sorting path with a nearest-hit scan. |
| Optimization 1 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_opt1_1x1.ppm` | 1 | 0.011738 s | Single-run timing after replacing the intersection-array allocation and sorting path with a nearest-hit scan. |
| Optimization 2 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt2.ppm -H 500 -W 500` | 1 | 0.890050 s | Single-run timing after compiling core source files with `-O3`. |
| Optimization 2 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g2.mp4` | 1 | 67.983910 s | Single-run timing; all 24 frames rendered and MP4 encoding completed. Image-creation timer excludes encoding. |
| Optimization 2 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt2_f1.ppm` | 1 | 1.567893 s | Single-run timing after compiling core source files with `-O3`. |
| Optimization 2 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_opt2_1x1.ppm` | 1 | 0.007792 s | Single-run timing after compiling core source files with `-O3`. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 1 | 0.892069 s | Not a valid Opt3 result: `#pragma omp` existed, but the top-level Makefile did not yet include `-fopenmp`, so the loop remained serial. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 2 | 0.888034 s | Not a valid Opt3 result; retained as build-configuration diagnosis only. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 3 | 0.893060 s | Not a valid Opt3 result; retained as build-configuration diagnosis only. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 1 | 0.889811 s | Not a valid Opt3 result: object compilation used `-fopenmp`, but the final `main.cpp` compile/link command still lacked it. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 2 | 0.888885 s | Not a valid Opt3 result; retained as build-configuration diagnosis only. |
| Pre-Opt3 check | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 3 | 0.894142 s | Not a valid Opt3 result; retained as build-configuration diagnosis only. |
| Optimization 3 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 1 | 0.268110 s | OpenMP pixel-loop parallelization enabled. |
| Optimization 3 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 2 | 0.298742 s | OpenMP pixel-loop parallelization enabled. |
| Optimization 3 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 3 | 0.265021 s | Minimum observed of three valid OpenMP runs. |
| Optimization 3 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g3_1.mp4` | 1 | 35.944862 s | All 24 frames rendered and MP4 encoding completed; image-creation timer excludes encoding. |
| Optimization 3 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g3_2.mp4` | 2 | 35.862736 s | Minimum observed of three valid OpenMP runs. |
| Optimization 3 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g3_3.mp4` | 3 | 35.922195 s | All 24 frames rendered and MP4 encoding completed; image-creation timer excludes encoding. |
| Optimization 3 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt3_f1.ppm` | 1 | 0.425542 s | Minimum observed of three valid OpenMP runs. |
| Optimization 3 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt3_f1.ppm` | 2 | 0.430412 s | OpenMP pixel-loop parallelization enabled. |
| Optimization 3 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt3_f1.ppm` | 3 | 0.452065 s | OpenMP pixel-loop parallelization enabled. |
| Optimization 3 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_opt3_1x1.ppm` | 1 | 0.008748 s | OpenMP setup and timing overhead dominate this extremely small workload. |
| Optimization 3 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_opt3_1x1.ppm` | 2 | 0.008072 s | OpenMP setup and timing overhead dominate this extremely small workload. |
| Optimization 3 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_opt3_1x1.ppm` | 3 | 0.007944 s | Minimum observed of three valid OpenMP runs. |
| Final capability | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt3_f24.ppm` | 1 | 10.375499 s | All 24 frames completed with final Opt3. This is a final-workload result, not a baseline speedup comparison. |
| Final capability | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt3_f24.ppm` | 2 | 10.384406 s | All 24 frames completed with final Opt3. |
| Final capability | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt3_f24.ppm` | 3 | 10.605807 s | All 24 frames completed with final Opt3. |
| Final capability | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt3_100x100.ppm` | 1 | 22.004694 s | Final Opt3 workload at a practical resolution; not used for a baseline speedup comparison. |
| Final capability | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt3_100x100.ppm` | 2 | 18.758718 s | Minimum observed of three final Opt3 runs. |
| Final capability | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt3_100x100.ppm` | 3 | 18.970547 s | Final Opt3 workload at a practical resolution; not used for a baseline speedup comparison. |

### Reported VM Results

| Version | Scene | Resolution | Frames | Reported runtime | Rule |
| --- | --- | --- | --- | --- | --- |
| Baseline | Piano Room | 500 x 500 | 1 | 2.391404 s | Minimum observed, n=5 |
| Baseline | Globe | 1000 x 1000 | 24 | 150.013977 s | Minimum observed, n=3; image-creation timer excludes movie encoding |
| Baseline | Sphere mesh | 100 x 100 | 1 | 184.056992 s | Minimum observed, n=3 |
| Baseline | Real Elephant | 1 x 1 | 1 | 47.799919 s | Minimum observed, n=1; 10 x 10 baseline pilot was impractical |
| Optimization 1 | Piano Room | 500 x 500 | 1 | 1.631119 s | Minimum observed, n=1; 1.47x vs. VM baseline |
| Optimization 1 | Globe | 1000 x 1000 | 24 | 115.591682 s | Minimum observed, n=1; 1.30x vs. VM baseline |
| Optimization 1 | Sphere mesh | 100 x 100 | 1 | 2.959240 s | Minimum observed, n=1; 62.20x vs. VM baseline |
| Optimization 1 | Real Elephant | 1 x 1 | 1 | 0.011738 s | Minimum observed, n=1; 4072.26x vs. VM baseline |
| Optimization 2 | Piano Room | 500 x 500 | 1 | 0.890050 s | Minimum observed, n=1; 1.83x vs. Opt1; 2.69x vs. VM baseline |
| Optimization 2 | Globe | 1000 x 1000 | 24 | 67.983910 s | Minimum observed, n=1; 1.70x vs. Opt1; 2.21x vs. VM baseline |
| Optimization 2 | Sphere mesh | 100 x 100 | 1 | 1.567893 s | Minimum observed, n=1; 1.89x vs. Opt1; 117.39x vs. VM baseline |
| Optimization 2 | Real Elephant | 1 x 1 | 1 | 0.007792 s | Minimum observed, n=1; 1.51x vs. Opt1; 6134.49x vs. VM baseline |
| Optimization 3 | Piano Room | 500 x 500 | 1 | 0.265021 s | Minimum observed, n=3; 3.36x vs. Opt2; 9.02x vs. VM baseline |
| Optimization 3 | Globe | 1000 x 1000 | 24 | 35.862736 s | Minimum observed, n=3; 1.90x vs. Opt2; 4.18x vs. VM baseline |
| Optimization 3 | Sphere mesh | 100 x 100 | 1 | 0.425542 s | Minimum observed, n=3; 3.68x vs. Opt2; 432.53x vs. VM baseline |
| Optimization 3 | Real Elephant | 1 x 1 | 1 | 0.007944 s | Minimum observed, n=3; 0.98x vs. Opt2; difference is within overhead/noise for this tiny workload |

### Final Capability Results

| Scene | Version | Resolution | Frames | Minimum observed | Rule | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Sphere mesh | Optimization 3 | 100 x 100 | 24 | 10.375499 s | n=3 | All 24 frames completed. The original unmodified Docker run was killed, so this row is not used for a same-machine baseline speedup. |
| Real Elephant | Optimization 3 | 100 x 100 | 1 | 18.758718 s | n=3 | Large 111,748-triangle mesh completed at a practical resolution. This row is not used for a same-machine baseline speedup. |

### VM Correctness Checks

The optimized program produced byte-identical PPM output to the VM baseline for all three direct comparisons below. The SHA-256 checks establish that the measured speedups did not come from changing the rendered image.

| Scene | Compared files | SHA-256 | Result |
| --- | --- | --- | --- |
| Piano Room | `output/pianoroom_baseline.ppm`, `output/pianoroom_opt3.ppm` | `efd0a73f8b2bffde94501ae34e8f29cee99071cb22909455a15d102faac3096a` | Identical |
| Globe, frame 0 | `output/g0_1.mp4.tmp.0000000.ppm`, `output/g3_2.mp4.tmp.0000000.ppm` | `10a9e888fe701e316e64218fded396e44ff3a892baf1ee8c7e7979c4f0bf3286` | Identical |
| Sphere mesh | `output/sphere_baseline_f1.ppm`, `output/sphere_opt3_f1.ppm` | `0e66bcfed3acf7ab5c3f69547d6fb03908abe0a4228c00ec029869d83a137ea3` | Identical |
| Real Elephant | `output/realelephant_baseline_1x1.ppm`, `output/realelephant_opt3_1x1.ppm` | `06b43088df11eac50b3c3c43fe0be6aa9cd9739979387cd1de7542e8cc53a968` | Identical |

### VM Perf Profiles

| Version | Workload | Collection command | Result | Interpretation |
| --- | --- | --- | --- | --- |
| Baseline | Piano Room, 500 x 500, 1 frame | `sudo perf record -g --call-graph dwarf -o /tmp/perf_piano_baseline.data -- ./main.exe -i inputs/pianoroom.ray --ppm -o /tmp/pianoroom_perf.ppm -H 500 -W 500` | 9,785 task-clock samples; no lost samples | The call graph attributes 98.04% of samples to `refresh(Autonoma*)`, with 97.46% below `calcColor(...)`. The flat profile's largest self-time symbols are `calcColor(...)` at 15.26% and `solveScalers(...)` at 14.92%; the call tree also shows `getLight(...)` at 14.35% and `Box::getIntersection(Ray)` at 9.38%. |
| Baseline | Globe, 1000 x 1000, 24 frames | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_globe_baseline.data -- ./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --no-movie -F 24 -o /tmp/globe_perf.ppm` | 14,989 task-clock samples; no lost samples | Flat-profile leaders: `calcColor(...)` 10.51%, texture-coordinate wrapper `fix(double)` 9.42%, `Vector` construction 7.71%, `getLight(...)` 5.80%, `ImageTexture::getColor(...)` 5.37%, `Sphere::getIntersection(Ray)` 4.77%, and `insertionSort(...)` 2.11%. `malloc` and `cfree` account for 1.43% and 1.82%, respectively. |
| Baseline | Sphere mesh, 100 x 100, 1 frame | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_sphere_baseline.data -- ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o /tmp/sphere_perf.ppm` | 18,335 task-clock samples; no lost samples | `calcColor(...)` dominates self time at 96.11%. The next symbols are `_int_malloc` at 0.89%, `Triangle::getIntersection(Ray)` at 0.43%, and `solveScalers(...)` at 0.40%. This identifies the work inside `calcColor()` as the first optimization target. |
| Baseline | Real Elephant, 1 x 1, 1 frame | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_realelephant_baseline.data -- ./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o /tmp/realelephant_perf.ppm` | 4,770 task-clock samples; no lost samples | `calcColor(...)` is 50.86%. Kernel memory-management symbols are also prominent: `do_user_addr_fault` 12.20%, `__clear_pages_unrolled` 7.71%, and `_raw_spin_unlock_irqrestore` 5.28%. Combined with the Docker RSS growth, this supports the diagnosis of excessive per-ray allocation and memory churn in the baseline. |
| Optimization 1 | Piano Room, 500 x 500, 1 frame | `sudo perf record -g --call-graph dwarf -o /tmp/perf_piano_opt1.data -- ./main.exe -i inputs/pianoroom.ray --ppm -o /tmp/pianoroom_opt1_perf.ppm -H 500 -W 500` | 6,747 task-clock samples; no lost samples | The nearest-hit scan reduces `calcColor(...)` self time from the baseline's 15.26% to 3.68%. Allocation and sorting symbols no longer appear among the top functions. The largest remaining costs are `solveScalers(...)` 21.62%, `Box::getLightIntersection(...)` 10.60%, `Vector` construction 10.18%, and `Plane::getIntersection(...)` 8.58%. |
| Optimization 1 | Globe, 1000 x 1000, 24 frames | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_globe_opt1.data -- ./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --no-movie -F 24 -o /tmp/globe_opt1_perf.ppm` | 11,432 task-clock samples; no lost samples; render timer 115.153893 s | `calcColor(...)` decreases from 10.51% to 8.13%. `malloc`, `cfree`, and `insertionSort(...)`, which appeared in the baseline profile, no longer appear in the top 50. The remaining leading costs are `fix(double)` 12.56%, `Vector` construction 8.33%, `Sphere::getIntersection(Ray)` 6.18%, `ImageTexture::getColor(...)` 5.70%, and `getLight(...)` 5.40%. |
| Optimization 1 | Sphere mesh, 100 x 100, 1 frame | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_sphere_opt1.data -- ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o /tmp/sphere_opt1_perf.ppm` | 296 task-clock samples; no lost samples; render timer 2.986516 s | `calcColor(...)` decreases from 96.11% to 7.43%. The remaining work is geometric: `solveScalers(...)` 28.04%, `Triangle::getIntersection(Ray)` 27.03%, and `Plane::getIntersection(Ray)` 13.51%. The short run yields fewer samples, but the removal of allocation and sort overhead is unambiguous. |
| Optimization 1 | Real Elephant, 10 x 10, 1 frame (profiling only) | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_realelephant_opt1.data -- ./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 10 -H 10 -o /tmp/realelephant_opt1_perf.ppm` | 130 task-clock samples; no lost samples; render timer 1.178623 s | This larger profiling-only workload was used because the formal 1 x 1 run was too short for useful sampling. `calcColor(...)` is 6.92%; remaining rendering work is `solveScalers(...)` 26.92%, `Plane::getIntersection(Ray)` 15.38%, and `Triangle::getIntersection(Ray)` 15.38%. `__vfscanf_internal` at 3.08% is mesh-input parsing. |
| Optimization 2 | Piano Room, 500 x 500, 1 frame | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_piano_opt2.data -- ./main.exe -i inputs/pianoroom.ray --ppm -o /tmp/pianoroom_opt2_perf.ppm -H 500 -W 500` | 89 task-clock samples; no lost samples; render timer 0.906369 s | The short execution limits sample precision. The same numerical kernels remain dominant: `solveScalers(...)` 26.97%, `Box::getLightIntersection(...)` 17.98%, `Box::getIntersection(...)` 11.24%, and `Plane::getIntersection(...)` 8.99%. The normal timing improves from 1.631119 s in Opt1 to 0.890050 s, consistent with compiler optimization of these kernels. |
| Optimization 2 | Globe, 1000 x 1000, 24 frames | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_globe_opt2.data -- ./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --no-movie -F 24 -o /tmp/globe_opt2_perf.ppm` | 6,764 task-clock samples; no lost samples; render timer 68.004822 s | The largest remaining costs are `fix(double)` 20.06%, `calcColor(...)` 9.31%, `getLight(...)` 8.78%, `ImageTexture::getColor(...)` 8.31%, `solveScalers(...)` 5.68%, and `Sphere::getIntersection(Ray)` 5.07%. `Vector::Vector(...)` falls from 8.33% in Opt1 to 0.43%, consistent with `-O3` reducing construction overhead. |
| Optimization 2 | Sphere mesh, 100 x 100, 1 frame | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_sphere_opt2.data -- ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o /tmp/sphere_opt2_perf.ppm` | 155 task-clock samples; no lost samples; render timer 1.565475 s | `calcColor(...)` decreases from 7.43% in Opt1 to 3.87%; `Vector::Vector(...)`, which was 6.76% in the Opt1 profile, no longer appears. The remaining dominant work is `solveScalers(...)` 34.19%, `Triangle::getIntersection(Ray)` 30.32%, and `Plane::getIntersection(Ray)` 11.61%. The short run limits fine-grained percentage comparisons. |
| Optimization 2 | Real Elephant, 10 x 10, 1 frame (profiling only) | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_realelephant_opt2.data -- ./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 10 -H 10 -o /tmp/realelephant_opt2_perf.ppm` | 83 task-clock samples; no lost samples; render timer 0.725696 s | The short run limits percentage precision. The remaining work is `solveScalers(...)` 25.30%, `Triangle::getIntersection(Ray)` 21.69%, `Plane::getIntersection(Ray)` 14.46%, and `calcColor(...)` 12.05%. `__vfscanf_internal` 6.02% is mesh parsing. No allocation or sort hotspot reappears. |
| Optimization 3 | Piano Room, 500 x 500, 1 frame | `sudo perf record -F 999 -g --call-graph dwarf -o /tmp/perf_piano_opt3.data -- ./main.exe -i inputs/pianoroom.ray --ppm -o /tmp/pianoroom_opt3_perf.ppm -H 500 -W 500` | 915 task-clock samples; no lost samples; render timer 0.267596 s | OpenMP is active: `refresh(Autonoma*) [clone ._omp_fn.0]` is the compiler-generated parallel loop, and `libgomp` appears in the profile with low overhead. Remaining work is `solveScalers(...)` 28.85%, `Box::getLightIntersection(...)` 12.79%, `getLight(...)` 9.73%, `Vector::dot(...)` 9.07%, and `Box::getIntersection(...)` 7.43%. |
| Optimization 3 | Globe, 1000 x 1000, 24 frames | `sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_globe_opt3.data -- ./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --no-movie -F 24 -o /tmp/globe_opt3_perf.ppm` | 6,851 task-clock samples; no lost samples; render timer 35.898937 s | OpenMP is active through `refresh(Autonoma*) [clone ._omp_fn.0]` at 0.70% and low `libgomp` overhead. The major per-pixel costs remain `fix(double)` 20.19%, `calcColor(...)` 10.04%, `getLight(...)` 8.61%, `ImageTexture::getColor(...)` 7.84%, and `solveScalers(...)` 5.20%. |
| Optimization 3 | Sphere mesh, 100 x 100, 1 frame | `sudo perf record -F 999 -g --call-graph dwarf -o /tmp/perf_sphere_opt3.data -- ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o /tmp/sphere_opt3_perf.ppm` | 1,587 task-clock samples; no lost samples; render timer 0.427501 s | The geometric kernels remain dominant: `solveScalers(...)` 36.42%, `Triangle::getIntersection(Ray)` 30.88%, and `Plane::getIntersection(Ray)` 8.57%. `libgomp` samples are collectively below 0.4%, so OpenMP runtime overhead is small relative to rendering work. |
| Optimization 3 | Real Elephant, 10 x 10, 1 frame (profiling only) | `sudo perf record -F 999 -g --call-graph dwarf -o /tmp/perf_realelephant_opt3.data -- ./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 10 -H 10 -o /tmp/realelephant_opt3_perf.ppm` | 856 task-clock samples; no lost samples; render timer 0.188440 s | The profile remains dominated by `solveScalers(...)` 24.88%, `Triangle::getIntersection(Ray)` 21.14%, `calcColor(...)` 13.90%, and `Plane::getIntersection(Ray)` 11.10%. `libgomp` samples total below 1%, so OpenMP runtime is not the dominant cost. Mesh parsing remains visible through `__vfscanf_internal` 4.21%. |

### VM Baseline Conclusion And Optimization 1 Plan

The VM profiles consistently identify `src/shape.cpp::calcColor()` as the first target. It accounts for 15.26% of Piano Room self time, 10.51% of Globe self time, 96.11% of Sphere mesh self time, and 50.86% of Real Elephant self time. The large-mesh profiles also expose substantial memory-system activity: Real Elephant has 12.20% in `do_user_addr_fault` and 7.71% in `__clear_pages_unrolled`.

Source inspection of the baseline `calcColor()` explains this behavior. For every ray, lines 53-67 allocate a `TimeAndShape` array, allocate and copy a larger replacement array for every shape, and insertion-sort all intersections. The function only consumes the nearest intersection after sorting. In the skybox return path, the allocated `times` array is not freed. Docker RSS sampling previously showed that the unmodified 24-frame Sphere run grew from roughly 45 MB to 3.58 GB and was killed after frame 8.

Optimization 1 changes only `src/shape.cpp::calcColor()`: the `times` allocation/copy/sort block is replaced with one linear scan that retains the closest finite intersection in `curTime` and `curShape`. The skybox path then returns without requiring cleanup because it has no allocation to release. This preserves the nearest-hit behavior while removing the leak, dynamic allocation, array copying, and sort. The Opt1 profiles confirm the expected shift: Piano Room's `calcColor()` drops from 15.26% to 3.68%, Globe's drops from 10.51% to 8.13%, Sphere mesh's drops from 96.11% to 7.43%, and Real Elephant's 10 x 10 profiling run shows 6.92%. Allocation/sort symbols disappear, leaving intersection and scalar-solving code as the dominant work.

### Optimization 2 Plan: Compile Core Sources With `-O3`

**Observation.** After Optimization 1, the leading functions are numerical kernels rather than allocation or sorting. For example, the Opt1 Sphere profile reports `solveScalers(...)` at 28.04%, `Triangle::getIntersection(Ray)` at 27.03%, and `Plane::getIntersection(Ray)` at 13.51%. The Opt1 Real Elephant profiling run shows the same pattern: `solveScalers(...)` at 26.92% and both plane and triangle intersection at 15.38%.

**Build inspection.** `src/Makefile` compiles the core raytracer source files with `FLAGS := -O0 -g -Werror`. The top-level Makefile includes `-O3` only in the final link command; that does not optimize code already compiled into the `src/*.obj` files.

**Change.** Replace `-O0` with `-O3` in `src/Makefile`, then run `make clean && make -j` so every object file is rebuilt. This leaves the rendering algorithm and inputs unchanged, while allowing the compiler to optimize the remaining tight numeric code through transformations such as inlining and loop/code simplification.

**Verification plan.** Re-run the same four VM timing configurations once after rebuilding, then collect post-change perf profiles. The timing table will use the program's `Total time to create images` output; perf remains a separate hotspot-validation tool.

### Optimization 2 Conclusion And Optimization 3 Plan

**Opt2 result.** Compiling the core sources with `-O3` improved every VM timing configuration: Piano Room changed from 1.631119 s to 0.890050 s (1.83x), Globe from 115.591682 s to 67.983910 s (1.70x), Sphere mesh from 2.959240 s to 1.567893 s (1.89x), and formal Real Elephant from 0.011738 s to 0.007792 s (1.51x). The profiles still place most runtime in per-pixel geometric and lighting work, especially `solveScalers(...)`, shape intersection, and vector arithmetic. This supports moving from single-thread code generation to parallel execution across independent pixels.

**Source inspection.** `main.cpp::refresh(Autonoma*)` has one sequential loop over `n = 0` through `H * W - 1`. Each iteration computes one camera ray and writes only its own three-byte output slot, `DATA[3*n]`, while reading the shared scene data.

**Optimization 3 change.** Add `#pragma omp parallel for` immediately before this pixel loop, and add `-fopenmp` to the top-level Makefile's `FLAGS` so the compiler enables and links OpenMP. No scheduling policy will be added initially; the default static distribution keeps the modification small and makes the effect easy to interpret.

**Verification plan.** Rebuild with `make clean && make -j`, run the same four VM timing configurations, then use perf to confirm that execution remains dominated by rendering work rather than synchronization overhead.

## Baseline Results

| Scene | Command | Resolution | Frames | Runtime | Notes |
| --- | --- | --- | --- | --- | --- |
| Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500` | 500 x 500 | 1 | 1.666835 s | Baseline runtime recorded before optimization |
| Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24` | 1000 x 1000 | 24 | 101.688324 s | Baseline runtime records image creation time only; ffmpeg movie encoding output is excluded |
| Elephant | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --movie -F 24 -W 100 -H 100 -o output/sphere.mp4` | 100 x 100 | 24 | Did not complete | Baseline process was killed after producing temporary frames through `output/sphere.mp4.tmp.0000008.ppm`; no final `Total time to create images` was produced |
| Elephant reduced | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/elephant_f1.ppm` | 100 x 100 | 1 | 123.998543 s | Reduced baseline used because the full 24-frame Elephant baseline did not complete |

## Running Notes

- The baseline should be rerun before each major optimization phase if the build flags, Docker image, input scene, output format, or machine load changes.
- Keep all reported paper numbers tied to the exact command and scene configuration used to generate them.
- For later optimization entries, record both the new runtime and the speedup relative to the baseline.

## Diagnostic Attempts

| Purpose | Command | Result | Notes |
| --- | --- | --- | --- |
| Check memory leaks with Valgrind | `valgrind --leak-check=summary ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 20 -H 20 -o output/elephant_valgrind.ppm` | Did not run | Docker image does not include Valgrind: `bash: valgrind: command not found` |
| Track memory growth during Sphere mesh baseline | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_memcheck.ppm` with periodic `/proc/<pid>/status` `VmRSS` sampling | Process killed after frame 8 | Resident memory grew from roughly 45 MB early in frame 0 to roughly 3.58 GB after frame 8, consistent with cumulative memory growth/leak under the baseline implementation |
| Track memory after minimal leak fix | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_memcheck_fixed.ppm` with periodic `/proc/<pid>/status` `VmRSS` sampling | Completed all 24 frames in 3001.693359 s | Resident memory stayed bounded at roughly 4.5-4.7 MB throughout the run, confirming that the leak fix removed the unbounded memory growth observed in the baseline |

## Current Diagnosis And Fix Plan

The Sphere mesh baseline is not failing because of a command-line mistake. The run was reproduced twice and was killed after frame 8. A follow-up memory sampling run showed resident memory increasing steadily from roughly 45 MB early in frame 0 to roughly 3.58 GB after frame 8, close to the Docker memory limit. This is consistent with cumulative memory growth or a memory leak.

The current most likely source is `src/shape.cpp::calcColor()`. For each ray, the baseline implementation allocates a `times` array, repeatedly reallocates it while scanning shapes, sorts all intersections, and then uses only the nearest intersection. If no shape is hit, the function returns through the skybox/background path before freeing `times`. In mesh scenes, many rays miss the object, so this leak can accumulate quickly across pixels and frames.

Planned repair before further benchmarking:

1. Apply a minimal leak fix in `src/shape.cpp::calcColor()` by freeing `times` before returning from the skybox/background path.
2. Rebuild the program in Docker.
3. Rerun the Sphere mesh 24-frame command at 100 x 100 and monitor whether memory remains bounded.
4. If the leak fix allows the full Sphere mesh baseline to complete, record the result separately as a leak-fixed run, not as the original unmodified baseline.
5. After the leak-fixed run is stable, continue with performance optimizations one at a time.

## Optimization Results

| Version | Scene | Command | Resolution | Frames | Runtime | Speedup vs. Baseline | Notes |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Leak fix | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_memcheck_fixed.ppm` | 100 x 100 | 24 | 3001.693359 s | N/A: original 24-frame baseline did not complete | Added `free(times)` before the skybox/background return in `src/shape.cpp::calcColor()` |
| Optimization 1 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt1.ppm -H 500 -W 500` | 500 x 500 | 1 | 1.320240 s | 1.26x | Replaced all-intersection allocation and sorting with a nearest-hit scan in `src/shape.cpp::calcColor()` |
| Optimization 1 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/globe_opt1.mp4` | 1000 x 1000 | 24 | 83.553696 s | 1.22x | Image creation completed; later ffmpeg step failed because the generated shell command was truncated for the longer output filename, but movie encoding is excluded from the benchmark time |
| Optimization 1 | Sphere mesh reduced | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt1_f1.ppm` | 100 x 100 | 1 | 2.324806 s | 53.34x | Nearest-hit scan removes per-ray allocation, copying, and sorting overhead; compared against the 1-frame reduced baseline |
| Optimization 1 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt1_f24.ppm` | 100 x 100 | 24 | 57.674206 s | 52.05x vs. leak-fixed run | Original unmodified 24-frame baseline did not complete; speedup is computed against the leak-fixed 24-frame run |
| Optimization 1 | Real Elephant sanity | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 10 -H 10 -o output/realelephant_opt1_10x10.ppm` | 10 x 10 | 1 | 0.862067 s | N/A | Sanity run after creating `inputs/realelephant.ray` with the 111,748-triangle mesh; used to confirm the large mesh loads and renders after Optimization 1 |
| Optimization 1 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt1_f1.ppm` | 100 x 100 | 1 | 84.670029 s | N/A | Large 111,748-triangle mesh after Optimization 1; no original baseline speedup is reported because the unoptimized large mesh baseline was not practical to run |
| Optimization 2 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt2.ppm -H 500 -W 500` | 500 x 500 | 1 | 0.477970 s | 2.76x vs. Optimization 1; 3.49x vs. baseline | Enabled `-O3` for raytracer object files by changing `src/Makefile` from `-O0` to `-O3` |
| Optimization 2 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g2.mp4` | 1000 x 1000 | 24 | 39.017620 s | 2.14x vs. Optimization 1; 2.61x vs. baseline | Enabled `-O3` for raytracer object files; image creation time excludes ffmpeg movie encoding |
| Optimization 2 | Sphere mesh reduced | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt2_f1.ppm` | 100 x 100 | 1 | 0.777964 s | 2.99x vs. Optimization 1; 159.39x vs. reduced baseline | Enabled `-O3` for raytracer object files |
| Optimization 2 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt2_f24.ppm` | 100 x 100 | 24 | 19.241474 s | 3.00x vs. Optimization 1; 156.00x vs. leak-fixed run | Enabled `-O3` for raytracer object files |
| Optimization 2 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt2_f1.ppm` | 100 x 100 | 1 | 28.728830 s | 2.95x vs. Optimization 1 | Enabled `-O3` for raytracer object files; original unoptimized large mesh baseline was not practical to run |
| Optimization 3 | Piano Room | `./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom_opt3.ppm -H 500 -W 500` | 500 x 500 | 1 | 0.191778 s | 2.49x vs. Optimization 2; 8.69x vs. baseline | Parallelized the pixel loop in `main.cpp::refresh()` with OpenMP |
| Optimization 3 | Globe | `./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/g3.mp4` | 1000 x 1000 | 24 | 21.318403 s | 1.83x vs. Optimization 2; 4.77x vs. baseline | Parallelized the pixel loop in `main.cpp::refresh()` with OpenMP; image creation time excludes ffmpeg movie encoding |
| Optimization 3 | Sphere mesh reduced | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_opt3_f1.ppm` | 100 x 100 | 1 | 0.258692 s | 3.01x vs. Optimization 2; 479.33x vs. reduced baseline | Parallelized the pixel loop in `main.cpp::refresh()` with OpenMP |
| Optimization 3 | Sphere mesh | `./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/sphere_opt3_f24.ppm` | 100 x 100 | 24 | 6.149237 s | 3.13x vs. Optimization 2; 488.14x vs. leak-fixed run | Parallelized the pixel loop in `main.cpp::refresh()` with OpenMP |
| Optimization 3 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/realelephant_opt3_f1.ppm` | 100 x 100 | 1 | 7.762796 s | 3.70x vs. Optimization 2; 10.91x vs. Optimization 1 | Parallelized the pixel loop in `main.cpp::refresh()` with OpenMP |
| Optimization 3 | Real Elephant | `./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 24 -W 100 -H 100 -o output/realelephant_opt3_f24.ppm` | 100 x 100 | 24 | 205.985916 s | N/A | Full 24-frame large-mesh run after Optimization 3; average time was 8.58 s per frame |

## Optimization 1 Summary

Optimization 1 changes `src/shape.cpp::calcColor()`. The original implementation computed an intersection time for every shape, repeatedly allocated and copied a growing `times` array, sorted the full array, and then used only the first entry. The optimized version keeps only the closest intersection found so far while scanning the shape list.

This is a work-reduction optimization: it does not change the set of shapes tested against each ray, but it removes per-ray allocation, copying, and sorting. The effect is modest for Piano Room and Globe because those scenes have relatively few shapes. The effect is much larger for the Sphere mesh workload because `inputs/elephant.ray` expands the mesh into thousands of triangles, so the removed per-ray overhead was repeated many more times.

Measured impact:

- Piano Room improved from 1.666835 s to 1.320240 s, a 1.26x speedup.
- Globe improved from 101.688324 s to 83.553696 s, a 1.22x speedup.
- Sphere mesh reduced, 1 frame at 100 x 100, improved from 123.998543 s to 2.324806 s, a 53.34x speedup.
- Sphere mesh, 24 frames at 100 x 100, improved from the leak-fixed runtime of 3001.693359 s to 57.674206 s, a 52.05x speedup against the leak-fixed run. The original unmodified 24-frame baseline did not complete.

## Optimization 2 Summary

Optimization 2 changes `src/Makefile` by compiling the raytracer object files with `-O3` instead of `-O0`. The top-level `Makefile` already used `-O3` when linking `main.cpp`, but most of the raytracer implementation is compiled through `src/Makefile`, including shape intersection, vector math, lighting, camera, and texture-related code. Before this change, those object files were built without compiler optimization.

This is a compiler optimization. It does not change the raytracing algorithm or scene inputs, but it allows the C++ compiler to optimize low-level numeric code such as vector operations, triangle intersection tests, loops, and function calls.

Measured impact:

- Piano Room improved from 1.320240 s after Optimization 1 to 0.477970 s, a 2.76x speedup over Optimization 1 and 3.49x over the original baseline.
- Globe improved from 83.553696 s after Optimization 1 to 39.017620 s, a 2.14x speedup over Optimization 1 and 2.61x over the original baseline.
- Sphere mesh reduced, 1 frame at 100 x 100, improved from 2.324806 s after Optimization 1 to 0.777964 s, a 2.99x speedup over Optimization 1.
- Sphere mesh, 24 frames at 100 x 100, improved from 57.674206 s after Optimization 1 to 19.241474 s, a 3.00x speedup over Optimization 1.
- Real Elephant, 1 frame at 100 x 100, improved from 84.670029 s after Optimization 1 to 28.728830 s, a 2.95x speedup over Optimization 1.

The mesh workloads show a consistent roughly 3x improvement from compiler optimization, which suggests that the remaining time is dominated by repeated numeric intersection work. However, the Real Elephant runtime is still much larger than the Sphere mesh runtime because both versions still scan mesh triangles linearly for each ray.

## Optimization 3 Summary

Optimization 3 changes `main.cpp::refresh()` and the top-level `Makefile`. The pixel loop in `refresh()` was parallelized with a simple OpenMP `parallel for`, and the top-level build flags were updated with `-fopenmp` so that `main.cpp` is compiled and linked with OpenMP support.

This is a parallelism optimization. Each loop iteration computes one pixel by constructing a ray, tracing it through the scene, and writing the resulting RGB value to a distinct location in `DATA`. Since different pixels write to different output addresses and mostly read shared scene data, the loop is a natural target for parallel execution.

Measured impact:

- Piano Room improved from 0.477970 s after Optimization 2 to 0.191778 s, a 2.49x speedup over Optimization 2 and 8.69x over the original baseline.
- Globe improved from 39.017620 s after Optimization 2 to 21.318403 s, a 1.83x speedup over Optimization 2 and 4.77x over the original baseline.
- Sphere mesh reduced, 1 frame at 100 x 100, improved from 0.777964 s after Optimization 2 to 0.258692 s, a 3.01x speedup over Optimization 2.
- Sphere mesh, 24 frames at 100 x 100, improved from 19.241474 s after Optimization 2 to 6.149237 s, a 3.13x speedup over Optimization 2.
- Real Elephant, 1 frame at 100 x 100, improved from 28.728830 s after Optimization 2 to 7.762796 s, a 3.70x speedup over Optimization 2.
- Real Elephant, 24 frames at 100 x 100, completed in 205.985916 s after Optimization 3, averaging 8.58 s per frame.

The OpenMP speedup is strongest on the mesh workloads, especially Real Elephant, because each pixel has enough intersection work to keep worker threads busy. Globe sees a smaller improvement, likely because some of its runtime is spent in work that is less parallelized by the pixel loop or has lower per-pixel cost. The main remaining issue for the mesh workloads is that each ray still scans triangles linearly, so larger meshes still scale roughly with the number of triangles.
