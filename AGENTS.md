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

- Everything is in the `wisdom::` namespace.
- Trailing return types: `auto fn() -> ReturnType`.
- `[[nodiscard]]` on factory functions and getters.
- `wisdom::narrow` and `wisdom::narrow_cast` for narrowing conversions.
- `expects (cond)` / `ensures (cond)` (`engine/global.hpp`) check caller
  input and throw. `noexcept_expects` aborts and belongs only in `noexcept`
  functions.
- Report through `logEmergency()` (`engine/logger.hpp`) before
  terminating, never raw `std::cerr`. Every `Logger` implements
  `emergency()` without buffering, and every frontend's `main()` starts by
  calling `setEmergencyLogger()` and `installEmergencyTerminateHandler()`.
- Create a `Game` through its factory functions (`createStandardGame`,
  `createGameFromFen`, ...); its constructors are private. Other classes
  keep plain constructors.
- Frontends (console, QML, WASM/React) observe the game through
  `GameStatusUpdate`; a change to the `Game` API has to reach all of them.

The style linter is built with the project and enforces the formatting
rules (`--list-rules` names them, e.g. `foo (x)` but `bar()`, and
`CHECK( x )` in tests):

```bash
./build/scripts/linter/wisdom-linter <cppfile>
cmake --build build --target lint
```

## Building and testing

Build recipes for every frontend are in `README.md`; the CMake options are
in the top-level `CMakeLists.txt`. Document a new option in both. For
development, configure a Release build with the slow tests on and run
everything through `ctest`:

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

Repetition and fifty-move draw scores belong to the path, not the board,
so `search()` does not store a node whose subtree hit the draw check. The
halfmove clock is not part of the hash; `game_test.cpp` pins the cases.
See `features/2026/09/transposition-table-ownership.md` and
`path-dependent-draw-scores.md` for the reasoning.
