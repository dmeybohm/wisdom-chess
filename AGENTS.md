# Agent Instructions for Wisdom Chess

## Git

Work on a feature branch. If `main` is checked out, ask which feature
branch to switch to before starting.

Commit after each significant step, with a `Co-Authored-By` annotation.
Amending the previous commit is fine; to undo anything older, prefer a new
commit over a rebase.

Keep pull request descriptions short: the title, a summary of a sentence or
two, and optionally a bulleted list of the essential items, one line each.
Leave the details to the feature log and link to it.

## Feature log

Plans and design rationale live in `features/$year/$month/$branch.md`.
Record implementation status there too, under an "Implementation Progress"
heading with "Session #N" subsections, rather than in markdown files
elsewhere. There is no index to update. Consult `features/` when the code
does not make clear why something was done the way it was.

## Code style

The layout, naming and the style linter that enforces them are in
`docs/coding-style.md`. Run `cmake --build build --target lint` before
committing C++. The conventions below are about what the code does.

- Everything is in the `wisdom::` namespace.
- `[[nodiscard]]` on factory functions and getters.
- `wisdom::narrow` and `wisdom::narrow_cast` for narrowing conversions.
  `narrow` throws `PreconditionError` when the value does not fit;
  `narrow_cast` is a `static_cast`.
- `EXPECTS( cond )` / `ENSURES( cond )` (`engine/error.hpp`) check caller
  input and throw. `NOEXCEPT_EXPECTS( cond )` aborts and belongs only in
  `noexcept` functions. `ASSERT( cond )` replaces `assert()`: it aborts
  only when `Debugging` is on, and otherwise the condition is not
  evaluated. The macros quote the condition in the failure message, so
  the functions under them (`expects()`, ...) are not called directly.
- Report through `logEmergency()` (`engine/logger.hpp`) before
  terminating, never raw `std::cerr`. Every `Logger` implements
  `emergency()` without buffering, and every frontend's `main()` starts by
  calling `setEmergencyLogger()` and `installEmergencyTerminateHandler()`.
- Qt is not exception-safe, so no exception may enter it. Every
  function Qt calls in the QML frontend is `noexcept`: slots,
  `Q_INVOKABLE`s, property accessors, model overrides, singleton
  `create()` functions and lambdas given to `connect`. `ChessEngine`'s
  slots run their bodies through `guarded()`, which reports a failure
  with `engineFailed` instead. See
  `features/2026/09/qt-exception-safety.md`.
- Create a `Game` through its factory functions (`createStandardGame`,
  `createGameFromFen`, ...); its constructors are private. Other classes
  keep plain constructors.
- Frontends (console, QML, WASM/React) observe the game through
  `GameStatusUpdate`; a change to the `Game` API has to reach all of them.
- A `Game` decides its own draws (`DrawArbiter::GameEngine`) unless the
  caller does, as UCI's GUI does (`DrawArbiter::External`). The search
  knows neither: `Game::getDrawLimits()` gives it the `DrawLimits` to
  apply. See `features/2026/10/draw-arbiter.md`.
- Raw pointers never own; ownership is `unique_ptr` or `shared_ptr`.
  Spell a non-owning pointer by its nullability (`engine/ptr.hpp`):
  `nonnull<Type>` or `nullable<Type>`, which cannot be dereferenced: test
  it, then take `value()` for a `nonnull`, which throws when null. A
  `nonnull` checks for null when constructed, where a null aborts, and
  not when dereferenced; pass `get()` to an API that takes a raw pointer.
  See `features/2026/10/single-nonnull.md`. A C string is `czstring` or
  `zstring`. `owning<Type>` is an owning raw pointer, only where
  something outside C++ arranges the deletion, such as Qt's
  `deleteLater()`. For an object handed to JavaScript, hold a
  `unique_ptr` and `release()` it in the `return`.
- The linter's `raw-pointer` rule enforces the pointer rules. Qt types and
  `auto*` locals are exempt. For a pointer an API requires (`main`, QML's
  singleton `create()`, the WebIDL bindings, `EM_JS`), end the line with
  `// lint-allow(raw-pointer): <reason>`. `lint-allow(<rule>)` silences
  any rule on its line.
- Use `nonnull<Type>` in preference to a mutable `Type&` reference, for
  both functions and member variables. Use `const Type&` for const
  references.

## Building and testing

A configure that does not find Qt 6 disables the QML frontend and its
tests without failing. To build them, pass `-DWISDOM_CHESS_QML_UI=ON`
and `-DWISDOM_CHESS_QT_DIR=<Qt>/gcc_64`.

Build recipes for every frontend and the table of CMake options are in
`docs/building.md`; the options themselves are defined in the top-level
`CMakeLists.txt`. Document a new option in both. `README.md` is written
for players and points at `docs/building.md`. For development, configure
a Release build with the slow tests on and run everything through
`ctest`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DWISDOM_CHESS_SLOW_TESTS=On
cmake --build build -j8
ctest --test-dir build -j 4          # -L fast / -L slow to pick one
```

Run new engine tests in a Debug build as well. `Board::withMove()` and
`generateLegalMoves()` take a color that must be the side to move, and
Debug builds assert it; `isCheckmated()`, `isStalemated()` and
`hasLegalMove()` read it from the board instead.

The `UCI: ...` and `Console: ...` tests (`cmake/CliTests.cmake`) script the
binaries' standard input. A UCI script that starts a search must send
`stop` before `quit`, or no `bestmove` is printed.

The `Fatal: ...` tests cover what doctest cannot catch because it ends
the process: a failed `NOEXCEPT_EXPECTS` or `ASSERT`, a null `nonnull`, an
uncaught exception. Add one as a function and an entry in `Fatal_Cases`
in `engine/test/fatal_test_main.cpp`; the build asks the program for the
list.

The `QML: ...` tests (`src/wisdom-chess/ui/qml/test`) use Qt Test, styled like doctest:
`QCOMPARE( a, b )`. Add one with `wisdom_chess_add_qml_test()` or, for a
test that loads the real QML, `wisdom_chess_add_qml_ui_test()` on
`application_fixture.hpp`. Things to know:

- Any QML warning fails the test.
- Click through the fixture's `clickItem()`, which waits for layout first.
- `GameModel` holds an engine move back for `animationDelay` milliseconds;
  a test that would race it calls `setAnimationDelay()`.
- An enum QML compares against must be in the `wisdom::ui` meta-object
  (`src/wisdom-chess/ui/qml/main/ui_types.hpp`), or it is silently `undefined`.
- Pin a known, unfixed defect with `QEXPECT_FAIL` so fixing it fails the
  test.
- When run by hand, set `QT_QPA_PLATFORM=offscreen` and
  `QT_QUICK_BACKEND=software` (plus `QT_QUICK_CONTROLS_STYLE=Fusion` on
  macOS) as `ctest` does. CI uses Qt 6.9; `./scripts/install-ci-qt.sh`
  installs a matching one for reproducing a CI-only failure.
- `cmake --build build --target all_qmllint` runs Qt's `qmllint` over the
  QML module; CI runs it on Linux and it fails on any warning
  (`src/wisdom-chess/ui/qml/.qmllint.ini`).

An Android build runs its tests on the connected device or emulator,
through `adb` (`cmake/AndroidTests.cmake`). Test discovery does too, so
without a device the build itself fails unless the tests are off. A test
there cannot use a path from the build directory. CI runs them on an
emulator, through `scripts/android-tests.sh`, which works locally too and
gives a failed test a second try.

The `QML: ...` tests run there too, each as a package that Qt's
`androidtestrunner` installs and runs. They draw on the device's display,
in the Material style, and the ones that load the application load the
mobile layout (`Platform_Main_Qml_File`). That style animates its popups,
so compare a popup's position with `QTRY_COMPARE`.

### Sanitizers

CI runs AddressSanitizer and UndefinedBehaviorSanitizer with Clang over the
QML UI and both test suites. To reproduce locally:

```bash
CC=clang-18 CXX=clang++-18 cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWISDOM_CHESS_QML_UI=On -DWISDOM_CHESS_SLOW_TESTS=On -DWISDOM_CHESS_ASAN=On
cmake --build build-asan -j $(nproc)
ASAN_OPTIONS=detect_leaks=1:strict_string_checks=1:check_initialization_order=1 \
UBSAN_OPTIONS=print_stacktrace=1 ctest --test-dir build-asan -j 4 --output-on-failure
```

ThreadSanitizer needs a Qt built with it, which `./scripts/build-tsan.sh`
takes care of; its header explains the options. A sanitizer report
entirely inside third-party code goes in `scripts/sanitizers/*.supp`,
never one that touches our own code.

### WASM types

The React frontend's TypeScript types are generated from
`src/wisdom-chess/ui/wasm/wisdom-chess.idl`. After changing the IDL, run
`npm run generate:wasm-types` in `src/wisdom-chess/ui/react` and commit the result; CI
fails if it is stale. Use enum types in the IDL, not `long`.

## Transposition table

The table is search state, not game state: `Game::findBestMove()` borrows
the caller's, and whoever runs searches (`UciInterface`, `ConsoleGame`,
`worker::GameState`, `ChessEngine`) owns one, keeps it across moves and
clears it on a new game. `TranspositionTable` is move-only, and a frontend
that hands it to a search thread must not touch it until that thread is
done.

The table and `History` both key on `Board::getBoardCode()`, which leaves
out an en passant target that no legal capture can use.
`getUnnormalizedBoardCode()` keeps the target as FEN records it. See
`features/2026/09/en-passant-normalization-cost.md`.

Repetition and fifty-move draw scores belong to the path, not the board,
so `search()` does not store a node whose subtree hit the draw check. The
halfmove clock is not part of the hash; `game_test.cpp` pins the cases.
See `features/2026/09/transposition-table-ownership.md` and
`path-dependent-draw-scores.md` for the reasoning.
