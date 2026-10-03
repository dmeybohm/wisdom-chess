# A `noexcept` search

## Motivation

After `noexcept-fixes`, `logger-noexcept` and `noexcept-mechanical`, the
engine's rule is that a failure inside it terminates with a report. The
search is the one place that still catches and rethrows:
`IterativeSearchImpl::iterativelyDeepen()` wraps any `Error` in a
`SearchError` whose extra info ends with the root board. That mechanism
serves its own two tests and nothing else; no frontend catches a
`SearchError` and recovers.

The recursion, `search()` and `quiesce()`, pushes a tentative position
before each child and pops it after, by hand. An exception unwinding
between the two would skip the pop and leave that `History` with an
extra board code and a nesting count above zero. It is harmless today
only because the search works on its own copy of the `History`, which
dies with it. A `noexcept` recursion has no unwinding path at all, so
the pair can only be left by `return`, and the manual pairing is as safe
as a guard object would be. It is kept manual on purpose: a guard would
be a non-trivial destructor on the hot path, the one thing that gives
the compiler a cleanup to keep live.

## What it costs today

Table-based unwinding puts no instructions on the non-throwing path. The
cost is what the compiler must forgo around a call that might throw:
state kept live for cleanups, no reordering across the call. The search
frame holds a `MoveList` and a `Board`, both trivially destructible, so
there are no cleanups. What remains is four potentially throwing calls
per node: `MoveTimer::isTriggered()` through the caller's
`std::function`, `History::addTentativePosition()` through the vector,
`TranspositionTable::store()` through a `narrow`, and
`evaluateWithoutMateTest()` through its `ENSURES`. The expected gain
from marking them is a percent or two at most, quite possibly below the
noise of a measurement. This branch stands on the contract, and the
measurement is recorded whatever it shows.

## Design

- `search()`, `quiesce()` and `isLegalMove()` are `noexcept`.
  `iterativelyDeepen()` stays throwing for its own `ENSURES`, which run
  outside the recursion, and `iterate()` for its logging.

- `TranspositionTable::store()` is `noexcept`. Its `narrow<int16_t>` on
  the depth becomes `noexcept_narrow`.

- `History::addTentativePosition()` is `noexcept`. The vector is
  reserved when the search is created, for the deepest line a search
  can reach: the positions already in the history, the search depth,
  and the quiescence line, which the material bounds. The one allocation
  moves outside the recursion; a reservation that proves short
  terminates, where it threw before.

- `MoveTimer::isTriggered()` is `noexcept`, which makes the periodic
  function part of the contract: it must not throw. The three frontends'
  notifiers set a flag or emit a Qt signal, so they already honour it,
  and the header says so.

- `NOEXCEPT_ENSURES( cond )` joins the contract macros in `error.hpp`,
  the postcondition twin of `NOEXCEPT_EXPECTS`: it reports a
  "Postcondition" failure through `terminateOnCheckFailure()` and
  aborts, for a `noexcept` function where `ensures()` could not
  propagate its exception. `evaluate()` and `evaluateWithoutMateTest()`
  use it for their score-range check and become `noexcept`, which makes
  the whole evaluation honest rather than throwing into a `noexcept`
  caller. `AGENTS.md` lists the macro with the others.

- The `SearchError` class and the `catch` in `iterativelyDeepen()` go.

- The root position is logged once per search, at the start of
  `iterativelyDeepen()`, through `debug()`, as a FEN on one line, so
  that a `BufferedLogger` holds it when an emergency drains the buffer.
  That is what the `SearchError` carried; it is the fact needed to
  reproduce a termination mid-search, and a FEN is what a UCI
  `position fen` command takes. One line per search, against the string
  builds each depth already does for "Searching depth" and "finding
  moves for", so it costs nothing a measurement can see. Nothing is
  logged per node.

## Tests

- `search_test.cpp`, "An error during the search is rethrown with the
  board", and `Fatal: search-error`: both inject a throw through the
  periodic function and expect the `SearchError`. They go.
- A new `Fatal: periodic-function-throws` case throws from the periodic
  function and expects the terminate handler's "Uncaught error: boom",
  pinning the new contract for `MoveTimer::isTriggered()`.
- A new `Fatal: history-reservation-exceeded` is not added: the
  reservation is a performance measure, and a vector that grows anyway
  still works, it only allocates.
- A `search_test.cpp` case checks that the root position reaches the
  logger's `debug()` exactly once per search.
- `Fatal: noexcept-ensures-failure`, next to `expects-through-noexcept`,
  expecting "Postcondition failed at" with the condition quoted, and a
  `global_test.cpp` case that the macro passes a true condition.

## Measurement

The pinned alternating-rounds time share, before and after: a
`noexcept` change must leave moves, scores and node counts identical,
so only the time can differ, and a percent or two would not show in a
match. `wisdom-chess-benchmarks --search-report 7` from a scratch
build of each revision, `taskset -c 2`, eight rounds each, alternating.
The result goes under Implementation Progress whether or not it shows
a difference.

## Out of scope

- The other `ENSURES` in the engine, in `Board`'s constructor,
  `TranspositionTable`'s constructor and `iterativelyDeepen()`. They run
  outside any `noexcept` function and keep throwing.
- A `thread_local` root-board pointer for the emergency path. It would
  cost nothing per search but couples the search to the logger's
  emergency machinery, for one diagnostic the debug line already gives.

## Implementation Progress

### Session #1

Branched from `noexcept-mechanical`, which it needs for the `noexcept`
move generators. The log was written first for review, and the review
added `NOEXCEPT_ENSURES` for the evaluators.

Implemented as written. The linter's two macro lists name the new macro,
so `NOEXCEPT_ENSURES( cond )` is spaced like the others. The history
reservation is `total_depth + Max_Quiescence_Line`, with the line set to
`Num_Squares`: a capture-only line has at most one ply per piece taken,
and past `Max_Quiescence_Evasion_Ply` a check ends the line, so 64 is
generous and a search never grows the vector.

Verification: Release and Debug builds compile without a warning, the
linter passes, all 284 Release tests and all 248 Debug tests pass, the
fourteen `Fatal: ...` cases among them. The frontends were not rebuilt:
nothing they define changed its signature, and none caught
`SearchError`.

Measurement, `noexcept-mechanical` (`9b69ad93`) against this branch:
every row's move, score and node counts are identical between the two
builds. The whole report takes a median 10.01 s before and 10.07 s
after; depth 7 alone 7.96 s and 8.00 s. The rounds spread about 2%
either side of their medians, so a change of half a percent is noise.
The six positions at depth 7 range from +0.0% to +1.1%, all within
that spread. As expected, there is nothing to measure: the frame has no
cleanups, so removing the unwinding paths changes no code the hot loop
runs. The branch stands on the contract.

### Session #2

Review of the pull request: the root position was logged as
`Board::asString()`, which spans many lines, and `UciLogger::debug()`
prefixes only the first with `info string`, so in UCI debug mode the
other rows reached the GUI bare. The line is now a FEN, which is one
line and is what `position fen` takes to reproduce the search.

A `UCI: ...` test that runs `debug on` and a search and forbids any
output line without a UCI prefix then found a second offender that
predates this branch: the search's statistics message spans two lines,
and its "transposition table" line reached the GUI bare too. So
`UciLogger::debug()` and `info()` now write every line of a message
with the prefix, through `sendPrefixedLines()`, under one hold of the
output lock, as `sendEmergencyLines()` already did for emergencies.
`wisdom-chess-uci-tests` pins it.
