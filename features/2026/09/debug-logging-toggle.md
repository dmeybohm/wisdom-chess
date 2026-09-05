# Runtime debug-logging toggle with a retained ring buffer

## Problem

Every frontend passes a logger to `Game::findBestMove` that prints the
engine's full search trace on every engine move ("Searching depth N",
nodes/sec, transposition-table stats, "move selected"). The console
always uses `makeStandardLogger()` at debug level, the WASM `WebLogger`
forwards everything to `console.log`, and the QML build gates on a
compile-time `NDEBUG` check. This is overwhelming during normal play,
but simply discarding the output would make bugs that show up before
logging is enabled impossible to diagnose.

## Design

- A **Debug Logging** toggle is exposed as an in-app setting in each
  frontend: a checkbox in the QML Settings dialog and the React
  Settings modal (next to Flip Board), and a `debug on` / `debug off`
  command in the console. Default is off. Like every other setting it
  is not persisted across launches.
- **Off does not discard output.** The engine gains a `BufferedLogger`
  that wraps a frontend "sink" logger and a `LogRingBuffer` holding the
  last 64 KB of timestamped lines. While disabled, lines go into the
  ring buffer (oldest whole lines are evicted). When the toggle flips
  to on, the retained lines are replayed into the sink in order and the
  buffer is cleared; subsequent lines are forwarded live.
- `BufferedLogger` prepends a `[HH:MM:SS.mmm]` local-time timestamp to
  every line, buffered or live, so replayed history is visibly old and
  the sinks stay simple.
- `NullLogger` remains truly null so the engine tests keep using it.
- The composite lives in the engine (`logger.hpp/.cpp`) so the
  off-to-on transition is implemented and unit-tested once rather than
  in each frontend. Each frontend holds one long-lived `BufferedLogger`
  and calls `setEnabled (bool)` when its settings change.

### Per-frontend wiring

- **Console** (`ui/console/play.cpp`): `ConsoleGame` owns the logger.
  New `PlayCommand::SetDebugLogging` and a `debug` command.
- **QML** (`ui/qml/main/`): `GameSettings` gains a `debugLogging`
  property (and equality), `ChessGame::Config` gains `debugLogging`,
  `ChessGame` gains a `config()` getter, and `ChessEngine` owns the
  logger and syncs it in `updateConfig` / `reloadGame`. The compile-time
  `Log_Level` is removed. A checkbox is added to `SettingsDialog.qml`.
- **WASM + React**: `wisdom::GameSettings` gains `debugLogging`, sent to
  the worker as a fifth `int` over the existing `workerReceiveSettings`
  RPC (`"iiiii"`). The worker's `GameState` owns the logger. The IDL
  exposes the attribute, and the React types, `toWebSettings`,
  `handleApplySettings`, `SettingsModal`, and test fixtures carry it.

Toggling the QML setting goes through the same path as changing depth
or time, so it restarts an in-flight search. That is acceptable.

## Plan

1. Feature doc (this file).
2. Engine: `LogRingBuffer`, `BufferedLogger`, `formatLogTimestamp`,
   `makeBufferedLogger`, plus `engine/test/logger_test.cpp`.
3. Console `debug` command.
4. QML setting, config, and engine-owned logger.
5. WASM RPC/IDL plumbing and React UI.
6. Record progress below.

## Implementation Progress

### Session #1

All six steps landed on `debug-logging-toggle`, one commit each.

- **Engine** (`logger.hpp/.cpp`): `LogRingBuffer` is a single
  preallocated byte buffer with wraparound. Each line is stored as a
  one-byte level and a four-byte length followed by its text
  (`Record_Overhead` bytes of header per line), and the oldest whole
  lines are evicted first. A line that cannot fit on its own is
  truncated. `push` takes a `string_view` because the text is copied
  into the ring. `BufferedLogger` wraps a sink, prepends a
  `[HH:MM:SS.mmm]` local-time stamp, buffers while disabled, and
  replays then clears on the off-to-on transition. It is not
  thread-safe; each frontend uses it from a single thread (the console
  main thread, the QML engine thread, the WASM worker).
  `formatLogTimestamp` uses `localtime_r` / `localtime_s` and
  `strftime`, so it builds under Emscripten and MSVC.
- **Tests** (`engine/test/logger_test.cpp`, fast suite): ring buffer
  ordering, byte accounting, eviction, truncation, a record straddling
  the end of storage, many-wrap stress with mixed lengths, drain,
  clear; `BufferedLogger` buffer/replay/re-disable semantics, capacity,
  and timestamp shape.
- **Console**: `debug on`, `debug off`, and a bare `debug` that
  prompts. Verified by piping commands: the engine's first search is
  silent, and `debug on` prints the retained timestamped trace.
- **QML**: `GameSettings.debugLogging`, `ChessGame::Config::debugLogging`,
  `ChessGame::config()`, `ChessEngine` owns the logger and re-syncs it
  in `updateConfig` and `reloadGame`. The `NDEBUG` log level is gone.
  Built and smoke-run headless; the dialog was not exercised
  interactively.
- **WASM + React**: fifth `int` on `workerReceiveSettings` (`"iiiii"`),
  `GameState::logger`, IDL attribute. Built with Emscripten 4.0.7 and
  confirmed `get_debugLogging` / `set_debugLogging` in the generated
  glue. React `tsc && vite build` passes and all 29 vitest tests pass.
  Not exercised in a browser.

Behavior change to be aware of: the console no longer prints the
search trace until `debug on` is entered.
