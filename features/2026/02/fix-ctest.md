# Enable CTest Integration

## Problem

The project has `fast_tests` and `slow_tests` executables built with doctest,
but `ctest` doesn't work because `enable_testing()` and `add_test()` are never
called in the CMake configuration. The GitHub Actions CI also uses
platform-specific scripts to invoke test executables directly, with
Windows/Linux branching logic.

## Solution

### 1. Enable CTest in CMake

Added `enable_testing()` after the `project()` call in the top-level
`CMakeLists.txt`.

### 2. Per-test-case discovery with doctest

Instead of registering each test executable as a single monolithic CTest test
via `add_test()`, we use doctest's `doctest_discover_tests()` function from the
bundled `scripts/cmake/doctest.cmake` module. This:

- Registers each individual `TEST_CASE` as a separate CTest test (104 total)
- Enables filtering by name: `ctest --test-dir build -R "castle"`
- Runs a post-build step that queries the executable with `--list-test-cases`
- Does not require re-running CMake when tests change

The module is included conditionally after CPM fetches doctest:

```cmake
if (doctest_FOUND)
    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
endif()
```

### 3. Simplified GitHub Actions CI

Replaced two platform-specific test steps (with Windows/Linux `if` branching)
with a single cross-platform `ctest` call:

```yaml
- name: Run Tests
  run: ctest --test-dir ${{ steps.strings.outputs.build-output-dir }} --build-config ${{ matrix.build_type }} --output-on-failure -j 2
```

- `--build-config` handles Windows multi-config generators
- `--output-on-failure` keeps CI output clean
- `-j 2` runs two tests in parallel

## Files Modified

- `CMakeLists.txt` — added `enable_testing()`
- `src/wisdom-chess/engine/CMakeLists.txt` — include `doctest.cmake` module
- `src/wisdom-chess/engine/test/CMakeLists.txt` — replace `add_test()` with `doctest_discover_tests()`
- `.github/workflows/cmake.yml` — replace platform-specific test steps with `ctest`
