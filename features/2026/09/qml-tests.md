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

### Session #3

The UI tests: `QML: application` loads the real `desktop_main.qml` with the
real `GameModel` and `PiecesModel`, wired as `main.cpp` wires them, and
plays by clicking squares. It covers plan items 2 and 4 and, in practice,
most of item 3. Eleven tests, about 3.5 seconds, under
`QT_QPA_PLATFORM=offscreen` and `QT_QUICK_BACKEND=software`.

- **Getting the QML into a test.** The build tree's copy of the module
  cannot be loaded from disk: its `qmldir` says
  `prefer :/qt/qml/WisdomChess/`, so one file cannot name another unless
  the resources exist ("ImageToolButton is not a type"). The test
  executable embeds the same files under the same resource paths. The
  lists come from the application target's `QT_QML_MODULE_QML_FILES` and
  `QT_QML_MODULE_RESOURCES` properties, plus the three generated `qmldir`
  files, so nothing is listed twice and the application target is still
  untouched. The test then loads `qrc:/qt/qml/WisdomChess/main/desktop_main.qml`
  exactly as the application does.
- **`GameModel` is usable in a test after all.** Item 3 was thought to be
  blocked by `~GameModel` deleting a running thread. The application avoids
  that by calling `applicationExiting()` from the window's closing handler,
  which stops the thread and waits. The test fixture does the same in its
  destructor. The hazard is real: the first prototype returned early from
  a failed check, skipped that call, and died with `QThread: Destroyed
  while thread '' is still running`. The bug-list item stays open.
- Both players are made human first, through `setGameSettings()`, so the
  engine thread runs but never searches and the tests do not depend on the
  engine's choice of move.
- Items are found by their properties, walking `childItems()`: a
  `Repeater`'s delegates are not QObject children of anything the window
  owns, so `findChildren()` sees none of them. The QML has no
  `objectName`s and none were added.
- Clicks are real mouse events on the offscreen window. A move is two
  clicks, because `Board.qml` moves a piece when focus passes from one
  square to another. That needs an active window;
  `QTest::qWaitForWindowActive()` works offscreen.
- Tests: the starting position is drawn where the squares are; every piece
  image loads; a move; an illegal move sets and then clears "Illegal
  move"; a capture removes a delegate; castling; promotion to a knight
  through the dropdown, which takes one click to highlight an entry and a
  second to choose it; check; checkmate, after which the board takes no
  more moves; restart; flipping the board, after which clicks still reach
  the right squares. Every test also fails if the QML engine reported any
  warning, which is what would have caught the `uiSettings`
  ReferenceError, had it been in the desktop QML.
- The castling test samples where the rook is drawn every 10 ms through
  castling, the opponent's next move, and a later move by the same rook.
  It must stay between its source and destination squares throughout.
- **It catches the naive version of queued fix 3.** With `dataChanged`
  emitted when the castling roles clear, the model tests still pass, but
  this test fails: on the move after castling the rook is drawn outside
  its path, and after its next move it is not on its square. Reverted.
- Not covered: `mobile_main.qml` and `MobileRoot.qml`, which are only part
  of the Android module; the dialogs and the game menu; drag and drop, if
  any; the engine actually moving, which needs a search and so a timeout
  or a depth-1 setting.
- Verified: GCC Release and Debug against Qt 6.11.2 with no warnings; the
  four QML tests repeated 15 times at `-j 8` with no failure; all 121 fast
  tests pass; linter clean. Not verified: the CI platforms. The software
  renderer and the offscreen plugin ship with Qt on all three, but window
  activation offscreen on Windows and macOS is the part most likely to
  need attention.

### Session #4

The three queued fixes, each with a test that failed first.

- **Castling roles.** `PiecesModel::playerMoved` now emits `dataChanged`
  for `IsCastlingRookRole` and `CastlingSourceColumnRole` on a row whose
  roles it clears, and only on such a row. `Piece.qml` changed with it:
  `onIsCastlingRookChanged` returns at once when the role has turned off,
  and the two move `Behavior`s are also disabled while
  `castlingRookAnimation` is running, so a reply that arrives before the
  rook has finished sliding cannot start a second animation on the same
  property.
  - What this fixes for a player: after castling, the rook's delegate kept
    `isCastlingRook` true for the rest of the game, which left its move
    animations switched off. A castled rook jumped to its square on every
    later move while all other pieces slid.
  - Tests that failed before: the model announces the clearing
    (`clearingTheCastlingRolesIsAnnounced`), and in the UI the delegate's
    role goes back to false and the rook is seen between e1 and f1 while
    moving (`theCastledRookAnimatesItsLaterMoves`). Tests that guard the
    hazard: the rook stays on its path through castling and the next
    moves, including a reply 100 ms into the animation.
  - A sampling probe, since removed, showed the castled rook covering
    f1 to e1 with the same easing curve as a knight move.
  - The UI test needs a 100 ms pause between the rook arriving and the
    next move. Arriving is not quite the end of the castling animation,
    and no player moves within a millisecond of it.
- **`UISettings::my_flipped`** is initialized to false.
  `uiSettingsStartUnflipped` default-initializes one without braces.
- **`ChessGame::fromPlayers()`** puts the players it is given into the
  config it stores, so the arguments are honoured and `config().players`
  agrees with the game. Both callers in `game_model.cpp` already passed
  players equal to the config's, so the application behaves as before.
  `fromFen()` has no player arguments and still takes them from the config;
  a test pins that. Three tests that relied on the config overriding the
  named players were rewritten to say what they mean.
- Still open, noticed here: `ChessGame::setPlayers()` changes the game's
  players but not `config().players`.
- Verified: Release and Debug, 121 tests each, no warnings; the QML tests
  repeated 15 times at `-j 8`; linter clean; the real `WisdomChessQml`
  binary, whose QML is compiled ahead of time, started offscreen with no
  QML errors. The animation itself has not been looked at on a screen.

### Session #5

Rebased on `main` after PR #243 (`more-tests`) and PR #244, which stops the
engine thread in `~GameModel` through a new `stopEngineThread()`.

- One conflict, in `AGENTS.md`: both branches had added a testing note
  after the same code block. Both notes are kept. This branch never edited
  `game_model.cpp`, so the destructor fix applied untouched.
- Session #3 worked around the destructor by calling
  `applicationExiting()` from the test fixture's destructor. That is
  removed, so all the UI tests now end the way the fix is for: a
  `GameModel` destroyed while its thread runs, with no closing handler
  first. Three tests name the cases: destroyed while running, destroyed
  after `applicationExiting()` called twice, and destroyed without ever
  being started. With the `stopEngineThread()` call taken out of the
  destructor, the first dies with `QThread: Destroyed while thread '' is
  still running`. Put back.
- The Session #3 paragraph above about `GameModel` describes the state
  before that fix.
- Verified: Release and Debug with all 178 fast tests passing, which now
  include the `more-tests` work; no warnings; linter clean.

### Session #6

- First CI run, on the branch as pushed at `12a1973`: the tests passed on
  macOS and Windows, as reported from GitHub Actions. That settles the open
  question from Sessions #1 and #3. The offscreen platform plugin, the
  software renderer, window activation through
  `QTest::qWaitForWindowActive()` and mouse clicks on the offscreen window
  all work on the hosted macOS and Windows runners, and the Windows test
  executables found the Qt DLLs through the `PATH` that
  `install-qt-action` sets. No workflow change was needed.
- The "Not verified: the CI platforms" remarks in Sessions #1, #3 and #4
  describe the state before this run.

### Session #7

The rest of the UI: the status bar, the menu, every dialog, an engine move
and the mobile QML. Two more executables, `QML: dialogs` (19 tests, about
2 seconds) and `QML: mobile` (5 tests, half a second). The fixture moved to
`application_fixture.hpp` so that all three UI executables share it, and
`wisdom_chess_add_qml_ui_test()` embeds the QML in each.

- **Finding, fixed: the draw offer dialogs never opened.** The shared
  view-model refactoring in January (`29208c8`) moved
  `DrawByRepetitionStatus` into the view-model library, which has no Qt, and
  its `Q_ENUM_NS` was lost on the way. In QML
  `DrawByRepetitionStatus.Proposed` became undefined, so
  `visible: _myGameModel.thirdRepetitionDrawStatus == DrawByRepetitionStatus.Proposed`
  in `Dialogs.qml` was never true, silently. The model did reach
  `Proposed`; nothing showed it. Both draw dialogs and the two assignments
  in their handlers were affected.
  - Adding `Q_ENUM_NS (DrawByRepetitionStatus)` to `ui_types.hpp` does
    nothing: moc only registers an enum it sees declared, and says nothing
    when it does not.
  - The fix is a mirror enum in `ui_types.hpp`, `QmlDrawByRepetitionStatus`,
    whose values are written in terms of the real enum's, so they cannot
    drift. QML finds an enum value by its key in the namespace's
    meta-object, whatever the enum is called, so the QML files are
    unchanged.
  - Tests: each of the four keys is found exactly once in the meta-object
    with the right value, and in the UI a threefold repetition opens the
    dialog, Yes ends the game with the draw announced, and No lets play go
    on without the offer being repeated. Writing the enum property from QML
    with a number works, which the accept and decline tests show.
- **Finding, fixed in Session #8: the About dialog has no OK button.**
  `AboutDialog.qml` sets `standardButtons: Dialog.Ok` on a custom `footer`,
  but a `Dialog` gives its footer the dialog's own `standardButtons`, which
  that file never sets. The footer reports `NoButton` and a count of 0,
  where the New Game dialog's reports `Yes|No` and 2. The dialog closes
  with Escape or a click outside it, and one test does that. The likely fix
  is to set `standardButtons` on the `Dialog`.
  `theAboutDialogHasAnOkButton` is an expected failure.
- **Finding, fixed in Session #8: the first click after a draw offer is
  lost.** The
  offer opens while `Board.qml` is still handling the click that caused it,
  so the dialog remembers that square as the item to give focus back to.
  When it closes, the board takes the next click as the target of a move
  from that square. Dialogs opened from the menu do not do this, and a test
  shows that. `theFirstClickAfterADrawOfferSelectsAPiece` is an expected
  failure; the two draw tests spend a click on an empty square first and
  say why.
- The two expected failures use `QEXPECT_FAIL`, so they are reported as
  `XFAIL`, do not fail the run, and turn into a failure (`XPASS`) the day
  the behaviour is fixed, which is the reminder to delete the marker.
- Settings dialog: Apply takes effect after its 350 ms timer, Cancel drops
  the edits and a reopened dialog shows the settings in force, and the
  dialog opens showing the current players and slider values. The radio
  buttons and check boxes have no names, so they are found by class and
  ordered by position.
- Quit: No closes the dialog with no `quit` signal; Yes emits the engine's
  `quit` once. Outside an event loop that is harmless.
- The engine: with Black set to the computer at depth 1, a click move is
  answered and the delegates match the board afterwards, twice in a row.
  About 0.4 seconds, most of it the engine slot's 200 ms sleep.
- Popups are found with `findChildren()`, since they are objects that
  belong to their declaring item, and their buttons by walking the item
  tree, which includes the overlay they are drawn in.
- **Mobile.** `mobile_main.qml` and `MobileRoot.qml` are only in the
  Android build's module. `QML: mobile` embeds them too, with a module
  description made at build time from the application's plus one line
  naming `MobileRoot` (`append_lines.cmake`). `Helper.isMobile()` is false
  on a desktop, so this shows that the files load and work, not how they
  look on a phone. The orientation handler is run by emitting
  `QScreen::primaryOrientationChanged`. With the bug list's
  `uiSettings.squareSize` put back, that test fails with the
  ReferenceError. Reverted.
- Verified: Release and Debug with all 180 fast tests passing, no warnings;
  the six QML tests repeated 15 times at `-j 8`; linter clean; the real
  binary starts offscreen without QML errors. Not yet run on CI.

### Session #8

The two findings that Session #7 pinned as expected failures, fixed.

- **About dialog.** `standardButtons: Dialog.Ok` moved from the footer to
  the `Dialog`, which is where a `Dialog` takes its footer's buttons from.
  The custom footer stays for its alignment. Tests: the dialog closes with
  its OK button, and still closes with Escape.
- **The click lost after a draw offer.** `Board.qml`'s
  `onFocusObjectChanged` now lets go of the clicked square's focus before
  it makes the move, not after. The move is what can open a draw offer,
  and a dialog gives the focus back to whatever had it when it opened. The
  coordinates are read into constants first, since clearing the focus
  re-enters the window's focus handler. Promotion, castling and the rest
  of `QML: application` pass unchanged.
- The markers did their job. With the `Board.qml` fix in, the
  expected-failure test reported `XPASS`, which fails the run, and the two
  draw tests that had spent a click on an empty square to get round the
  bug failed because that click now counted. The marker and both
  workarounds are gone, and no `QEXPECT_FAIL` is left in the tests.
- Verified: Release and Debug with all 180 fast tests passing, no
  warnings; the six QML tests repeated 15 times at `-j 8`; linter clean;
  the real binary, whose QML is compiled ahead of time, starts offscreen
  without QML errors. Not yet run on CI, and not looked at on a screen.

### Session #9

The second CI run, on `0f78ef5` (run 35463217588), failed `QML: dialogs` on
every platform and `QML: mobile` on macOS Release. `QML: application` and
everything else passed. CI installs Qt 6.9.3; the tests had only been run
against 6.11.2.

- **Linux: No acted as Yes.** On the Debug job all five failures were one
  thing: declining a new game restarted it, declining a draw drew the game,
  declining Quit quit. The Release job hit it once. OK, Cancel and Apply
  worked, and no QML warnings were printed.
- An ASAN and UBSAN Debug build against 6.11.2 was clean and passed 40 runs
  of the dialogs test, eight at a time. That rules out a memory error in
  our code but cannot see into Qt, which is not instrumented.
- **Reproduced with Qt 6.9.3**, installed with `aqtinstall` into a scratch
  directory: three failures locally, all No acting as Yes.
- **Cause.** Straight after the dialog opens on 6.9.3, both buttons report
  the same position, centred at (348, 426.5). The button row is laid out
  in the next polish, before the first frame is drawn; 300 ms later No is
  at (434, 426.5). The test aimed at No's centre, which was Yes's, and the
  click went to Yes. On 6.11.2 the row is already laid out by then.
  Nobody can click a frame that has not been drawn, so this is a test bug,
  not an application bug.
- **Fix.** `Application::clickItem()` calls `QQuickTest::qWaitForPolish()`
  on the window before reading the item's position, and `click()` on a
  square goes through it. The UI tests link `Qt6::QuickTest`, which the
  default Qt install includes; the test directory is only added when it is
  found. The one-shot text checks after a dialog opens, one of which
  failed on macOS RelWithDebInfo for the same reason (the text had no size
  yet), are `QTRY_VERIFY` now.
- **macOS: SIGSEGV at 0x4, 0x5 and 0x6.** Each crash came in the first
  test after one that ended with a menu left open, or, once, two tests
  after a dialog was left open. It does not happen on Linux with either Qt
  version. With no Mac and no Valgrind this is unconfirmed. The fixture's
  destructor now closes any open menu or dialog, and waits for it to close,
  before the application is destroyed, which is what a real session always
  does before its window goes away.
- **Windows** printed no test output at all, so nothing is known about its
  failure beyond the exit code. It is expected to be the No-as-Yes problem.
- **Finding, not fixed: the mobile menu button cannot close the menu.**
  While writing a check for it: pressing the button is a press outside the
  open menu, which closes it; the button's click then fires on release,
  sees the menu closed, and opens it again (`mobile_main.qml`,
  `gameMenu.visible ? gameMenu.close() : gameMenu.open()`). Probed: the
  menu is hidden after the press and shown again after the release. The
  test that was to check it is renamed `theMenuOpens` and checks only
  that. The desktop layout only opens the menu from its button.
- Verified: all 180 fast tests pass against Qt 6.9.3 (Debug), 6.11.2
  (Release) and the ASAN build; the six QML tests repeated 20 times at
  `-j 8` against 6.9.3; linter clean. Not verified: macOS and Windows,
  which need the next CI run.

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

All three were fixed in Session #4.

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

Held back until the UI tests existed, then done in Session #4: the
`UISettings` initializer, `fromPlayers()` ignoring its arguments, and the
castling roles cleared without `dataChanged`. Session #3 showed why the
order mattered: emitting the signal without changing `Piece.qml` made its
`onIsCastlingRookChanged` handler, which ran on any change, send the rook
to column -1 and replay the castling animation on the move after castling.
The model tests could not see that. The UI test did.

### Next

Push and check the macOS and Windows jobs: the macOS crash fix is the
unconfirmed one. `ChessGame::setPlayers()` and the mobile menu button are
open in the bug list. Someone should watch a castled rook move, a draw
offer open, and the About dialog close on a real screen once.
