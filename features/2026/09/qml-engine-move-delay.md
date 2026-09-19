# QML engine move delay

## Motivation

The bug list
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md))
has this open item under "Frontends":

> `QThread::usleep (200000)` in the engine slot to wait for animation
> (`ui/qml/main/chess_engine.cpp:126`).

The line is now at `ui/qml/main/chess_engine.cpp:131`, in the slot that
starts a search once it is the engine's turn:

```cpp
    // Wait for animation to finish
    QThread::usleep (200000); // 200 ms

    auto who = game_state->getCurrentTurn();
    ...
    auto optionalMove = game_state->findBestMove (my_logger);
```

It blocks the engine thread for 200 ms before every search, so that the
player's piece has finished moving on the board before the engine's reply
starts to move. The thread cannot handle anything else while it sleeps,
and the 200 ms is a copy of `animationDelay` in `DesktopRoot.qml` and
`MobileRoot.qml`, not tied to it. The `QML: dialogs` test
`theEngineAnswersAMove` spends most of its 0.4 seconds in this sleep.

Three more things are wrong with it:

- The wait is serial with the search. A search that takes two seconds
  still pays the 200 ms first, although the animation would have finished
  long before the reply. The wait only matters when the search ends sooner
  than the animation: at a low depth, with a short time limit, or in a
  forced position.
- It runs when nothing is animating: the engine's first move as White, a
  search resumed after a draw dialog, and a search restarted by a config
  change.
- It never covered castling. The rook waits 225 ms and then moves for
  200 ms (`Piece.qml`), so a fast reply starts while the rook is still
  moving.

The plan for removing it comes after the preliminary action item below.

## Preliminary action item: a script to install the Qt that CI uses

The `qml-tests` branch found failures that only happened on CI, because
CI builds with a different Qt from the one used locally. The fastest way
to reproduce them was to install CI's Qt locally (see Session #9 of
[qml-tests.md](qml-tests.md)). That was done by hand in a scratch
directory. This change starts by turning it into a script, so the next
CI-only failure, including any in the UI tests this change will touch,
can be reproduced in minutes.

### What CI installs

- `.github/workflows/cmake.yml` uses `jurplel/install-qt-action@v4` with
  `version: '6.9.*'`, `cache: true` and no `modules:` list, so it gets the
  newest 6.9 release with the default modules.
- The newest 6.9 is 6.9.3 (checked on 2026-09-19 with
  `aqt list-qt linux desktop --spec 6.9 --latest-version`). CI's test logs
  confirm it: "Using QtTest library 6.9.3".
- `.github/workflows/installers.yml` also asks for `6.9.*`, and
  `.github/workflows/web.yml` pins `6.9.3` exactly. The script follows
  `cmake.yml`, whose jobs run the tests.
- The local default is `~/Qt/6.11.2/gcc_64`.

### What worked by hand

All without root, on Linux:

```bash
python3 -m venv <dir>/aqt-venv
<dir>/aqt-venv/bin/pip install aqtinstall          # got aqtinstall 3.3.0
<dir>/aqt-venv/bin/aqt list-qt linux desktop --arch 6.9.3
                                                   # -> linux_gcc_64
<dir>/aqt-venv/bin/aqt install-qt linux desktop 6.9.3 linux_gcc_64 \
    --outputdir <dir>/Qt
```

- The download and unpacking took about 20 seconds. The result is 1.7 GB
  at `<dir>/Qt/6.9.3/gcc_64`.
- The default module set includes everything the build and tests need:
  Qt6Quick, Qt6QuickControls2, Qt6Svg, Qt6Test and Qt6QuickTest.
- Building against it:

  ```bash
  cmake -S . -B build-qt69 -DCMAKE_BUILD_TYPE=Debug \
      -DWISDOM_CHESS_QT_DIR=<dir>/Qt/6.9.3/gcc_64 \
      -DWISDOM_CHESS_QML_UI=ON -DWISDOM_CHESS_SLOW_TESTS=OFF
  cmake --build build-qt69 -j8
  ctest --test-dir build-qt69 -R "^QML:" -j 6
  ```

  `ctest` supplies the UI tests' environment (`QT_QPA_PLATFORM=offscreen`,
  `QT_QUICK_BACKEND=software`). A test executable run by hand needs both
  set.
- Debug matched CI best: the Linux Debug job was the one that failed
  every time.

### What the script should do

A new `scripts/install-ci-qt.sh`:

1. Read the version from `.github/workflows/cmake.yml` (`6.9.*`) and let
   `aqt` pick the newest matching release, as CI does, so the script
   follows CI when the workflow changes. Accept an explicit version, such
   as `6.9.3`, as an override.
2. Install into a directory outside the source tree and outside `/tmp`.
   The default is `${XDG_CACHE_HOME:-$HOME/.cache}/wisdom-chess/qt`; an
   argument or environment variable can change it. `/tmp` is a 7.7 GB
   in-memory filesystem here and was 88% full after the manual install.
3. Keep `aqtinstall` in a virtual environment next to the Qt install. Do
   not install it globally, and do not use `sudo`. Run `aqt` from that
   directory: it writes `aqtinstall.log` to the current directory, which
   would otherwise land in the source tree.
4. Do nothing if that version is already installed. Print the
   `-DWISDOM_CHESS_QT_DIR=...` path to use either way.
5. Support Linux (`linux_gcc_64`). For macOS, `aqt` also offers
   `mac desktop ... clang_64`, but that is untested; the script should
   stop with a clear message on anything else.
6. Check for `python3` with the `venv` module and say what is missing if
   it is not there, rather than failing halfway.

Then point `AGENTS.md` at the script where it now spells out the `aqt`
commands, and delete the manual install left in the scratch directory.

### Verification of the script

- A run from nothing installs the newest 6.9 and prints the path.
- A second run is a no-op.
- A Debug build against that path passes all fast tests, including the
  six `QML:` tests.
- `shellcheck`, if available, reports nothing.

## Plan: hold the engine's move in the GUI

The engine searches as soon as it is its turn. `GameModel`, on the GUI
thread, holds an engine move that arrives before the previous move has
finished animating, and shows it when the animation is done. The search
and the animation overlap, so a search longer than the animation pays
nothing, and the engine thread never blocks.

### Alternatives considered

- **`QTimer::singleShot` in the engine thread.** The smallest change, and
  the thread's event loop stays alive. It keeps the serial 200 ms and the
  copied constant, and adds a state the code does not have today: a config
  change or a reloaded game during the wait could queue a second search, so
  it would need a guard.
- **A handshake with QML when the animation finishes.** No timing constant
  at all. But the animations are `Behavior`s on each of up to 32 delegates
  with no single finished signal, a captured piece's delegate is destroyed
  while it animates, and the UI tests run offscreen with the software
  backend, where animation timing is the least reliable.
- **Deleting the sleep.** At depth 1 the reply arrives within milliseconds,
  so two pieces move at once, and when the engine recaptures, the piece
  that is still sliding disappears.

### Design

`chess_engine.cpp`: delete the `usleep` and its comment. `<QThread>` stays,
because `quit()` uses it.

`game_model.hpp` and `game_model.cpp`:

- Two properties, so that the durations have one source that both C++ and
  QML read:
  - `animationDelay`, 200 ms, with `setAnimationDelay()` and
    `animationDelayChanged`. It is settable so that a test can choose a
    long hold.
  - `castlingRookPause`, 225 ms, constant.
- New members: a `QElapsedTimer` for when the last move was shown, the hold
  that move needs in milliseconds, a single-shot `QTimer` with
  `Qt::PreciseTimer`, and an `optional` held move (`move`, `who`,
  `game_id`).
- `handleMove()` runs for human and engine moves alike and is where
  `PiecesModel::playerMoved` is triggered from. It restarts the elapsed
  timer and sets the hold to `animationDelay`, plus `castlingRookPause` when
  the move is castling.
- `engineThreadMoved()` keeps its game id check first. If no move has been
  shown yet, or the hold has elapsed, it shows the move at once. Otherwise
  it stores the move and starts the timer for the remainder. The code that
  applies and announces the move goes into a private `showEngineMove()`.
- When the timer fires, it takes the held move, checks its game id against
  `gameId()` again, and calls `showEngineMove()`.
- `restart()` stops the timer, clears the held move and invalidates the
  elapsed timer. A new board has nothing animating, so an engine playing
  White moves at once.

QML: `DesktopRoot.qml` and `MobileRoot.qml` bind `animationDelay` to
`_myGameModel.animationDelay` and gain a `castlingRookPause` bound the same
way. The `PauseAnimation` in `Piece.qml` uses `root.castlingRookPause`
instead of `225`. Check `desktop_main.qml`, `mobile_main.qml` and
`wasm_main.qml` for other literal copies.

Why it stays correct:

- Engine against engine stays paced. The engine thread starts its next
  search from `GameModel::engineMoved`, through
  `ChessEngine::receiveEngineMoved`, and that signal is now emitted when
  the move is shown.
- For the same reason only one engine move can be outstanding, so one held
  slot is enough.
- The human cannot move during a hold. The GUI's `Game` still has the
  engine to move, and `ChessGame::isLegalMove` rejects a move then.
- A pause (an open menu or dialog) does not interact. A held move is shown
  the way a queued `engineMoved` signal is shown today.
- A held move that belongs to an earlier game is dropped twice over:
  `restart()` clears it, and the timer's slot checks the game id.

### Tests

In `src/wisdom-chess/ui/qml/test/dialogs_test.cpp`, next to
`theEngineAnswersAMove`, with the helpers in `application_fixture.hpp`:

- A depth-1 reply is not shown before the hold ends. Spy on `humanMoved`
  and `engineMoved` and expect at least `animationDelay` between them.
- New Game drops a held move. Set `animationDelay` to five seconds, move,
  wait long enough for a depth-1 search, call `restart()`, and expect no
  `engineMoved`, 32 pieces, and `piecesMatchTheBoard()`.
- After castling, the hold is `animationDelay` plus `castlingRookPause`.
- The engine's first move as White is shown without a hold.
- Record which of these fail with the change reverted. The New Game test
  has no counterpart in the old code.

### Verification

- Release and Debug builds with the QML UI and no warnings; the linter
  clean on the C++ files touched.
- `ctest -R "^QML:"` passes. Record the time of `theEngineAnswersAMove`
  before and after.
- The same in a Debug build against CI's Qt, installed by
  `scripts/install-ci-qt.sh`.
- By hand in the desktop app: a game at depth 1, a recapture of the piece
  that just moved, castling followed by a fast reply, engine against
  engine, and New Game while a reply is held.
- Afterwards, tick the `usleep` item in
  [bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md) and
  point it here. If the tests use `setAnimationDelay()`, mention it in the
  QML testing notes in `AGENTS.md`.

## Implementation Progress
