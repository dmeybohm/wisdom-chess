# FIL-C Auto-Detection and Documentation

## Summary

Auto-detect the FIL-C compiler at CMake configure time so that
`WISDOM_CHESS_FILC_COMPAT` is enabled automatically, removing the
need for a manual `-DWISDOM_CHESS_FILC_COMPAT=ON` flag.

## Problem

When building with FIL-C, `WISDOM_CHESS_FILC_COMPAT` must be
enabled to disable POSIX signals in doctest (which FIL-C doesn't
support). This required passing the flag manually, which is easy
to forget.

## Design

### Detection Mechanism

FIL-C's compiler defines the preprocessor macro
`__PIZLONATOR_WAS_HERE__`. We use CMake's
`check_cxx_source_compiles` to test for this at configure time:

```cmake
check_cxx_source_compiles("
    int main() {
    #ifndef __PIZLONATOR_WAS_HERE__
        #error Not FIL-C
    #endif
        return 0;
    }
" WISDOM_CHESS_FILC_DETECTED)
```

When detected, `WISDOM_CHESS_FILC_DETECTED` is set to `ON`, which
becomes the default value for `WISDOM_CHESS_FILC_COMPAT`. The
option can still be manually overridden with
`-DWISDOM_CHESS_FILC_COMPAT=ON/OFF`.

## Changes

- **`CMakeLists.txt`**: Added `include(CheckCXXSourceCompiles)`,
  compile test for `__PIZLONATOR_WAS_HERE__`, and changed
  `WISDOM_CHESS_FILC_COMPAT` default from `OFF` to
  `${WISDOM_CHESS_FILC_DETECTED}`.
- **`README.md`**: Added "Building with FIL-C" section with
  build instructions and manual override examples.
- **`CLAUDE.md`**: Updated build options table to reflect
  auto-detection.

## CI Integration

A dedicated `build-filc` job was added to
`.github/workflows/cmake.yml` to build and test with FIL-C on
every push and PR to `main`.

### Job Details

- **Runs on**: `ubuntu-latest`
- **Depends on**: `lint` (same as the main build matrix)
- **FIL-C version**: pinned to `0.678` via `FILC_VERSION` env var
- **Cache**: FIL-C installation is cached by version to avoid
  re-downloading on every run

### Build Configuration

- Console UI, QML UI, slow tests, and linter are all disabled
- Fast tests are enabled
- `CMAKE_CXX_COMPILER` is set to `fil++` — no C compiler is
  needed since the project declares `project(WisdomChess CXX)`
- `WISDOM_CHESS_FILC_COMPAT` is auto-detected (no need to pass
  it explicitly)

### Setup Steps

On cache miss, the job installs `patchelf` (required by FIL-C's
`setup.sh`), downloads the FIL-C tarball, extracts it, and runs
`setup.sh` to patch the binaries for the runner's environment.

## Implementation Progress

### Session #1

All changes implemented and verified:
- CMake configure without FIL-C correctly defaults
  `WISDOM_CHESS_FILC_COMPAT` to `OFF`
- All 85 fast tests pass
- Committed as `b63a543`

### Session #2

Added FIL-C CI job to GitHub Actions:
- New `build-filc` job in `.github/workflows/cmake.yml`
- Downloads and caches FIL-C v0.678, builds with `fil++`, runs
  fast tests
- Updated this feature log with CI integration details
