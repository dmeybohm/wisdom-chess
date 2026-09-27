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

1. **Move ordering in the main search.** `compareMoves()`
   (`engine/generate.cpp:497`) orders by the table move, then captures
   by material difference, then promotions, then coordinates. Quiet
   moves are in coordinate order. Session #7 of
   [quiescence-search.md](quiescence-search.md) already found the main
   search visiting 2.4 times the baseline's nodes in the Italian game
   and named killer moves or a history heuristic as the remedy. This is
   the item with evidence behind it, so it goes first.
2. **Static exchange evaluation in quiescence.** Also named in Session
   #7: in the middlegame position quiescence is the whole cost, 297.8M
   of 364.7M nodes. Losing captures are searched in full today.
3. **A cheaper `hasLegalMove()`.** `quiesce()` calls it at every node
   not in check (`engine/search.cpp:318`). It generates every
   pseudo-legal move and sorts them (`engine/generate.cpp:657`) to
   answer a yes or no question. When
   [faster-legal-move-test.md](faster-legal-move-test.md) measured it,
   it ran on the 2 to 5% of leaves that were in check; it now runs on
   the other 95%. Option 2 of that document, generating one piece at a
   time without sorting, was left unmeasured because it could save
   little then. The only figure since is a test-suite time (24.05s
   against 23.71s), which is not a search measurement. Measure with
   `--search-report` first.
4. **Pruning and reductions.** None of null-move pruning, late-move
   reductions, principal-variation search, aspiration windows or check
   extensions are present, and no feature log has considered them. Each
   depends on good move ordering, so they come after item 1. Each
   changes what the search returns and needs its own branch and
   measurement. Not measured.
5. **Transposition table.** One entry per index, replaced by any other
   position (`engine/transposition_table.cpp:154`), and quiescence does
   not probe or store. [tt-index-metadata.md](tt-index-metadata.md)
   measured a hit rate of 19.3 to 19.7%. Candidates: buckets of two or
   four entries with a depth-preferred slot, and probing in quiescence,
   which [quiescence-search.md](quiescence-search.md) lists as a later
   experiment. Not measured.

### Engine: move generation

6. **Staged move picking.** Every node sorts its whole move list with
   `std::sort` (`engine/generate.cpp:563`), though a cutoff usually
   comes within the first few moves. The usual design tries the table
   move before generating anything, then captures, then quiet moves.
   [faster-legal-move-test.md](faster-legal-move-test.md) calls this a
   generator redesign and the piece of work shared with item 3. Not
   measured; a profile of the `search/*` benchmarks would show what the
   sort costs.

### Engine: evaluation

7. **Evaluation terms.** `evaluateWithoutMateTest()`
   (`engine/evaluate.cpp:70`) sums material, piece-square tables and a
   castling term. There is no pawn structure, mobility, king safety, or
   separate endgame table for the king. This affects playing strength,
   not code quality, and the engine-match script is the way to measure
   it. Lowest priority of the engine items, since a deeper search gains
   more per line of code.

### Layering

8. **`global.hpp` holds unrelated things.** Standard-library aliases,
   the pointer types, contracts, `Error`, board dimensions, material
   weights and search constants (`engine/global.hpp`). Splitting out the
   pointer types and contracts would let a file that needs `nonnull`
   avoid the score constants. It is precompiled, so expect no build-time
   gain.
9. **Legality functions live in `evaluate.cpp`.**
   `isLegalPositionAfterMove()`, `isCheckmated()` and `isStalemated()`
   (`engine/evaluate.cpp:95-142`) are rules, not evaluation. The move
   generator calls them, so `generate.cpp` includes `evaluate.hpp`.
10. **Includes that nothing needs.** `board.hpp:7` includes
    `generate.hpp` and uses nothing from it; `evaluate.cpp:4` includes
    `search.hpp` and uses nothing from it. Other files may rely on the
    first one transitively, so removing it means adding the include
    where it is used.
11. **File formats in `Game`.** `Game::save()` and `Game::load()`
    (`engine/game.cpp:159`, `:247`) pick a format from the file name
    through two mutable file-scope objects (`engine/game.cpp:16-17`).
    Free functions taking a `Game` would keep file handling out of the
    class every frontend depends on.

### Frontends

12. **`GameModel` is 742 lines** (`ui/qml/main/game_model.cpp`), and
    `setupNewEngineThread()` makes 13 signal connections by hand. The
    thread is created with `new` and deleted only when it is not running
    (`game_model.cpp:41-49`), which is deliberate on the web. The
    move-holding timer and the engine-thread wiring are separable from
    the model.
13. **`App.tsx` holds the whole engine adapter** (348 lines): the worker
    message handler, the move and draw handlers, the settings transfer
    and a `throttle` helper. A `useEngine` hook would leave the
    component with rendering.
14. **Unvalidated worker message.** `App.tsx:139` casts the result of
    `JSON.parse` to the draw-status shape. The sender is the project's
    own worker, so the risk is a silent mismatch after a change, not
    hostile input.

### Small items

15. **Node counters mix widths.** `my_nodes_visited` and the cutoff
    counters are `int`; the totals are `int64_t`
    (`engine/search.cpp:84-89`). An `int` holds 2.1 billion; the
    depth-8 middlegame search in
    [quiescence-search.md](quiescence-search.md) counted 365 million, so
    two more plies would pass it.
16. **Zero as the empty marker.** `TranspositionTable::store()` counts
    an entry as new when `hash_code == 0`
    (`engine/transposition_table.cpp:157`). A position that hashes to
    zero is counted again on every store. Statistics only.

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
