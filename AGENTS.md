# Claude Instructions for Wisdom Chess

## Git

Unless otherwise asked, we should do the work on a feature branch.
Often the branch will already be checked out, but we should check
out a new feature branch before starting to work. If the main
branch is currently checked out, ask which feature branch to switch
to prior to beginning work.

We should commit after each significant step. You should add a
`Co-Authored-By` annotation for Claude to each commit.

Be wary of rebasing to undo a mistake. You can amend the previous
commit, but if you have to go back further in the history, prefer
to make a new commit instead.

## Feature log

When planning how to implement a feature, we should store the plans
in markdown format in the feature logs.

This is to keep track of development and design rationales. The
feature log is located in the `features/` directory, and it is
organized by year and then month. All plan documents for each
feature should go in the `features/$year/$month/{$git_branch_name}.md`.
Each document there should have the plan for implementing the
feature.

If you need to document the implementation status, prefer to add a
"Implementation Progress" section in the feature's corresponding
markdown file, as opposed to adding markdown files in other
locations, to keep the feature development documentation organized.
You can include a "Session #1" subsection for the first
implementation progress part, and then subsequently increment the
number for additional sections. There is no generated index of the
feature documents, so you only need to update the individual documents.

You can also consult the `features/` directory if you're confused
about how something was implemented in order to try to clarify, if
the code is not clear.

## Code Style Guidelines

### C++ Code Style

Follow these guidelines:

1. **Code Formatting**:

The project follows an idiosyncratic format for C++ code. The C++ style linter
is built automatically with the project. After building, run:

```bash
# Run linter on specific file
./build/scripts/linter/wisdom-linter <cppfile>

# Run linter on entire source tree
cmake --build build --target lint

# Run linter tests
./scripts/linter/tests/run-tests.sh build/scripts/linter/wisdom-linter
```

**Linter Rules:**

- **namespace-braces**: Namespace opening braces must be on their own line
- **trailing-return-type**: Functions must use `auto fn() -> ReturnType` syntax
- **test-macro-spacing**: Test macros, doctest's and Qt Test's, need spaces inside parens: `CHECK( x )` not `CHECK(x)`, `QCOMPARE( a, b )` not `QCOMPARE (a, b)`
- **function-call-spacing**: Functions with args need space before paren: `foo (x)` not `foo(x)`; zero-arg functions have no space: `bar()` not `bar ()`
- **no-tabs**: No tab characters; indent with spaces

**Configuration:** There is no configuration file. Rule severities come from `getDefaultConfig()` in `scripts/linter/linter.cpp`; `--rules` selects rules on the command line and `--list-rules` shows them.

**Adding Rules:** Rules are in `scripts/linter/rules/`. Create a new `Rule` subclass, implement `name()`, `description()`, and `check()`, then register it in `rules/init.cpp`.

2. **Code Style Guidelines**:
   - Use trailing return types with auto: `auto functionName() -> ReturnType`
   - Use `[[nodiscard]]` for functions that return values that shouldn't be ignored
   - Everything is in the `wisdom::` namespace
   - Use `wisdom::narrow` and `wisdom::narrow_cast` for narrowing conversions

## Build Instructions

### Prerequisites

- **C++ Compiler**: GCC, Clang, or MSVC with C++20 support
- **CMake**: Version 3.20 or higher
- **Optional**: Qt 6.8+ for QML UI
- **Optional**: Emscripten SDK for WebAssembly/React build
- **Optional**: Node.js for React frontend development

### Desktop Build (Console + Tests)

```bash
# Create build directory
mkdir build && cd build

# Configure (includes slow tests; use Release for acceptable speed)
cmake .. -DCMAKE_BUILD_TYPE=Release -DWISDOM_CHESS_SLOW_TESTS=On

# Build
cmake --build . -j8

# Run all tests in parallel (recommended - better pipelining)
ctest -j 4 --test-dir .

# Run only fast or slow tests separately if needed
ctest -j 4 --test-dir . -L fast
ctest -j 4 --test-dir . -L slow
```

### Qt QML UI Build

```bash
# Configure with Qt path (adjust path as needed)
cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.6.2/gcc_64 -DCMAKE_BUILD_TYPE=Release

# Or force QML UI to be built (fails if Qt not found)
cmake .. -DWISDOM_CHESS_QML_UI=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --target WisdomChessQml
```

### Installer Build (Qt Installer Framework)

```bash
# Requires QtIFW 4.x (auto-detected under ~/Qt or C:/Qt, else -DCPACK_IFW_ROOT=...)
cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DCMAKE_BUILD_TYPE=Release \
    -DWISDOM_CHESS_INSTALLER=ON

# Produces wisdom-chess-<version>-<OS>-<arch>.run / .exe / .dmg in the build dir
cmake --build . --target installer
```

The CPack/IFW configuration lives in `cmake/Installer.cmake`; the Qt runtime
deployment script and install rules are in `src/wisdom-chess/ui/qml/CMakeLists.txt`;
the installer's component script (shortcuts, .desktop entry, vc_redist) is
`installer/installscript.qs`. Icons are regenerated with `scripts/generate-icons.py`.
On Windows the installer build requires MSVC and `vc_redist.x64.exe`
(`VCToolsRedistDir` from a VS developer prompt, or `-DWISDOM_CHESS_VCREDIST=<path>`).

CI: `.github/workflows/installers.yml` builds all three installers on
`ubuntu-latest`, `windows-latest` and `macos-latest` (Qt 6.9, QtIFW 4.7 via
`jurplel/install-qt-action`), smoke-tests each with
`scripts/smoke-test-installer.sh` (headless install, tree checks, purge) and
uploads them as artifacts. It runs on pull requests touching installer files,
on `workflow_dispatch`, and on `v*` tags, where it also creates a GitHub
Release with the installers attached. The tag must match the `VERSION` in the
top-level `CMakeLists.txt`.

### WebAssembly + React Build

```bash
# Setup Emscripten environment (adjust path as needed)
source ~/projects/3rdparty/emsdk/emsdk_env.sh

# Create build directory
mkdir build-wasm && cd build-wasm

# Configure (React build is integrated by default for WASM)
emcmake cmake .. \
  -DWISDOM_CHESS_CONSOLE_UI=OFF \
  -DWISDOM_CHESS_QML_UI=OFF \
  -DWISDOM_CHESS_FAST_TESTS=OFF \
  -DWISDOM_CHESS_SLOW_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release

# Build WASM engine and React frontend
cmake --build . --target wisdom-chess-react

# The integrated build will:
# 1. Build the WASM chess engine
# 2. Copy WASM files to React public directory
# 3. Run npm install
# 4. Run npm build to create production build

# To run development server instead:
cmake --build . --target wisdom-chess-react-dev
```

The TypeScript types for the WASM module are generated from
`src/wisdom-chess/ui/wasm/wisdom-chess.idl`. After changing the IDL, run
`npm run generate:wasm-types` in `src/wisdom-chess/ui/react` and commit
`src/lib/wisdom-chess-module.d.ts` and `src/test/wasm-enum-values.ts`. CI
runs `npm run check:wasm-types` and fails if they are stale. The generator
(`webidl-dts-gen`) is run through a pinned `npx` and is deliberately not a
dependency. Use enum types in the IDL, not `long`, for enum values.

### WebAssembly + Qt QML Build

```bash
# Setup Emscripten environment
source ~/projects/3rdparty/emsdk/emsdk_env.sh

# Create build directory
mkdir build-qml-wasm && cd build-qml-wasm

# Configure with Qt for WebAssembly
emcmake cmake .. \
  -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/wasm_multithread \
  -DWISDOM_CHESS_QML_UI=ON \
  -DCMAKE_BUILD_TYPE=Release

# Build QML WebAssembly application
cmake --build . --target WisdomChessQml

# Note: The QML WebAssembly build uses wasm_main.qml and does not use the wasm/ directory
# The resulting .wasm and .js files need to be served by a web server
```

### Build Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `WISDOM_CHESS_CONSOLE_UI` | Bool | ON | Build console chess game |
| `WISDOM_CHESS_QML_UI` | String | AUTO | Build QML UI: AUTO/ON/OFF |
| `WISDOM_CHESS_QT_DIR` | Path | - | Path to Qt installation |
| `WISDOM_CHESS_REACT_UI` | Bool | ON | Build React UI |
| `WISDOM_CHESS_REACT_BUILD_INTEGRATED` | Bool | ON (WASM) / OFF (others) | Integrate Node.js build with CMake |
| `WISDOM_CHESS_FAST_TESTS` | Bool | ON | Build fast tests |
| `WISDOM_CHESS_SLOW_TESTS` | Bool | OFF | Build slow tests |
| `WISDOM_CHESS_PCH_ENABLED` | Bool | ON | Use precompiled headers |
| `WISDOM_CHESS_ASAN` | Bool | OFF | Enable address sanitizer |
| `WISDOM_CHESS_BUILD_LINTER` | Bool | ON | Build C++ style linter (native builds only) |
| `WISDOM_CHESS_FILC_COMPAT` | Bool | Auto-detected | Enable FIL-C runtime compatibility (auto-detected via `__PIZLONATOR_WAS_HERE__`) |
| `WISDOM_CHESS_INSTALLER` | Bool | OFF | Build a Qt Installer Framework installer for the desktop QML app (`installer` target; needs QtIFW, see `CPACK_IFW_ROOT`) |

### QML UI Options

- `AUTO`: Build if Qt6 is found (default)
- `ON`: Require Qt6, fail if not found
- `OFF`: Disable even if Qt6 is available

### Testing

```bash
# Run all tests in parallel (recommended)
cd build && ctest -j 4 --test-dir .

# Run only fast or slow tests separately if needed
cd build && ctest -j 4 --test-dir . -L fast
cd build && ctest -j 4 --test-dir . -L slow

# Run a specific test directly with success output
build/src/wisdom-chess/engine/test/wisdom-chess-fast-tests --success
```

Besides the engine's doctest executables, `ctest` runs:

- `wisdom-chess-viewmodel-tests` (`ui/viewmodel/test`), doctest cases for the
  shared view-model.
- The `UCI: ...` and `Console: ...` tests, which run those binaries with
  scripted standard input and match the output against regular expressions.
  Add one with `wisdom_chess_add_cli_test()` from `cmake/CliTests.cmake`.
  A UCI script that starts a search must send `stop` before `quit`, or no
  `bestmove` is printed.

`isCheckmated()`, `isStalemated()` and `hasLegalMove()` read the side to move
from the board. `generateLegalMoves()` and `Board::withMove()` still take a
color, which must be the side to move; `withMove()` asserts it in Debug. Run
new engine tests in a Debug build as well as Release.

When the QML UI is built and `Qt6Test` is found, `ctest` also runs the
`QML: ...` tests in `src/wisdom-chess/ui/qml/test`. They use Qt Test, not
doctest, and cover the QML frontend's C++ classes that need no thread or
display. Add one with `wisdom_chess_add_qml_test()` in that directory's
`CMakeLists.txt`; the sources under test are listed in
`wisdom-chess-qml-test-support` there. The Qt Test macros are styled like the
doctest ones: `QCOMPARE( a, b )`.

`QML: application`, `QML: dialogs` and `QML: mobile` load the real QML with
the real models and work it by clicking. They share
`application_fixture.hpp`; add one with `wisdom_chess_add_qml_ui_test()`.
`ctest` runs them with `QT_QPA_PLATFORM=offscreen` and
`QT_QUICK_BACKEND=software`, and on macOS with `QT_QUICK_CONTROLS_STYLE=Fusion`
because the native macOS style crashes without Cocoa; set these when running
an executable by hand.
`GameModel` holds an engine move back until the move before it has
finished animating, for `animationDelay` milliseconds and
`castlingRookPause` more after castling. A test that would otherwise race
the hold calls `setAnimationDelay()` to make it long enough to measure or
short enough to ignore.
Each test fails on any QML warning. Click through the fixture's
`clickItem()`, which waits for pending layout first: until then an item's
position can be stale, and on Qt 6.9 a click aimed at a dialog's No button
lands on Yes. CI uses Qt 6.9, not the Qt under `~/Qt`;
`./scripts/install-ci-qt.sh` installs a matching one for reproducing a
CI-only failure and prints the `-DWISDOM_CHESS_QT_DIR=...` to build
against. It needs no root and takes a version and a directory as optional
arguments. A known defect that is not being fixed
yet is pinned with `QEXPECT_FAIL`, so that fixing it fails the test and the
marker gets removed.

An enum that QML compares against must be in the meta-object of
`wisdom::ui` (`ui/qml/main/ui_types.hpp`). A missing one is `undefined` in
QML and no warning is given. Enums from the Qt-free view-model library need
a mirror enum there, as `DrawByRepetitionStatus` has.

### Linting and Type Checking

When making changes, always run:
```bash
# For C++ (example, adjust based on project setup)
# Check for compilation errors
cmake --build . --target wisdom-chess-core

# For React/TypeScript
cd src/wisdom-chess/ui/react
npm run build  # This runs tsc && vite build
```

## Project Structure

- `src/wisdom-chess/engine/` - Core chess engine
- `src/wisdom-chess/ui/console/` - Console UI
- `src/wisdom-chess/ui/qml/` - Qt QML UI (desktop, mobile, and WebAssembly)
- `src/wisdom-chess/ui/wasm/` - WebAssembly bindings for React frontend
- `src/wisdom-chess/ui/react/` - React web frontend

Note: There are two WebAssembly frontends:
1. Qt QML compiled to WebAssembly (uses `qml/` directory with `wasm_main.qml`)
2. React frontend with WebAssembly chess engine (uses `wasm/` + `react/` directories)

## API Design Notes

### Game Class
- The `Game` class uses factory functions to encapsulate its construction complexity:
  - `Game::createStandardGame()` - standard chess setup
  - `Game::createGame(players)` - custom player configuration
  - `Game::createGameFromFen(fen)` - load from FEN string
  - `Game::createGameFromBoard(builder)` - custom board setup
- Game constructors are private to ensure proper initialization
- This pattern is specific to Game due to its complex initialization requirements

### General API Guidelines
- All public API is in the `wisdom::` namespace
- Use `[[nodiscard]]` for factory functions and getters where appropriate
- Prefer simple constructors for most classes (allows `make_unique`, aggregate init, etc.)
- Only use factory functions when there's a clear benefit (complex initialization, multiple construction paths, etc.)

### Contracts and Fatal Errors
- `expects (cond)` and `ensures (cond)` in `engine/global.hpp` throw `PreconditionError` / `PostconditionError`. Use them for checks on caller input.
- `noexcept_expects (cond)` reports the failure and aborts. Use it only inside `noexcept` functions, where an exception could not propagate.
- The engine does not abort on a failed search. `iterativelyDeepen()` throws `SearchError` (`engine/search.hpp`), whose extra info ends with the board being searched. Uncaught, it reaches the terminate handler below.
- Before terminating, report through `logEmergency()` (`engine/logger.hpp`), never raw `std::cerr`. It writes to `std::cerr` and to the logger registered with `setEmergencyLogger()`.
- Every `Logger` must implement `emergency()` without buffering.
- Each frontend calls `setEmergencyLogger()` and `installEmergencyTerminateHandler()` as the first statements of `main()`.

## Common Tasks

### Working with the Game Class
1. Always use factory functions to create Game objects
2. Update all UI frontends when changing Game creation APIs
3. Ensure tests use the appropriate factory functions

### Modifying CMake Build
1. Add options to top-level CMakeLists.txt
2. Use consistent `WISDOM_CHESS_` prefix
3. Document the option in this file

### Working with Multiple Frontends
The project uses the Observer pattern with `GameStatusUpdate` interface:
- Console UI
- QML UI
- WASM/React UI
All frontends implement this interface for game state updates.
