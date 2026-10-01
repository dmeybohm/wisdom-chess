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

### The search is wrong

`IterativeSearchImpl::search()` and `quiesce()` call
`isProbablyDrawingMove()` on entering a node, before generating moves.
A node whose clock has reached the limit is scored as a draw, even when
the side to move there is checkmated. The limit is 100 halfmoves, or
150 once the fifty-move draw has been declined.

So the engine does not see a mate in one on the last move of the count.
In

    6k1/5ppp/8/8/8/8/8/Rn2K3 w - - 149 110

Ra8 is mate, but the search scores it 0 and plays Rxb1 instead, which
resets the clock. The same happens at clock 99 when the draw has not
been declined.

The defect is not fixed here. A fix would check for checkmate only at
a node `probableDrawCategory()` reports as `ByNoProgress`, which is
rare enough not to cost anything in the search. A mate score does not
depend on the path, so such a node would not count towards
`my_draw_nodes`.

## Tests

- `game_status_test.cpp`, "Checkmate takes precedence over the
  seventy-five move draw": passes.
- `search_test.cpp`, "A checkmate that completes the move count is not
  scored as a draw", with a subcase for each limit: fails, and is pinned
  with `doctest::should_fail()` so that fixing the search fails the
  test until the decorator is removed.

## Implementation Progress

### Session #1

Created `seventy-five-move-checkmate` in its own worktree from
`origin/main` at `b64bf5b0`, and added the two tests above.

Release `ctest` with slow tests passed 268 of 268. Both new tests gave
the same results in a Debug build, and the `lint` target is clean. The
QML tests were not built, because this machine has no Qt.
