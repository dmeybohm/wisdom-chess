# Remove the GSL

Branch: `remove-gsl`, based on `single-nonnull`.

## Motivation

After `single-nonnull` replaced `gsl::not_null` with our own class, the
engine used five names from the GSL, each once, to define a `wisdom::`
name:

| GSL name | Defined | What the GSL provided |
|---|---|---|
| `gsl::narrow` | `narrow` | A round-trip check that throws `gsl::narrowing_error` |
| `gsl::narrow_cast` | `narrow_cast` | A `static_cast` |
| `gsl::czstring` | `czstring` | An alias for `const char*` |
| `gsl::zstring` | `zstring` | An alias for `char*` |
| `gsl::owner` | `owning` | An alias for `T*` |

Only `narrow` did anything, and `isLosslessConversion()` in `narrow.hpp`
already made the same check for constant expressions.

`gsl::owner` is an annotation for static analyzers: clang-tidy's
`cppcoreguidelines-owning-memory` and the owner rules of MSVC's Core
Guidelines checker. Neither runs on this project, and clang-tidy does
not see through an alias of it. Run on `owning<Thing> a = new Thing;`,
clang-tidy 18 reports "initializing non-owner 'owning<Thing>'".

For this the build fetched the library at configure time, and
`types.hpp`, which every file includes, pulled in all of `<gsl/gsl>`.

## Design

The names stay, so no caller changes.

- `czstring` and `zstring` are aliases for `const char*` and `char*`.
  Nothing uses `zstring`, but the linter's `raw-pointer` rule names both
  as what to write for a C string, so both stay.
- `owning<T>` is an alias for `T*`. It stays an alias: the feature log of
  `nonnull-observer-params` (2026/09) records that a class would change
  what moc and the rest of Qt see.
- `narrow_cast` is a `static_cast` at runtime. In a constant expression
  it still refuses a value that does not fit, as before.
- `narrow` checks with `isLosslessConversion()` and throws
  `PreconditionError` through `throwPreconditionError()`.

### What `narrow` throws

`gsl::narrowing_error` was a `std::exception` with no message, and not an
`Error`. So the handlers that catch `Error` missed it: the search did
not wrap it in a `SearchError` with the board, and the console's `main()`
did not catch it.

A value that does not fit is a broken precondition of the call, so
`narrow` now throws `PreconditionError`. It takes a
`std::source_location` that defaults to the caller, and the message
reads `Precondition failed at logger.cpp:148: narrow: the value fits in
the target type`. `EXPECTS` inside `narrow` would have named `narrow.hpp`
for every caller. `throwPreconditionError()` is called directly for that
reason, as the `switch` defaults that describe what was expected do.

In a constant expression the failing path calls a function that is not
`constexpr`, which is a compile error. The separate
`std::is_constant_evaluated()` branch, which threw `std::runtime_error`
for the same purpose, is gone.

## Implementation Progress

### Session #1

Removed the `CPMAddPackage` and the `Microsoft.GSL::GSL` link from
`engine/CMakeLists.txt` and the entry from `cpm-package-lock.cmake`.
`global.hpp` included `<gsl/gsl>` and used nothing from it. No file
turned out to depend on a standard header that came in through the GSL,
with libstdc++ or with libc++.

The `narrow` test checks the exception type, and two cases were added:
that the error names the caller's file, and that `narrow_cast` wraps at
runtime. The linter fixture `trailing-return-type/qualified-type.cpp`
spelled its qualified return type `gsl::czstring`; it is now
`wisdom::czstring`.

**Code size.** Bytes of `.text`, GCC 13, Release, against
`single-nonnull`:

| Binary | Before | After |
|---|---|---|
| `WisdomChessQml` | 492,434 | 492,386 |
| `wisdom-chess-uci` | 239,502 | 239,374 |
| `wisdom-chess-console` | 239,322 | 239,210 |

**Verification.**

- GCC 13, Release with the QML frontend, the slow tests, the tools and
  the benchmarks: 278 of 278 tests pass, `lint` is clean, and the
  linter's 58 fixture tests pass.
- GCC 13, Debug: the 236 fast tests pass.
- Clang 18 with AddressSanitizer and UndefinedBehaviorSanitizer,
  warnings as errors, QML frontend: 278 of 278.
- Emscripten: `wisdom-chess-web` builds and the 186 fast tests pass under
  Node.

Not run: Android, Windows and macOS builds, ThreadSanitizer, and the
React frontend in a browser. MSVC's standard library is the one most
likely to miss a header that the GSL used to include.
