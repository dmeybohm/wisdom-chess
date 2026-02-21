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

## Implementation Progress

### Session #1

All changes implemented and verified:
- CMake configure without FIL-C correctly defaults
  `WISDOM_CHESS_FILC_COMPAT` to `OFF`
- All 85 fast tests pass
- Committed as `b63a543`
