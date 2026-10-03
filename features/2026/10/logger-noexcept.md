# A `noexcept` `Logger`

## Motivation

`Logger::debug()`, `info()` and `emergency()` were the engine's only
virtuals a `noexcept` function might want to call, and none was
`noexcept`. A `noexcept` function that logged had to accept that the
call could throw, and the QML frontend, where every function Qt calls is
`noexcept`, could log only through `guarded()`.

The review that produced `noexcept-fixes.md` suggested marking them, and
this branch does. It makes a design decision explicit: a logger that
cannot get its message out terminates the process, rather than turning
its caller into a throwing function. For a logger the only real failure
is running out of memory while building a line, and an engine that
cannot allocate a log line is not going to recover anyway.

## Design

The three pure virtuals are `noexcept`, and so is every override.

`debug()` and `info()` need no `try`/`catch`. An exception escaping a
`noexcept` function reaches `emergencyTerminateHandler()`, which reports
it through `logEmergency()` and aborts; the
`Fatal: expects-through-noexcept` case pins that path. Catching and
terminating by hand would only duplicate it.

`emergency()` is different. The process is already ending, and the
message should reach as many sinks as it can, so a sink that fails is
swallowed, not fatal. `BufferedLogger::emergency()` already did this for
its timestamp; `StandardLogger::emergency()` and `UciLogger::emergency()`
now do it for their stream writes. The UCI one matters most: it is the
emergency logger of a running engine, and `std::cout` throws
`std::ios_base::failure` once stream exceptions are on and a write has
failed, which without the `try` would have ended the process before
`logEmergency()` could contain it.

`BufferedLogger` was the one implementation with throwing expressions on
the `debug()` and `info()` path: six `narrow` calls in
`formatLogTimestamp()` and `LogRingBuffer::push()`, all on values that
cannot fail. They are `noexcept_narrow` now, and `push()` is `noexcept`.
`formatLogTimestamp()` still returns a `string`, so it is not.

## Overrides

- Engine: `NullLogger`, `StandardLogger`, `BufferedLogger`.
- Frontends: `UciLogger` in `uci_interface.cpp`, `ChessEngineLogger` in
  the QML frontend, `WebLogger` in the WASM frontend. Qt and `EM_JS` do
  not throw; the UCI logger's `debug()` and `info()` build a string and
  take a lock, which terminate on failure like any other logger, and its
  `emergency()` swallows, as above.
- Tests: `RecordingLogger` and `ReentrantLogger` in `logger_test.cpp`,
  `DepthTrackingLogger` in `search_test.cpp`, `MarkedLogger` in
  `fatal_test_main.cpp`.

## Tests

Three tests used a logger as the way to throw from inside the engine,
which the contract now forbids.

- `logger_test.cpp`, "a logger that throws does not let the exception
  escape": removed. The scenario is impossible by contract. The
  `std::cerr`-throws fixture and the other emergency subcases stay.
- `search_test.cpp`, "An error during the search is rethrown with the
  board", and `Fatal: search-error`: both threw from `info()` to see the
  search wrap the error in a `SearchError` with the board. They now throw
  from the timer's periodic function, the one caller-supplied hook the
  search still runs. It is called every `Calls_Between_Clock_Checks`
  nodes, 1024, which depths 1 to 3 of the opening position do not
  reach, so both searches run to depth 4.

- `uci_logger_test.cpp`, a new doctest suite `wisdom-chess-uci-tests`
  that links `uci_interface.cpp` without `main.cpp`. It checks that the
  UCI emergency logger writes each line as an `info string`, and that a
  `std::cout` made to throw neither escapes the `noexcept` override nor
  stops `logEmergency()` reaching `std::cerr`, nor leaves the emergency
  path locked out afterwards.

## Out of scope

- The `GameStatusUpdate` virtuals. Their overrides are the frontends',
  and nothing `noexcept` needs to call them.
- The engine functions that cannot throw and lack `noexcept`, which the
  review counted at about 190. A later branch.

## Implementation Progress

### Session #1

Branched from `noexcept-fixes`, which it needs for `noexcept_narrow()`.
Implemented as above. `AGENTS.md` records the rule.

The two search-error tests first ran to depth 3, which the opening
position finishes in fewer than 1024 nodes, so the periodic function
never ran and the search did not throw. Depth 4 reaches it.

Verification: a Release build with the QML frontend (Qt 6.11) and the
slow tests, and a Debug build, compile without a warning; the linter
passes; all 288 Release tests pass, the `QML: ...` ones among them, and
all 244 Debug tests; the WASM engine and its fast and fatal test suites
build under Emscripten, and the logger and fatal tests pass there under
node. The React frontend was not rebuilt; only `web_logger` changed on
that side.

### Session #2

Review of the pull request: `UciLogger::emergency()` wrote through
`sendEmergencyLines()` to `std::cout` with no handling, so a stream
failure would have terminated the process inside the `noexcept`
override. It now swallows the failure like `StandardLogger`, and the
new `wisdom-chess-uci-tests` suite pins it with a throwing `std::cout`.
