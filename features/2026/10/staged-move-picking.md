# Staged move picking

## Motivation

Item 6 of [improvement-suggestions.md](../09/improvement-suggestions.md):
every node generates its whole move list and sorts it, though a cutoff
usually comes within the first few moves. A staged picker hands out one
move at a time: the table move before anything is generated, then the
captures, then the killers, and the quiet moves only when nothing before
them cut off.

## Design

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
printed by `--search-report` after each position. They count, per move
list a node searched: the moves generated, the moves taken from the list
(illegal ones included), where in the list the cutoff came and which
stage the cutoff move belongs to. A stage is classified the way
`sortKey()` ranks it. A list is counted once per node that generates
one, so a quiescence node that stands pat is not counted. The probe is
a measurement and will be reverted.

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
