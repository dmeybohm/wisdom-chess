# Staged move picking

## Motivation

Item 6 of [improvement-suggestions.md](../09/improvement-suggestions.md):
every node generates its whole move list and sorts it, though a cutoff
usually comes within the first few moves. A staged picker hands out one
move at a time: the table move before anything is generated, then the
captures, then the killers, and the quiet moves only when nothing before
them cut off.

## What this branch changes

Lazy selection, the part of the design the measurements below support:
a node takes its moves from an `OrderedMoveList` (`engine/move_list.hpp`),
which holds each move with its `sortKey()`. The first move is found by
selection; the rest are sorted only when a second is taken. Most nodes
that cut off do so on their first move (Session #1), so they never sort.
The order is the one the full sort gave, so moves, scores and node
counts are unchanged.

`generateOrderedMoves()` and `generateOrderedCaptures()` fill a list the
caller gives them. A list's keys must outlive the node's children, and at
4 KB a list in every frame would exhaust Emscripten's 64 KB stack, where
a frame held a 512-byte `MoveList` before. So `IterativeSearchImpl`,
which is only created on the heap, owns one list per ply, for
`Max_Search_Depth` plies plus `Max_Quiescence_Line`, and `search()` and
`quiesce()` borrow their ply's list as a `nonnull`. That is about 520 KB
per search. `generateAllPotentialMoves()` and `generateCaptures()` drain
a list into a `MoveList`, so every caller gets the same order.

A quiescence line is at most 50 plies: 30 captures, 16 promotions to a
queen without a capture, and 4 evasions, since a check past
`Max_Quiescence_Evasion_Ply` ends the line. `Max_Quiescence_Line` is 64,
and `moveListAt()` checks the ply with `EXPECTS`.

## Staged picker, not built

A `MovePicker` with a stage enum and a `next()` that returns
`optional<Move>`, replacing the range-for over a `MoveList` in
`search()`. Each stage generates its moves as a batch with the existing
generator functions, so the state machine is at the level of stages,
not inside `slide()`, `pawn()` and the rest.
[faster-legal-move-test.md](../09/faster-legal-move-test.md) rejected a
resumable iterator over the generation loops; this avoids it.

Not C++20 coroutines. A stackless coroutine can only `co_yield` from its
own body, and moves are appended deep in the generator's helpers, so
each of those would become a coroutine with its own frame. A frame is
allocated on the heap unless the compiler elides it, and every resume is
an indirect call.

The table move and the killers are tried before generation, so they need
a pseudo-legality test of their own. Today a stale move only changes a
sort key and is never played; under staging it would be.

If the picker yields the current order, moves, scores and node counts
stay identical: `makeSortKey()` makes the order total.

## Probe

Counters in `search()` and `quiesce()` (`engine/move_picking_probe.hpp`),
printed by `--search-report` after each position, removed again before
the change was made. They count, per move
list a node searched: the moves generated, the moves taken from the list
(illegal ones included), where in the list the cutoff came and which
stage the cutoff move belongs to. A stage is classified the way
`sortKey()` ranks it. A list is counted once per node that generates
one, so a quiescence node that stands pat is not counted.

## Implementation Progress

### Session #1

- Committed the probe. With it, `--search-report 7` gives the same move,
  score, node and quiescence node counts as `origin/main` at `d2b65aa9`
  at every depth of every position.
- Ran once at depth 7, totals across depths 1 to 7. Another session was
  compiling, so no times are recorded; the counts do not depend on load.

Main search:

| Position | Lists | Moves each | Captures and promotions each | Taken | Cutoff | At first move | Table / capture / killer / quiet |
|---|---|---|---|---|---|---|---|
| starting | 49,253 | 25.6 | 0.8 | 6.6 | 78.5% | 78.4% | 21 / 30 / 47 / 1.9% |
| kiwipete | 167,182 | 45.0 | 7.6 | 9.8 | 77.3% | 98.8% | 33 / 67 / 0.1 / 0.0% |
| italian | 129,126 | 38.3 | 3.1 | 7.6 | 80.0% | 86.5% | 26 / 66 / 7.3 / 0.5% |
| position3 | 14,981 | 18.0 | 1.1 | 5.5 | 71.6% | 83.2% | 28 / 44 / 25 / 3.9% |
| position4 | 50,411 | 40.1 | 7.5 | 6.9 | 82.6% | 98.1% | 21 / 78 / 0.3 / 0.1% |
| middlegame | 310,962 | 42.6 | 3.0 | 10.1 | 76.7% | 86.4% | 30 / 58 / 12 / 0.5% |

"Cutoff" is the share of lists that cut off; "at first move" and the
stages are shares of the cutoffs. Promotions are counted with captures;
they caused under 0.1% of the cutoffs.

- 69 to 83% of the lists cut off before reaching a quiet move other than
  a killer, so a staged picker would generate and sort no quiet moves
  there. 17 to 25% cut off on the table move, and would generate
  nothing.
- When a quiet move cut off, 13 to 26% of the quiet moves had been taken
  from the list.
- The table move was in the generated list every time it was given:
  48,583 of 48,583 in kiwipete, and the same in every position. The
  killers were in the list 59 to 82% of the time, so the pseudo-legality
  test matters for them.
- The lists that do not cut off, 17 to 28%, take every move, and staging
  saves nothing there.

Quiescence, the captures of a node not in check or every move of a node
in check:

| Position | Lists | Moves each | Taken | Cutoff | At first move |
|---|---|---|---|---|---|
| starting | 98,149 | 3.5 | 1.3 | 29.4% | 86.7% |
| kiwipete | 419,218 | 9.5 | 5.8 | 45.4% | 66.0% |
| italian | 606,694 | 12.4 | 3.3 | 41.0% | 59.4% |
| position3 | 25,664 | 4.8 | 2.2 | 32.7% | 65.1% |
| position4 | 417,221 | 4.9 | 3.0 | 55.8% | 67.6% |
| middlegame | 1,528,680 | 8.2 | 2.4 | 41.9% | 77.2% |

- Quiescence has 2 to 8 times as many lists as the main search, but
  shorter ones. The moves generated in each are about equal: 12.5M in
  quiescence and 13.2M in the main search in the middlegame, 7.5M and
  4.9M in the Italian game.
- Quiescence has no table move or killers, so a picker there could
  only select lazily from the sorted captures.

What the counts do not say is the time: how much of a search the sort
and the quiet generation take. The next step is a build that picks the
lowest key on each `next()` instead of sorting the list, which keeps
the order and the node counts, and to time it against the baseline in
alternating rounds.

### Session #2

- Timed lazy selection against `origin/main` at `d2b65aa9`, in builds
  from copies of the source outside the repository, without the probe.
  The prototype adds a `KeyedMoveList` to `generate.hpp`: it holds each
  move with its `sortKey()`, and `next()` swaps the lowest key of the
  rest to the front and returns it. `search()` and `quiesce()` take
  moves from it in a `while` loop in place of the range-for over a
  sorted `MoveList`. Generation and the keys are unchanged; only the
  sort is deferred.
- After a set number of moves taken lazily, `next()` sorts the rest
  once, since selection costs O(n²) at a node that takes every move.
  Three builds: 1 move, 3 moves, and every move taken lazily.
- A first version assigned the list in `quiesce()` to a variable declared
  earlier, which copied the whole 4 KB array, where a `MoveList` copies
  only its moves. At depth 7 it was 2 to 5% slower than the baseline.
  Generating once, after the stand-pat test, removed the copy.
- `--search-report 8`, seven alternating rounds, pinned to one core,
  medians, all depths of each position summed. Every build gave the same
  move, score, node and quiescence node counts in every round.

| Build | Total | starting | kiwipete | italian | position3 | position4 | middlegame |
|---|---|---|---|---|---|---|---|
| baseline | 58.39s | 1.485 | 9.096 | 11.991 | 0.245 | 3.348 | 31.666 |
| 1 lazy, then sort | 54.25s (−7.1%) | 1.408 | 8.390 | 11.272 | 0.239 | 3.231 | 29.858 |
| 3 lazy, then sort | 55.06s (−5.7%) | 1.395 | 8.562 | 11.245 | 0.240 | 3.289 | 30.046 |
| all lazy | 55.49s (−5.0%) | 1.424 | 8.715 | 11.291 | 0.241 | 3.364 | 29.706 |

- The baseline's rounds ranged from 57.51 to 60.24s, and the lazy
  builds' from 53.66 to 57.44s: all 21 lazy runs were faster than every
  baseline run. The three lazy builds overlap, and their order is not
  settled by seven rounds.
- Deferring the sort is worth 5 to 7%. The sort is therefore a small
  part of the search; most of the cost of a move list is elsewhere,
  in generating the moves and computing their keys, which the prototype
  still does for every move. What a staged picker saves beyond this is
  the quiet moves' generation and keys at the 69 to 83% of lists that
  cut off before them (Session #1), which this prototype does not
  measure.

### Session #3

- A captures-only list walks the same squares as a full one:
  `captures_only` is a filter in `appendMove()`, and only castling is
  skipped before the walk. A capture stage saves the quiet moves'
  `transformMove()`, append and sort key, not the walk, and a node that
  reaches the quiet moves walks the board a second time. Only the table
  move stage skips the walk.
- To bound what any picker can save, two ceiling builds from copies of
  `origin/main` at `d2b65aa9` call the generator a second time at each of
  its three call sites in the search, and discard the result behind an
  empty `asm volatile` with a memory clobber. No LTO is configured, so
  the call stays. **Full** repeats the same call: generation, keys and
  sort. **Walk** calls a new `generateUnsortedMoves()`: generation and
  append, no keys, no sort. `hasLegalMove()` is not repeated.
- `--search-report 8`, seven alternating rounds, pinned to one core,
  medians. Every run matched the baseline's counts.

| Build | Total | starting | kiwipete | italian | position3 | position4 | middlegame |
|---|---|---|---|---|---|---|---|
| baseline | 58.39s | 1.361 | 9.079 | 12.030 | 0.242 | 3.395 | 31.960 |
| full, twice | 79.42s (+36%) | 1.869 | 11.779 | 16.234 | 0.304 | 4.314 | 43.557 |
| walk, twice | 76.41s (+31%) | 1.843 | 11.162 | 15.583 | 0.293 | 4.314 | 43.336 |

- Producing the move lists is about a third of the search: 21s of 58s,
  of which 18s is the walk and the appends, and 3s the keys and the
  sort. Session #2's lazy selection saved 4s, about what this puts on
  the keys and sort; the two are from different runs, so the
  difference is within the noise.
- The cost is per list, and the walk costs about the same whether it
  keeps every move or only the captures. Quiescence has 2 to 8 times as
  many lists as the main search (Session #1), so most of the 18s is
  likely quiescence's capture generation, where a staged picker has no
  table move to try first. The split between the two was not measured.
- The walk starts with `Board::allCoords()`, testing all 64 squares for
  a piece of the side to move, before generating anything.

### Session #4

- Removed the probe, merged `origin/main` at `5656ed41`, and made the
  change described under "What this branch changes". The prototype of
  Session #2 kept its list in each frame; the lists moved to the search
  for the WASM stack.
- Taking one move by selection and then sorting the rest was the fastest
  of Session #2's three builds, though not by a settled margin; it also
  bounds the cost at a node that takes every move to one selection and
  one sort.
- `--search-report 7` gives the same move, score, node and quiescence
  node counts as `main` at every depth of every position.
- Tests: `OrderedMoveList` order and `clear()`, a list filled again for
  another position, and a fatal case for appending past the end. The
  ordering tests in `generate_test.cpp` now run through the list, since
  `generateAllPotentialMoves()` does. All 338 tests pass in Release with
  the slow tests on, and the fast and medium ones in Debug.
- `--search-report 8` against `main` at `5656ed41`, seven alternating
  rounds pinned to one core, each run started only when the one-minute
  load was under 1.5. A first set, taken while the machine was busy
  after a boot (load 3 to 7), was discarded. Every run gave the same
  counts as `main`.

| Build | Total | starting | kiwipete | italian | position3 | position4 | middlegame |
|---|---|---|---|---|---|---|---|
| `main` | 58.89s | 1.425 | 8.985 | 12.294 | 0.235 | 3.395 | 32.535 |
| this branch | 54.23s (−7.9%) | 1.353 | 8.162 | 11.361 | 0.226 | 3.197 | 29.888 |

  `main` ranged from 58.73 to 58.93s and the branch from 54.01 to
  54.63s.
- Item 6 of [improvement-suggestions.md](../09/improvement-suggestions.md)
  stays open: the staged picker was not built, and Session #3 puts its
  likely gain beyond this change at a few percent.
