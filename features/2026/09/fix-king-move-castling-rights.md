# Fix castling rights surviving a king move

## Motivation

The deployed WASM build logged a series of `Uncaught <number>` errors
from `_wasmWorkerRunPostMessage` during a computer-vs-computer game. The
numbers are C++ exception pointers: Release WASM builds disable exception
catching, so a C++ throw escapes to JavaScript without its message.

## Root cause

`Board::updateAfterKingMove()` only cleared the castling rights when
`ableToCastle (who, Either_Side)` was true. Commit `5c560be` ("Flip
semantics of CastlingEligibility") changed that check from "either side is
eligible" to "both sides are eligible". Since then a king that moves with
only one castling right left keeps that right, including after castling
itself.

Move generation then offers castling from the king's new square, and the
engine can play it. The move crosses the worker boundary as the text
`O-O` / `O-O-O`, which the main thread parses as castling from the king's
home square: a different move. From then on the worker's and the main
thread's games disagree, and later moves throw when they are applied.

A random-game check reproduced this in 121 of 3.5 million generated moves
that failed to round-trip through `asString()` and `moveParse()`, all of
them castling moves from a king off its home square.

## Fix

Clear both castling rights on every king move, without the guard.
`removeCastlingEligibility()` masks the bits, so it is harmless when the
rights are already gone.

## Implementation Progress

### Session #1

- Added "King move removes the remaining castling right" to
  `castle_test.cpp`; it failed before the fix and passes after.
- Removed the guard in `updateAfterKingMove()`.
- All 138 tests pass. The random-game round-trip check now reports no
  mismatches in 3.6 million moves.
- Engine-vs-engine fuzzing of the WASM build through Node (worker
  messages, pauses, settings changes, new games) found no other
  exceptions.

### Session #2

- Checked the other uses of `Either_Side`. Everything else, tests
  included, already treats it as "both sides": it is set as the default
  eligibility, compared for equality in the FEN writer and in
  `unableToCastlePenalty()`, and has the value 3. `updateAfterRookMove()`
  also uses it as a "not a corner rook" placeholder that is never passed
  to `ableToCastle()`.
- Renamed it to `Both_Sides` so the name matches its meaning.
- Fixed 22 combined checks in `castle_test.cpp` that passed
  `Kingside | Kingside` where `Kingside | Queenside` was meant. Each now
  expects true only when the next assertion shows both rights remaining;
  the seven that follow a lost queenside right are negated.
- Found while debugging the new test in a Debug build:
  `FenParser::buildBoard()` ignores the side to move in the FEN, unlike
  `FenParser::build()`. Fixed in Session #3.

### Session #3

- `FenParser::buildBoard()` now sets the side to move from the FEN, as
  `FenParser::build()` already did. Added a test that failed before the
  change. The benchmarks that call `buildBoard()` read the side to move
  separately, so they were unaffected.
- `updateAfterRookMove()` now uses `optional<CastlingEligibility>` for
  "not a corner rook", as `updateAfterRookCapture()` does, instead of the
  `Both_Sides` placeholder.
- All tests pass: 116 in Debug, 139 in Release with the slow tests.
