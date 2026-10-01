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
- The search has nothing to say. `search()` tests
  `isProbablyDrawingMove()` on the root as on any other node, returns
  the draw score at ply 0, and `Game::findBestMove()` comes back empty.

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
that crosses 100 ends at `NotReached` too, and this change does not
reach it, because the clock is 0 when the game is constructed.

## Related finding: UCI plays random moves in a claimable draw

Found while checking the above, with the UCI binary of the
`seventy-five-move-checkmate` branch. Not part of this change.

When the search returns no move, `UciInterface` sends
`pickRandomLegalMove()` (`uci_interface.cpp:412`). UCI never answers a
draw proposal, so both statuses stay at `NotReached` and the root is a
draw from the moment one can be claimed:

- `position fen 6k1/5ppp/8/8/8/8/8/R3K3 w - - <clock> 90`, then
  `go depth 3`, three runs each. Ra8 is mate. Clock 98: `a1a8` every
  time. Clock 100: `a1a2`, `a1d1`, `a1a6`. Clock 120: `a1a8`, `a1c1`,
  `e1f1`.
- `position startpos moves g1f3 g8f6 f3g1 f6g8 g1f3 g8f6 f3g1 f6g8`,
  the third occurrence of the starting position: seven different moves
  in eight runs. After one shuffle fewer it is `b1c3` every time.

This proposal fixes only the FEN case above 100. A clock of exactly
100, a game that reaches 100 through `moves`, and a third repetition
stay random. UCI has no way to claim a draw, so it should decline both
draws whenever it builds a game. With that in place UCI no longer
depends on this change.

## Plan

1. Failing tests first, in `game_status_test.cpp`:
   - a FEN at clock 100 is `FiftyMovesWithoutProgressReached`;
   - at 101 it is `Playing`;
   - at 150 it is `SeventyFiveMovesWithoutProgressDraw`;
   - at 120, one player's answer to a fifty-move proposal leaves the
     status at `Playing`;
   - a `BoardBuilder` with a clock of 120 behaves like the FEN.
2. A failing test in `game_test.cpp`: `findBestMove()` returns a move
   for a FEN at clock 120.
3. Add the check to the main `Game::Impl` constructor.
4. Leave the tests that decline explicitly after loading a clock above
   100 (`game_status_test.cpp`, `evaluate_test.cpp`,
   `game_viewmodel_base_test.cpp:215`). They still pass; the explicit
   answer becomes redundant.
5. Say in the comment on `createGameFromFen()` in `game.hpp` what a
   clock above 100 means.
6. Run the Release `ctest` with slow tests, the new tests in Debug, and
   the `lint` target.

## Open questions

- Should `Game::load()` make the same inference after its replay?
- Should the UCI fix come first? It is the larger defect, and it takes
  UCI out of this change's reach.

## Implementation Progress

### Session #1

Created `fen-declined-fifty-move-draw` in its own worktree from
`origin/main` at `b64bf5b0`. Wrote this plan. No code or tests have
been changed yet.
