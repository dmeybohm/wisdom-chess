# Move ordering in the main search

## Motivation

Item 1 of [improvement-suggestions.md](../09/improvement-suggestions.md).
`search()` orders a node's moves once, when
`generateAllPotentialMoves()` sorts the whole list with
`compareMoves()` (`engine/generate.cpp`): the transposition table's
move first, then captures by the material difference between the
captured and the capturing piece, then promotions by the promoted
piece, then everything else by source and destination square. Quiet
moves, which are most of the list, come out in board order, so the
first quiet move tried at a node is whatever stands nearest a8.

Alpha-beta's cost depends on trying a good move early. Session #7 of
[quiescence-search.md](../09/quiescence-search.md) found the main
search visiting 2.4 times the baseline's nodes in the Italian game
once quiescence was on, before quiescence itself added anything, and
put it down to ordering: scores that include quiescence lie closer
together, so a captures-first order finds fewer early cutoffs. It named
killer moves or a history heuristic as the remedy, and item 4's pruning
and reductions all assume an ordering that tends to be right.

## Design

Two tables, both search state, both read by the sort.

### Killer moves

A quiet move that caused a beta cutoff at a ply is likely to cut off
the sibling nodes at the same ply, since they differ by one move of
the opponent. The search keeps two killers per ply. On a cutoff by a
quiet move at ply `p`, the move goes into slot 0 of ply `p` and the
old slot 0 moves to slot 1, unless it is already in a slot. Captures
and promotions are not stored: the material order already puts them
ahead. A quiet move is one that `isAnyCapturing()` and `isPromoting()`
both deny; castling counts as quiet.

Killers are sorted after the captures and promotions and before the
other quiet moves. A killer need not be legal, or even possible, in a
sibling position: the generator lists only the moves the position has,
and the sort only ranks the ones that are in the list, so a stale
killer costs nothing but its place in the order.

The table is indexed by ply, and `search()` runs at plies below the
depth limit, which the UCI frontend clamps to 64. The limit becomes a
named constant in `global.hpp`, shared by the clamp and the table, so
the two cannot drift apart.

### History heuristic

Among the quiet moves that are not killers, the ones that caused
cutoffs anywhere in the tree go first. A table of counters indexed by
color, source square and destination square is raised by `depth *
depth` when a quiet move causes a cutoff, so a cutoff near the root,
which saved a bigger subtree, counts for more. The sort compares two
quiet moves by their counters and falls back to the square order when
they are equal, so the output is still a total order and the same
moves still sort the same way when the tables are empty.

The table is 2 x 64 x 64 `int32_t`, 32 KB. The counters are not aged
within a search in the first version: a search here is a few hundred
million nodes at most, far from overflowing, and a counter that only
grows still ranks moves against each other. Decay between the
iterations of deepening is the first knob to try if the measurement
says the early, shallow cutoffs are drowning the later ones.

### Where the tables live

Both tables are members of `IterativeSearchImpl`, created empty with
each search and shared by all its iterations, so what depth 5 learned
orders depth 6. They are not kept across the moves of a game. The
transposition table is kept across moves because a position recurs
with its full result attached; a killer or a counter is a hint about a
ply or a square pair, rebuilt in the first few thousand nodes, and
keeping it would add owner and lifetime rules for a hint. If the
measurement shows the first iteration paying for empty tables, the
place to revisit is the `IterativeSearch` factory, which already takes
the caller's transposition table.

### How the sort sees them

`generateAllPotentialMoves (board, who, priority_move)` takes the table
move today. It takes a `MoveOrdering` in its place: the priority move,
the two killers for the ply and a reference to the history table. The
overload without ordering stays for the callers that want the moves in
their plain order (`generateLegalMoves()`, the tools and the tests),
and `generateCaptures()` is unchanged, so `quiesce()` is not touched.
`compareMoves()` gains the two steps between the captures and the
square order. The sort is still one `std::sort` over the whole list;
replacing it with staged picking is item 6 and stays there.

The sort's cost does not change in kind: two more comparisons per pair
and, for quiet pairs, two table reads. Whether it changes in degree is
part of the measurement.

### What should and should not change

At a fixed depth alpha-beta returns the same score whatever the move
order, so each report position should show the same score at every
depth as `main`, and fewer nodes. Three things can move the result
anyway, and a difference has to be traced to one of them before it is
accepted:

- Ties. When two root moves score the same, the one tried first wins,
  and the order of quiet moves is what changes here.
- The transposition table. A different order stores different
  entries at the shared depths, and a probe at a later depth may then
  return a bound it did not have before. The score is still exact
  within the window, but the move stored with it can differ.
- Draw scores. `Search_Draw_Contempt` makes a repetition worth less to
  the side to move than a quiet line, so a line found in a different
  order can pass through a repetition the old order did not reach. See
  [path-dependent-draw-scores.md](../09/path-dependent-draw-scores.md).

Node counts at depth 8 are the number to watch. Session #7's figure
for the Italian game is the one the change exists to bring down.

## Measuring

As in [cheaper-has-legal-move.md](../09/cheaper-has-legal-move.md):

1. Baseline: `wisdom-chess-benchmarks --search-report 7` from
   `origin/main`, pinned to one core, seven rounds alternating with
   the branch build, medians; one run of each at depth 8. The Session
   #1 table in that document is from before `DrawArbiter` and the
   root-draw change, so a fresh baseline is needed.
2. Ordering quality: a scratch counter of the cutoffs made by the
   first legal move tried at a node, as a share of all cutoffs, before
   and after. It is read from a throwaway build, not committed. The
   report already prints the node counts.
3. Strength: `./scripts/run-engine-match.sh base=<main> new=<branch>`
   with its defaults, 500 games at 8+0.08. At a time limit the saving
   turns into depth, and the match is what shows whether the depth is
   worth anything.

Killers alone are measured before the history table is added, so the
log can say what each one bought.

## Testing

- `generate_test.cpp`: with a `MoveOrdering`, the table move is first,
  then captures, then a killer that is in the list comes before the
  other quiet moves, a killer that is not in the list changes nothing,
  and two quiet moves are ordered by their history counters and by
  square when the counters tie. The existing cases on the plain
  overload are unchanged.
- The killer table: a store puts the move in slot 0 and demotes the
  old one; storing a move already in a slot leaves the table alone;
  a capture or a promotion is not stored.
- The perft tests already compare the generated set with known counts,
  so a sort that lost or duplicated a move would fail them.
- The search tests pin moves and scores at fixed depths. One whose
  move changes is examined against the three causes above before it
  is updated.
- Everything runs in Debug as well as Release.

## Plan

1. The depth-limit constant, the killer table and its tests. Measure.
2. The history table and its tests. Measure.
3. The engine match.
4. Update the ordering paragraph in
   [engine-architecture.md](../../../docs/engine-architecture.md),
   which describes the order the sort produces, and tick item 1 in
   the suggestion list.

The branch is `move-ordering`, from `origin/main` at `e613ef39`.

## Implementation Progress

### Session #1

- Wrote this document. No code changed and nothing measured.

### Session #2

- Merged `origin/main` at `7bcff428`, which brought the search's
  `noexcept` work and the logger changes.
- Baseline: `--search-report 8` from `origin/main` at `7bcff428`, pinned
  to one core. Its moves, scores and node counts at depths 6 and 8 are
  those of Session #1 of
  [cheaper-has-legal-move.md](../09/cheaper-has-legal-move.md), so the
  `DrawArbiter` and root-draw changes did not move the report.
- Step 1 of the plan: `Max_Search_Depth` (64) in `global.hpp`, now the
  bound of the UCI clamp and the `Depth` option, of `Game::setMaxDepth()`,
  of the console's depth command and of `IterativeSearch::create()`.
  `KillerTable` and `MoveOrdering` are in `move_ordering.hpp`.
  `generateAllPotentialMoves()` takes a `MoveOrdering` in place of the
  priority move. The table is a member of `IterativeSearchImpl` and is
  written on every beta cutoff of `search()`, not `quiesce()`.
- Verified: Release, 288 of 288 tests; Debug, 252 of 252 fast tests; no
  warnings; the lint target passes. Qt was not configured, so the QML
  tests did not run.
- Measured with `--search-report 7`, seven alternating rounds, pinned,
  medians. Moves and scores are those of `main` at every depth from 1
  to 8 in every position, so none of the three causes in "What should
  and should not change" came up.

  | Position | Move | Score | Nodes before | After | Nodes | Before | After | Faster by | Rounds faster |
  |---|---|---|---|---|---|---|---|---|---|
  | starting | e2 e4 | 63 | 862,023 | 378,524 | 0.44x | 0.293s | 0.127s | 2.31x | 7 of 7 |
  | kiwipete | e2xa6 | 66 | 1,717,412 | 1,707,762 | 0.99x | 0.703s | 0.704s | 1.00x | 3 of 7 |
  | italian | b1 c3 | -32 | 1,831,708 | 1,710,775 | 0.93x | 0.881s | 0.824s | 1.07x | 7 of 7 |
  | position3 | b4xf4 | 81 | 112,520 | 73,156 | 0.65x | 0.043s | 0.027s | 1.59x | 7 of 7 |
  | position4 | c4 c5 | -928 | 925,565 | 872,645 | 0.94x | 0.355s | 0.341s | 1.04x | 7 of 7 |
  | middlegame | f3 g5 | 57 | 14,655,186 | 19,211,770 | 1.31x | 5.922s | 7.932s | 0.75x | 0 of 7 |

  The six searches together: 8.20s before, 9.96s after, all of the
  loss in the middlegame.
- Measured with `--search-report 8`, one run of each build.

  | Position | Move | Score | Nodes before | After | Nodes | Before | After | Faster by |
  |---|---|---|---|---|---|---|---|---|
  | starting | e2 e4 | 0 | 3,801,392 | 1,290,214 | 0.34x | 1.46s | 0.56s | 2.6x |
  | kiwipete | d5xe6 | 48 | 8,144,988 | 8,099,877 | 0.99x | 3.66s | 3.89s | 0.94x |
  | italian | d1 e2 | -54 | 40,692,807 | 15,421,626 | 0.38x | 21.64s | 8.53s | 2.5x |
  | position3 | b4xf4 | 81 | 261,172 | 161,154 | 0.62x | 0.10s | 0.07s | 1.5x |
  | position4 | c4 c5 | -928 | 2,620,947 | 2,542,537 | 0.97x | 1.00s | 1.04s | 0.96x |
  | middlegame | f3 g5 | 48 | 309,896,444 | 117,455,329 | 0.38x | 142.76s | 55.97s | 2.6x |

  The six depth-8 searches together: 170.6s before, 70.1s after. The
  Italian game's depth-8 count, the figure this change exists for,
  fell to 0.38 of `main`'s.
- Ordering quality, from a throwaway build of each that counts the
  cutoffs made by the first legal move tried at a node, over the whole
  iterative search to depth 7:

  | Position | Cutoffs before | First move | Share | Cutoffs after | First move | Share |
  |---|---|---|---|---|---|---|
  | starting | 56,190 | 29,986 | 53.4% | 32,311 | 24,235 | 75.0% |
  | kiwipete | 87,251 | 86,454 | 99.1% | 87,251 | 86,455 | 99.1% |
  | italian | 85,151 | 74,648 | 87.7% | 85,890 | 74,701 | 87.0% |
  | position3 | 10,605 | 8,672 | 81.8% | 6,981 | 6,278 | 89.9% |
  | position4 | 33,162 | 32,728 | 98.7% | 33,051 | 32,666 | 98.8% |
  | middlegame | 532,076 | 416,830 | 78.3% | 851,183 | 706,489 | 83.0% |

- Kiwipete and position 4 are tactical: nearly every cutoff there is a
  capture already, and the killers change neither the count nor the
  order. What they cost there, 4 to 6% at depth 8, is the extra
  comparisons in the sort.
- Not explained: the middlegame at depth 7 visits 1.31 times `main`'s
  nodes although its first-move share rose, and every other depth of
  the same position fell. The order changed what the transposition
  table holds when depth 7 starts, which is the likeliest place to
  look, but it was not traced. Check whether the history table of step
  2 removes it before spending time on it.
- Not measured yet: the engine match, which is step 3, after the
  history table.

### Session #3

- Step 2 of the plan. The table is `CutoffHistory`, so that it is not
  confused with the game's `History`. `MoveOrdering` holds it as a
  `nullable<const CutoffHistory>`, null for the plain order. A counter
  stops at `Max_Score` (2^30) rather than overflowing; the design's
  argument that a search cannot get there stands, and the cap costs
  one comparison per cutoff.
- Measured against `main` and the killers alone, seven alternating
  rounds of `--search-report 7`, pinned, medians. Moves and scores were
  `main`'s at every depth.

  | Position | Nodes, main | Killers | Killers and history | Time, main | Killers and history |
  |---|---|---|---|---|---|
  | starting | 862,023 | 378,524 | 354,803 | 0.278s | 0.134s |
  | kiwipete | 1,717,412 | 1,707,762 | 1,705,192 | 0.676s | 0.789s |
  | italian | 1,831,708 | 1,710,775 | 1,670,231 | 0.838s | 0.918s |
  | position3 | 112,520 | 73,156 | 79,449 | 0.040s | 0.032s |
  | position4 | 925,565 | 872,645 | 851,219 | 0.335s | 0.349s |
  | middlegame | 14,655,186 | 19,211,770 | 6,359,105 | 5.622s | 2.613s |

  The history table removes the middlegame's depth-7 regression of
  Session #2, so it was not traced further. The first-move share of
  cutoffs there rose to 86.0%, from 78.3% on `main` and 83.0% with the
  killers alone.
- The sort cost more than the nodes saved: kiwipete visited `main`'s
  nodes and took 17% longer, and the Italian game was slower than
  `main` with fewer nodes. The comparator worked out the capture test,
  both pieces' weights, the killer slots and two history lookups again
  for every pair it compared.
- So each move now gets one 64-bit sort key, computed once, and
  `std::sort` compares keys (`MoveGeneration::sortKey()`). From the
  most significant bits: the kind of move (priority, capture,
  promotion, first killer, second killer, other quiet), a score within
  the kind (the material difference of a capture, or the inverted
  history counter), the promoted piece, then the source and
  destination squares. It is still one sort over the whole list; item
  6's staged picking is untouched.
- The key orders as the comparator did, with one difference: two
  promotions to the same piece with the same material difference were
  equal to the comparator, and `std::sort` left them in whatever order
  it reached. The key puts them in square order. Node counts moved by
  4 in kiwipete and 27 in position 4, the two report positions with
  promotions in reach, and nowhere else.
- A new test walks two plies from three positions with promotions,
  gives each node a priority move, killers and history scores, and
  checks every pair of the sorted list against the rules stated one
  pair at a time. Swapping the first killer's kind for a quiet move's
  fails it 85 times.
- Verified: Release, 289 of 289 tests; Debug, 253 of 253 fast tests; no
  warnings; the lint target passes. Qt was not configured.
- The finished branch against `main`, seven alternating rounds of
  `--search-report 7`, pinned, medians:

  | Position | Move | Score | Nodes before | After | Nodes | Before | After | Faster by | Rounds faster |
  |---|---|---|---|---|---|---|---|---|---|
  | starting | e2 e4 | 63 | 862,023 | 354,803 | 0.41x | 0.294s | 0.108s | 2.72x | 7 of 7 |
  | kiwipete | e2xa6 | 66 | 1,717,412 | 1,705,188 | 0.99x | 0.727s | 0.566s | 1.28x | 7 of 7 |
  | italian | b1 c3 | -32 | 1,831,708 | 1,670,231 | 0.91x | 0.911s | 0.684s | 1.33x | 7 of 7 |
  | position3 | b4xf4 | 81 | 112,520 | 79,449 | 0.71x | 0.044s | 0.028s | 1.57x | 7 of 7 |
  | position4 | c4 c5 | -928 | 925,565 | 851,246 | 0.92x | 0.366s | 0.298s | 1.23x | 7 of 7 |
  | middlegame | f3 g5 | 57 | 14,655,186 | 6,359,105 | 0.43x | 6.085s | 1.994s | 3.05x | 7 of 7 |

  The six searches together: 8.43s before, 3.68s after. Kiwipete and
  position 4, where the ordering saves almost no nodes, are now faster
  than `main` too: the keys made the sort cheaper than `main`'s.
- And at depth 8, one run, against the baseline of Session #2:

  | Position | Move | Score | Nodes before | After | Nodes | Before | After | Faster by |
  |---|---|---|---|---|---|---|---|---|
  | starting | e2 e4 | 0 | 3,801,392 | 1,202,514 | 0.32x | 1.46s | 0.46s | 3.2x |
  | kiwipete | d5xe6 | 48 | 8,144,988 | 7,993,490 | 0.98x | 3.66s | 3.04s | 1.2x |
  | italian | d1 e2 | -54 | 40,692,807 | 12,831,964 | 0.32x | 21.64s | 5.83s | 3.7x |
  | position3 | b4xf4 | 81 | 261,172 | 159,554 | 0.61x | 0.10s | 0.06s | 1.7x |
  | position4 | c4 c5 | -928 | 2,620,947 | 2,528,112 | 0.96x | 1.00s | 0.90s | 1.1x |
  | middlegame | f3 g5 | 48 | 309,896,444 | 69,023,405 | 0.22x | 142.76s | 28.92s | 4.9x |

  The six depth-8 searches together: 170.6s before, 39.2s after.
- Step 4: updated the ordering paragraph of
  [engine-architecture.md](../../../docs/engine-architecture.md).
- Step 3: `./scripts/run-engine-match.sh base=7bcff428 new=97729105`
  with its defaults, 500 games at 8+0.08, four at a time; 47 minutes.

  | Pairing | Games | W / D / L | Score | Elo | 95% range |
  |---|---|---|---|---|---|
  | new vs base | 500 | 170 / 255 / 75 | 59.5% | +67 | +46 to +88 |

  Of the 250 opening pairs, `main` won both games of 1 and the branch
  won both of 28. No game was lost on time or to an illegal move.

### Session #4

- The WASM CI job failed the ordering tree test with "memory access out
  of bounds". A `CutoffHistory` is 32 KB, and the test kept one on the
  stack in each recursive frame, which exhausts Emscripten's 64 KB
  default stack. The search keeps its table in the heap-allocated
  `IterativeSearchImpl`, so only the tests change: they allocate it
  with `make_unique`. Reproduced locally; all 166 fast WASM tests pass
  with the fix.
- Rewrote item 1 of `improvement-suggestions.md` to describe the old
  ordering in the past tense and the current one in its "Done" note.
