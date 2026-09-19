# QML engine thread shutdown

## Motivation

`GameModel::~GameModel` deleted the engine `QThread` without stopping it.
Deleting a running `QThread` makes Qt abort with `QThread: Destroyed while
thread is still running`. This is the open item in
[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md).

The destructor was only safe because the QML window's `onClosing` handler
calls `GameModel::applicationExiting()`, which cancels the search, asks the
thread to quit and waits for it. Any exit that skips `onClosing` reached the
destructor with the thread still running. One such exit exists today: when
the root QML file fails to load, `main.cpp` calls
`QCoreApplication::exit (-1)`. No window was ever created, so `onClosing`
never fires, but `game_model.start()` has already started the thread.

The wait is also needed for memory safety, not only to avoid the abort. The
notifier built by `buildNotifier()` holds pointers to `my_game_id`,
`my_config_id` and `my_paused`, and the engine thread reads them during a
search, so the thread has to finish before those members are destroyed.

## Plan

1. Move the shutdown sequence out of `applicationExiting()` into a private
   `stopEngineThread()`, unchanged: bump the game id, emit
   `terminationStarted`, and `wait()` except under Emscripten. It returns
   early when the thread is not running, so calling it a second time is
   harmless.
2. Call it from `applicationExiting()` and from the destructor before the
   `delete`.

Not changed:

- `setupNewEngineThread()` also begins with `delete my_chess_engine_thread`.
  Its only caller is `init()`, which runs once from the constructor while
  the pointer is null. `GameModel::restart()` reuses the existing thread and
  engine, so the thread is never replaced.
- Emscripten still skips `wait()`, because blocking the browser's main
  thread hangs. There `main()` does not return in practice, a single
  `GameModel` lives on its stack, and it is not creatable from QML, so the
  destructor does not run.
- `QThread::quit()` is thread-safe, so the destructor could call it directly
  and the `terminationStarted` signal and `ChessEngine::quit` slot could be
  removed. Left as a possible follow-up to keep this fix to the existing,
  exercised sequence.

## Implementation Progress

### Session #1

- Made the change above in `src/wisdom-chess/ui/qml/main/game_model.cpp` and
  `game_model.hpp`.
- Reproduced the failure first. A `qt.conf` beside the binary with
  `QmlImports=/nonexistent` makes the root QML file fail to load without a
  rebuild. Run with `QT_QPA_PLATFORM=offscreen`, the binary built from
  `main` printed `QThread: Destroyed while thread '' is still running` and
  dumped core (exit 134). With the fix the same run exits with 255, the
  `-1` that `main.cpp` asks for.
- The QML target builds with no warnings and the linter is clean on both
  files.
- Not exercised: closing the window normally, which needs a GUI session. It
  runs the same sequence as before through `applicationExiting()`, and the
  destructor's second call then returns early. The Emscripten build was not
  rebuilt; the code it compiles is the same sequence it had before.
