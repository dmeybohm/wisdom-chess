# Error hygiene: open items from the bug list

Branch: `error-hygiene`.

## Motivation

[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md) still has
open entries about error handling. The `emergency-logger` branch has since
given every frontend a terminate handler that reports an uncaught
`wisdom::Error`, with its message and extra info, through `logEmergency()`
before aborting. The engine library therefore no longer needs to end the
process itself to get a useful report.

## Plan

### Search errors propagate as `SearchError`

`IterativeSearchImpl::iterativelyDeepen` (`engine/search.cpp`) catches
`Error`, reports it with the board through `logEmergency()` and aborts. A
library should not decide that for its caller, and the path cannot be tested
inside doctest.

- Add `class SearchError : public Error` to `engine/search.hpp`.
- The catch block throws a `SearchError` with the original message, and an
  extra info holding the original extra info followed by the board being
  searched. The original dynamic type is lost; its text is kept.
- No frontend changes. The UCI search thread, the QML engine slot and the
  wasm worker reach the terminate handler, which already prints message and
  extra info, so the board is still reported. The console's `main()` already
  catches `Error`. Recovering from a failed search in UCI is a possible
  follow-up.
- Considered and rejected: deleting the `try`/`catch`. It keeps the original
  exception type but drops the board from the fatal report.
- Tests: a doctest case with a periodic function that throws, expecting
  `SearchError` with the message, the extra info and the board; and a
  `search-error` fatal test that lets it escape to the terminate handler.

### `Error` copy that cannot throw

`Error`'s copy constructor is `noexcept` but copies two strings
(`engine/global.hpp`). An allocation failure there terminates.

- Hold the text in shared immutable storage, as `std::runtime_error` does: a
  private `struct Text` with the two strings, behind a
  `shared_ptr<const Text>`.
- Remove the hand-written copy constructor. The implicit copy only bumps a
  reference count.
- The constructors allocate, so they lose their `noexcept`.
- Tests: `static_assert`s on `is_nothrow_copy_constructible`, and a copy that
  outlives the original.

### `Board::dump()`

It writes to `std::cerr` because it is a helper to call from a debugger.
Comment the declaration and close the item; no code change.

### `no-tabs` linter rule

The mixed tabs and spaces item suggested a linter rule. Add `no-tabs` to
`scripts/linter/rules/`, register it, give it `Severity::Error`, and add
self-tests. The source tree has no tabs today.

### Records

Check the items off in the bug list, and document `SearchError` and the new
rule in `AGENTS.md`.

## Implementation Progress

### Session #1

- `iterativelyDeepen` throws `SearchError` instead of reporting and
  aborting. `search.cpp` no longer needs `<cstdlib>`.
- The tests make the search fail with a logger whose `info()` throws, not
  with a throwing periodic function as first planned. The move timer calls
  the periodic function only every 10,000 to 1,000,000 checks, and the
  interval is adapted at run time, so that test would have needed a deep
  search and still not been deterministic. The logger is called at the top
  of the first depth, inside the same `try`.
  - "An error during the search is rethrown with the board" in
    `engine/test/search_test.cpp` checks the message and that the extra info
    is the original extra info followed by the board. `search_test.cpp` is
    part of the slow suite, so the Debug CI job does not run it.
  - `Fatal: search-error` lets the exception escape and requires the
    emergency report to hold the message, the extra info and the board. It
    is a fast test and runs in Debug too. The report is the same text the
    old abort produced.
  - Mutation check: with the old abort restored, the doctest case fails
    with "Subprocess aborted".
- `Error` holds its text in a `shared_ptr<const Text>`. The copy
  constructor and copy assignment are declared as defaulted and `noexcept`.
  Declaring them also keeps the class without a move constructor, as it was
  before: a move would leave the pointer empty and `message()` would then
  dereference null. No subclass constructor was `noexcept`. Tests in
  `engine/test/global_test.cpp`.
- `Board::dump()` has a comment saying it is for a debugger.
- `no-tabs` linter rule with `clean` and `violation` self-tests. It reports
  one violation per line, at the first tab. The `lint` target stays clean.
- `AGENTS.md` lists the rule and says that a failed search throws
  `SearchError`.
- The "Fatal sites" section of [emergency-logger.md](emergency-logger.md)
  describes the search catch-all as reporting and aborting; this branch
  supersedes that. `terminateOnPreconditionFailure` is now the only place
  where the engine ends the process itself.
- Verified: Release build (GCC) with no warnings and all 205 tests passing
  (176 fast, 29 slow); Debug build with no warnings and the 176 fast tests
  passing; QML build against Qt 6.11.2 with no warnings and its 34 `QML`,
  `UCI` and `Console` tests passing; the wasm target builds under Emscripten
  with no warnings; 23 linter self-tests pass. Not verified: MSVC and
  AppleClang, which only CI can show, and the wasm build in a browser.
- Possible follow-up: the UCI search thread could catch `SearchError`,
  report it as `info string` lines and still answer `bestmove`, instead of
  ending the process.
