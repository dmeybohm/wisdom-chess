# Cost of normalizing en passant targets

## Motivation

`repetition-en-passant-key` (PR #286) normalizes a board's en passant
target before `History` stores or compares it: the target is cleared
unless a legal en passant capture exists. Search reaches that code at
every node. `addTentativePosition()` runs for each legal child
(`search.cpp:213`, `:363`), and the child's own `search()` call then
normalizes the same board again in `isProbablyNthRepetition()` through
`isProbablyDrawingMove()`.

The first version called `generateLegalMoves()` for every board with a
target, and search took roughly twice as long. 868654a added
`hasPawnAdjacentToEnPassantTarget()`, which skips move generation when
no capturing pawn is adjacent. That recovers most of the cost, but not
all of it. A depth-7 search after 1.d4 d5 2.c4, Release build, user
time:

| Build | Run 1 | Run 2 |
| --- | --- | --- |
| Merge base (0b64075) | 0.67 s | 0.71 s |
| 868654a | 0.79 s | 0.79 s |

That is roughly 12–15% slower. Adjacent pawns are common in closed
openings, and those positions still pay for full move generation.

## Ideas

1. **Check only the en passant captures.** Build the one or two
   candidate moves directly with `Move::makeEnPassant()`, one for each
   adjacent capturing pawn. Apply each with `Board::withMove()` and
   test it with `isLegalPositionAfterMove()`. That is at most two board
   copies and two king-threat checks instead of a full pseudo-legal
   generation plus a legality pass per move. The result is the same
   whenever the target is valid. Legality after the move already covers
   pins, a discovered check along the rank when both pawns leave it,
   and being in check.

   This also removes the two load-time Debug aborts found in review:
   - A FEN target with no enemy pawn in front of it hit the assert in
     `MoveGeneration::enPassant()`. Checking for the victim pawn first
     and clearing the target when it is missing avoids that.
   - Castling rights with the king off its home square hit the assert
     in `applyForCastlingMove()`. Normalization would no longer generate
     castling moves at all.

   Those FENs should still be rejected by `FenParser`; this just stops
   normalization from being the thing that trips over them.

2. **Normalize each child board once.** Search normalizes the same
   child twice: in `addTentativePosition()`, and again at the top of
   the child's `search()` / `quiesce()` call. Normalizing the child
   once and passing that board to both would halve the calls. This
   changes the search's internal call pattern but not what `History`
   stores.

3. **Skip the probe-side normalization when the halfmove clock is 0.**
   A board with a target has just had a double push, which resets the
   clock. `isProbablyNthRepetition()` then scans only one entry, and a
   count of 3 or 5 can't be reached from one entry. Returning early
   when `reversible_count < repetition_count` avoids the normalization
   and the scan together. It's a narrower win than idea 2, but it is
   local to `History`.

4. **Rejected: clear unusable targets in `Board::withMove()`.** This
   would make every board canonical and remove normalization from
   `History` entirely. However, 0b64075 deliberately keeps the target
   after every double push for FEN output, as the PGN/FEN specification
   requires.

## Plan

1. Measure a baseline on this branch: depth 7 from the position above,
   and from the start position. Also count how many normalizations
   reach the full `generateLegalMoves()` fallback.
2. Implement idea 1 in `Board::withNormalizedEnPassantTarget()`,
   replacing both the adjacency pre-check and the fallback. Add tests
   for the cases the review listed as untested:
   - two capturers, one of them pinned
   - a diagonal pin
   - side to move in check
   - an edge-file double push
   - a FEN target with no victim pawn (Debug must not abort)
3. Re-measure. Add idea 3 if a measurable gap remains, and idea 2 only
   if 3 isn't enough.
4. Run `ctest` in Release with slow tests, and the new tests in Debug.
