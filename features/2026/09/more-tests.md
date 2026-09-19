# More tests

## Motivation

The "Tests" section of
[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md) has three
open items:

1. Perft asserts node counts for positions 1 and 2 only, and `MoveCounter`
   counts nodes, captures and en passant captures but not castles,
   promotions or checks.
2. There are no tests for `evaluate.cpp`, `game_status.cpp`,
   `move_timer.cpp`, `output_format.cpp`, the console UI, UCI, the
   view-model or the QML C++.
3. `generate_test.cpp` compares `asString()` output, which breaks whenever
   the text format or the generation order changes.

While reading the code for item 2 it also turned out that no test calls
`Game::status()`, the function every frontend uses to decide whether the
game is over.

This branch adds tests only. A defect a new test finds is recorded here and
fixed separately, unless the fix is needed for the test to exist.

## Plan

### 1. Perft

- Add `castles`, `promotions`, `checks` and `checkmates` to `MoveCounter`.
  A check is counted when the opponent's king is attacked after the move,
  and a checkmate when that opponent then has no legal move. The checkmate
  count exercises `isPlayerCheckmated` and `hasLegalMove` against published
  numbers, which the `faster-legal-move-test` change makes worth having.
- Add the full counter rows for positions 1 to 4 from
  <https://www.chessprogramming.org/Perft_Results>, and the node counts for
  positions 5 and 6, which is all that page publishes for them.
- Depths are chosen so that each new case stays under a few seconds. The
  kiwipete case already takes 17 seconds at depth 5 and is not extended.

### 2. Engine unit tests (fast suite)

- `evaluate_test.cpp`: checkmate and stalemate detection for both colors,
  `isLegalPositionAfterMove` including castling through and out of check,
  `evaluate` symmetry, the castling penalty, mate scores by distance,
  `evaluateWithoutLegalMoves`, `isProbablyDrawingMove` and `DrawCategory`.
- `game_status_test.cpp`: every `GameStatus` reaches the matching
  fine-grained method and the matching hook, and `Playing` reaches neither.
  `Game::status()` for each status it can return, including the accepted and
  declined draw proposals.
- `move_timer_test.cpp`: not triggered before `start()`, triggered by a zero
  budget, cancellation, the periodic function, and `start()` resetting the
  state. The timer only looks at the clock every 10,000 calls at the least,
  so the tests call `isTriggered()` in a loop and never sleep.
- `output_format_test.cpp`: both formats write what `Game::loadGame` and
  `FenParser` read back, and `Game::save` picks the format from the file
  name.

### 3. Move generation tests

Compare `Move` values, not text. Where order is not the point of the test,
compare as sets.

### 4. View-model

A new `wisdom-chess-viewmodel-tests` executable next to the library, with a
small concrete `GameViewModelBase` that records which change callbacks ran.
Covers the status messages, check detection, the draw proposal flow, change
callbacks firing only on a change, `isLegalMove`, `needsPawnPromotion` and
`getFirstHumanPlayerColor`.

### 5. UCI and console

Both read standard input and write standard output, so they are tested as
processes, the same way the fatal tests are: a CMake script feeds a file of
commands and matches the output against regular expressions. That works on
every CI platform without a test-only seam in the frontends.

For UCI the scripts end with `stop` before `quit`. `quit` and end of input
both supersede a running search, which then prints nothing; after `stop`
the search owes a `bestmove` and `quit` waits for it.

### Out of scope

The QML C++ classes need a Qt test harness and a GUI session in CI. That is
its own piece of work and stays open on the checklist.

## Implementation Progress
