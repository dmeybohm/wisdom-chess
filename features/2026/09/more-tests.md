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

### Session #1

Everything in the plan except the QML C++. The suite goes from 140 tests to
202 (174 fast, 28 slow). The slow suite's wall time is unchanged at about 17
seconds, because kiwipete at depth 5 is still the longest test.

- Perft. `MoveCounter` has `castles`, `promotions`, `checks` and
  `checkmates`. Every published counter for positions 1 to 4 matches, as do
  the node counts for positions 5 and 6, and the mirrored position 4 gives
  Black the same numbers. Depths: position 3 to 6, position 4 to 5,
  positions 5 and 6 to 4.
- Counting checks tests the king at every leaf, which took kiwipete at
  depth 5 from 17 to 29 seconds. `Stats::count_checks` switches it off. That
  one row leaves checks out, and so does `perftResults()`, which only
  reports nodes, so the `perft` tool is no slower than before.
- `evaluate_test.cpp`, `game_status_test.cpp`, `move_timer_test.cpp` and
  `output_format_test.cpp` as planned. The castling penalty is tested
  exactly, by subtracting the material and position scores from
  `evaluate()`. The timer tests take about 20 ms and never sleep.
- `Game::status()` had no test at all. It now has one for each status it
  returns, the precedence of checkmate over a draw by move count, and the
  draw proposal rules: one player claiming is enough, and nothing changes
  until both have replied.
- `generate_test.cpp` and the perft move-list test compare `Move` values.
- View-model: `wisdom-chess-viewmodel-tests` in `ui/viewmodel/test`, nine
  cases. `ViewModelSettings` is left untested, since the bug list has it
  down as unused and a candidate for removal.
- UCI (16 cases) and console (14 cases) run as processes through
  `wisdom_chess_add_cli_test()` in `cmake/CliTests.cmake`. They cover the
  regressions fixed by hand in the bug list's Sessions #14 and #16: `stop`
  is answered with a `bestmove`, a superseded search stays silent,
  `bestmove (none)` only when checkmated, a lowercase promotion letter, a
  capital `Y` at the draw prompt, an out-of-range number at the `maxdepth`
  prompt, and end of input without `quit`. The console scripts make both
  players human so that no search runs. Repeated 25 times at `-j 16` with no
  failure.
- The UCI cases are deterministic although a search thread is involved. The
  timer calls the periodic function, which is what notices `stop`, only
  every 10,000 nodes or more, and the depth-2 searches finish well inside
  that. The `go infinite` cases accept any legal move.
- Lesson from `main`, picked up in the rebase: `hasLegalMove()` and
  `isStalemated()` may only be asked about the side to move, because
  `Board::withMove()` asserts it in Debug. Three checks in the first draft
  of `evaluate_test.cpp` broke that rule and were removed. All fast tests
  were then run in Debug as well as Release.
- Mutation check: changing the castling penalty, swapping the stalemate
  dispatch, breaking the timer's comparison and dropping the newline in the
  move format made seven of the new test cases fail. Reverted afterwards.
- Verified: GCC Release (202 tests) and Debug (174 fast tests) with no
  warnings, linter clean. There is no system Clang on this machine, so the
  new and changed test sources were only syntax-checked with Emscripten's
  Clang 21 under `-Wall -Wextra`: no diagnostics. Not verified: MSVC and
  AppleClang, and the process tests on Windows, which only CI can show.

### Findings

Noticed while writing the tests. None is fixed here.

- `GameViewModelBase`'s class comment says `formatBold()` returns
  `<b>text</b>`. It returns `<strong>text</strong>`, which is what the
  tests pin.
- `updateDisplayedGameState()` clears `inCheck` and the game-over status
  before setting them again, so a second update in the same check or
  finished position fires `onInCheckChanged()` twice and
  `onGameOverStatusChanged()` twice, although nothing changed. In QML each
  of those is a signal.
- `GameViewModelBase::setProposedDrawStatus()` leaves the view-model's own
  `thirdRepetitionDrawStatus()` at `Proposed`; the QML frontend sets it
  separately. The tests do not pin that either way.
- UCI: `quit`, or the end of input, during a search prints no `bestmove`.
  That is within the protocol, but it means a script must send `stop`
  first.
- `makeOutputFormat()` looks for ".fen" anywhere in the path, not at the
  end, so a directory called `my.fen.games` turns every save into a FEN
  file.
