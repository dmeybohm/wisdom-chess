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

### Session #1

Item 1 works as a prototype. Three Qt Test executables, about 0.1 seconds
in total. Qt Test reports 57 passes; it counts each data row, and each
executable's built-in init and cleanup steps, as one.

- `ui/qml/test/CMakeLists.txt` builds `wisdom-chess-qml-test-support`, a
  static library holding a second compilation of `pieces_model`,
  `chess_game`, `game_settings`, `ui_settings` and `ui_types`, and one
  executable per test file through `wisdom_chess_add_qml_test()`. The
  `WisdomChessQml` target is not touched. The subdirectory is added when
  `WISDOM_CHESS_FAST_TESTS` is on, the build is not for Android or
  WebAssembly, and `Qt6Test` is found.
- The tests are `QML: PiecesModel` (24 passes), `QML: ChessGame` (18) and
  `QML: settings and types` (15), all labelled `fast`.
- The `PiecesModel` tests play moves on a `Game` and on the model, then
  compare the model with a fresh one filled from the board, so the board is
  the oracle for every kind of move. `QAbstractItemModelTester` is attached
  throughout.
- Confirmed headless: all three pass with `DISPLAY`, `WAYLAND_DISPLAY` and
  `XDG_SESSION_TYPE` removed from the environment, as expected under
  `QCoreApplication`.
- The QML directory declares `cmake_minimum_required (VERSION 3.16)`, which
  switches off policy CMP0110, so `add_test()` cut the test names at the
  first space and no test ran. The test directory sets the policy.
- Qt Test macros are written `QCOMPARE (a, b)`, with the call spacing the
  linter wants and the QML sources already use for `Q_PROPERTY (...)`. The
  padded `CHECK( x )` form is only exempt for the doctest macros.
- Verified: GCC Release and Debug against Qt 6.11.2, no warnings, all 120
  fast tests pass, linter clean. Not verified: the three CI platforms. On
  Windows the test executables need the Qt DLLs on `PATH`, which
  `install-qt-action` sets up by default.

### What the Session #16 removal bug really was

With the `i--; continue;` fix in `PiecesModel::playerMoved` reverted, the
first version of these tests still passed, including a capture where the
mover directly follows the captured piece in the list. The bug list says
the old loop skipped the element after a removal. It did not quite: the
`piece_model` reference kept pointing at the same slot, which now held the
next element, so the rest of the loop body ran on that element in the same
iteration. What the element missed was the top of the loop, where the
castling roles are cleared. A captured piece at the end of the list also
left the reference one past the last element.

The observable defect is therefore narrow: capture the piece listed
directly before a rook that has just castled, and the rook keeps
`isCastlingRook` and its source column, which drive the castling
animation. `aCaptureBesideTheCastledRookStillClearsItsRoles` sets that up
(queenside castling, then a capture on h2) and fails without the fix:
`isCastlingRook` stays true.

### Findings

Not fixed here.

- `UISettings::my_flipped` has no initializer. `GameModel` value-initializes
  its member, which zeroes it, but `UISettings settings;` anywhere else
  reads an indeterminate `bool`. `GameSettings` initializes every member.
- A `ChessGame::Config`'s players override the players the `Game` was
  created with, because the constructor calls `setConfig()`. The
  `fromPlayers (white, black, config)` arguments are therefore ignored
  whenever they differ from `config.players`. The tests pin this.
- `PiecesModel::playerMoved` clears the castling roles without emitting
  `dataChanged` for them, so a view is only told about the clearing if the
  same row changes for another reason.

### Next

Item 2, the QML load smoke test, is the next step and the first to need
`QT_QPA_PLATFORM=offscreen`. Item 3 waits for the `~GameModel` thread
shutdown fix.

