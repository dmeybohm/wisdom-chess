# Consolidate the draw state in `Game`

Branch: `consolidate-draw-state`, based on `main`.

## Motivation

The state of a proposed draw was split across two classes. `Game::Impl`
held each player's answer, as a `BothPlayersDrawStatus` pair per
proposal, and `History` held the status those answers reduce to, as
`DrawStatus` fields with getters and setters. `Game` wrote the second
from the first once both players had replied, in
`updateThreefoldRepetitionDrawStatus()` and its fifty-move twin, and
read it back in `getStatus()` and `getDrawLimits()`.

The status lived in `History` because the search once read it there.
Since `draw-arbiter.md` (2026/10) the search takes `DrawLimits` from
`Game::getDrawLimits()` instead, and nothing but `Game` read or wrote
the history's copy. That log lists moving it as a follow-up.

The split had a cost beyond reading two files. The two halves could
disagree: `fen-declined-fifty-move-draw.md` notes that setting only the
history's status would leave the pair at `NotReached`, and the next
answer would recompute the status from the pair and overwrite it. And
the helpers that reduce the pair, `updateDrawStatus()`,
`drawStatusIsReplied()` and `bothPlayersReplied()`, were public in
`history.hpp` with only `game.cpp` using them.

## Design

`Game::Impl` is the one holder of the draw state: the two pairs of
answers. The status is derived from them, not stored.

- `Impl::drawStatus (ProposedDrawType)` returns the status of a
  proposal: `NotReached` until both players have answered, then
  `Accepted` when either claims the draw and `Declined` otherwise. That
  is the reduction `Game` already applied; it now runs on every read
  instead of once on the last answer, which is the same value since the
  pairs change only through `setProposedDrawStatus()`.
- `getStatus()` and `getDrawLimits()` ask `drawStatus()`.
  `setProposedDrawStatus()` only records the answer.
- `History` loses the two status fields and their accessors, and the
  pair helpers move into `game.cpp`'s unnamed namespace.
  `BothPlayersDrawStatus` goes with them: the public overload of
  `setProposedDrawStatus()` that takes both answers already spells
  `pair<DrawStatus, DrawStatus>`.
- `history.hpp` no longer includes `game_status.hpp`. `evaluate.hpp` and
  `search.hpp`, which use `DrawLimits` and got the header through it,
  include it themselves.

The public API of `Game` and the observable behaviour are unchanged.
`game_status_test.cpp` ("Nothing changes until both players have
replied", the accepted and declined cases) and `game_test.cpp` ("A
declined draw raises its own limit") cover both halves of the old
state through the new one.

## Implementation Progress

### Session #1 (2026-10-24)

- Made the change above. No test changes were needed: no test read the
  history's status or used the pair helpers.
