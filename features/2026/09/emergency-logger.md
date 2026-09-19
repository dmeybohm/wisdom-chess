# Emergency logger for fatal engine errors

Branch: `emergency-logger`, created from `bug-list-and-engine-warnings`. It
depends on `noexcept_expects` and `engine/global.cpp`, which exist only there.

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
| `UciLogger` | Prints `info string <message>` to stdout and flushes, regardless of debug mode. UCI GUIs show `info string` lines and usually drop stderr. |
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
  before the logger runs and survives if the logger fails.
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
