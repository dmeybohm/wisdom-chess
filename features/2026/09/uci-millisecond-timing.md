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
