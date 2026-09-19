# Faster legal-move test at leaf nodes

## Motivation

The 2026-09-09 code-quality review
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md))
recorded that leaf evaluation runs full legal-move generation when the
side to move is in check.

`evaluate()` calls `isPlayerCheckmated()`, which does one
`isKingThreatened` test on every leaf. When the king is in check it calls
`generateLegalMoves()`, which:

- generates every pseudo-legal move,
- sorts them with `std::sort` for move ordering, which is no use here,
- copies the `Board` for each move and tests legality,
- appends every legal move to a list.

The caller only asks whether that list is empty. `isStalemated()` has the
same shape.

## Analysis

Measured on 2026-09-19 with a throwaway probe: a copy of `evaluate.cpp`
with counters and a switch between the current code and an early-exit
loop, linked in front of the Release engine library so that nothing in
the repository changed. Each position was searched to depth 8 with a
cleared transposition table.

| Position | Leaves | In check | Mates among those | Pseudo-legal moves per in-check leaf | Moves tried before the first legal one |
|---|---|---|---|---|---|
| starting | 7.18M | 1.7% | 0.2% | 29 | 3.2 |
| kiwipete | 1.89M | 4.7% | 0.8% | 38 | 6.5 |
| italian | 3.92M | 5.4% | 1.1% | 38 | 5.3 |
| position4 | 0.88M | 2.4% | 5.1% | 36 | 8.0 |

An in-check leaf is almost never a mate, and a legal move turns up within
the first 3 to 8 tries of the sorted list. The current code still makes
all 30 to 40 board copies and legality tests.

## Alternatives

1. **Stop at the first legal move.** Add a `hasLegalMove (board, who)`
   next to `generateLegalMoves()` that runs the same loop and returns
   `true` on the first move that passes `isLegalPositionAfterMove()`.
   `isPlayerCheckmated()` and `isStalemated()` both use it. About ten
   lines, and the search results cannot change. In the probe the moves,
   scores and leaf counts were identical in every run. Best of three
   alternating rounds at depth 8:

   | Position | Faster by |
   |---|---|
   | starting | 4.4% |
   | kiwipete | 5.4% |
   | italian | 10.5% |
   | position4 | 5.0% |

   Depth 6, best of five, gave 5 to 8%. The laptop's clock speed drifts
   by 10 to 24% over a run (see Session #20 of the bug list), so the
   exact figures are loose, but the early-exit loop was faster in all
   eight comparisons.

2. **Skip the sort and try king moves first.** After option 1 the
   remaining waste is generating and sorting about 35 moves per in-check
   leaf. King steps are the likeliest evasions, so testing them before
   generating anything else would often skip generation entirely. Needs
   an unsorted entry point into `MoveGeneration`. Not measured; it can
   save at most what option 1 leaves behind.

3. **Extend the search by one ply when in check at depth 0.** The
   conventional answer: do not evaluate the leaf, search one ply deeper,
   and let the normal move loop detect mate through
   `evaluateWithoutLegalMoves()`. The legality work then does useful
   searching. It is not a pure speed-up: it adds nodes, changes what a
   depth means and interacts with the discarded odd depths in
   `iterativelyDeepen`. It belongs with the quiescence-search design.

4. **Drop the mate test at leaves.** Cheapest, but every mate is seen one
   ply later, which is one full iteration later while odd depths are
   discarded. It saves no more than option 1. Rejected.

5. **A dedicated evasion test.** Find the checking pieces, then test king
   steps, captures of the checker and blocks directly. Fastest, but
   `InlineThreats` only returns a `bool`, so it needs new attacker-finding
   code. Not worth it while 2 to 5% of leaves are in check.

## Plan

1. Implement option 1: add `hasLegalMove()` to `generate.hpp` /
   `generate.cpp` and use it in `isPlayerCheckmated()` and
   `isStalemated()`.
2. Test it directly against `generateLegalMoves()` on positions with and
   without legal moves, including checkmate and stalemate.
3. Compare the `search/*` benchmarks before and after, in alternating
   rounds, and check that the chosen moves and scores are unchanged.
4. Look at option 2 only if a profile still shows
   `generateAllPotentialMoves` under `evaluate`. Leave option 3 for the
   quiescence work.

## Implementation Progress

### Session #1

- Wrote this document from the analysis above. No code changed yet.
