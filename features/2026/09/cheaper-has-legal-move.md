# A cheaper `hasLegalMove()`

## Motivation

`quiesce()` calls `hasLegalMove()` at every node that is not in check,
to tell a stalemate from a position it may stand pat on (Session #5 of
[review-fixes.md](review-fixes.md)). The function generated every
pseudo-legal move and sorted them, to try one.

This is item 3 of
[improvement-suggestions.md](improvement-suggestions.md), which lists
nine options and the measurements behind the choice. That document is
on the `improvement-suggestions` branch and is not in `main` yet. What
it measured, on `main` at `6e2ef7f`:

- The test was about 70% of the search time: the six report positions
  took 630.7s at depth 8, and 188.2s with the call removed.
- Each call generated and sorted 18 to 45 moves and ran 1.00 to 1.08
  legality tests. The first move tried was legal 95 to 99.9% of the
  time.
- No node out of 293.6 million was a stalemate.

## Design

Option 3 of that document, with option 2 as the fallback. Both are in
`hasLegalMove()`; the search changes only in passing what it knows.

1. **Moves that need no test.** When the king is not in check, a move
   can only leave it attacked by opening one of its lines. So a move is
   legal without a test when the piece is not the king, its square
   shares no row, column or diagonal with the king, and the move is not
   an en passant capture, which takes a second pawn off another square.
   The first such move answers the question, with no board copy and no
   threat test.
2. **One piece at a time.** Otherwise, and always when in check, the
   moves of one piece are generated, unsorted, and tested before the
   next piece is looked at. The king goes first, since most evasions
   are moves of the king.

`hasLegalMove (board, in_check)` takes what the caller already knows,
and `quiesce()` has it. A Debug build asserts that it is right.
`hasLegalMove (board)` finds it out and calls the other.

The line test does not look for blockers or for an attacker on the
line, so it sends some moves to the fallback that pin detection would
have passed. The probe found that it answers 99.86% of the nodes or
more in five positions and 92% in position 3.

Not done here: the call still runs at every node not in check. The
capture loop as the proof (option 5) and a callback in place of the
move list (option 9) stay in the other document, with staged move
picking.

## Testing

- The tests of `hasLegalMove()` gain a stalemate whose only movable
  piece is pinned, a blocked pawn away from the king's lines, the same
  pawn with a capture, and an en passant capture that would expose the
  king.
- A fast test compares both overloads with `generateLegalMoves()` at
  every position of a tree two plies deep, from nine positions.
- A slow test does the same three to five plies deep: 338,677
  positions, 161 of them checkmates and 94 stalemates. It fails if the
  trees contain neither.
- Both run in Debug, where `withMove()` asserts the side to move.

## Implementation Progress

### Session #1

- Branched from `origin/main` at `132cd5e`.
- Implemented the design in `generate.cpp`, and passed `in_check` from
  `quiesce()`.
- The first version of the tree test took 26 seconds in Debug, twice
  the rest of the fast suite. The deep trees went to the slow tests and
  the fast test walks two plies.
- One of the endgame positions first written for the tests was illegal,
  with the side not to move in check. Release passed it; Debug aborted
  in `makeMove()` when the walk took the king. The same lesson as
  Session #3 of [faster-legal-move-test.md](faster-legal-move-test.md).
- Verified: Release, 262 of 262 tests; Debug, 227 of 227 fast tests;
  no warnings; the lint target passes. Qt was not configured, so the
  QML tests did not run.
- Measured with `--search-report 6`, seven alternating rounds, each run
  pinned to one core, medians. "Without the test" is the build from
  [improvement-suggestions.md](improvement-suggestions.md) that removes
  the call at nodes not in check. The moves, scores and node counts
  were the same in all three builds, at every depth.

  | Position | Move | Score | Nodes | Before | After | Faster by | Without the test | Rounds faster |
  |---|---|---|---|---|---|---|---|---|
  | starting | b1 c3 | 0 | 81,883 | 0.073s | 0.031s | 2.4x | 0.027s | 7 of 7 |
  | kiwipete | e2xa6 | 102 | 489,295 | 0.950s | 0.219s | 4.3x | 0.244s | 7 of 7 |
  | italian | b1 c3 | -32 | 502,318 | 0.774s | 0.281s | 2.8x | 0.297s | 7 of 7 |
  | position3 | b4xf4 | 182 | 18,999 | 0.014s | 0.008s | 1.8x | 0.007s | 7 of 7 |
  | position4 | c4 c5 | -984 | 281,179 | 0.469s | 0.105s | 4.5x | 0.145s | 7 of 7 |
  | middlegame | f3 g5 | 54 | 1,972,588 | 3.528s | 0.966s | 3.7x | 1.118s | 7 of 7 |

  The six searches together: 5.81s before, 1.61s after, 1.84s without
  the test. The change is faster than the build without the test in
  four positions, because that build kept the old function at nodes in
  check at the evasion limit, and the new one is cheaper there too.
