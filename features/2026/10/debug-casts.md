# Debug-checked conversions in place of `_cast`

## Motivation

`narrow_cast` and `to_unsigned_cast` were `static_cast`s with a name: no
build ever checked them, though they sit on the hottest paths (`Coord`,
`Move`, `ColoredPiece`, the move ordering tables). Checking them always
would add a compare and a failure call at every inlined site. A form that
checks where `ASSERT` does keeps Release unchanged and gives the Debug
test runs, including the Debug CI job, a check on every one.

GSL's `narrow_cast` is unchecked and its name does not say so; LLVM's
`cast<>` checks only in assertion builds and its name does not say that
either. Rust's `debug_assert!` and Chromium's `DCHECK` put the build mode
in the name, which is what the `_debug` suffix does here. It means what
`Debugging` in `error.hpp` means: `NDEBUG` is unset, so Release and
RelWithDebInfo do not check.

## Design

- `narrow_debug` and `to_unsigned_debug` replace `narrow_cast` and
  `to_unsigned_cast`. They terminate like the `_noexcept` forms, but only
  when `Debugging` is on; in a constant expression a value that does not
  fit is a compile error in every build, as before.
- `widen`, `widen_noexcept` and `widen_cast` become one `widen`, with
  `widen_cast`'s rule: a strictly wider target, and a signed target for a
  signed source. It cannot fail, so it needs no other form. A checked
  signed-to-unsigned widening is `to_unsigned`.
- `toLower()` in the UCI frontend narrowed `std::tolower`'s `int` to
  `char`, which wraps a byte above 127 on purpose. It is a plain
  `static_cast<char>` now, as in `str.hpp`, so Debug builds do not abort
  on non-ASCII input.
- Unsigned-to-signed conversions, mostly container sizes into `int` or
  `ptrdiff_t`, stay with `narrow`, which checks the value at any width.

## Implementation Progress

### Session #1

- Replaced the `_cast` forms and consolidated `widen`, with tests and two
  fatal cases that run when `Debugging` is on.
- The Release disassembly of 88 of 90 project objects is identical before
  and after; the other two are the test files that changed.
- Checked that a signed-to-unsigned or same-width `widen`, and a constant
  that does not fit a `_debug` form, no longer compile with `NDEBUG`.
- Verified lint, all 294 Release tests and the 260 fast Debug tests. The
  QML frontend was not configured; its two renamed calls are left to CI.
