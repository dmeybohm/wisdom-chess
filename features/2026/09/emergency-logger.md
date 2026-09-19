# Emergency logger for fatal engine errors

Branch: `emergency-logger`. It builds on `noexcept_expects` and
`engine/global.cpp` from `bug-list-and-engine-warnings`, and was started from
that branch before it merged. It has since been rebased onto `main`.

## Motivation

`noexcept_expects` reported a failed precondition with
`fprintf (stderr, ...)` and then called `std::terminate()`. On the desktop
console that is visible. On the QML app launched from a GUI, and on the
WebAssembly build where the engine runs in a Wasm Worker, stderr goes nowhere
useful, so the one message explaining why the app died was lost. The search's
catch-all had the same problem with `std::cerr`.

## Design

### `Logger::emergency()`

`Logger` gains a third pure virtual, next to `debug()` and `info()`:

```cpp
// A fatal message, sent just before the process terminates. Must not buffer.
virtual void emergency (const string& output) const = 0;
```

It is pure virtual so that every logger states what it does:

| Logger | `emergency()` behaviour |
|---|---|
| `NullLogger` | Nothing. |
| `StandardLogger` | Writes the line to `std::cerr`. |
| `BufferedLogger` | Forwards straight to the sink's `emergency()` with the usual timestamp prefix, bypassing the ring buffer and ignoring the enabled flag. It never touches the buffer, so it is safe from any thread. |
| `UciLogger` | Prints each line of the message as its own `info string` line on stdout, regardless of debug mode, under the same output lock as `sendLine()` (see Session #5). UCI GUIs show `info string` lines and usually drop stderr. |
| `ChessEngineLogger` (QML) | `qCritical()`, which goes through Qt's message handler. `qDebug`, used by `info`/`debug`, is commonly filtered out in release configurations. |
| `WebLogger` (wasm) | `console.error` through a new `EM_JS` `consoleError`. |

`Logger::LogLevel` and `LogEntry` are unchanged, since emergencies are never
buffered.

### Registration and `logEmergency()`

In `engine/logger.hpp`:

```cpp
void setEmergencyLogger (shared_ptr<Logger> logger);
void logEmergency (const string& message) noexcept;
void installEmergencyTerminateHandler();
```

- The engine holds the registered logger as a `shared_ptr`, so frontends have
  no lifetime rule to remember. It is guarded by a mutex; `std::atomic` of a
  `shared_ptr` was avoided because Emscripten's libc++ support is recent.
- `logEmergency()` writes to `std::cerr` first, then calls the registered
  logger's `emergency()`. `std::cerr` is unit-buffered, so the message is out
  before the logger runs and survives if the logger fails. The two sinks are
  tried independently (see Session #3).
- It never throws. It also does not recurse: if the logger itself reports an
  emergency, the nested call only writes `std::cerr`.
- `installEmergencyTerminateHandler()` sets a `std::terminate` handler that
  describes the current exception (`wisdom::Error` with its extra info,
  `std::exception::what()`, or "unknown"), reports it through
  `logEmergency()`, and aborts. This also covers `expects()` thrown through
  a `noexcept` function.
- The API lives in `logger.hpp` and not `global.hpp`, because `global.hpp` is
  the precompiled header and includes no project headers. Only `global.cpp`
  includes `logger.hpp`. `global.hpp` did not change, so the hot-path inline
  functions that call `noexcept_expects` are untouched.

### Fatal sites

- `terminateOnPreconditionFailure` (`engine/global.cpp`) reports through
  `logEmergency()` and calls `std::abort()`. It no longer calls
  `std::terminate()`, because the terminate handler would add a second, less
  specific message. The exit status is unchanged.
- The catch-all in `IterativeSearchImpl::iterativelyDeepen`
  (`engine/search.cpp`) reports the error, its extra info and the board in a
  single `logEmergency()` call, then aborts. The board used to be dumped to
  `std::cerr` separately, which would not have reached the logger. This
  closes the `search.cpp` entry of the "Direct `std::cout`/`std::cerr` in
  library code" item in `bug-list-and-engine-warnings.md`; the `board.cpp`
  and `game.cpp` sites in that item are not fatal paths and stay open. The
  suggestion there to let the exception propagate is a separate decision.

### Frontend registration

Each frontend registers as the first statements of `main()`, before any other
object is constructed and before any engine thread exists.

| Frontend | Logger registered |
|---|---|
| Console | None. `std::cerr` is already visible, and a `StandardLogger` would print the line twice. Only the terminate handler is installed; the existing catch block reports through `logEmergency()`. |
| UCI | `makeUciLogger (false)`, a new factory in `uci_interface.hpp`, since `UciLogger` sits in an anonymous namespace. |
| QML | `ChessEngine::ChessEngineLogger`, before `QGuiApplication` is constructed. `qCritical()` does not need the application object. |
| WASM/React | The existing static `WebLogger` from `wisdom::worker::makeLogger()`. `main()` runs on the browser's main thread and the engine in a Wasm Worker; they share memory, so the worker sees the registered logger. Output goes to the browser console only. |

A failure during static initialization, before `main()` runs, cannot be
covered by any placement.

## Implementation Progress

### Session #1

- Added `Logger::emergency()` and implemented it in all seven loggers,
  including the `RecordingLogger` test double. New `BufferedLogger` subcase:
  while disabled, `info()` is buffered but `emergency()` reaches the sink
  immediately with a timestamp and leaves the buffer untouched.
- Added `setEmergencyLogger`, `logEmergency` and
  `installEmergencyTerminateHandler`. The "Emergency logger" test case
  captures `std::cerr` and covers: delivery as an emergency and not as an
  info line, the unregistered case, a logger that throws (and that it is
  not left locked out afterwards), a logger that re-enters `logEmergency`,
  and replacing the logger.
- Switched `engine/global.cpp` and `engine/search.cpp` over.
- Registered at the top of `main()` in the console, UCI, QML and wasm
  frontends, and added `makeUciLogger()`.
- Documented the contract helpers and the fatal-error rules in `AGENTS.md`.
- Verified:
  - Desktop (GCC), QML (Qt 6.11.2) and wasm (Emscripten Clang) builds with
    no warnings; all 123 tests pass; linter clean; a GCC syntax pass without
    `NDEBUG` over the changed files is clean.
  - Standalone probes linked against the engine library: a failed
    `noexcept_expects`, an uncaught `wisdom::Error`, an uncaught
    `std::exception` and a bare `std::terminate()` each print exactly once
    to `std::cerr` and once through the registered logger, then exit with
    status 134.
  - A scratch UCI binary with a forced precondition failure prints
    `info string Precondition failed at ...` on stdout and the plain message
    on stderr. The real UCI binary still answers `uci`, `go movetime` and
    `stop` as before.
- Not verified, needs a manual run: that the message appears in a browser's
  devtools console for the wasm build, and in the Qt message output or
  logcat for the QML build.

### Session #2

Tests for the terminating paths, which cannot run inside doctest.

- Considered and rejected: a switch that makes `noexcept_expects` log without
  terminating, so doctest could assert on the log. Execution would continue
  into the code the check guards. For `MoveList::append` at capacity that is
  a write past the end of the array, so the test itself would be undefined
  behaviour. Making it sound would need early returns in the hot path that
  exist only for tests, the failure helper could no longer be
  `[[noreturn]]`, and the tests would prove "log and continue", which
  production never does.
- Instead each fatal case runs in its own process under CTest.
  `engine/test/fatal_test_main.cpp` builds `wisdom-chess-fatal-tests`, which
  registers a logger that prints `[emergency] <message>`, installs the
  terminate handler, and runs the case named on its command line:
  `append-overflow`, `remove-from-empty`, `bad-castling-flags`,
  `bad-en-passant-row`, `uncaught-error` and `expects-through-noexcept`.
  The last one throws `expects()` through a `noexcept` lambda and checks that
  the terminate handler reports it.
- CTest fails any test that dies from a signal, whatever it printed, so the
  pass expression alone was not enough: the first attempt reported all six
  as "Subprocess aborted", and each wrote a core dump, about a second apiece.
  The executable now handles `SIGABRT`, prints `[aborted]` and exits
  normally. Calling stdio there is allowed because the signal comes from
  `abort()`, not asynchronously.
- Each test's `PASS_REGULAR_EXPRESSION` requires the `[emergency]` line with
  the expected message followed by `[aborted]`, and its
  `FAIL_REGULAR_EXPRESSION` is `[survived]`, which the executable prints if a
  case returns. So a case passes only if the failure was reported through
  the emergency logger and the process really did abort. The tests carry
  the `fast` label and are added by a small `wisdom_chess_add_fatal_test()`
  helper in `engine/test/CMakeLists.txt`.
- On MSVC the executable calls `_set_abort_behavior` so an abort raises no
  dialog or error report. Not verified locally; the Windows CI job will show.
- Verified that the tests can fail: with the `noexcept_expects` removed from
  `MoveList::removeLast`, "Fatal: remove-from-empty" failed on the
  `[survived]` marker; the check was then restored. All six pass in both
  Release and Debug builds, in about 0.01 seconds in total. Full suite: 129
  tests passing, no build warnings, linter clean.

### Session #3

Two review comments.

- **A failed `std::cerr` write skipped the registered logger.** `logEmergency`
  wrapped both sinks in one `try` and wrote `message + "\n"`, so a failed
  allocation for that temporary, or a stream set to throw, jumped to the
  `catch` before `emergency()` was called. Fixed by streaming
  `message << '\n'`, which needs no temporary, and by giving each sink its own
  `try`. C++ streams are kept: the allocation came from the concatenation,
  not from the stream. The same weakness sat one level up, where the fatal
  sites build their message before calling `logEmergency`; on failure
  `terminateOnPreconditionFailure` and the terminate handler now fall back to
  a plain stream write with no allocation, and `BufferedLogger::emergency()`
  falls back to the bare message if it cannot add the timestamp.
  New test: with `std::cerr` replaced by a buffer that rejects every write and
  exceptions enabled on the stream, the registered logger still receives the
  message. Confirmed that this test fails against the old single-`try` body.
- **The fatal tests failed under FIL-C.** They relied on catching `SIGABRT`
  to turn the abort into a normal exit, because CTest fails any test that
  dies from a signal. FIL-C turns `abort()` into a trap that cannot be
  caught. The tests were adapted, not skipped, since a memory-safety
  build is where these checks are most interesting. Each test now runs
  `engine/test/run_fatal_test.cmake` through `cmake -P`. The script launches
  the case, merges its output, and fails if `[survived]` appears, if the
  result is `0`, or if the `[emergency]` line with the expected message is
  missing. The verdict no longer depends on how the process dies, so
  `PASS_REGULAR_EXPRESSION`, `FAIL_REGULAR_EXPRESSION` and the `[aborted]`
  marker are gone. The `SIGABRT` handler remains only to avoid a core dump
  per case (about a second each here) and is compiled out when
  `WISDOM_CHESS_FILC_COMPAT` is set.
- Verified: all six pass normally in about 0.05 seconds. With
  `WISDOM_CHESS_FILC_COMPAT=ON` in a scratch tree, which removes the handler
  so each process dies from an uncaught signal and the script sees
  "Subprocess aborted", all six still pass. Called directly, the script
  rejects a wrong expected message, a case that reports nothing, and a
  process that exits `0`; the earlier mutation check covers `[survived]`.
  FIL-C itself is not installed here, so the FIL-C CI job is the real
  confirmation. Full suite: 129 tests passing, no build warnings.

### Session #4

- Rebased onto `main` after `bug-list-and-engine-warnings` merged as pull
  request 235 with three further commits. One conflict, in
  `ui/uci/uci_interface.cpp`: `main` had changed the search lambda to capture
  a copy of the debug flag, so the search thread no longer reads the member,
  while this branch had switched the same line to `makeUciLogger()`. Kept
  both: `makeUciLogger (debug_mode)`.
- The bug list document is now on this branch, so its entries for the
  `search.cpp` fatal path were updated there.
- Verified after the rebase: desktop, QML and wasm builds with no warnings,
  all 129 tests passing, and the UCI binary answering `isready` during a
  search and `bestmove` after `stop`.

### Session #5

Review comment: the UCI emergency output bypassed `sendLine()` and its
`output_mutex`, both of which arrived from `main` with the rebase. A fatal
message from the search thread could therefore interleave with `readyok` or
another reply from the command thread.

- `UciLogger::emergency()` now calls a new `sendEmergencyLines()` in
  `ui/uci/uci_interface.cpp`, which writes under `output_mutex`.
- It does not simply call `sendLine()`, for two reasons. `sendLine()` takes a
  ready-made string, which would mean allocating in a fatal path. And a
  process about to abort must not wait indefinitely for a lock, so
  `output_mutex` became a `std::timed_mutex` and the emergency writer waits
  at most 250 ms for it, then writes regardless. An unserialized message is
  better than a process that never aborts.
- Found while fixing it: only the first line of a multi-line message got the
  `info string` prefix. The search's fatal message carries the board and an
  uncaught `Error` carries its extra info, so the remaining lines reached
  the GUI as garbage. Every line is now prefixed, and one trailing newline
  no longer produces an empty `info string` line.
- Verified with a stress probe: three threads writing through `info()`
  while the main thread sends 3,000 two-line emergencies. Against the old
  file, 6,704 of 69,000 lines were malformed; none of the second lines was
  prefixed and 356 first lines were cut by other output. Against the new
  file, 0 of 66,000 were malformed and all 6,000 emergency lines were
  intact. Scratch binaries confirm the single-line and multi-line formats,
  and the real UCI binary still answers `isready` during a search and
  `bestmove` after `stop`. All 129 tests pass with no build warnings.
- Not tested: the 250 ms timeout path itself, since the mutex cannot be
  reached from outside the file.
- A limit this does not remove: if the GUI has stopped reading and the
  stdout pipe is full, the emergency write blocks like any other write and
  the abort is delayed. The message has already gone to `std::cerr` by
  then, because `logEmergency()` writes there first.
