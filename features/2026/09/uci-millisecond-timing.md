# Millisecond search timing in UCI

## Motivation

Measuring playing strength means engine-against-engine matches over UCI
at short clocks, such as 8 seconds plus 0.08 per move. The UCI frontend
could not play them: `handleGo()` converted the search budget with
`duration_cast<std::chrono::seconds>` and raised 0 to one second,
because `MoveTimer` and `Game::setSearchTimeout()` only took whole
seconds. Every search took about a second, whatever it was asked for:

- `go movetime 100` took 1,038 ms.
- `go movetime 1500` took 1,024 ms, rounded down rather than up.
- `go wtime 150` took 1,010 ms, which loses on time.

This branch starts from `main` at `185c0d5`, so it can be merged into
`main`, `quiescence-search` and `static-exchange-evaluation`, which are
the three engines the matches compare.

## Changes

- `MoveTimer` holds its limit as `chrono::milliseconds`. Its accessors
  are `getTimeLimit()` and `setTimeLimit()`; the old `getSeconds()` and
  `setSeconds()` would have been misleading once they carry
  milliseconds. The constructor that takes an `int` still means seconds.
- `Game::getSearchTimeout()` and `setSearchTimeout()` use
  `std::chrono::milliseconds`. Callers that pass `std::chrono::seconds`
  convert implicitly, so the console, QML and wasm frontends are
  unchanged.
- UCI passes its budget through unrounded.
- UCI never allocates more than half of the remaining clock. The old
  100 ms minimum could ask for more time than was left.

## Implementation Progress

### Session #1

- Made the changes above. Updated `move_timer_test.cpp`,
  `search_test.cpp` and the QML `chess_game_test.cpp` for the new names
  and units, and added a test that a 50 ms budget triggers after at
  least 50 ms and well within 500 ms.
- Measured by timing the reply to `go` through the UCI binary, fresh
  process, cleared table:

  | Command | Before | After |
  |---|---|---|
  | `go movetime 100` | 1,038 ms | 134 ms |
  | `go movetime 250` | 1,024 ms | 258 ms |
  | `go movetime 1500` | 1,024 ms | 1,501 ms |
  | `go wtime 8000 winc 80` | 1,012 ms | 358 ms |
  | `go wtime 150` | 1,010 ms | 104 ms |
  | `go wtime 20` | 1,040 ms | 32 ms |
  | `go movetime 100`, quiet middlegame | 1,021 ms | 112 ms |

  A search overshoots its budget by up to about 35 ms. The timer reads
  the clock only every 10,000 to 1,000,000 calls, adjusted to land 25 to
  50 ms apart, so that is its resolution. It matters only below about
  100 ms left: with 20 ms on the clock the reply took 32 ms. At 8
  seconds plus 0.08 per move the increment keeps the clock well above
  that. Tightening the resolution is left alone unless matches show
  losses on time.
- Full suite passes (220 tests, including QML), linter clean, no
  warnings.

### Session #2

- Review finding: short clocks could still lose on time. With `go wtime
  10` five runs answered in 10.3 to 12.1 ms, and `go wtime 20` exceeded
  20 ms in two of five. Two causes:
  - `MoveTimer` read the clock only once per batch of calls, and
    `TimingAdjustment` sized the batch so that reads landed 25 to 50 ms
    apart, never fewer than 10,000 calls, with the size saved
    process-wide between searches. The overshoot did not shrink with the
    budget, so halving the budget could not help.
  - UCI kept nothing back for the work outside the search: copying the
    game, starting the search thread, answering, and scheduling delay.
- Considered:
  - A larger margin in UCI alone would have to cover 50 ms of timer
    resolution, a large share of every move at short clocks.
  - Scaling the adaptive interval to the budget keeps the mechanism but
    adds budget-dependent bounds and rescaling of the saved batch size.
  - Chosen: read the clock every fixed 1,024 calls
    (`Calls_Between_Clock_Checks`, a power of two so the test is a
    mask), and remove `TimingAdjustment`, its saved state and its four
    constants. The periodic functions run on the same batch. All three
    (UCI, QML, wasm) only load atomics and perhaps cancel, so running
    them every half millisecond instead of every 25 to 50 ms costs
    nothing, and `stop`, pausing and settings changes take effect sooner.
- The cost of a clock read is not guaranteed, since the standard does not
  say how `steady_clock::now()` is implemented. Measured here: 13.9 ns
  on Linux, with no system calls under `strace` (the time-stamp counter
  clock source); 105 to 122 ns in WebAssembly under Node, where it calls
  out to `performance.now()`. From the implementations, macOS reads a
  page the kernel maps into every process, and Windows'
  `QueryPerformanceCounter` stays in user space on hardware with a stable
  time-stamp counter. It can become a system call or a hypervisor exit:
  on Linux without a usable time-stamp counter (`acpi_pm` or HPET), and
  on some Windows VMs and older machines. Batching bounds the cost
  either way. One read per 1,024 calls, which take about 0.4 ms here, is
  0.003% of search time at 14 ns, 0.25% at 1 us, and about 1% at a
  pessimistic 5 us.
- Added the `Move Overhead` UCI option, milliseconds kept back from the
  clock, default 10 and range 0 to 5,000, as in Stockfish and other
  engines. The allocation is now:

  ```
  available     = max (remaining - move_overhead, 0)
  time_for_move = available / 30 + increment
  time_for_move = max (time_for_move, 100)
  time_for_move = min (time_for_move, available / 2)
  time_for_move = max (time_for_move, 1 ms)
  ```

  The last line matters because a zero budget means "no limit", which
  falls back to the game's 2-second default. With 1 ms the search stops
  before depth 1 finishes, and UCI's existing fallback answers with a
  random legal move.
- Tests: `move_timer_test.cpp` now checks exactly that a spent budget
  triggers at call 1,024 and that the periodic function runs once per
  batch; the `TimingAdjustment` tests are gone. A new CLI test checks
  that a clock inside the overhead still answers with a legal move, and
  the handshake test expects the new option.
- Reply time to `go wtime N btime N`, 20 runs per clock alternating the
  starting position and the quiet middlegame:

  | Clock | Idle, max | 5 busy processes, max | 7 busy, max | 7 busy, overhead 30, max |
  |---|---|---|---|---|
  | 10 ms | 1.6 ms | 18.1 ms (1 over) | 12.0 ms (1 over) | 5.4 ms |
  | 20 ms | 5.8 ms | 21.7 ms (1 over) | 22.1 ms (1 over) | 1.7 ms |
  | 50 ms | 20.5 ms | 30.4 ms | 33.2 ms | 11.7 ms |
  | 100 ms | 45.6 ms | 48.8 ms | 59.8 ms | 36.4 ms |
  | 150 ms | 70.6 ms | 81.2 ms | 82.5 ms | 60.8 ms |
  | 300 ms | 100.5 ms | 100.9 ms | 100.6 ms | 106.4 ms |

  Idle, a search now overshoots its budget by under a millisecond, and
  every reply came inside its clock. Under load, the medians barely
  moved, but occasional scheduling delays of 10 to 17 ms put one run in
  twenty over a 10 or 20 ms clock. That is what `Move Overhead` is for:
  at 30 ms nothing went over, even with seven busy processes. The
  default stays at 10 ms. Matches that run several games at once should
  set it higher.
- Speed: `go depth 6` on four positions, five alternating rounds against
  the previous commit. Median 0.849 s against 0.873 s, within noise, with
  the same moves.
- Full suite passes (220 tests, including QML), linter clean, no
  warnings.
