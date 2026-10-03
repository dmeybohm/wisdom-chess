# `noexcept` functions that can throw

## Motivation

A review of every engine header for missing `noexcept` (see
`move-list-noexcept.md` for the first instalment) turned up something
more pressing than missing markers: four functions carry `noexcept`
while their bodies can throw, and two throw sites that can never fire
formally taint the move-generation and draw-detection chains above
them. A throw out of a `noexcept` function is `std::terminate`.

This branch fixes those six sites and adds the helper they need. The
mechanical work of marking the roughly 190 functions that cannot throw
is left for a later branch, so that this one, which changes behaviour
under failure, is reviewed on its own.

## `noexcept_narrow()`

Five of the six sites call `narrow<T>()` on a value that always fits:
a masked bit field, or an iterator difference bounded by the board.
`narrow` is the throwing form. Replacing it with `narrow_cast` would
make the functions `noexcept` but drop the check, and a corrupted value
would be silently truncated.

`noexcept_narrow<Target> (value)` in `narrow.hpp` keeps the check:
it is `constexpr` and `noexcept`, and a value that does not fit aborts
through `terminateOnCheckFailure()`, as `NOEXCEPT_EXPECTS` does, with the
caller's location in the message, as `narrow` reports it. The name
follows `noexcept_expects`. The three conversions then read:

- `narrow`: throws `PreconditionError`. For caller input.
- `noexcept_narrow`: aborts. For an invariant inside a `noexcept`
  function.
- `narrow_cast`: no check. For a conversion the caller has proven.

Tests: a `global_test.cpp` case for the in-range values and a
`Fatal: noexcept-narrow-overflow` case for the abort, next to the
`narrow` cases.

## The four `noexcept` defects

- `BoardCode::asString()` returns a `std::string` built by
  `std::bitset::to_string()`, which allocates. The `noexcept` comes off.
  Its one caller is `operator<<`, which is not `noexcept`.

- `BoardCode::decodeEnPassantTarget()` calls `narrow<int8_t>` on a value
  masked to 0..7. It becomes `noexcept_narrow`. Every `noexcept` en
  passant getter on `BoardCode` and `Board` sits above it, and through
  `classifyEnPassantTarget()` so do `makeMove()` and `withMove()`.

- `makeCastlingEligibilityFromInt()` wraps `narrow<uint8_t>`. It is not
  `noexcept` itself, but its only engine caller,
  `BoardCode::getCastleState()`, masks the argument to two bits, and the
  `noexcept` functions `Board::getCastlingEligibility()`,
  `ableToCastle()`, `removeCastlingEligibility()`,
  `updateAfterRookMove()`, `updateAfterRookCapture()` and `makeMove()`
  all reach it. It becomes `noexcept_narrow`, and the function is marked
  `noexcept`. A value above 3 already aborted in the
  `CastlingEligibility` constructor's `NOEXCEPT_EXPECTS`; a value above
  255 now aborts too, in `noexcept_narrow`, where it threw before.

- `MoveTimer::setPeriodicFunction()` copy-assigns a `std::function` into
  an `optional`. The frontends pass capturing lambdas, so the copy can
  allocate and throw `bad_alloc`. The `noexcept` comes off.
  `Game::setPeriodicFunction()` above it is not `noexcept`, and the QML
  callers are already `noexcept` slots whose bodies run under
  `guarded()`, so nothing else changes.

## The two throw roots

- `needPawnPromotion()` in `generate.cpp` ends with `default: throw
  Error`, immediately after `ASSERT( isColorValid (who) )`. That one
  branch is the only reason `generateAllPotentialMoves()`,
  `generateCaptures()`, `generateLegalMoves()`, `hasLegalMove()`,
  `isCheckmated()`, `isStalemated()`, `mapCoordinatesToMove()` and
  `Game::getStatus()` are formally throwing. The `ASSERT` becomes a
  `NOEXCEPT_EXPECTS`, the switch becomes a conditional expression, and
  the function is marked `noexcept`.

- `Board::findFirstCoordWithPiece()` does `narrow<int>` on an iterator
  difference that is at most 64. It is reached from
  `Material::checkmateIsPossible()` and so from `probableDrawCategory()`
  and `isProbablyDrawingMove()`. It becomes `noexcept_narrow`, and the
  function is marked `noexcept`.

Neither is reachable with a valid board. The change is to what the
compiler and the type traits can see, and to what happens on a corrupted
color: an abort with the condition in the message, rather than an
`Error` nobody catches.

## Tests

- `Fatal: need-pawn-promotion-bad-color`, since the throw it replaces
  had no test and the abort is now the contract.
- `Fatal: noexcept-narrow-overflow` and the `global_test.cpp` case
  above.
- `castling_eligibility_test.cpp` and `pawn_promotion_test.cpp` keep
  their existing cases; neither tested the throwing paths.

## Follow-up: a `noexcept` `Logger`

Not in this branch; recorded here because the review raised it.

`Logger::debug()`, `info()` and `emergency()` are the engine's only
virtuals a `noexcept` function might want to call, and today none is
`noexcept`. Marking the three pure virtuals would let any context log,
and would make the design decision explicit: a logger that cannot get
its message out terminates the process rather than turning its caller
into a throwing function.

What it would take:

- Every override gains `noexcept`: `NullLogger`, `StandardLogger` and
  `BufferedLogger` in the engine; `UciLogger`, the QML
  `ChessEngineLogger` and the WASM `WebLogger`; and the test loggers in
  `logger_test.cpp`, `search_test.cpp` and `fatal_test_main.cpp`.
- `debug()` and `info()` bodies need no `try`/`catch`. An exception
  escaping a `noexcept` function reaches `emergencyTerminateHandler()`,
  which reports it through `logEmergency()` and aborts; the
  `Fatal: expects-through-noexcept` case pins that path. The real
  throwers are only `bad_alloc` from string building and, in
  `BufferedLogger`, the six `narrow` calls in `formatLogTimestamp()`
  and `LogRingBuffer::push()`, which become `noexcept_narrow`.
- `emergency()` is different: the process is already dying, and the
  message should reach as many sinks as it can. `StandardLogger::emergency()`
  would swallow a stream failure in a `try`/`catch`, as
  `BufferedLogger::emergency()` already does for its timestamp.
- Two `logger_test.cpp` cases assume `emergency()` may throw: the
  `ThrowingLogger` subcase "a logger that throws does not let the
  exception escape" becomes impossible by contract and goes; the
  `std::cerr`-throws fixture stays valid once `StandardLogger` swallows.
- The QML and WASM frontends have to be built to verify it, so it is a
  branch of its own.

## Out of scope

- Marking the functions that cannot throw and lack `noexcept`. A later
  branch, done bottom-up from `coord.hpp` and `piece.hpp`.
- The `Logger` virtuals, as above, and the `GameStatusUpdate` virtuals,
  whose overrides are the frontends'.
- `MoveTimer::isTriggered()`, which runs a caller-supplied function and
  genuinely can throw.

## Implementation Progress

### Session #1

The log was written first and reviewed. Two suggestions from the review
shaped it: `noexcept_narrow()` in place of a plain `narrow_cast`, so the
checks stay, and a `noexcept` `Logger`, kept for its own branch and
recorded above.

Implemented as written. The `main` the branch starts from is the
layering-improvements merge.

Verification: Release and Debug builds compile without a warning, the
linter passes, the thirteen `Fatal: ...` cases pass in Debug, the two
new ones among them, and all 280 tests pass in Release. The QML and
WASM frontends were not rebuilt; the only header they include that
changed its signature is `move_timer.hpp`, where `noexcept` came off a
function their `noexcept` callers already wrap.
