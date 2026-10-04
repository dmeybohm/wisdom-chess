# Remove ChessEngine::guarded()

`features/2026/09/qt-exception-safety.md` ran every `ChessEngine` slot
through `guarded()`, which caught an exception, reported it, emitted
`engineFailed`, and stopped the engine until the next game, while
`GameModel` showed "Engine error" and refused moves. Since
`exception-removal.md`, the engine and the frontends throw nothing of
their own, so `guarded()` caught only the standard library's exceptions,
in practice `std::bad_alloc`.

## Design

- **`guarded()` goes, with everything that served it:** `fail()`,
  `my_has_failed`, the `engineFailed` signal, the `runGuarded()` test
  seam, and `GameModel`'s `engineThreadFailed()`, `my_engine_failed` and
  its "Engine error" status.
- **The slots stay `noexcept`.** An exception that escapes one reaches
  `std::terminate()`, and the emergency terminate handler logs
  "Uncaught exception: " and its `what()`, then aborts. If describing
  the exception fails, it writes "Terminating after an uncaught
  exception" to `std::cerr` instead. The exception's message still
  reaches the emergency logger, as it did when `fail()` logged
  "Engine error: " and the message. What changes is the wording, and
  that the app ends instead of offering a new game.
- Why the recovery was not worth keeping:
  - It did nothing in the QML WebAssembly build, which has no exception
    catching, so a throw aborted there anyway.
  - It covered only the engine thread. A `std::bad_alloc` in a
    `GameModel` slot already ended the process.
  - Carrying on after running out of memory, with part of the failed
    slot's work done, is doubtful, which is why the engine refused
    everything until a new game.
  - Elsewhere a failure nothing can handle is reported and ends the
    process, since `contracts-abort.md`.
- Tests: the two `chess_engine_test.cpp` tests of a throwing slot, and
  the three in `application_test.cpp` and one in `dialogs_test.cpp`
  that drove `engineThreadFailed()` directly, go with the code.

## Implementation Progress

### Session #1

- Removed the code, the tests and their helpers, and updated AGENTS.md.
- A Release build with the QML UI, against Qt 6.11.2, passes all 310
  tests, the fast ones and the QML ones, without warnings, and `lint`
  is clean. Not run locally: the QML WebAssembly build, MSVC and
  Android.
