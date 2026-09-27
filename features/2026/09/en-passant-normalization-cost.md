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

## Interface

Normalization moved from `History` into `Board`:

- `generateLegalEnPassantMoves (board)` is idea 1 as a generator. It
  returns the legal en passant captures for the side to move, and an
  empty list means the target is unusable.
- `Board::getBoardCode()` returns the code without an unusable target.
  `Board::getUnnormalizedBoardCode()` returns it as FEN records it, and
  replaces `getCode()`.
- `Board::operator==` compares the squares and the normalized codes. It
  already ignored both move clocks, so it was the repetition comparison
  in all but the en passant target.
- `Board::withNormalizedEnPassantTarget()` is gone. `History` is back to
  storing the board it is given and calling `getBoardCode()`, so an
  entry point can't forget to normalize.

`getBoardCode()` returns eight bytes where
`withNormalizedEnPassantTarget()` copied the whole board, which search
did twice per node.

Nothing the normalization calls may use `Board::operator==` or
`getBoardCode()`, or it would recurse. `withMove()`,
`isLegalPositionAfterMove()` and `BoardCode::applyMove()` read the
squares and the unnormalized code only.

## Transposition table key

`repetition-en-passant-key` left the table on the unnormalized code.
That loses transpositions. Since 0b64075 every double push leaves a
target, so 1.e4 a6 2.d4 and 1.d4 a6 2.e4 reach the same position with
different codes. The table now uses `getBoardCode()`.

Nodes and user time at depth 8, Release build:

| Build | 1.d4 d5 2.c4 | Start position | Italian (after 3...Bc5) |
| --- | --- | --- | --- |
| a2d46e1, before 0b64075 | 5,859,672 / 2.40 s | 3,174,855 / 1.16 s | 46,749,259 / 25.85 s |
| Merge base (0b64075) | 6,317,950 / 2.59 s | 3,348,728 / 1.26 s | 49,531,211 / 27.68 s |
| Unnormalized table key | 6,317,947 / 2.62 s | 3,348,738 / 1.26 s | 49,531,355 / 28.03 s |
| Normalized table key | 5,859,695 / 2.35 s | 3,174,862 / 1.16 s | 46,749,258 / 25.77 s |

0b64075 cost the search 5 to 8% more nodes. The normalized key returns
to the node counts from before it, and takes no longer than that build
did. Node counts come from the time and rate that the search logs for
each depth, so the last few digits are rounding.

### Rejected: caching the normalized code in a mutable field

The normalized-key build computes the code twice for each child, once
in `addTentativePosition()` and once for the table probe. It still
matches the time of a2d46e1, which never normalizes, so a cache has
nothing measurable to save. It would also cost something:

- A `const Board` read from two threads would race on the cache.
- Every mutation would have to reset it.
- `withMove()` copies the board for each pseudo-legal move, and the
  copy would grow.

If the cost ever shows up, compute the answer where the target is set
instead. `updateEnPassantEligibility()` would record whether a legal
capture exists in a spare metadata bit of `BoardCode`. That needs no
mutable state, and the adjacent-pawn test rejects about 99% of double
pushes before any legality check.

## Implementation Progress

### Session #1

Implemented plan steps 1 to 4 with the interface above, plus idea 3
and the normalized transposition table key. Idea 2 was not needed.

User time for one search, Release build, median of three runs. Depth 8
needs `go depth 8 movetime 120000`, or the default move time cuts the
search off at two seconds.

| Build | 1.d4 d5 2.c4, depth 8 | Start position, depth 8 |
| --- | --- | --- |
| Merge base (0b64075) | 2.60 s | 1.25 s |
| b66fa00 (adjacency pre-check) | 2.83 s | 1.37 s |
| Idea 1 in `getBoardCode()` | 2.65 s | 1.28 s |
| Idea 1 and idea 3 | 2.62 s | 1.25 s |

The branch started about 9% slower than the merge base. Idea 1 left
about 2%, and idea 3 brought it within noise. All four builds choose the
same move. These rows use the unnormalized table key. The table under
"Transposition table key" has the final numbers.

Step 1's count of normalizations reaching the `generateLegalMoves()`
fallback was not repeated. `repetition-en-passant-key` Session #3
already measured it at 3,178 of 900,512 nodes, and the fallback no
longer exists.

Tests:

- `generateLegalEnPassantMoves` in `generate_test.cpp` covers the five
  cases from the plan and a capture that uncovers a check along the
  taken pawn's diagonal. "Agrees with generateLegalMoves" walks seven
  positions to depth 3 and compares every node with the en passant
  moves `generateLegalMoves()` returns.
- "Board code and equality leave out an unusable en passant target" in
  `en_passant_test.cpp` replaces the `withNormalizedEnPassantTarget`
  case.
- "Double pushes made in either order reach the same position" in
  `en_passant_test.cpp` and "The root is stored without an en passant
  target nothing can capture" in `search_test.cpp` cover the table key.
- The first two were checked against two temporary mutations. Skipping the
  legality test failed six `generateLegalEnPassantMoves` subcases and
  the pinned-pawn repetition test. Returning the unnormalized code from
  `getBoardCode()` failed the board code and repetition tests.

Release `ctest` with slow tests passed 241 of 241, and the `lint` target
is clean. The Debug build passed the 207 fast tests. It also compiled
the tools and benchmarks, which the `getCode()` rename touches. The
Debug slow tests were not run.

### Session #2

Merged `main` after PR #286 landed there with three more commits.
`FenParser` now rejects an en passant target that has no pawn to take,
so "A target without a pawn to take has none" builds its boards with
`BoardBuilder`. `generateLegalEnPassantMoves()` keeps its own check,
because a builder can still set such a target.

After the merge, Release `ctest` with slow tests passed 244 of 244, the
Debug build passed the 210 fast tests, and the `lint` target is clean.
