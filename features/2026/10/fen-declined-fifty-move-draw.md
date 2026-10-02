# Treat the fifty-move draw as declined in a game that starts past it

## Motivation

A game played in this engine only gets past a halfmove clock of 100 if
both players declined the fifty-move draw. FEN carries the clock but
not that answer, so a game built from a FEN whose clock is above 100
starts with the draw status at `NotReached`. No game reached by play
can be in that state.

Three things follow, with `4k3/8/8/8/8/8/8/R3K3 w - - 120 90` as the
example:

- `Game::getStatus()` returns `FiftyMovesWithoutProgressReached`, so
  the draw is proposed again as soon as the position is loaded.
- At a clock of 150 or more the status is still
  `FiftyMovesWithoutProgressReached`. `getStatus()` tests the fifty-move
  block first and only reaches the seventy-five move test once the draw
  has been declined, so a game that is already drawn asks whether to
  draw.
- The search scores every move that is not a capture or a pawn move as
  a draw, because the limit it applies is still 100.

## Proposal

When a `Game` is built on a board whose halfmove clock is above 100,
start it with the fifty-move draw declined by both players.

The cut-off is strictly above 100. At exactly 100 the claim has only
just become available and nobody has answered, so the draw is still
proposed.

This is an inference. Under FIDE 9.3 a player may claim on any later
move while the last fifty moves made no progress, so a clock of 120
does not show that anyone gave the claim up. The engine already treats
a declined draw as final until the seventy-five move limit, and the
proposal follows the engine's model, not FIDE's. Someone who loads a
FEN at clock 120 is no longer offered the draw.

## Where it goes

Every `Game::Impl` constructor delegates to the one taking a builder,
the players and the turn (`game.cpp:33`), and `FenParser::build()`
reaches it through `Game::createGameFromBoard()`. The check goes there,
after the history is created. It covers `createGameFromBoard()` as well
as `createGameFromFen()`.

It has to set both halves of the state:

- `Impl::fifty_moves_without_progress_draw`, the answer of each
  player, to `Declined` for both.
- The history's status, through
  `updateFiftyMovesWithoutProgressDrawStatus()`.

Setting only the history would leave the pair at `NotReached`, and the
next `setProposedDrawStatus()` would recompute the status from it.

Not in `FenParser` or `History::fromInitialBoard()`: the parser has no
`Game` state to set, and the history holds only half of it.

## Who is affected

`Game::createGameFromFen()` has three callers outside the tests and
benchmarks:

- The console's FEN prompt (`play.cpp:353`). This is where a person
  can load such a position, and where the repeated proposal shows.
- UCI's `position fen` (`uci_interface.cpp:302`). See below.
- QML's `ChessGame::clone()`, which copies the game to the engine
  thread as FEN. It is called when a `GameModel` is constructed and in
  `restart()`, both with a new game, so no declined draw is lost there
  today. Later answers reach the engine thread by signal.

The React frontend does not load FEN.

`Game::load()` replays a saved move list on a standard game. A replay
that crosses 100 ends at `NotReached` too, and the constructor does not
reach it, because the clock is 0 when the game is constructed. It makes
the same inference once the replay is done.

## UCI

Checking the above turned up that UCI played a random move in any
position where a draw could be claimed, because the search returned no
move for a root that was a draw. That is fixed on its own branch; see
`search-root-draw-move.md`. It does not depend on this change.

On its own this change would still reach UCI through `position fen`:
with a clock above 100 the search would apply the limit of 150 instead
of 100. The `draw-arbiter` branch closes that. UCI's games have an
external arbiter, whose limits ignore the players' answers, so the
inference only affects a game that arbitrates its own draws. This
change is to be built on that branch; see `draw-arbiter.md`.

## Plan

1. Failing tests first, in `game_status_test.cpp`:
   - a FEN at clock 100 is `FiftyMovesWithoutProgressReached`;
   - at 101 it is `Playing`;
   - at 150 it is `SeventyFiveMovesWithoutProgressDraw`;
   - at 120, one player's answer to a fifty-move proposal leaves the
     status at `Playing`;
   - a `BoardBuilder` with a clock of 120 behaves like the FEN.
2. A failing test in `game_test.cpp`: `getDrawLimits()` gives 150
   halfmoves for a game built from a FEN at clock 120, and 100 for the
   same game under `DrawArbiter::External`.
3. A failing test under "Loading a saved game" in `game_test.cpp`: a
   saved game whose replay ends above 100 loads as `Playing`.
4. Add the check as a `Game::Impl` member function, called from the
   main constructor and from `Game::load()` after the replay.
5. Leave the tests that decline explicitly after loading a clock above
   100 (`game_status_test.cpp`, `evaluate_test.cpp`,
   `game_viewmodel_base_test.cpp:215`). They still pass; the explicit
   answer becomes redundant.
6. Say in the comment on `createGameFromFen()` in `game.hpp` what a
   clock above 100 means.
7. Run the Release `ctest` with slow tests, the new tests in Debug, and
   the `lint` target.

## Decisions

- `Game::load()` makes the same inference after its replay.
- The UCI defect is fixed first, on `search-root-draw-move`.

## Implementation Progress

### Session #1

Created `fen-declined-fifty-move-draw` in its own worktree from
`origin/main` at `b64bf5b0`. Wrote this plan. No code or tests have
been changed yet.

### Session #2

Recorded the two decisions above and reworked the plan around them:
`Game::load()` is in, and the search test gave way to one on
`probableDrawCategory()`, since `search-root-draw-move` makes
`findBestMove()` return a move here on its own. Still no code or tests
changed.

### Session #3

`draw-arbiter` now exists, and the plan builds on it: the UCI section
says the inference stops at an external arbiter, and the second test
is on `Game::getDrawLimits()`, which replaced the status the search
used to read from the history. Still no code or tests changed.
