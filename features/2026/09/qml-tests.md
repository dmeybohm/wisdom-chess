# QML C++ tests

## Motivation

The QML frontend's C++ classes are the last untested code named in the
"Tests" section of
[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md). One fix
there, the row removal in `PiecesModel::playerMoved` (Session #16), has
never been run, because reaching it needs a GUI session.

CI already installs Qt on Ubuntu, macOS and Windows and builds the QML
frontend, and Qt Test comes with the base Qt install, so no new CI
dependency is needed.

## Plan

The work is split by how much of Qt a test needs to be running:

1. **Classes without threads or a GUI**: `PiecesModel`, `ChessGame`,
   `GameSettings`, `UISettings` and the `ui_types` mappings. This document
   and branch cover this item only, as a prototype of the harness.
2. A smoke test that loads the root QML files offscreen and fails on any
   QML warning.
3. `GameModel` and `ChessEngine`, which start a thread. Blocked on the
   open bug-list item about `~GameModel` deleting its thread without
   `quit()` and `wait()`.
4. Interaction tests written in Qt Quick Test.

### Design for item 1

- **Qt Test, not doctest.** `QSignalSpy` and `QAbstractItemModelTester`
  are the tools for a list model, and the later items need Qt Test's event
  loop support, so the harness starts with it. Each Qt Test executable is
  one `ctest` test.
- **The shipping target is left alone.** Every source is compiled directly
  into the `WisdomChessQml` executable, which a test cannot link. Moving
  them into a library would touch the target that the installer, Android
  and WebAssembly builds hang off. The tests compile the sources they need
  a second time, into a static library used only by the tests.
- **No display.** These classes need only `QtCore`, so the tests use
  `QTEST_GUILESS_MAIN` and start no platform plugin at all. Items 2 and
  later will need `QT_QPA_PLATFORM=offscreen`.
- The tests are built when the QML UI is built, `WISDOM_CHESS_FAST_TESTS`
  is on, and `Qt6Test` is found. They carry the `fast` label.

### What the tests cover

- `PiecesModel`: the 32 starting pieces and their roles; a plain move; a
  capture with the captured piece before and after the mover in the list;
  both castling moves for both colors, with the rook's animation roles; en
  passant for both colors; promotion, with and without a capture; a new
  game replacing the rows. `QAbstractItemModelTester` runs throughout and
  fails the test on any inconsistent row signal.
- `ChessGame`: `MaxDepth`, `Config::fromGameSettings`, the factories,
  `setConfig`, `clone`, `isLegalMove` and `moveFromCoordinates`.
- `GameSettings` and `UISettings`: defaults, properties written the way QML
  writes them, and equality. The `ui_types` mappings round-trip.

## Implementation Progress
