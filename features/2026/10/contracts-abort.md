# Contract checks abort

Step 3 of `exception-removal.md`. After step 1 no outside input reaches a
contract check, so a failed one is a bug, and it now aborts instead of
throwing.

## Design

- `EXPECTS` and `ENSURES` report through `logEmergency()` and abort, as
  the `_NOEXCEPT` forms did, which are gone. `expects()` and `ensures()`
  are `noexcept`.
- `narrow` and `to_unsigned` abort, as `narrow_noexcept` and
  `to_unsigned_noexcept` did, which are gone. The `_debug` forms are
  unchanged.
- `PreconditionError`, `PostconditionError`, `throwPreconditionError()`
  and `throwPostconditionError()` are gone. The enum conversions of the
  QML frontend (`ui_types.hpp`) and the WASM frontend (`web_types.hpp`),
  `pieceFromChar()` and the `FenParser` constructor call
  `terminateOnCheckFailure()` instead.
- `nullable::value()` aborts on null.
- The linter no longer knows the `_NOEXCEPT` macros.
- Tests: the doctest checks of a thrown precondition error became fatal
  cases, one per distinct check. Where several values broke the same
  condition, as -1 and 0 for `setMaxDepth()`, one case remains. The
  doctest cases keep what they checked besides the throw. There are 39
  fatal cases in Release, up from 14.
- `ChessEngine::guarded()` stays, for exceptions from the standard
  library. Its two QML tests triggered it with an out-of-range depth,
  which now aborts, so they throw through `runGuarded()` instead, a
  private member the test class reaches as a friend.
- `chess_game_test.cpp` no longer checks that out-of-range settings and a
  bad FEN throw. The engine's checks under them have fatal cases, apart
  from `GameSettings::applyTo()`'s `EXPECTS( isInRange() )`, whose
  library the fatal test program does not link.
- Not done: marking functions `noexcept` whose only throw was a contract
  check.

## Implementation Progress

### Session #1

- Made the change and converted the tests.
- Verified the Release build and its 277 fast tests, including the 39
  fatal cases, before the QML changes, and the QML build with Qt 6.11.2
  and its 8 test programs after them. Not run on this branch: the
  medium and slow tests, a Debug build, Clang, MSVC and Emscripten.
