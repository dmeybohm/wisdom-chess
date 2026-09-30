# Exception safety at the Qt boundary

Branch: `qt-exception-safety`. Session #1 is the investigation and
Session #2 implements its recommendation.

## Motivation

The engine reports errors by throwing: `EXPECTS` and `ENSURES`,
`narrow`, `Error` and its subclasses, and `std::bad_alloc`. Qt is not
written to have exceptions pass through it. The QML frontend calls the
engine from slots, invokables, property accessors and a worker thread,
so an engine exception there has Qt frames between it and any handler.

The WebAssembly builds are left out. What Emscripten does with an
exception is decided by the compiler and its flags, not by where the
frontend catches.

## What Qt documents

From [Exception Safety](https://doc.qt.io/qt-6/exceptionsafety.html):

- "Qt itself will not throw exceptions. Instead, error codes are used."
- "Throwing an exception from a slot invoked by Qt's signal-slot
  connection mechanism is considered undefined behaviour, unless it is
  handled within the slot", because the exception unwinds "the stack of
  the Qt code (which is not guaranteed to be exception safe)".
- "the only supported use case for recovering from exceptions thrown
  within Qt (for example due to out of memory) is to exit the event loop
  and do some cleanup before exiting the application."

## What the frontend does today

`src/wisdom-chess/ui/qml/main` and `src/wisdom-chess/ui/viewmodel`
contain no `try`, no `catch` and no `noexcept`. Every function Qt calls
is an unguarded boundary:

- `GameModel`: the invokables (`start`, `restart`, `canMoveFrom`,
  `needsPawnPromotion`, ...), the slots (`movePiece`, `promotePiece`,
  `engineThreadMoved`, `receiveChessEngineDrawStatus`,
  `updateEngineConfig`, ...), the property accessors QML binds to, and
  the lambdas given to `connect`.
- `PiecesModel`: its slots and the `QAbstractListModel` overrides that
  the view calls.
- `ChessEngine`: all seven slots, which run on the engine thread. This
  is where a search runs.
- The singleton `create()` functions that the QML engine calls.

`main()` calls `app.exec()` with no handler around it, and starts by
installing the emergency terminate handler.

## What happens when an exception escapes

Measured with a probe program that throws from each kind of boundary,
built against Qt 6.9.3, which CI uses, and Qt 6.11.2. Both gave the
same results. "Handler" means `std::terminate` ran and the terminate
handler could still read the exception, which is what
`installEmergencyTerminateHandler()` needs to report it.

| Thrown from | As it is today | Entry point declared `noexcept` |
|---|---|---|
| Slot run by the event loop, main thread | Unwinds through Qt to `exec()`, Qt prints a warning and rethrows, the exception leaves `exec()` | Handler |
| Slot delivered by a queued connection | The same | Handler |
| Slot on a worker thread | Handler, after unwinding through Qt | Handler |
| Invokable called from QML while it loads | Handler, after unwinding through the QML engine | Handler |
| Invokable or slot called from a QML handler later | Unwinds through the QML engine and Qt to `exec()` | Handler |
| Property accessor read by a QML binding | Handler, after unwinding through the QML engine | Handler |
| `QAbstractListModel::data()` called by a view | Handler, after unwinding through the QML engine | Handler |

So today the application always ends, and because `main()` has no
handler, always through the terminate handler, which logs the message.
Nothing is swallowed and nothing carries on with broken state. But in
every row the exception first unwinds through Qt or the QML engine,
which Qt calls undefined behaviour. That it ends well is an
observation about two Qt versions on one platform, not a guarantee.

Two remedies were tried in the probe.

**Catching in `QCoreApplication::notify()`**, the usual global
approach, is not safe here. It does catch an exception from a plain C++
slot, on the main thread and on a worker thread, and the event loop
carries on. But when the exception came through the QML engine, the
process kept running and then aborted at shutdown with "Object
destroyed while one of its QML signal handlers is in progress": the
unwinding had left the QML engine in a broken state. It also does not
cover an invokable called while QML loads, a property accessor, or a
model's `data()`. Those still reached the terminate handler.

**Catching in the emitter of a direct connection** between two C++
objects worked: the second slot still ran on the next emission, and the
emitter was deleted cleanly. It is still unwinding through
`QMetaObject::activate()`, which the documentation does not allow.

## What can actually be thrown

Going through every throwing call reachable from those entry points:

- **Contract violations.** `EXPECTS` in `GameSettings::applyTo` (depth
  and thinking time in range), in `GameModel` (no second held move, a
  human color exists) and in the singletons; the `mapColor`,
  `mapPlayer` and `mapPiece` conversions for an enum value QML should
  never send; `Game::move` and the rest of the engine's preconditions.
  The values come from QML we wrote: the sliders are bounded by the
  same constants, and `Board.qml` rejects a drop outside the board.
- **Search failures**, on the engine thread: `SearchError`, which wraps
  any `Error` from the search, including the `ENSURES` checks on the
  result and on the evaluation.
- **`FenParserError`**, from `ChessGame::clone()`, which round-trips
  the board through FEN. The FEN is our own output.
- **`std::bad_alloc`**: the transposition table when an engine is
  created, the history as a game grows.

None of these is a condition a user can cause and recover from. The
QML frontend has no file loading and no FEN entry. Every exception
that reaches Qt is a bug or an exhausted heap.

## Options

**A. Declare every function Qt calls `noexcept`.** This keeps the
current policy, that a bug ends the program with a report, and makes it
defined: the exception stops at our boundary and never enters a Qt
frame. `moc` accepts `noexcept` on slots, invokables and accessors.
The probe's third column is this option. It needs no audit of whether
state is consistent after a failure, because nothing continues.

**B. Contain failures on the engine thread.** `ChessEngine`'s slots
catch `Error` and `std::exception`, log through `logEmergency()`, and
emit a new `engineFailed (QString)` signal. `GameModel` shows the
message as the game-over status, and the user can start a new game.
This is the one place where recovery is both valuable and safe:

- The GUI thread is healthy when the engine thread fails, and ending
  the whole application loses the position on the board.
- The search works on its own copies of the board and history, so a
  failed search leaves the engine's `Game` as it was.
- `reloadGame()`, which a new game calls, already replaces the game,
  clears the transposition table and resets both flags.

The catch would sit inside the slot, so nothing unwinds through Qt.

**C. Catch on the main thread and show a dialog.** Not recommended.
Each `GameModel` function updates several pieces of state in sequence
(the game, the displayed status, the turn, the held move, the signals
it emits), so carrying on after a failure part way needs an audit of
every one, for failures that are bugs anyway. Qt's own advice after an
exception is to leave the event loop.

**D. The `notify()` override.** Rejected, by the measurement above.

## Recommendation

A on every entry point, and B on the engine thread on top of it: the
slots are `noexcept`, and catch what they can report. Then no path lets
an exception into Qt, an engine failure becomes a message instead of a
vanished window, and everything else behaves as it does now, but by
definition rather than by luck.

Two things would keep it that way:

- A linter rule, or a review convention in `AGENTS.md`, that a `slots:`
  or `Q_INVOKABLE` declaration is `noexcept`.
- A test that a throwing engine produces `engineFailed` and leaves the
  GUI able to start a new game, and a fatal-style test that an
  exception in a main-thread slot reaches the emergency logger.

## Beside the point, but found on the way

`GameModel::canMoveFrom`, `needsPawnPromotion` and `movePiece` pass
QML's row and column to `makeCoord()`, which checks them only with
`ASSERT`. In a Release build an out-of-range value would index past
the board. `Board.qml` range-checks a drop before calling, and a press
comes from inside the board item, so nothing reaches it today. A check
on the C++ side would stop relying on that.

## Not covered

- The Qt for WebAssembly and React builds.
- Android. It uses the same Qt 6.9 sources, but the probe was not run
  there.
- Windows and macOS. The probe ran on Linux with GCC only. How an
  exception behaves when it unwinds through Qt frames is exactly the
  part that may differ by platform, which is one more argument for
  option A.

## Implementation Progress

### Session #1

- Investigation only. The probe is a single-file Qt program with a
  `QObject` whose slot, invokable and property accessor throw, a list
  model whose `data()` throws, and one command-line case per row of the
  table above; a second build declares those functions `noexcept`, and
  a flag swaps in a `QGuiApplication` whose `notify()` catches. It was
  run offscreen with the software backend. It is not in the repository.

### Session #2

Options A and B, as recommended.

**`noexcept` on everything Qt calls.** In `GameModel`: the nine
invokables, the six public slots and `showHeldMove`, every property
reader and writer, and the two lambdas given to `connect`. In
`PiecesModel`: both slots and `rowCount`, `data` and `roleNames`. All
seven `ChessEngine` slots. The accessors of the `GameSettings` and
`UISettings` gadgets, the two singleton `create()` functions and the
lambda connected in `main()`. The `EXPECTS` that sat directly in
`engineThreadMoved` became `NOEXCEPT_EXPECTS`. Functions that our own
code calls, such as the view-model base and the `on...Changed`
overrides, are unchanged: their caller stops the exception.

**The engine thread contains its failures.** Each `ChessEngine` slot
that can throw runs its body through `guarded()`, which catches `Error`
and `std::exception`, reports through `logEmergency()`, sets
`my_has_failed` and emits `engineFailed (message, game_id)`. While the
flag is set `guarded()` runs nothing, because the engine's copy of the
game may no longer match the GUI's. `reloadGame()` clears the flag
before it replaces the game.

There is no `catch (...)`: on Linux a thread cancellation unwinds as an
exception that must not be swallowed, and anything that is not a
`std::exception` should reach the `noexcept` boundary and the terminate
handler.

`GameModel::engineThreadFailed` ignores a failure from an earlier game,
and otherwise sets `my_engine_failed`. While that is set the
`gameOverStatus` property reads "Engine error - Start a new game to
continue.", `canMoveFrom` answers no and a move is refused. `restart()`
clears it. It is held beside the view-model's own game-over status
rather than in it, because `updateDisplayedGameState()` recomputes that
one from the game.

The message stays out of the status bar: it is a file, line and
condition, which is for the log. The signal carries it for whoever
wants it later.

Tests:

- `QML: ChessEngine`: a slot that throws emits `engineFailed` with the
  message and the game id; afterwards the engine ignores `init()` and
  `updateConfig()`, and a `reloadGame()` has it move again. The failure
  is a search depth out of range, which breaks a precondition in every
  build type.
- `QML: application`: a failure shows the status, refuses moves, and a
  new game clears it; a failure from an earlier game is ignored.

Not tested: the queued connection between the two, which only compiles
against both signatures, and a main-thread slot reaching the terminate
handler, which the probe showed but which would need a test that ends
its own process.

Not done: the linter rule for `noexcept` on slots and invokables. The
convention is in `AGENTS.md` for now.

**A correction to two earlier logs.** A default configure on the
machine used does not find Qt, and then disables the QML frontend
without failing. So the local builds recorded in `contract-macros.md`
and `postconditions.md` as including the QML UI did not. Both logs are
corrected, CI had built and tested the QML frontend for both, and
`AGENTS.md` now says how to point the build at Qt.

- Verified, both with `WISDOM_CHESS_QML_UI=ON` and a configure that
  reported "Qt6 found. Building QML UI.":
  - GCC Release with `-Werror` against Qt 6.9.3: lint and `all_qmllint`
    clean, all 275 fast and slow tests pass, including the eight `QML:`
    programs.
  - Clang 18 Debug with `-Werror` against Qt 6.11.2: all 241 tests
    pass.
- Not run: Windows, macOS, Android and the WebAssembly builds. CI
  covers the first three.
