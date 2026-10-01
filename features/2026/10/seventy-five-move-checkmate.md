# Checkmate on the move that completes the seventy-five move count

## Motivation

When the move that brings the halfmove clock to 150 is also checkmate,
the game should be won, not drawn. Nothing tested that case.

FIDE's Laws of Chess say so directly:

- 9.6.2: the game is drawn when "any series of at least 75 moves have
  been made by each player without the movement of any pawn and without
  any capture. If the last move resulted in checkmate, that shall take
  precedence."
- 5.1.1: checkmate "immediately ends the game", so the same holds for
  the fifty-move rule (9.3), which is only a claim by the player to
  move.

Source: [FIDE Laws of Chess, 2023](https://handbook.fide.com/chapter/E012023).

## Findings

### The game status is right

`Game::getStatus()` tests `isCheckmated()` before any draw, so a mating
move at clock 149 gives `GameStatus::Checkmate`. The fifty-move case
already had a test ("Checkmate takes precedence over a draw by the move
count"); the seventy-five move case now has one beside it.

### The search was wrong

`IterativeSearchImpl::search()` and `quiesce()` call
`isProbablyDrawingMove()` on entering a node, before generating moves.
A node whose clock had reached the limit was scored as a draw, even
when the side to move there was checkmated. The limit is 100 halfmoves,
or 150 once the fifty-move draw has been declined.

So the engine did not see a mate in one on the last move of the count.
In

    6k1/5ppp/8/8/8/8/8/Rn2K3 w - - 149 110

Ra8 is mate, but the search scored it 0 and played Rxb1 instead, which
resets the clock. The same happened at clock 99 when the draw had not
been declined.

## Fix

`probableDrawCategory()` returns `NoDraw` for a checkmated board whose
clock has reached the limit. Both call sites in the search go through
it, so the node is searched as usual, finds no legal move and gets the
mate score from `evaluateWithoutLegalMoves()`.

- `isCheckmated()` runs only at a node whose clock has reached the
  limit, and looks for a legal move only when the king is in check.
  The cost has not been measured.
- The mated node no longer counts towards `my_draw_nodes`, so its
  parent's score may be stored in the transposition table. A mate score
  does not depend on the path.
- Repetition and insufficient material are unchanged. A stalemate at
  the limit is a draw either way.

## Tests

- `game_status_test.cpp`, "Checkmate takes precedence over the
  seventy-five move draw".
- `search_test.cpp`, "A checkmate that completes the move count is not
  scored as a draw", with a subcase for each limit. It failed before
  the fix: the search played Rxb1.
- `evaluate_test.cpp`, "A checkmate is not a draw by the move count",
  under the `probableDrawCategory()` tests.

## Implementation Progress

### Session #1

Created `seventy-five-move-checkmate` in its own worktree from
`origin/main` at `b64bf5b0`, and added the status and search tests. The
search test failed, and was pinned with `doctest::should_fail()`.

Release `ctest` with slow tests passed 268 of 268. Both new tests gave
the same results in a Debug build, and the `lint` target is clean. The
QML tests were not built, because this machine has no Qt.

### Session #2

Applied the fix, removed `should_fail()` from the search test and added
the `probableDrawCategory()` subcase.

Release `ctest` with slow tests passed 268 of 268 and the `lint` target
is clean. The three tests above pass in a Debug build; the rest of the
suite was not run there.

Loading a FEN whose clock is already past 100 is a separate matter, in
`fen-declined-fifty-move-draw.md`.
