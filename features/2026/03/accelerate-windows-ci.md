# Accelerate Windows GitHub Actions CI

## Problem

Windows CI jobs are the slowest in the matrix, taking 220-286s vs 83-125s
for Linux/macOS. The bottlenecks are:

- **Slow configure** (28-85s): The Visual Studio generator creates
  `.sln`/`.vcxproj` files, which is much slower than Ninja
- **Slow build** (132-148s): MSBuild with no compilation caching
- **CPM re-downloads**: GSL and doctest re-downloaded every configure

## Changes

All changes are in `.github/workflows/cmake.yml`.

### 1. Switch to Ninja generator on Windows

- Added `ilammy/msvc-dev-cmd@v1` step to set up MSVC environment
  (required for Ninja to find the MSVC toolchain)
- Added `cmake_generator: Ninja` matrix variable for Windows
- Pass `-G Ninja` to CMake configure via
  `${{ matrix.cmake_generator && format('-G {0}', matrix.cmake_generator) }}`
- Non-Windows jobs have no `cmake_generator` set, so the expression
  evaluates to empty string (no change to their behavior)

**Expected savings:** 35-85s per Windows job (configure + build)

### 2. Cache CPM dependency downloads

- Added `actions/cache@v4` for `~/cpm-cache` keyed on CMakeLists.txt hashes
- Set `-DCPM_SOURCE_CACHE=~/cpm-cache` in the configure step
- Cache applies to all platforms, not just Windows

**Expected savings:** 3-10s on configure

## Implementation Progress

### Session #1

- Implemented both changes in `.github/workflows/cmake.yml`
- Pushed to `fix-serialize-vuln` branch for CI verification
