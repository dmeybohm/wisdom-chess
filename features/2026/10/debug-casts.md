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
  `char`, which wraps a byte above 127 on purpose, so `narrow_debug`
  would abort Debug builds on non-ASCII input. It calls
  `wisdom::toLower (char)` from `str.hpp` instead, which needs no
  conversion and maps the same characters, since the program runs in
  the "C" locale.
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

### Session #2

Measured what the Debug CI job, which runs only the fast tests, checks,
with a Debug `--coverage` build and `gcovr`:

| Run | Wall time | Engine lines |
|---|---|---|
| Fast tests | 30 s | 92.3% |
| Slow tests | 17 min, 13.5 of it kiwipete perft | 68.3% |
| `search_test.cpp` alone | 13 s | |

- The fast tests run every one of the engine's 99 `ASSERT` and `_debug`
  sites, the busiest millions of times. The whole slow suite passes in
  Debug as well, so no other conversion wraps on purpose.
- What the fast tests missed was the search's harder paths: quiescence's
  evasion limit, a stalemate inside quiescence, the clock stopping inside
  quiescence, and keeping a partial root result. `search_test.cpp` covers
  18 of the 25 engine lines the slow suite adds, all but one of them in
  the search; perft adds only some parsing and two `Board` lines.
- `search_test.cpp` had been in the slow executable since 2020, which
  `error-hygiene.md` notes keeps it out of the Debug job. It takes 7.75 s
  in Release run serially, 7 s of it one test that runs on the clock.
  It moves to the fast executable, so the Debug, sanitizer, WASM and
  Android jobs run it too. The Debug fast suite is 281 tests in 20.6 s.
- Still uncovered in `search.cpp`: line 398, the clock stopping after a
  quiescence child returns, and the unused
  `IterativeSearch::getMoveTimer()`.
- Verified lint, all 294 Release tests and the 281 fast Debug tests.
