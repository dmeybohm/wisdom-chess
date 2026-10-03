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
