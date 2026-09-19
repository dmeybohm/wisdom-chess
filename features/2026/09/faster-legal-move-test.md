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

   The cheap form: `MoveGeneration::generate (piece, coord)` already
   works one piece at a time, so `hasLegalMove()` can generate one
   piece's moves (at most 27), test them and return before touching the
   next piece, king first. About 15 lines. A true resumable iterator over
   all moves was considered and rejected: it means rewriting the nested
   generation loops as a state machine, `MoveList` is a fixed 1 KB array
   on the stack so there is no memory to save, and the search could not
   use it, because alpha-beta needs the transposition-table move and the
   captures first. Ordering without a full list needs staged generation
   (table move, captures, quiet moves), which is a generator redesign.

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

## Effect on quiescence search

Quiescence search replaces the `evaluate()` call at `depth <= 0`, which is
exactly where this mate test runs, so the two designs meet.

- A quiescence node normally may not stand pat while in check: it
  searches every evasion instead. That is option 3. Mate is then found by
  the move loop through `evaluateWithoutLegalMoves()`, and `evaluate()`
  no longer needs a mate test at all, because a side that is not in check
  cannot be checkmated. The one `isKingThreatened` call per leaf stays;
  it becomes the test for whether standing pat is allowed.
- The table above sizes that cost: 2 to 5% of horizon nodes are in check,
  each with 3 to 8 legal evasions out of about 35 pseudo-legal moves. The
  legality work `hasLegalMove()` spends there today would be spent
  searching those evasions.
- Evasion searches can chain through a series of checks. Bound it, by
  allowing evasions only in the first quiescence plies or by relying on
  the repetition check, and measure the node count either way.
- If the first quiescence version only searches captures even when in
  check, a node in check with no legal capture still needs a mate test,
  and `hasLegalMove()` is the function to call. Do not go back to
  `generateLegalMoves()` there.
- Stalemate at the horizon is not detected today and would not be under
  quiescence either.
- Quiescence needs a captures-only generator, and its nodes outnumber the
  main search's, so the full generate-and-sort that option 2 avoids
  matters more there. The staged generation described under option 2 is
  the shared piece of work.

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

### Session #2

- Added `hasLegalMove (board, who)` to `generate.hpp` / `generate.cpp`:
  the `generateLegalMoves()` loop, returning `true` at the first legal
  move. `isPlayerCheckmated()` and `isStalemated()` use it.
  `isStalemated()` now tests the king first, so a side in check skips
  move generation entirely.
- Tests in `generate_test.cpp`: the starting position, a checkmate
  (fool's mate), a stalemate, a check with several evasions, a check
  whose only evasion is a block, and agreement with
  `generateLegalMoves()` for both colors on four perft positions. The
  mate and stalemate cases also assert `isPlayerCheckmated()` and
  `isStalemated()`, which had no direct tests.
- Verified: no warnings, all 140 tests pass (117 fast, 23 slow), linter
  clean on the changed files.
- Measured by linking the same driver against the engine library twice,
  once with the previous `evaluate.cpp` in front of it, so only that file
  differs. Depth 8, cleared table, five alternating rounds, medians:

  | Position | Before | After | Faster by |
  |---|---|---|---|
  | starting | 2.335s | 2.230s | 4.5% |
  | kiwipete | 1.932s | 1.838s | 4.9% |
  | italian | 2.671s | 2.400s | 10.1% |
  | position4 | 0.453s | 0.433s | 4.4% |

  The new code was faster in all 20 paired runs, and the moves and scores
  were identical. The `search/*` benchmarks were not used because the
  build tree has benchmarks off; the driver searches the same positions
  the same way.
- Recorded the per-piece form of option 2, why a full move iterator was
  rejected, and how quiescence search interacts with this work.

### Session #3

- Review finding: the Ubuntu Debug CI job aborted in the new test with
  `Assertion 'who == my_code.getCurrentTurn()' failed`. Three places
  called `hasLegalMove()` for the side not to move; it applies moves
  through `Board::withMove()`, which asserts the color in Debug. Release
  compiles the assert away, which is why Session #2 did not see it. This
  is the Session #11 lesson from the bug list again: run the tests in
  both modes.
- The tests now ask only about the side to move. The starting-position
  case plays 1. e4 before asking about Black, the checkmate case drops
  the Black query, and the agreement case uses each position's own side
  to move, with three Black-to-move positions added to keep both colors
  covered. `hasLegalMove()` itself is unchanged; `generateLegalMoves()`
  has the same precondition.
- Reproduced the abort in a Debug build first. All 111 fast tests now
  pass in both Debug and Release.

