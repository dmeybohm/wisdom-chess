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
- Qt Test macros were first written `QCOMPARE (a, b)`, because the linter
  only knew the doctest macros as test macros. Session #2 changed the
  linter and the tests to the padded form.
- Verified: GCC Release and Debug against Qt 6.11.2, no warnings, all 120
  fast tests pass, linter clean. Not verified: the three CI platforms. On
  Windows the test executables need the Qt DLLs on `PATH`, which
  `install-qt-action` sets up by default.

### Session #2

- The linter treats the Qt Test macros as test macros, so they are written
  like the doctest ones: `QCOMPARE( a, b )`, no space before the
  parenthesis and padding inside it. `QVERIFY`, `QVERIFY2`, `QCOMPARE` and
  its `_EQ` family, the `QTRY_` forms, the two `QVERIFY_THROWS_` forms,
  `QFETCH`, `QFETCH_GLOBAL`, `QFAIL`, `QSKIP`, `QEXPECT_FAIL` and the three
  `QTEST_..._MAIN` macros were added to the list in `test-macro-spacing`
  and to the exemptions in `function-call-spacing`. Other Qt macros such as
  `Q_PROPERTY (...)` keep the ordinary call spacing.
- Two fixtures in `scripts/linter/tests/test-macro-spacing` cover it: one
  clean file, including a multi-line `QCOMPARE(`, and one with each kind of
  violation. The linter's self-tests go from 19 to 21.
- The three QML test files were converted. Linter clean over the tree.

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

### Queued fixes

The three findings above are to be fixed after the UI tests (items 2 and
4) exist, because the fixes can change what the QML does and the tests
should be there to show it.

1. Give `UISettings::my_flipped` an initializer.
2. `ChessGame::fromPlayers()` ignores its player arguments. Both callers
   are in `game_model.cpp`: the constructor passes Human and ChessEngine,
   which is what the default `GameSettings` say anyway, and `restart()`
   passes the current game's players next to `gameConfig()`. Decide whether
   the arguments or the config should win, then drop the loser.
3. Emitting `dataChanged` when the castling roles clear is not safe as
   things stand. `Piece.qml`'s `onIsCastlingRookChanged` runs on any
   change, true to false included: it sets the x translation to
   `castlingSourceColumn * squareSize`, which would be column -1, and
   restarts the castling animation. Today the delegate is never told the
   role went back to false, so after castling the rook's delegate keeps
   `isCastlingRook` true. Its `Behavior on x` and `Behavior on y` stay
   disabled, so its later moves are not animated. A second castling by the
   same rook cannot happen, so the handler never needs to fire twice. A fix
   has to change the handler and the model together, and needs a UI test
   that watches the rook's delegate through castling and the moves after.

### Next

Item 2, the QML load smoke test, is the next step and the first to need
`QT_QPA_PLATFORM=offscreen`. Item 3 waits for the `~GameModel` thread
shutdown fix.

