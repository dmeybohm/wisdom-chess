# Improvement suggestions

## Motivation

A code review on 2026-09-27 scored the project 8/10, up from the 6/10 of
the 2026-09-09 review
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md)). It
read about 15 core files out of roughly 190 and ran the fast suite (211
of 211 passed), so it is a sample, not an audit. Testing, CI and
correctness discipline scored 9 or above. What held the score down was
the engine's search and move ordering, some tangled layering, and two
large frontend units.

This document records those suggestions so each can become its own
branch. It changes no code. Stylistic inconsistencies are listed
separately in [style-inconsistencies.md](style-inconsistencies.md).

Line numbers are as of commit `131bb67`.

## Corrections to the review

Checking the review against the feature logs and the code changed three
of its points:

- **The `hasLegalMove()` call in quiescence is deliberate.** The review
  said to remove it. It was added on 2026-09-26 as a correctness fix:
  a stand-pat node can be stalemate, and no material test rules that out
  (Session #5 of [review-fixes.md](review-fixes.md)). The suggestion is
  to make the call cheaper, not to drop it. See item 3.
- **`global.hpp` is precompiled.** The review counted its 25 standard
  includes as a cost to every translation unit. Every target precompiles
  it, so the cost is paid once per target. What remains is cohesion. See
  item 8.
- **Copying the board per node is cheap.** `sizeof (Board)` is 120
  bytes, measured with GCC at `-O2`. Copy-make at that size is a
  reasonable design and is not a suggestion here.

## Suggestions

Ordered by expected value. "Not measured" means the review has no
number for it, and the first step is to get one.

### Engine: search

1. [x] **Move ordering in the main search.** Before this item,
   `compareMoves()` in `engine/generate.cpp` ordered by the table move,
   then captures by material difference, then promotions, then
   coordinates, so quiet moves were in coordinate order. Session #7 of
   [quiescence-search.md](quiescence-search.md) found the main search
   visiting 2.4 times the baseline's nodes in the Italian game and named
   killer moves or a history heuristic as the remedy. This was the item
   with evidence behind it, so it went first.
   **Done** on the `move-ordering` branch
   ([move-ordering.md](../10/move-ordering.md)): `compareMoves()` is
   gone. `sortKey()` gives each move one key, which orders the table
   move, then captures by material difference, then promotions, then
   the ply's two killer moves, then the other quiet moves by a history
   of the cutoffs they caused. It is 1.1 to 4.9 times faster at depth 8
   and +67 Elo over 500 games at 8+0.08.
2. [ ] **Static exchange evaluation in quiescence.** Also named in Session
   #7: in the middlegame position quiescence is the whole cost, 297.8M
   of 364.7M nodes. Losing captures are searched in full today.
   **Tried** on the local `static-exchange-evaluation` branch
   (2026-09-26, before item 3): its log records a 500-game match at
   8+0.08 against plain quiescence at +3 Elo, range -16 to +23, so it
   was not merged. The branch predates item 3 and `DrawArbiter`, and
   its measurements would have to be taken again.
3. [x] **A cheaper `hasLegalMove()`.** `quiesce()` calls it at every node
   not in check (`engine/search.cpp:318`). It generates every
   pseudo-legal move and sorts them (`engine/generate.cpp:657`) to
   answer a yes or no question. When
   [faster-legal-move-test.md](faster-legal-move-test.md) measured it,
   it ran on the 2 to 5% of leaves that were in check; it now runs on
   the other 95%. Option 2 of that document, generating one piece at a
   time without sorting, was left unmeasured because it could save
   little then. The only figure since is a test-suite time (24.05s
   against 23.71s), which is not a search measurement. Measure with
   `--search-report` first. The candidates are listed under
   [Item 3](#item-3-options-for-a-cheaper-haslegalmove) below.
   **Done** on the `cheaper-has-legal-move` branch
   ([cheaper-has-legal-move.md](cheaper-has-legal-move.md)): 2.3 to
   4.5 times faster at depth 8 and +115 Elo over 500 games at 8+0.08.
4. [ ] **Pruning and reductions.** None of null-move pruning, late-move
   reductions, principal-variation search, aspiration windows or check
   extensions are present, and no feature log has considered them. Each
   depends on good move ordering, so they come after item 1. Each
   changes what the search returns and needs its own branch and
   measurement. Not measured. Delta pruning in quiescence was tried
   and rejected (Session #6 of
   [quiescence-search.md](quiescence-search.md)): it dropped the lines
   quiescence exists to search.
5. [ ] **Transposition table.** One entry per index, replaced by any other
   position (`engine/transposition_table.cpp:154`), and quiescence does
   not probe or store. [tt-index-metadata.md](tt-index-metadata.md)
   measured a hit rate of 19.3 to 19.7%. Candidates: buckets of two or
   four entries with a depth-preferred slot, and probing in quiescence,
   which [quiescence-search.md](quiescence-search.md) lists as a later
   experiment. Not measured.

### Engine: move generation

6. [ ] **Staged move picking.** Every node sorts its whole move list with
   `std::sort` (`engine/generate.cpp:563`), though a cutoff usually
   comes within the first few moves. The usual design tries the table
   move before generating anything, then captures, then quiet moves.
   [faster-legal-move-test.md](faster-legal-move-test.md) calls this a
   generator redesign and the piece of work shared with item 3. Not
   measured; a profile of the `search/*` benchmarks would show what the
   sort costs.
17. [ ] **Board copies for moves that turn out illegal.** Added after item
    3 landed. Both search loops make every pseudo-legal move with
    `Board::withMove()` and then test it with
    `isLegalPositionAfterMove()`. Session #2 of
    [cheaper-has-legal-move.md](cheaper-has-legal-move.md) counted 6.64
    million tests in the depth-6 report, of which 2.30 million (35%)
    found the move illegal, each after a board copy made for nothing.
    Most of those should be at nodes in check, where every pseudo-legal
    move is tried for the 3 to 8 evasions, but the count was not split
    by node type. Two ways to spend less: an evasion generator for nodes
    in check, or a legality test that runs before the move is made,
    from the pins and the attacked squares. Not measured. Already
    measured and not worth a branch: item 3's shortcut applied to
    `isLegalPositionAfterMove()` gains 2%, because the copy stays and
    only the threat test is saved.

### Engine: evaluation

7. [ ] **Evaluation terms.** `evaluateWithoutMateTest()`
   (`engine/evaluate.cpp:70`) sums material, piece-square tables and a
   castling term. There is no pawn structure, mobility, king safety, or
   separate endgame table for the king. This affects playing strength,
   not code quality, and the engine-match script is the way to measure
   it. Lowest priority of the engine items, since a deeper search gains
   more per line of code.

### Layering

8. [x] **`global.hpp` holds unrelated things.** Standard-library aliases,
   the pointer types, contracts, `Error`, board dimensions, material
   weights and search constants (`engine/global.hpp`). Splitting out the
   pointer types and contracts would let a file that needs `nonnull`
   avoid the score constants. It is precompiled, so expect no build-time
   gain. **Done** on other branches: `error.hpp`, `ptr.hpp` and
   `narrow.hpp` were split out on 2026-09-30
   ([contract-macros.md](contract-macros.md)) and `types.hpp` on
   2026-10-01 (`small-fixups`, PR #311). None of the four includes
   `global.hpp`, which now includes them and keeps the board
   dimensions, the material weights and the search constants.
9. [x] **Legality functions live in `evaluate.cpp`.**
   `isLegalPositionAfterMove()`, `isCheckmated()` and `isStalemated()`
   (`engine/evaluate.cpp:95-142`) are rules, not evaluation. The move
   generator calls them, so `generate.cpp` includes `evaluate.hpp`.
10. [x] **Includes that nothing needs.** `board.hpp:7` includes
    `generate.hpp` and uses nothing from it; `evaluate.cpp:4` includes
    `search.hpp` and uses nothing from it. Other files may rely on the
    first one transitively, so removing it means adding the include
    where it is used.
11. [x] **File formats in `Game`.** `Game::save()` and `Game::load()`
    (`engine/game.cpp:159`, `:247`) pick a format from the file name
    through two mutable file-scope objects (`engine/game.cpp:16-17`).
    Free functions taking a `Game` would keep file handling out of the
    class every frontend depends on.
    **Items 9, 10 and 11 done** on the `layering-improvements` branch
    ([layering-improvements.md](../10/layering-improvements.md)).

### Frontends

12. [ ] **`GameModel` is 742 lines** (`ui/qml/main/game_model.cpp`), and
    `setupNewEngineThread()` makes 13 signal connections by hand. The
    thread is created with `new` and deleted only when it is not running
    (`game_model.cpp:41-49`), which is deliberate on the web. The
    move-holding timer and the engine-thread wiring are separable from
    the model.
13. [ ] **`App.tsx` holds the whole engine adapter** (348 lines): the worker
    message handler, the move and draw handlers, the settings transfer
    and a `throttle` helper. A `useEngine` hook would leave the
    component with rendering.
14. [ ] **Unvalidated worker message.** `App.tsx:139` casts the result of
    `JSON.parse` to the draw-status shape. The sender is the project's
    own worker, so the risk is a silent mismatch after a change, not
    hostile input.

### Small items

15. [x] **Node counters mix widths.** `my_nodes_visited` and the cutoff
    counters are `int`; the totals are `int64_t`
    (`engine/search.cpp:84-89`). An `int` holds 2.1 billion; the
    depth-8 middlegame search in
    [quiescence-search.md](quiescence-search.md) counted 365 million, so
    two more plies would pass it.
16. [x] **Zero as the empty marker.** `TranspositionTable::store()` counts
    an entry as new when `hash_code == 0`
    (`engine/transposition_table.cpp:157`). A position that hashes to
    zero is counted again on every store. Statistics only.
    **Items 15 and 16 done** on the `small-engine-fixups` branch
    ([small-engine-fixups.md](../10/small-engine-fixups.md)).

## Item 3: options for a cheaper `hasLegalMove()`

Read from `main` at `6e2ef7f`. Nothing in this section is measured.

### What a quiescence node not in check does today

1. One `isKingThreatened()` call.
2. `hasLegalMove()`: `generateAllPotentialMoves()` walks the board,
   generates every pseudo-legal move and sorts them with
   `compareMoves()`. Each move is then applied to a copy of the board
   and tested, until one is legal.
3. The static evaluation and the stand-pat test.
4. Without a cutoff, `generateCaptures()` walks the board and sorts a
   second time.

A side not in check can only make an illegal move with a pinned piece,
with the king, or by en passant. The first move tried is therefore
expected to be legal nearly always, which would put the cost in the
generation and the sort, not in the legality loop.

### Two questions

- *Is this move legal?* Asked of every move the search plays. No option
  below removes a legality test from the loop in `quiesce()`.
- *Does the side have any legal move?* The stalemate test. One legal
  move answers it, so this is the only test that may stop early.

The stalemate test is needed at every node not in check, not only at
the first one below the horizon: a capture further down can leave the
opponent stalemated.

### Options

| # | Option | Size | Search results |
|---|---|---|---|
| 1 | Skip the sort | Tiny | Identical |
| 2 | One piece at a time, unsorted | About 15 lines | Identical |
| 3 | Alignment fast path, then option 2 | 20 to 30 lines | Identical |
| 4 | Pin detection from the king | Larger | Identical |
| 5 | The capture loop as the proof | Moderate, in `quiesce()` | Identical |
| 6 | Generate once per node | Moderate | Identical |
| 7 | Test only when the window needs it | Small, subtle | Node counts change |
| 8 | Probe the table in quiescence | Item 5 | Change |
| 9 | A callback in place of the list | Moderate, in the generator | Identical if captures stay sorted |

1. **Skip the sort.** An unsorted entry point into the generator for
   `hasLegalMove()`. Still generates about 35 moves.
2. **One piece at a time.** Option 2 of
   [faster-legal-move-test.md](faster-legal-move-test.md):
   `MoveGeneration::generate (piece, coord)` for one piece, test its
   moves, return at the first legal one. Other pieces before the king
   when not in check, the king first when in check.
3. **Alignment fast path.** When not in check, a piece other than the
   king that shares no row, column or diagonal with its own king cannot
   be pinned, so any pseudo-legal move it has is legal. No board copy
   and no threat test. En passant is excluded, because it takes a second
   pawn off another square. Falls back to option 2 when no such piece
   has a move. This is a sound form of the material gate that Session #5
   of [review-fixes.md](review-fixes.md) removed.
4. **Pin detection.** Walk the eight rays from the king once and collect
   the pinned pieces. Exact in more positions than option 3, but new
   code: `InlineThreats` only returns a `bool`.
5. **The capture loop as the proof.** The first capture that passes its
   legality test in `quiesce()` shows the node is not stalemate. Two
   cases still need a test of their own: a node with no legal capture,
   where the quiet moves have to be tried, and a stand-pat cutoff, which
   returns before the loop runs.
6. **Generate once per node.** Captures sort first, so one list serves
   the stalemate test and the capture loop. It does not help a node that
   cuts off on stand-pat, and it overlaps with item 6.
7. **Test only when the window needs it.** Stalemate changes the result
   from the static score to 0, which matters only when the window tells
   the two apart. The returned values would need clamping to stay valid
   bounds for the parent's table entry. Ranked low.
8. **Probe the table in quiescence.** A hit skips the node. Not a fix
   for this call.
9. **A callback in place of the list.** `appendMove()` hands each move
   to a callback. Captures are tested for legality as they are
   generated, quiet moves are stored, and the stored moves are tested
   only when no capture was legal. It is option 2 at the granularity of
   one move instead of one piece, in the shape of options 5 and 6: one
   walk of the board, captures first, quiet moves as the fallback.
   To settle:
   - Stopping early means each generator function (`slide()`, `pawn()`
     and the rest) passes a stop signal up. Option 2 leaves them alone.
   - For the stalemate test alone a quiet move proves as much as a
     capture, so storing the quiet moves gains nothing over testing each
     move as it is generated. Storing them pays when the capture tests
     are the search's own, as in option 5.
   - The search wants captures in `compareMoves()` order and a callback
     sees them in board order. Searching from the callback would change
     the node counts. Keeping the order means collecting and sorting the
     captures first, which leaves the single walk of the board as the
     saving.
   - The callback is a template parameter, not a `std::function`, so
     that it inlines.
   - [faster-legal-move-test.md](faster-legal-move-test.md) rejected a
     resumable iterator because the generation loops would become a
     state machine. A callback keeps the loops as they are.

Cheaper legality testing everywhere, without a board copy per move, is
a redesign of the search loop and not part of this item.

### Recommendation

Option 3 with option 2 as its fallback. The measurements below support
it. It removes the cost at nodes that cut off and nodes that do
not, cannot change a move, score or node count, and stays inside
`generate.cpp`. Option 5 is the alternative that adds no chess logic.
Option 9 is worth its larger change if the single walk of the board is
wanted for item 6 as well.

Before choosing:

- Take the `--search-report` baseline, and count per quiescence node
  not in check: how often stand-pat cuts off, how often a legal capture
  exists, how often the first move tried is legal, and how often the
  node is stalemate.
- Test any new function against `generateLegalMoves()` at every node of
  a perft walk, in Debug and Release.

### Measurements

Taken on 2026-09-28 from `main` at `6e2ef7f`, Release, GCC, on the
laptop (i5-1145G7, `powersave` governor), each run pinned to one core.
Three builds of `wisdom-chess-benchmarks` from copies of the source
outside the repository:

- **Baseline:** `main` unchanged.
- **Without the test:** the `hasLegalMove()` call at a node not in check
  removed. This is the review's suggestion and is wrong at a stalemate;
  it is here only as the ceiling on what a cheaper test can save.
- **Counting probe:** the baseline with counters in `quiesce()`, and a
  prototype of option 3 that is run and counted but not acted on.

All three returned the same move, score and node counts in every
search, at every depth.

#### Depth 6

`--search-report 6`, seven alternating rounds, medians. The counts are
totals across the depths of the search, as in
[quiescence-search.md](quiescence-search.md).

| Position | Move | Score | Nodes | Quiescence nodes | Baseline | Without the test | Ceiling | Rounds faster |
|---|---|---|---|---|---|---|---|---|
| starting | b1 c3 | 0 | 81,883 | 22,727 | 0.074s | 0.028s | 62% | 7 of 7 |
| kiwipete | e2xa6 | 102 | 489,295 | 338,366 | 0.950s | 0.243s | 74% | 7 of 7 |
| italian | b1 c3 | -32 | 502,318 | 335,875 | 0.768s | 0.296s | 61% | 7 of 7 |
| position3 | b4xf4 | 182 | 18,999 | 7,672 | 0.014s | 0.007s | 50% | 6 of 7 |
| position4 | c4 c5 | -984 | 281,179 | 222,866 | 0.469s | 0.146s | 69% | 7 of 7 |
| middlegame | f3 g5 | 54 | 1,972,588 | 1,581,997 | 3.494s | 1.098s | 69% | 7 of 7 |

The six searches together take 5.77s, and 1.82s without the test. The
stalemate test is about two thirds of the search time.

Per quiescence node not in check:

| Position | Nodes | Stalemate | Moves generated | Legality tests | First move legal | Stand-pat cutoff |
|---|---|---|---|---|---|---|
| starting | 63,821 | 0 | 25.6 | 1.001 | 99.9% | 51% |
| kiwipete | 401,462 | 0 | 44.1 | 1.007 | 99.5% | 78% |
| italian | 389,581 | 0 | 36.5 | 1.009 | 99.3% | 59% |
| position3 | 13,321 | 0 | 17.1 | 1.078 | 92.5% | 56% |
| position4 | 234,159 | 0 | 36.9 | 1.010 | 99.3% | 58% |
| middlegame | 1,618,154 | 0 | 40.7 | 1.001 | 99.9% | 50% |

"Moves generated" and "legality tests" are per call of
`hasLegalMove()`: it generates and sorts 17 to 44 moves to try one.

What options 3 and 5 would find at the same nodes:

| Position | Legal capture exists: cutoff nodes | Other nodes | All | Option 3 answers | Pieces it examines | Wrong answers |
|---|---|---|---|---|---|---|
| starting | 56.2% | 49.3% | 52.8% | 100.00% | 1.17 | 0 |
| kiwipete | 99.9% | 97.0% | 99.2% | 99.86% | 1.32 | 0 |
| italian | 93.0% | 86.7% | 90.4% | 100.00% | 1.05 | 0 |
| position3 | 51.1% | 58.8% | 54.4% | 92.54% | 0.95 | 0 |
| position4 | 98.6% | 95.7% | 97.4% | 99.94% | 1.21 | 0 |
| middlegame | 94.6% | 91.3% | 92.9% | 99.98% | 1.02 | 0 |

Between 4% and 16% of the calls to `quiesce()` are in check, and 0.2%
to 12% are in check at the evasion limit, where `hasLegalMove()` runs
as well. No node returned a draw score.

#### Depth 8

`--search-report 8`, one run of each build. The depth-7 times are from
the same runs.

| Position | Move | Score | Nodes | Quiescence nodes | Baseline | Without the test | Ceiling | Depth 7 | Without the test | Ceiling |
|---|---|---|---|---|---|---|---|---|---|---|
| starting | e2 e4 | 0 | 3,801,392 | 1,575,160 | 3.97s | 1.36s | 66% | 0.79s | 0.25s | 69% |
| kiwipete | d5xe6 | 48 | 8,144,988 | 4,967,098 | 15.43s | 3.77s | 76% | 3.19s | 0.73s | 77% |
| italian | d1 e2 | -54 | 40,692,807 | 27,356,519 | 62.42s | 22.97s | 63% | 2.72s | 0.88s | 68% |
| position3 | b4xf4 | 81 | 261,172 | 94,521 | 0.17s | 0.08s | 51% | 0.07s | 0.03s | 52% |
| position4 | c4 c5 | -928 | 2,620,947 | 1,732,595 | 4.20s | 1.31s | 69% | 1.51s | 0.45s | 70% |
| middlegame | f3 g5 | 48 | 309,896,444 | 249,231,094 | 544.51s | 158.68s | 71% | 23.56s | 6.09s | 74% |

The six depth-8 searches together take 630.7s, and 188.2s without the
test. Session #7 of [quiescence-search.md](quiescence-search.md) timed
the middlegame at 186.5s before the test was added.

Per quiescence node not in check:

| Position | Nodes | Stalemate | Moves generated | Legality tests | First move legal | Stand-pat cutoff |
|---|---|---|---|---|---|---|
| starting | 3,096,343 | 0 | 28.9 | 1.002 | 99.9% | 49% |
| kiwipete | 6,640,917 | 0 | 44.7 | 1.010 | 99.3% | 81% |
| italian | 32,070,458 | 0 | 36.6 | 1.008 | 99.5% | 56% |
| position3 | 183,425 | 0 | 17.8 | 1.063 | 94.8% | 56% |
| position4 | 2,135,416 | 0 | 38.2 | 1.013 | 99.0% | 67% |
| middlegame | 249,517,610 | 0 | 41.1 | 1.001 | 99.9% | 52% |

| Position | Legal capture exists: cutoff nodes | Other nodes | All | Option 3 answers | Pieces it examines | Wrong answers |
|---|---|---|---|---|---|---|
| starting | 72.5% | 64.3% | 68.3% | 100.00% | 1.22 | 0 |
| kiwipete | 99.8% | 97.3% | 99.3% | 99.92% | 1.21 | 0 |
| italian | 94.9% | 87.4% | 91.6% | 100.00% | 1.07 | 0 |
| position3 | 46.4% | 57.2% | 51.1% | 91.93% | 0.95 | 0 |
| position4 | 98.8% | 96.3% | 98.0% | 99.92% | 1.26 | 0 |
| middlegame | 96.9% | 93.3% | 95.2% | 99.97% | 1.03 | 0 |

The counting probe does more work per node, and its middlegame search
reached the report's 600-second limit at depth 8 with 296.3M of the
309.9M nodes visited. The middlegame counts are from those nodes.
Every other search of the probe matched the baseline.

Between 5% and 15% of the calls to `quiesce()` are in check, and 0.1%
to 10% are in check at the evasion limit. Five nodes returned a draw
score.

#### What the measurements say

- The test is 50 to 76% of the search time, and about 70% where the
  time is longest. It cost more than the quiescence search it was added
  to.
- The cost is the generation and the sort. The legality loop runs 1.00
  to 1.08 times per call.
- No node was a stalemate, out of 293.6 million at depth 8. These
  positions do not exercise the case the test exists for, so
  correctness rests on the unit tests, not on the report.
- Option 3 answers at least 99.86% of the nodes in five positions and
  92% in position 3, where the pieces are few and often on a line with
  the king. It looks at about one piece per node. Its answer was never
  wrong.
- Option 5 covers less. A legal capture exists at 91 to 99% of the
  nodes in four positions, but at 53 to 68% in the starting position
  and 51 to 54% in position 3. About half the nodes cut off on
  stand-pat and generate no captures today, so option 5 adds a capture
  generation there.
- Options 1, 2 and 9 were not measured apart. How the cost divides
  between generation and sort is not known.
- The call at the evasion limit is up to 12% of the calls to
  `quiesce()`. The ceiling build kept it, so the ceiling leaves it out.
  That node is in check, where option 3 does not apply and option 2
  tries the king first.

## Plan

1. Record the suggestions (this document).
2. Take a baseline with `wisdom-chess-benchmarks --search-report` on
   `main` before any engine item.
3. Branch per item, in this order: 3 (small, and measures the newest
   cost), 1, 2, 6, then 4 and 5 one technique at a time. Compare moves,
   scores and node counts as well as time, in alternating rounds.
4. The layering and frontend items are independent of the engine work
   and of each other. Items 10, 15 and 16 are small enough to share a
   branch.
5. Item 7 waits until the search items have landed.

## Implementation Progress

### Session #1

- Wrote this document from the review. No code changed.
- Checked each suggestion against the existing feature logs, which
  produced the three corrections above.
- Measured `sizeof (Board)` = 120, `MoveList` = 512,
  `TranspositionEntry` = 24, `Move` = 2, with a throwaway program
  outside the repository.

### Session #2

- Listed nine options for item 3, with a recommendation and the
  measurements to take first. No code changed and nothing measured.
- The branch is 35 commits behind `main`, which has since changed
  `search.cpp`, `generate.cpp`, `evaluate.cpp` and `threats.hpp`. The
  options were read from `main`; the line numbers in the suggestions
  are still those of `131bb67`.

### Session #3

- Took the baseline of step 2 of the plan, and the counts for item 3,
  from `main` at `6e2ef7f`. No code in the repository changed: the
  three builds came from copies of the source in a scratch directory.
- The stalemate test in quiescence is about 70% of the search time.
  The tables are under "Measurements".
- `perf` was not usable (`perf_event_paranoid` is 4), so the share of
  time comes from a build without the call, not from a profile.
- The report's 600-second limit is close: the baseline's middlegame
  search at depth 8 took 544.5s, and the counting probe's reached the
  limit.
- Not measured: the options themselves, other than option 3's
  answers, and any position where a stalemate occurs.

### Session #4

- Item 3 is done on the `cheaper-has-legal-move` branch, with option 3
  and option 2 as its fallback. Marked it so.
- Added item 17 from what that branch's Session #2 measured: the board
  copies made for moves that turn out illegal. The same shortcut on
  `isLegalPositionAfterMove()` was measured there at 2% and is not an
  item.

### Session #5

- Added a tickmark to every suggestion and checked each against `main`
  at `1111e81c`. Items 3 and 8 are done, item 8 as a side effect of
  other branches.
- Item 2 was tried on the local `static-exchange-evaluation` branch
  before this document existed, and its 500-game match showed no
  measurable gain at 8+0.08. The item stays open: the branch is stale,
  and exchange evaluation should matter more at longer time controls,
  which were not measured.
- Everything else is unchanged in the code: no killer or history
  tables, no pruning or reductions, one table slot per index with the
  zero sentinel (`transposition_table.cpp:162`), the full sort at every
  node (`generate.cpp:564`), the legality functions in `evaluate.hpp`,
  both stray includes, the format objects in `game.cpp`, the `int` node
  counters, and the `JSON.parse` cast in `App.tsx`. `GameModel` has
  grown to 797 lines and `App.tsx` to 354.
- The plan's order stands. Item 1 is next.

### Session #6

- Items 9, 10 and 11 are done on the `layering-improvements` branch and
  marked so. That branch also moved `coordColor()` and
  `pawnDirection()` from `board.hpp` to `coord.hpp`, which was not an
  item.

### Session #7

- Items 15 and 16 are done on the `small-engine-fixups` branch and
  marked so. An empty table entry is now `BoundType::Empty`, not a zero
  hash.
