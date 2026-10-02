# Draw arbiter

## Motivation

A `Game` runs the draw rules itself. When a draw can be claimed it
proposes one, the players answer, and a declined draw moves the limit
from the third occurrence of a position to the fifth, or from 100
halfmoves without progress to 150.

The search read those answers out of `History`
(`getThreefoldRepetitionStatus()`,
`getFiftyMovesWithoutProgressStatus()`) to choose its limits. That
ties the search to a negotiation only some callers hold.

UCI does not hold it. The GUI decides when a game is drawn; UCI uses
the `Game` to keep the position and to run a search, never calls
`getStatus()` and never answers a proposal. Its answers stayed at
`NotReached` for ever, which gave the right limits by accident. Where
the accident ran out, UCI played random moves; see
`search-root-draw-move.md`.

## Design

`DrawArbiter` says who decides that a game is drawn:

- `GameEngine`, the default: the game does. The limits follow the
  players' answers, as before.
- `External`: the caller does, and it names the limits. They hold
  whatever the answers are.

`Game::setExternalDrawArbiter()` takes the caller's `DrawLimits`.
`UciInterface` calls it on every game it builds, with
`Claimable_Draw_Limits`.

There are two named sets of limits, and no default:

- `Claimable_Draw_Limits`: the third occurrence and 100 halfmoves, at
  which a player may claim a draw.
- `Automatic_Draw_Limits`: the fifth occurrence and 150 halfmoves, at
  which the game is drawn without a claim.

Under `GameEngine` each limit is the claimable one until that draw is
declined, and the automatic one after.

The search is given limits, not the arbiter. `Game::getDrawLimits()`
turns the arbiter and the answers into a `DrawLimits`, and
`Game::findBestMove()` passes that to `IterativeSearch::create()`.
`probableDrawCategory()` and `isProbablyDrawingMove()` take the limits
as an argument and no longer read the answers from the history. The
search knows nothing of arbiters or proposals.

The limits are a required argument all the way down. A search built
without a `Game`, as in the tests, says which limits it wants.

### Why UCI uses the claimable limits

UCI does not tell the engine the GUI's rules. A GUI that adjudicates
ends the game at the third occurrence and at fifty moves, and under one
that does not the opponent may still claim there. With the automatic
limits the engine would count on a third occurrence being safe, and
give away a draw when ahead.

Under these limits the game does not end on the engine's side. A
position below the root that reaches a limit is scored as a draw and
not searched further; the root is always searched.

## What does not change

- No frontend behaves differently. Under UCI the answers were never
  given, so the claimable limits are the ones it already had. The value
  is that this is now stated in the UCI code, not incidental: a later
  change to how a `Game` infers or records an answer cannot reach
  UCI's search.
- The root is searched under either arbiter. That is
  `search-root-draw-move`, on which this branch is built.
- `Game::getStatus()` reports the same statuses under either arbiter.
  An external arbiter is free to ignore them, and UCI does not ask.
- `setProposedDrawStatus()` still records an answer under `External`.
  It has no effect on the search there.
- The draw score. `Search_Draw_Contempt` models a draw the engine could
  claim on its own move but would rather not. Under UCI the engine
  cannot claim, and a GUI that adjudicates ends the game either way.
  Scoring a draw differently under `External` may be right, but it
  changes how the engine plays and needs a match to justify.

## Possible follow-ups

- The answers live in `History` only because the search read them
  there. Now that only `Game` reads them, they could move into
  `Game::Impl` beside the per-player answers.
- The draw score under `External`, as above.

## Tests

- `game_test.cpp`, "The draw arbiter decides the limits of the search":
  the default, a declined draw raising its own limit, an external
  arbiter's limits holding whatever the answers, a copied game keeping
  its arbiter and limits, limits that are not positive being rejected,
  and `findBestMove()` choosing a different move under each arbiter in
  a position past 100 halfmoves with the draw declined.
- `search_test.cpp`, "The search applies the draw limits it is given":
  a position a rook down scores 0 at the limit of 100 and below 0 at
  150.
- The `probableDrawCategory()` tests for a declined draw, and the
  seventy-five move subcase of "A checkmate that completes the move
  count is not scored as a draw", now pass the limits instead of
  setting an answer on the history.

There is no UCI script test: its output is the same before and after.

## Implementation Progress

### Session #1

Created `draw-arbiter` in its own worktree from
`search-root-draw-move` at `99c0f39e`, with `origin/main` at `413df52a`
merged in for `seventy-five-move-checkmate`. Both touch the code
changed here.

Implemented the design above. With `findBestMove()` made to pass
default limits, the last subcase of the arbiter test failed (`a1 a6`
instead of `a1xa7`), so it does cover the hand-over to the search.

Release `ctest` with slow tests passed 273 of 273. The new and changed
tests pass in a Debug build, and the `lint` target is clean. The QML
tests were not built, because this machine has no Qt. The search speed
was not measured; the limits are two integers read where two status
comparisons were made before.

### Session #2

Merged `origin/main` at `088810a5`, after `search-root-draw-move`,
`remove-gsl` and `single-nonnull` were merged there. GitHub reported
conflicts with the pull request; the merge itself applied without any
to resolve by hand.

Release `ctest` with slow tests passed 276 of 276 on the merged tree.
The new and changed tests pass in a Debug build, and the `lint` target
is clean.

### Session #3

Made the limits explicit. `External` used to mean the default
`DrawLimits`, which were the claimable limits only because of the
struct's default member values, so nothing in the UCI code said what
rules it searched by.

- `DrawLimits` has no default values. `Claimable_Draw_Limits` and
  `Automatic_Draw_Limits` name the two sets.
- `Game::setDrawArbiter()` gave way to `setExternalDrawArbiter()`,
  which takes the limits and rejects ones that are not positive.
  `Game::Impl` holds them as an optional, so there is no external
  arbiter without limits.
- `IterativeSearch::create()`, `probableDrawCategory()` and
  `isProbablyDrawingMove()` require the limits. The tests, the fatal
  test and the search benchmark pass them.

Behaviour is unchanged. Release `ctest` with slow tests passed 276 of
276. The new and changed tests and the UCI script tests pass in a Debug
build, the `lint` target is clean, and the benchmarks target builds.
