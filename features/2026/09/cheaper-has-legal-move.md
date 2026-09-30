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
- Measured with `--search-report 8`, one run of each build, the same
  way. The depth-7 times are from the same runs. The moves, scores and
  node counts were again the same at every depth. The build without the
  test is the run recorded in the other document, from `6e2ef7f`.

  | Position | Move | Score | Nodes | Before | After | Faster by | Without the test | Depth 7 before | After | Faster by |
  |---|---|---|---|---|---|---|---|---|---|---|
  | starting | e2 e4 | 0 | 3,801,392 | 4.08s | 1.48s | 2.8x | 1.36s | 0.81s | 0.28s | 2.9x |
  | kiwipete | d5xe6 | 48 | 8,144,988 | 15.42s | 3.73s | 4.1x | 3.77s | 3.21s | 0.68s | 4.7x |
  | italian | d1 e2 | -54 | 40,692,807 | 65.07s | 22.55s | 2.9x | 22.97s | 2.79s | 1.04s | 2.7x |
  | position3 | b4xf4 | 81 | 261,172 | 0.22s | 0.10s | 2.3x | 0.08s | 0.08s | 0.04s | 1.9x |
  | position4 | c4 c5 | -928 | 2,620,947 | 4.52s | 1.02s | 4.5x | 1.31s | 1.67s | 0.34s | 5.0x |
  | middlegame | f3 g5 | 48 | 309,896,444 | 554.72s | 146.73s | 3.8x | 158.68s | 24.30s | 5.79s | 4.2x |

  The six depth-8 searches together: 644.0s before, 175.6s after.
- Not measured here: play at a time limit, where the saving becomes
  depth instead of time. Session #3 has the engine match.

### Session #2

- Measured the same shortcut for `isLegalPositionAfterMove()` in the
  search loops, as a prototype outside the repository. Not adopted.
  - The prototype: `search()` finds out once per node whether the side
    is in check, which `quiesce()` already knows. A move from a square
    off the king's lines, other than en passant, skips the test.
  - `--search-report 7`, seven alternating rounds, medians, against
    this branch: 7.92s to 7.75s for the six depth-7 searches, 2.1%
    faster, between 0% and 4.1% by position. The moves, scores and
    node counts were the same.
  - Counted at depth 6: 6.64 million tests, of which 2.30 million (35%)
    could skip, and the shortcut was never wrong.
  - Why it is small: the board is still copied for every move, so the
    shortcut saves one `isKingThreatened()` and nothing else, and
    `search()` pays one more per node to use it. `hasLegalMove()` was
    different: it generated and sorted a whole list to try one move.
  - What the count did show: 2.30 million of the tests (35%) found the
    move illegal, after a board copy made for nothing. Most of those
    are at nodes in check, where every pseudo-legal move is tried for
    the few evasions. Testing a move before making it, or generating
    only evasions when in check, is where that cost would go. Not
    measured.

### Session #3

- Played the change against the commit before it, with
  `./scripts/run-engine-match.sh base=132cd5e new=b39f33f` and the
  script's defaults. The prototype of Session #2 is in neither engine.

  | | |
  |---|---|
  | Engines | base `132cd5e`, new `b39f33f` |
  | Time control | 8+0.08 |
  | Games | 500: 250 openings from `8moves_v3.pgn`, each with both colours |
  | Concurrency | 4, each game pinned to a core |
  | Move Overhead, hash, depth limit | 30 ms, 16 MB, 64 |
  | Seed | 1 |
  | fastchess | `60d7a7a` |
  | Duration | 45 minutes 42 seconds, on 2026-09-28 |

  | Pairing | Games | W / D / L | Score | Elo | 95% range |
  |---|---|---|---|---|---|
  | new vs base | 500 | 209 / 242 / 49 | 66.0% | +115 | +94 to +138 |

- The score was steady through the match: 67.1% after 35 games, 67.3%
  after 110, 66.2% after 198, 67.8% after 298 and 67.9% after 400.
- With White the new engine scored 107 / 117 / 26, and the old one
  23 / 125 / 102.
- How the games ended:

  | Ending | Games |
  |---|---|
  | Threefold repetition | 205 |
  | White mates | 130 |
  | Black mates | 128 |
  | Fifty-move rule | 29 |
  | Insufficient material | 7 |
  | Stalemate | 1 |

- Every game ended normally: none was lost on time or to an illegal
  move, no engine disconnected, and fastchess logged no warning.
- The results are in
  `~/.cache/wisdom-chess/match/results/20260928-110628-base-new`.
- Not measured: other time controls. The engines are the same program
  apart from this change, so the gain is what the extra speed buys at
  8+0.08, and may differ at a longer or shorter one.

### Session #4

- Brought the new code in line with the conventions the
  `style-inconsistencies` branch added since this branch was made, so
  that the later merge has nothing to fix: the file-local helpers are in
  an unnamed namespace instead of `static`, and the tests' lambda is
  `board_from_fen`. That branch's linter, with its `allman-braces` and
  `no-trailing-whitespace` rules, passes the changed files. A dry-run
  merge of the two branches has no conflicts, and the merged tree
  builds, lints and passes the fast tests.
