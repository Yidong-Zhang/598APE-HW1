# Benchmark Log

This document records baseline and optimization runtimes for the CS598 APE HW1 raytracer experiments. These numbers are intended to be referenced later in the paper.

## Environment

- Repository: `PRONTOLab/598APE-HW1`
- Local path: `/Users/zhangyidong/Desktop/CS598_APE/598APE-HW1`
- Runtime environment: Docker
- Output format for baseline runs: PPM unless otherwise noted

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
