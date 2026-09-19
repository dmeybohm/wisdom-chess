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
