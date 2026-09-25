# Quiescence search

## Motivation

The 2026-09-09 code-quality review
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md))
found that the engine has no quiescence search. `search()` scores a node
with `evaluate()` as soon as `depth <= 0` (`engine/search.cpp:163-166`), even
when a capture is still pending. A search that ends right after `QxP`
therefore counts the pawn as won, although the queen is lost on the next
move. This is the horizon effect.

The engine works around it in `iterativelyDeepen()`
(`engine/search.cpp:304-313`) by keeping an odd-depth result only when no
even-depth result exists yet, so the last move searched is always the
opponent's reply. That throws away about half of the search time, and it
does not help a capture sequence longer than one reply.

## Current behaviour

- `search()` does, in order: the draw check (`isProbablyDrawingMove`), the
  `depth <= 0` leaf `evaluate()`, a table probe at `ply > 0`, and a
  loop over `generateAllPotentialMoves()` that tests each child for
  legality after making it.
- `evaluate()` (`engine/evaluate.cpp`) runs the mate test,
  `isCheckmated()`: one `isKingThreatened()` per leaf and `hasLegalMove()`
  when in check. The rest is material, piece-square tables and a castling
  penalty.
- `generateAllPotentialMoves()` generates every pseudo-legal move and sorts
  them: table move first, then captures by victim weight minus attacker
  weight (a form of MVV-LVA), then promotions, then quiet moves. No
  generator returns captures only.
- The frontends count depth in full moves and pass `full_moves * 2` plies
  (`ui/viewmodel/viewmodel_types.hpp`). UCI passes `go depth N` through as
  plies.
- `my_draw_nodes` counts the nodes that scored a draw, so that a node whose
  subtree hit the draw check is not stored in the table
  ([path-dependent-draw-scores.md](path-dependent-draw-scores.md)).

"Effect on quiescence search" in
[faster-legal-move-test.md](faster-legal-move-test.md) sets out what this
work means for the in-check mate test: 2 to 5% of horizon nodes are in
check, with 3 to 8 legal evasions each out of about 35 pseudo-legal moves.

## Design

### The quiescence node

A new member function in `IterativeSearchImpl`:

```cpp
auto
quiesce (const Board& board, Color side, int alpha, int beta, int ply,
         int quiescence_ply)
    -> int;
```

`search()` calls it at `depth <= 0` instead of `evaluate()`. It does, in
order:

1. **Draw check** for every quiescence node except the first, since
   `search()` has already run it for that node. It increments
   `my_draw_nodes` the same way `search()` does, so a main-search node
   above a quiescence line that reached a draw is still kept out of the
   table. After a capture the half-move clock is zero, so the repetition
   scan is a single comparison. Insufficient material matters here: a
   capture sequence can end in bare kings.
2. **In check:** no stand-pat. Search every legal move (the evasions)
   with the same loop. If none is legal, return
   `evaluateWithoutLegalMoves()`, which scores the mate at this ply. This
   is option 3 in [faster-legal-move-test.md](faster-legal-move-test.md).
3. **Not in check:** stand pat on the static evaluation. If it is at
   least `beta`, return it; if it raises `alpha`, raise it. Then search
   captures and promotions only, best first, with the same fail-soft
   update and cutoff as `search()`.
4. Every child is pushed with `addTentativePosition` and popped
   afterwards, as in `search()`, so a repetition through quiet evasions is
   found.
5. The timer is tested in the move loop the same way as in `search()`,
   and a timeout propagates through `my_current_result.timed_out`.

**Termination.** A quiescence node not in check searches captures only,
so a quiet move can only happen as an evasion. Most evasion chains
therefore pass through a capture and end when the pieces run out. They
do not have to: an evasion can itself give check, by a discovery or a
king move that uncovers a line, and a series of such cross-checks has no
captures in it. The repetition check ends a cycle, but only after it has
been played out. So bound it explicitly: count the quiescence ply, and
past a limit (start with 4, then measure) a node in check stops
searching evasions, tests for mate with `hasLegalMove()` and otherwise
returns the static evaluation, as the faster-legal-move note
recommends. Do not go back to `generateLegalMoves()` there. The limit
applies to evasions only; capture sequences stay unbounded.

**Promotions.** Only promotions to a queen are searched in quiescence.
An under-promotion that matters is almost always a knight check, and the
main search still sees it.

**Table.** Quiescence does not probe or store. It keeps the first
version simple, and the existing rules for path-dependent scores do not
need extending to a new kind of node. The main search's entries now hold
scores that include quiescence, which is what they should hold. Probing
in quiescence is a later experiment, measured on its own.

**Mate test in the static evaluation.** At a stand-pat node the side to
move is not in check, so it cannot be checkmated, and `isCheckmated()`
would repeat the `isKingThreatened()` call quiescence has just made. Split
`evaluate()`: a new function without the mate test does the material,
position and castling terms, and `evaluate()` keeps its current contract
by calling the mate test and then that function. Quiescence calls the new
one. `evaluate()` stays public because `Game` and the tests use it.

### Captures-only generation

Quiescence nodes will outnumber the main search's, so the cost of
generating and sorting 35 moves to keep 3 matters.

1. First version: `generateCaptures (board, who)` in `generate.hpp` that
   generates all moves and keeps those with `isAnyCapturing()` or a queen
   promotion. Correct by construction, and a baseline to test against.
2. Then a real one: a flag in `MoveGeneration` that skips the quiet
   destinations as it generates. Sliding pieces still walk the ray and
   emit only the capture at its end; pawns emit captures, en passant and
   promotions only; no castling. The sort sees only the short list.
3. Test the generator against step 1 on every perft position for both
   colors, including en passant and promotions.

### Iterative deepening

Once quiescence settles the leaves, the odd-depth rule in
`iterativelyDeepen()` has no reason to exist. Take the result of every
completed depth. This is a separate step, measured separately, because
it changes what a timed search returns even with identical scores.

Two things to check at that point:

- `search_test.cpp` has "Root TT hit should not bypass iterative
  deepening search" and tests that search to a given depth. Their
  expectations may have depended on the odd-depth rule.
- The frontends map full moves to `2 * full_moves` plies. That mapping
  stays; the question is whether the default depths still finish in an
  acceptable time once each ply carries quiescence. The benchmarks
  answer it.

### Statistics

Count quiescence nodes separately from main-search nodes and log both
per iteration, next to the existing node and cutoff counts. The
benchmarks report both.

### Not in this branch

- Quiet checks in quiescence.
- Delta pruning and static exchange evaluation. Both are cheap to add
  once the node counts show where the time goes, and each needs its own
  measurement.
- Stalemate at the horizon, which is not detected today and still won't
  be. A stand-pat node does not know whether the side has a legal move.
- A per-depth result callback for UCI `info` lines (Session #16 of the
  bug list).

## Testing

- **Generator:** `generateCaptures` agrees with the filtered full list on
  the perft positions, en passant, promotion and capture-promotion
  positions, for both colors.
- **Horizon:** positions where a depth-1 or depth-2 search without
  quiescence takes a defended piece, or misses that its own piece hangs,
  and with quiescence does not. Confirm each fails on `main` first.
- **Mate at the horizon:** a position where the last main-search ply
  gives mate, found through the evasion search with the right distance
  in the score; and a check with an evasion, which must not score as
  mate.
- **Draw handling:** a capture into insufficient material scores as a
  draw, and a node above it is not stored in the table (reuse the
  `my_draw_nodes` pattern from the path-dependent draw tests).
- **Timeout:** a search with a zero budget still returns cleanly while
  in quiescence.
- **Existing suites:** all fast and slow tests, in Release and Debug
  (`withMove()` asserts the side to move in Debug, and quiescence calls
  it at every node). The UCI, console and QML tests make engine moves
  and may pin a particular reply.

## Measurement

Before any change, record the `search/*` benchmarks (`WISDOM_CHESS_BENCHMARKS=ON`)
on `main`: time, nodes, the chosen move and the score per position. After
each step, run them again in alternating rounds against the previous
step, as in Session #20 of the bug list, because the laptop's clock
drifts. Quiescence changes what a depth means, so a slower time per depth
is expected; the comparison that matters is the move and score reached,
and how long a fixed time budget takes to reach them. Record:

- main-search and quiescence node counts per depth,
- the maximum quiescence ply reached (to decide on the evasion bound),
- the time for the default frontend depth on each benchmark position.

### Search report

The `search/*` benchmarks time a search and print nothing else. Every
search change so far has needed the move, score and node count as well,
and each time a throwaway driver was written for it (Session #20 of the
bug list, [faster-legal-move-test.md](faster-legal-move-test.md), and
the first attempt at a baseline here). Keep one instead:

- `SearchResult` gains `nodes`, the nodes visited across every depth of
  the search, so a caller need not parse the log. Quiescence adds its
  own count next to it.
- `wisdom-chess-benchmarks --search-report [max-depth]` searches each
  report position once per depth, from 1 to the maximum, with a cleared
  table, and prints one line per search: the move, the score, the depth
  the result came from, the node count and the time. It runs instead of
  the timed suites, which take about 16 seconds and report nothing it
  needs. Without the flag the program behaves as before.
- The report positions are the starting position, Kiwipete, the Italian
  game, perft positions 3 and 4, and a quiet middlegame. The last was the
  most expensive search in the first baseline.

The depth the result came from shows the odd-depth rule at work: today
an odd-depth search returns the even depth before it.

## Plan

1. Add the search report, and take the baseline numbers with it before
   changing the search.
2. Split `evaluate()` into the mate test and a static evaluation.
   No behaviour change; existing tests cover it.
3. Add `generateCaptures()` as a filter over the full list, with tests.
4. Add `quiesce()`, call it from `search()`, and add the node counters.
   Horizon, mate and draw tests. Measure, and tune the evasion limit.
5. Replace the filter in `generateCaptures()` with generation that skips
   quiet moves. Test it against the filter, then measure.
6. Take every completed depth in `iterativelyDeepen()`. Update the
   affected tests and measure timed searches.
7. Review the default search depths against the measurements, and update
   `AGENTS.md` if the meaning of a depth changes for the frontends.
8. Check the item off in
   [bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md).

## Implementation Progress

### Session #1

- Wrote this plan. No code changed yet.

### Session #2

- Added the search report (step 1). `SearchResult::nodes` holds the
  nodes visited across every depth, tested in `search_test.cpp`, and
  `wisdom-chess-benchmarks --search-report [max-depth]` prints the
  table below. Documented in the build options table in `AGENTS.md`.
- Baseline on `main` (`8b59ede`), Release, GCC, cleared table for each
  search. Depths 6 and 8; "from" is the depth the result came from:

  | Position | Depth | Move | Score | From | Nodes | Seconds |
  |---|---|---|---|---|---|---|
  | starting | 6 | e2 e3 | -101 | 6 | 424,006 | 0.095 |
  | starting | 8 | e2 e3 | -128 | 8 | 9,233,813 | 2.185 |
  | kiwipete | 6 | e2xa6 | -179 | 6 | 183,982 | 0.120 |
  | kiwipete | 8 | e2xa6 | -197 | 8 | 2,942,256 | 1.838 |
  | italian | 6 | f3 g5 | -219 | 6 | 214,376 | 0.092 |
  | italian | 8 | f3 g5 | -213 | 8 | 5,358,645 | 2.375 |
  | position3 | 6 | e2 e3 | -27 | 6 | 30,704 | 0.010 |
  | position3 | 8 | b4xf4 | 36 | 8 | 175,296 | 0.053 |
  | position4 | 6 | g1 h1 | -1386 | 6 | 151,053 | 0.047 |
  | position4 | 8 | c4 c5 | -1221 | 8 | 1,110,834 | 0.432 |
  | middlegame | 6 | f3 g5 | -218 | 6 | 1,404,109 | 0.499 |
  | middlegame | 8 | d1 b1 | -164 | 8 | 103,433,445 | 37.937 |

- What the baseline shows:
  - Scores swing between odd and even depths. At even depths the side
    to move is scored after the opponent's last move, usually a capture
    that nothing answers, so the scores are pessimistic. From the
    starting position, White scores -101 after e2 e3 at depth 6 and +146
    after e2 e4 at depth 5.
  - The odd-depth searches are expensive and thrown away. The depth-7
    middlegame search takes 6.7s, thirteen times the depth-6 search, and
    returns the depth-6 move.
  - The quiet middlegame is the outlier at depth 8, with 103M nodes in
    38s. It is the position to watch when choosing depths later.

### Session #3

- Step 2: `evaluateWithoutMateTest()` holds the material, position and
  castling terms, and `evaluate()` runs the mate test and then calls it.
  The search report was identical to the baseline apart from timing.
- Step 3: `generateCaptures()` filters `generateAllPotentialMoves()` for
  captures, en passant and promotions to a queen, keeping the order.
  Tested on six positions for both colors against the same filter
  written out in the test, so the dedicated generator in step 5 has a
  reference.
- Step 4: `quiesce()` as designed, called from `search()` at
  `depth <= 0`, with `Max_Quiescence_Evasion_Ply = 4`. `SearchResult`
  gains `quiescence_nodes`; `nodes` now counts both kinds, and the
  report prints both.
- Tests in `search_test.cpp`, each run against the commit before
  quiescence as well:
  - a defended pawn is not taken at depth 1 (fails before),
  - a capture that wins after the recapture is taken at depth 2 (fails
    before),
  - a mate on the last ply is found through the evasion search, and a
    check with an evasion is not scored as mate (both pass before too,
    since `evaluate()` used to find the mate; they now guard the
    evasion path),
  - a knight left to a bishop capture that leaves king and bishop
    against king keeps the root out of the table (fails before), with a
    control that a root without a draw below it is stored.
  Not added: a timeout that lands inside quiescence. It cannot be aimed
  there reliably; the UCI `stop` tests exercise the timeout path.
- `UCI: a promotion is written in lower case` failed. With the black
  king out of reach, a depth-2 search played `a1b1`: quiescence still
  counts the promotion at the leaf, so a king move first scores as the
  promotion plus a better king square. Nothing stops the pawn, so this
  is a shallow-depth artifact rather than an error. The test is about
  the move's notation, so it now uses a position where the black king
  would win the pawn if White waited (`8/P1k5/8/8/8/8/8/K7 w`); it
  promotes there at depths 1 to 4, before and after this change.
- Evasion limit, depth 6. Node counts are deterministic; times are one
  run each:

  | Position | 0 | 2 | 4 | 8 | unbounded |
  |---|---|---|---|---|---|
  | starting | 198,907 | 90,710 | 89,236 | 89,869 | 89,983 |
  | kiwipete | 454,798 | 468,666 | 498,924 | 578,117 | 662,528 |
  | italian | 858,140 | 1,135,173 | 523,964 | 581,332 | 612,350 |
  | position3 | 30,674 | 22,157 | 18,286 | 18,459 | 18,459 |
  | position4 | 128,702 | 312,680 | 280,559 | 228,272 | 276,611 |
  | middlegame | 3,785,964 | 2,613,404 | 2,816,370 | 3,274,354 | 3,409,184 |

  - Unbounded evasions terminated everywhere. Against a limit of 4 they
    cost 33% more nodes in Kiwipete, 21% in the middlegame and 17% in the
    Italian game, and about the same elsewhere. No series of checks ran away on these
    positions, but they are not built to provoke one, so the limit stays
    as a safety net.
  - Limits 0 and 2 change the move in the Italian position (`d1 e2` and
    `d2 d3` instead of `b1 c3`) and cost more there, because the
    truncated scores order moves worse.
  - Limits 4, 8 and unbounded choose the same move everywhere. Scores
    differ in three positions by 6 to 56 (kiwipete 102 against 75,
    position4 -984 against -928, middlegame 54 against 48), so a limit
    of 4 does cut some real lines short. It is the cheapest of the three
    on five of the six positions, so it stays at 4.
- Cost against the baseline at depth 6: the starting position and
  position 3 need fewer nodes (89,236 against 424,006 and 18,286 against
  30,704), since the steadier scores order moves better. The other four
  need 1.9 to 2.7 times as many, and the time per node is higher too:
  the quiet middlegame takes 2.8s instead of 0.5s for twice the nodes.
  Quiescence nodes are 29 to 81% of the total. Two things should bring that down: the
  dedicated capture generator in step 5, and the odd depths in step 6,
  which now carry real results and can replace deeper even depths.
- Full suite passes (223 tests, including QML), linter clean, no
  warnings.

### Session #4

- Step 5: `MoveGeneration` has a `captures_only` flag. `appendMove()`
  drops a move to an empty square unless it is en passant or a
  promotion, `king()` skips castling, and `pawn()` skips
  under-promotions. `generateAllPotentialMoves()` and
  `generateCaptures()` share one generate-and-sort helper. Sliding
  pieces still walk their rays; only the append and the sort get
  shorter.
- The search report showed small node count changes in Kiwipete and
  position 4 (under 0.5%) with the same moves and scores. The cause is
  the move order, not the moves. `compareMoves` treats two queen
  promotions that capture pieces of equal value as equal, since
  `promotingOrCoordCompare` compares only the promoted piece when both
  moves promote. `std::sort` may put equal elements in any order, and
  it depends on the rest of the list, which in the full list includes
  the under-promotions. So the claim in step 3 that `generateCaptures()`
  keeps the full list's order was never guaranteed, and the test only
  passed because its positions had no such pair.
  - A scratch check over 53,017 positions from random games out of the
    starting position, Kiwipete and position 4 found no difference in
    the set of moves.
  - The test now compares sets and includes a position with two pawns
    that can each promote by capturing a rook.
  - Not changed here: breaking the tie in `promotingOrCoordCompare` by
    coordinates, as it already does for moves that do not promote. That
    would make the order total, but it changes the main search's move
    order too, so it belongs in its own change.
- Timing against step 4, depth 6, medians of five alternating rounds.
  Nodes and moves as above:

  | Position | Step 4 | Step 5 | Faster by |
  |---|---|---|---|
  | starting | 0.048s | 0.031s | 35% |
  | kiwipete | 0.349s | 0.244s | 30% |
  | italian | 0.435s | 0.302s | 31% |
  | position3 | 0.009s | 0.007s | 22% |
  | position4 | 0.236s | 0.144s | 39% |
  | middlegame | 2.785s | 1.548s | 44% |

  The middlegame rounds ranged from 2.770s to 2.794s before and 1.545s
  to 1.551s after. It is still three times the baseline's 0.499s at
  depth 6, for twice the nodes.
- Full suite passes (223 tests), linter clean, no warnings.
