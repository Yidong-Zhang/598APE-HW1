# 598APE-HW1

This repository contains code for homework 1 of 598APE.

In particular, this repository is an implementation of a Raytracer.

The raytracer program here is general and can be used to generate any number of different potential scenes. Once compiled, one can call the raytracer program as follows:
```bash
./main.exe --help
# Prints the following
# Usage ./main.exe [-H <height>] [-W <width>] [-F <framecount>] [--movie] [--no-movie] [--png] [--ppm] [--help] [-o <outfile>] [-i <infile>] [-a <animationfile>]
```

The raytracer program takes a scene file (a text file ending in .ray) and generates an image or sequence of images corresponding to the specified scene. Timer code surrounds the main computational loop inside `main.cpp`; the goal of this assignment is to reduce that printed runtime as much as possible while maintaining or increasing the complexity (i.e. resolution, number of frames) of the scene.

For example, running an unmodified checkout on a given machine may print something like:
```
Done Frame       0|
Total time to create images=1.334815 seconds
```
**This `1.334815 seconds` figure is only illustrative example output from one prior run on one machine; it is machine-dependent and is not the formal VM baseline used for grading.** The authoritative baseline and optimized timings, collected on the course VM with a fixed measurement methodology, are recorded in [`benchmark_log.md`](benchmark_log.md).

Input and output of images is already handled by the library. In particular, the PPM format (see https://en.wikipedia.org/wiki/Netpbm for an example) represents images as text data, which makes it easy to input and output without the use of a library. However, as this is not the most efficient, this application uses the ImageMagick tool to convert to and from the PPM format.

## Artifact Reproduction Guide

This section describes exactly how to rebuild and re-run the optimized code, and where the corresponding measured results are recorded.

### Final Evaluation Environment

The final benchmark and profiling numbers referenced in this repository (see [`benchmark_log.md`](benchmark_log.md)) were collected with:

* GNU Make 4.4.1
* g++ 15.2.0
* OpenMP (required for the parallel build; see below)
* FFmpeg (for `--movie` encoding)
* ImageMagick (for non-PPM image conversion)
* Linux `perf`, used **only** for hotspot profiling, never as a substitute for the benchmark timer

### Dependencies

This program assumes the following are installed on your machine:
* A working C++ compiler (g++ is assumed in the Makefile; Clang with `libomp` is supported on macOS, see the top-level `Makefile`)
* `make`
* **OpenMP**, required starting with Optimization 3. The top-level `Makefile` compiles and links `main.cpp` with `-fopenmp` (`-Xpreprocessor -fopenmp -lomp` on macOS through Homebrew's `libomp`); without an OpenMP-capable toolchain, the build will fail or run single-threaded.
* ImageMagick (for importing and exporting non-PPM images)
* FFmpeg (for exporting movies from image sequences)

### Build Instructions

From the repository root:
```bash
make clean
make -j
```
This rebuilds `src/*.obj` (via `src/Makefile`) and links `main.exe` at the top level using the flags described above.

### Reproducing Outputs by Version

Each block below checks out the commit for that stage, rebuilds, and regenerates output images for all four workloads. Run `git checkout main` (or your working branch) afterward to return to the latest code.

**Baseline** (commit `19bbc81`):

Commit `19bbc81` predates the addition of `inputs/realelephant.ray`, so that file does not exist in this checkout. `inputs/realelephant.ray` is only a workload/scene-description file (it points the existing raytracer at the larger 111,748-triangle mesh) — it contains no optimized raytracer source code, so supplying it separately does not change the baseline implementation in any way. Preserve it from `main` before checking out the baseline, using the repository's actual branch (`main`):
```bash
git show main:inputs/realelephant.ray > /tmp/realelephant.ray
git checkout 19bbc81
cp /tmp/realelephant.ray inputs/realelephant.ray
make clean && make -j
./main.exe -i inputs/pianoroom.ray --ppm -o output/baseline_pianoroom.ppm -H 500 -W 500
./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/baseline_globe.mp4
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/baseline_sphere_f1.ppm
./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/baseline_realelephant_1x1.ppm
```
Copying `realelephant.ray` into the baseline checkout only changes the benchmark input available on disk; it does not modify or add to the baseline source code, which remains exactly commit `19bbc81`.

**Optimization 1, 2, and 3** (nearest-hit scan, `-O3`, OpenMP pixel loop — bundled in commit `b6c8b44`):
```bash
git checkout b6c8b44
make clean && make -j
./main.exe -i inputs/pianoroom.ray --ppm -o output/opt123_pianoroom.ppm -H 500 -W 500
./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/opt123_globe.mp4
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/opt123_sphere_f1.ppm
./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/opt123_realelephant_1x1.ppm
```

**Optimization 4 and 5** (orthonormal-basis `solveScalers()`, unused triangle projection removed — bundled in commit `c3295ce`):
```bash
git checkout c3295ce
make clean && make -j
./main.exe -i inputs/pianoroom.ray --ppm -o output/opt45_pianoroom.ppm -H 500 -W 500
./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24 -o output/opt45_globe.mp4
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/opt45_sphere_f1.ppm
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --movie -F 24 -W 100 -H 100 -o output/opt45_sphere.mp4
./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/opt45_realelephant_1x1.ppm
```

As noted above, `b6c8b44` bundles Opt1/Opt2/Opt3 together and `c3295ce` bundles Opt4/Opt5 together; there is no separate commit/command set for each optimization in isolation.

### Baseline Memory Diagnosis

Baseline configuration was constrained by a preliminary memory diagnosis. In Docker, the original 24-frame Sphere mesh run at 100x100 was terminated after frame 8 while resident memory grew from approximately 45 MB to 3.58 GB. Source inspection identified an allocation on every ray in `calcColor()`; on the skybox return path, the allocated intersection array was not released. The final nearest-hit implementation in Optimization 1 removes this allocation entirely, thereby removing both the leak and the repeated allocation/copy/sort overhead.

For same-machine VM comparisons, we therefore used one 100x100 Sphere frame. The original Real Elephant baseline did not complete a 10x10 frame within several minutes, so cross-version Real Elephant timings use one 1x1 frame. These reduced configurations are used only for fair baseline comparisons. We separately report final-capability runs: the optimized renderer completes the 24-frame Sphere workload at 100x100 and one 100x100 Real Elephant frame.

### Optimization Versions

The optimizations were implemented incrementally and committed to this repository's git history. **The individual optimizations below are not all isolated into single, separately buildable commits** — some are bundled together in the same commit. Commit hashes are taken directly from `git log --oneline` on this repository; the closest reproducible commit/state is identified where a clean isolated commit does not exist.

| Version | Description | Files changed | Closest commit(s) |
| --- | --- | --- | --- |
| Baseline | Original, unmodified implementation | — | `19bbc81` (last commit before optimization work began; matches the unmodified upstream behavior used for the VM baseline measurements) |
| Opt1 | Nearest-hit scan replacing per-ray dynamic intersection-array allocation, copying, and sorting | `src/shape.cpp` | `b6c8b44` |
| Opt2 | Compile core sources with `-O3` instead of `-O0` | `src/Makefile` | `b6c8b44` |
| Opt3 | Pixel-loop parallelism via `#pragma omp parallel for` in `refresh()`, with `-fopenmp` added to the top-level `Makefile` | `main.cpp`, `Makefile` | `b6c8b44` |
| Opt4 | Replace the general `solveScalers()` linear-system solve with orthonormal-basis dot-product projections | `src/vector.cpp`, `src/vector.h` | `c3295ce` |
| Opt5 | Remove the unused third projection from `Triangle::getIntersection()` | `src/triangle.cpp` | `c3295ce` |

**Note on commit granularity:** Opt1, Opt2, and Opt3 were all introduced together in a single commit (`b6c8b44`, "Optimize raytracer benchmarks"); there is no earlier commit in this repository's history containing only the nearest-hit scan, or only the `-O3` flag change, or only the OpenMP pragma, in isolation. Similarly, Opt4 and Opt5 were introduced together in a single commit (`c3295ce`, "Optimization 4-5: solveScalers orthonormal projection, triangle intersection projection reduction"). Commit `02d7ade` ("Optimize ray calculations and support macOS OpenMP build") sits between these and adds macOS OpenMP build support plus a minor reordering/caching change in `src/shape.cpp` and `src/box.cpp`; it is not one of the five documented optimizations but is part of the reproducible history. Commits `a563ac5` and `fbe1bd4` only update `benchmark_log.md` and contain no source changes.

**Note on the Real Elephant workload file:** `inputs/realelephant.ray` was introduced in commit `b6c8b44`, after the `19bbc81` baseline commit. It must therefore be supplied separately (see the Baseline block above) when reproducing the baseline large-mesh Real Elephant test.

### Benchmark Commands

These are the exact commands used for the measurements in `benchmark_log.md`. In every case, the reported runtime is the `Total time to create images=...` value printed by the program itself; `perf` invocations (see below) are for profiling only and are never used as benchmark timings.

Piano Room, 500x500, 1 frame:
```bash
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

Globe, 24 frames, default (1000x1000) resolution:
```bash
./main.exe -i inputs/globe.ray --ppm -a inputs/globe.animate --movie -F 24
```

Sphere mesh (3,168-triangle mesh) reduced comparison, 100x100, 1 frame, no movie encoding:
```bash
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o output/sphere_f1.ppm
```

Sphere mesh final Opt5 benchmark, 100x100, 24 frames, with movie encoding:
```bash
./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --movie -F 24 -W 100 -H 100 -o output/sphere.mp4
```

Real Elephant (111,748-triangle mesh) cross-version comparison, 1x1, 1 frame, no movie encoding:
```bash
./main.exe -i inputs/realelephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 1 -H 1 -o output/realelephant_1x1.ppm
```

Profiling (not a benchmark timing) example, using Linux `perf`:
```bash
sudo perf record -F 99 -g --call-graph dwarf -o /tmp/perf_sphere.data -- ./main.exe -i inputs/elephant.ray --ppm -a inputs/elephant.animate --no-movie -F 1 -W 100 -H 100 -o /tmp/sphere_perf.ppm
```

### Measurement Methodology

Measurement rules differ across versions and are recorded exactly as follows; **not every optimization was measured with the same number of runs**, and the exact rule used for each reported number is in `benchmark_log.md`:

* Baseline Piano Room: minimum of `n=5` runs.
* Baseline Globe: minimum of `n=3` runs.
* Baseline Sphere mesh: minimum of `n=3` runs.
* Optimization 1 and Optimization 2 historical rows: `n=1` (single run each), as originally recorded.
* Optimization 3 onward, and later reduced-workload comparisons: repeated runs (`n=3`) reported as the minimum observed value, following the instructor's measurement rule.
* The formal Optimization 5 Sphere mesh 24-frame comparison uses a mean of 10 runs before the change versus a mean of 10 runs after the change, because this particular comparison is reported as a mean rather than a minimum.

Do not assume a uniform `n` across all rows; consult `benchmark_log.md` for the rule attached to each specific number.

### Correctness Verification

Correctness was checked by comparing PPM output produced before and after each optimization, using both a byte-for-byte comparison and a cryptographic hash:
```bash
cmp output/before.ppm output/after.ppm
sha256sum output/before.ppm output/after.ppm
```
`cmp` prints nothing when the files are identical, and matching `sha256sum` output confirms the hashes are equal. The direct comparisons recorded in `benchmark_log.md` (Piano Room, Globe, Sphere mesh, and Real Elephant) all produced byte-identical PPM output between versions, so measured speedups did not come from changing the rendered image.

### Sphere Mesh vs. Real Elephant

These are two different mesh workloads and should not be confused:
* **Sphere mesh** (`inputs/elephant.ray`): loads `data/x.txt` and `data/f.txt`, a 3,168-triangle mesh.
* **Real Elephant** (`inputs/realelephant.ray`): loads `data/elepx.txt` and `data/elepf.txt`, a much larger 111,748-triangle mesh, used to test practical performance on a high-complexity scene.

### Expected Performance Behavior

The following describes the qualitative effect of each optimization; exact runtimes are machine-, compiler-, and core-count-dependent and are not guaranteed to reproduce identically on a different machine. See `benchmark_log.md` for the actual measured numbers.

* **Opt1** (nearest-hit scan): large improvement on mesh-heavy scenes (Sphere mesh, Real Elephant), where the removed per-ray allocation/copy/sort was repeated many times; smaller improvement on Piano Room and Globe, which have far fewer shapes per ray.
* **Opt2** (`-O3` for core sources): a general improvement across all scenes, since the compiler can now optimize numeric kernels (vector math, intersection tests) that were previously compiled unoptimized.
* **Opt3** (OpenMP pixel parallelism): improves scenes where per-pixel work is large and independent; the improvement is bounded by available cores and by how much of the per-scene work is parallelizable versus memory- or texture-bound.
* **Opt4** (orthonormal-basis `solveScalers()`): improves scenes and light paths that call `solveScalers()` frequently (Piano Room, Sphere mesh); scenes whose hotspots lie elsewhere (e.g., Globe's texture sampling) see little change.
* **Opt5** (remove unused triangle projection): a small, targeted improvement specific to triangle-heavy scenes (Sphere mesh, Real Elephant); it has no material effect on scenes with few or no triangles.

For full measured timings, the exact command used for each row, the number of runs, and `perf` profiling results, see [`benchmark_log.md`](benchmark_log.md); this README intentionally does not duplicate that raw data.

## Input Programs
This project contains three (arguably four) input programs for you to optimize.

### PianoRoom

A simple room with a reflecting checkerboard floor, a stairwell, a sphere, a circular rug, and a mirror ref.

Here we want to produce the highest resolution single image of this format, as fast as possible. The relevant command for producing an output is:

```bash
./main.exe -i inputs/pianoroom.ray --ppm -o output/pianoroom.ppm -H 500 -W 500
```

### Globe

A video of the Earth floating on top of a sea with a sky in the background. The Earth and clouds are rotating (in opposite directions), and the sea beneath reflects the scene above, and moves.

Here we want to produce the highest resolution video, as fast as possible. The relevant command for producing an output is:

```bash
./main.exe -i inputs/globe.ray --ppm  -a inputs/globe.animate --movie -F 24 
```

Here, as we are generating multiple frames, the extra command `-a <animationfile>` is used to pass in a sequence of commands to generate subsequent frames.

The number of frames we wish to generate (24) is passed in as `-F <numframes>`.

Here we will produce 24 individual images for each frame. To produce a playable movie out of these images, the `--movie` command will call a program called FFMpeg to produce a playable video.

### Elephant

A mesh of objects. In practical graphics applications, designing a primitive for each possible object is too complex. Instead, one builds up a mesh of triangles to represent the object being shown. Given sufficiently many triangles, we can represent arbitrarily complex structures. Here, we wish to make a video circling around a Mesh object which we import.

The simple version of this program is generated by the following command:
```bash
./main.exe -i inputs/elephant.ray --ppm  -a inputs/elephant.animate --movie -F 24 -W 100 -H 100 -o output/sphere.mp4 
```

Note the reduced resolution (as the initial unoptimized code can be somewhat slow).

This initial mesh represents a sphere with 3168 triangles.

Here we produce a video in which we have the camera circles around the object.

If we inspect the input file `inputs/elephant.ray` we see that it loads the mesh from two files, as defined by the line
```
data/x.txt 1586 data/f.txt 3168 -1.58 -.43 2.7
```

The goal here is to speed up the program sufficiently to make a high resolution circle of the elephant mesh (found in `data/elepx.txt` and `data/elepf.txt`), which contains 111748 triangles. One can edit the `.ray` file and comment out the sphere mesh and replace it with `data/elepx.txt 62779 data/elepf.txt 111748 -1.58 -.43 2.7` (this is done in `inputs/realelephant.ray`).

## Code Overview

The raytracer contains several core utilities, defined in different files.

### Camera

The Camera class contains information about the position and direction we are facing. An image is constructed by creating a grid of points and sending out rays from each of these points, and determining what objects they collide with. Each result becomes an individual pixel in our resulting image.

### Shape

Each object in our scene is defined as a shape. There are several shapes subclasses in the application. This includes a plane (an infinitely long flat surface), a sphere (a collection of points equidistant from a center), a disk (a flat surface whose points are within a given distance of a center), a box (a flat rectangle), and a triangle.

Shapes have a position in space, and potentially an orientation (i.e. direction they face, as defined with the angles yaw pitch and roll).

Shapes also have a texture defining what color of each point of the shape, and optionally a "normalMap" texture which defines how light bounces off each point.

Core methods within shape include:
* `getIntersection`, which defines whether a given light ray will hit the shape, and if so returns time it takes the light to hit it (otherwise infinity).
* `getLightIntersection`: Given that a ray hits the shape, determine how a light source will illuminate the shape at that point based off of the color of the object, and its spectral properties (i.e. opaque, reflective, aminent lighting).
* `getNormal` determine the normal axis to the point of collision, in order to compute the direction in which light will bounce off the object.

### Texture

A texture object defines what color will be applied at a point in space. There are two textures implemented: a single color for all points, and one loaded from an image. Textures are used to define both the color of an object, and also can optionally be used to define normal axes for an object (using data stored in rgb to define the xyz axis).

### Light

Light objects illuminate a scene, resulting in differences in gradients of colors on an object and shadows. Lights have a color and a position.

### Autonoma

An Autonoma is a base class used to hold all of the shapes in scope, the camera, and all lights.


## Docker

For ease of use and installation, we provide a docker image capable of running and building code here. The source docker file is in /docker (which is essentially a list of commands to build an OS state from scratch). It contains the dependent compilers, and some other nice things.

You can build this yourself manually by running `cd docker && docker build -t <myusername>/598ape`. Alternatively we have pushed a pre-built version to `wsmoses/598ape` on Dockerhub.

You can then use the Docker container to build and run your code. If you run `./dockerrun.sh` you will enter an interactive bash session with all the packages from docker installed (that script by default uses `wsmoses/598ape`, feel free to replace it with whatever location you like if you built from scratch). The current directory (aka this folder) is mounted within `/host`. Any files you create on your personal machine will be available there, and anything you make in the container in that folder will be available on your personal machine.

`docker/Dockerfile` installs `imagemagick` and `ffmpeg` (required for image conversion and `--movie` encoding), `linux-tools-common`/`linux-tools-generic` (for `perf` profiling), and both a Clang/LLVM toolchain (`clang-19`, `libomp-19-dev`, for the macOS-style OpenMP build path) and `gcc`/`g++` with `libgomp1` explicitly installed so that `-fopenmp` links correctly when building with `g++`, matching the final optimized build's dependency on OpenMP described in the Artifact Reproduction Guide above. Note that the course VM used for the officially reported benchmark and profiling numbers in `benchmark_log.md` is a separate environment from this Docker image; the Docker image here is provided for local development and is not itself the measurement environment.