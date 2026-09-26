# Building and testing Wisdom Chess

This is the developer's guide. To play, see the [README](../README.md):
installers for every desktop platform are attached to each release, and
the web version needs no install at all.

## Prerequisites

- **C++ compiler**: GCC, Clang or MSVC with C++20 support
- **CMake**: 3.20 or newer
- **Qt 6.8 or newer** for the desktop, Android and Qt web builds and
  their tests (CI uses 6.9; `scripts/install-ci-qt.sh` installs a
  matching one)
- **Emscripten SDK** for either web build
- **Node.js** for the React frontend

The engine's other dependencies (GSL, doctest, nanobench) are fetched by
[CPM](https://github.com/cpm-cmake/CPM.cmake) at configure time, pinned
by `cpm-package-lock.cmake`.

## Console version

The simplest build, and the one to start with:

```bash
git clone https://github.com/dmeybohm/wisdom-chess.git
cd wisdom-chess
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
./build/src/wisdom-chess/ui/console/wisdom-chess-console
```

The same build produces `wisdom-chess-uci`, a
[UCI](https://en.wikipedia.org/wiki/Universal_Chess_Interface) engine
that plays in any chess GUI, next to the console binary under
`build/src/wisdom-chess/ui/uci/`.

## Web version (React + WebAssembly)

1. **Install the Emscripten SDK**:
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Build the WebAssembly engine and the React frontend together**:
   ```bash
   emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release
   cmake --build build-web --target wisdom-chess-react
   ```

   This builds the WebAssembly chess engine, runs `npm install` for the
   React dependencies and builds the production frontend into
   `src/wisdom-chess/ui/react/dist/`.

3. **Development server**:
   ```bash
   cmake --build build-web --target wisdom-chess-react-dev
   ```
   Starts a development server at `http://localhost:5173`.

`scripts/build-react-wasm.sh` is what CI runs for this build.

## Desktop version (Qt QML)

1. **Install Qt 6.8 or newer** from [qt.io](https://www.qt.io/).

2. **Build with Qt**:
   ```bash
   cmake -S . -B build-desktop -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DCMAKE_BUILD_TYPE=Release
   cmake --build build-desktop --target WisdomChessQml
   ./build-desktop/src/wisdom-chess/ui/qml/WisdomChessQml
   ```

## Installers (Qt Installer Framework)

The installers for Linux (x86_64), Windows (x64) and macOS (Apple
Silicon) that the [README](../README.md#download) describes are built by
the `Installers` GitHub Actions workflow, which also runs on pull requests
that touch the installer and can be started by hand from the Actions tab.

To package the desktop app locally with the
[Qt Installer Framework](https://doc.qt.io/qtinstallerframework/) (QtIFW):

1. **Install QtIFW 4.x** with the Qt Maintenance Tool (under *Qt > Developer
   and Designer Tools*). It is auto-detected under `~/Qt` or `C:/Qt`;
   otherwise pass `-DCPACK_IFW_ROOT=<path to Tools/QtInstallerFramework/<version>>`.

2. **Configure with `WISDOM_CHESS_INSTALLER=ON` and build the `installer` target**:

   Linux:
   ```bash
   cmake -S . -B build-installer -DCMAKE_BUILD_TYPE=Release \
       -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DWISDOM_CHESS_INSTALLER=ON
   cmake --build build-installer --target installer
   # -> build-installer/wisdom-chess-<version>-Linux-x86_64.run
   ```

   Windows (from an *x64 Native Tools Command Prompt for VS*, so that
   `vc_redist.x64.exe` can be found; or pass `-DWISDOM_CHESS_VCREDIST=<path>`):
   ```bat
   cmake -S . -B build-installer -G Ninja -DCMAKE_BUILD_TYPE=Release ^
       -DWISDOM_CHESS_QT_DIR=C:/Qt/6.9.2/msvc2022_64 -DWISDOM_CHESS_INSTALLER=ON
   cmake --build build-installer --target installer
   REM -> build-installer\wisdom-chess-<version>-Windows-AMD64.exe
   ```

   macOS:
   ```bash
   cmake -S . -B build-installer -DCMAKE_BUILD_TYPE=Release \
       -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/macos -DWISDOM_CHESS_INSTALLER=ON
   cmake --build build-installer --target installer
   # -> build-installer/wisdom-chess-<version>-Darwin-arm64.dmg
   ```

To check an installer without clicking through it, install it headlessly
with `scripts/smoke-test-installer.sh <build-dir> <install-root>`; this is
what CI runs on all three platforms. The release installers are built on
Ubuntu 24.04, so the Linux binary needs a distribution at least that new.

## Web version (Qt QML + WebAssembly)

The Qt QML interface can also be compiled to WebAssembly, which needs Qt
for WebAssembly, a separate Qt installation:

```bash
source ./emsdk_env.sh
emcmake cmake -S . -B build-qml-wasm -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/wasm_multithread -DCMAKE_BUILD_TYPE=Release
cmake --build build-qml-wasm --target WisdomChessQml
# Serve the generated files with a web server
```

`scripts/build-qml-wasm.sh` is what CI runs for this build.

## Android version

Use Qt Creator with the Android NDK configured. See the
[Qt Android documentation](https://doc.qt.io/qt-6/android-getting-started.html)
for setup details.

## Building with FIL-C

[FIL-C](https://github.com/pizlonator/fil-c) is auto-detected at configure
time via the `__PIZLONATOR_WAS_HERE__` preprocessor macro. When detected,
`WISDOM_CHESS_FILC_COMPAT` is enabled, which disables POSIX signals in
doctest (unsupported by FIL-C).

```bash
# Set FILC_ROOT to your FIL-C installation (adjust path as needed):
export FILC_ROOT=/usr/local/filc-0.678-linux-x86_64

cmake -S . -B build-filc \
  -DCMAKE_CXX_COMPILER=$FILC_ROOT/build/bin/fil++ \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-filc -j8
```

If binaries fail to find the FIL-C runtime libraries at execution time,
add an rpath via linker flags:
```bash
cmake -S . -B build-filc \
  -DCMAKE_CXX_COMPILER=$FILC_ROOT/build/bin/fil++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,$FILC_ROOT/pizfix/lib"
```

To override the FIL-C compatibility setting by hand:
```bash
cmake -S . -B build-filc -DWISDOM_CHESS_FILC_COMPAT=OFF  # Force disable
cmake -S . -B build-filc -DWISDOM_CHESS_FILC_COMPAT=ON   # Force enable
```

## Build options

All are defined in the top-level `CMakeLists.txt`.

| Option | Default | Description |
|--------|---------|-------------|
| `WISDOM_CHESS_QML_UI` | `AUTO` | Qt QML UI: `AUTO` builds it if Qt 6 is found, `ON` requires Qt 6, `OFF` disables it |
| `WISDOM_CHESS_QT_DIR` | empty | Directory of the Qt installation to use |
| `WISDOM_CHESS_CONSOLE_UI` | `ON` | Build the console game and the UCI engine |
| `WISDOM_CHESS_REACT_UI` | `ON` | Build the React frontend's WebAssembly engine (Emscripten builds) |
| `WISDOM_CHESS_REACT_BUILD_INTEGRATED` | `ON` for Emscripten, `OFF` otherwise | Run the Node.js build of the React frontend as part of the CMake build |
| `WISDOM_CHESS_FAST_TESTS` | `ON` | Build the fast test suite |
| `WISDOM_CHESS_SLOW_TESTS` | `OFF` | Build the slow test suite (perft, hash collisions, search) |
| `WISDOM_CHESS_BUILD_LINTER` | `ON` | Build the C++ style linter and the `lint` target |
| `WISDOM_CHESS_WERROR` | `OFF` | Treat compiler warnings as errors, as CI does |
| `WISDOM_CHESS_ASAN` | `OFF` | Build with AddressSanitizer and UndefinedBehaviorSanitizer |
| `WISDOM_CHESS_TSAN` | `OFF` | Build with ThreadSanitizer (see `scripts/build-tsan.sh`) |
| `WISDOM_CHESS_PCH_ENABLED` | `ON` | Use precompiled headers |
| `WISDOM_CHESS_BENCHMARKS` | `OFF` | Build the benchmarks (`wisdom-chess-benchmarks`, with `--search-report`) |
| `WISDOM_CHESS_TOOLS` | `OFF` | Build the research and analysis tools |
| `WISDOM_CHESS_FILC_COMPAT` | auto-detected | FIL-C runtime compatibility: disables POSIX signals in doctest |
| `WISDOM_CHESS_INSTALLER` | `OFF` | Build a Qt Installer Framework installer (`installer` target) |
| `WISDOM_CHESS_VCREDIST` | auto-detected | `vc_redist.x64.exe` to bundle in the Windows installer |

For example:
```bash
# Require the Qt GUI (fails if Qt is missing)
cmake -S . -B build -DWISDOM_CHESS_QML_UI=ON

# Disable the Qt GUI
cmake -S . -B build -DWISDOM_CHESS_QML_UI=OFF

# Build the slow tests too, and fail on any compiler warning
cmake -S . -B build -DWISDOM_CHESS_SLOW_TESTS=ON -DWISDOM_CHESS_WERROR=ON
```

## Running tests

Every test is registered with CTest, so one command runs them all. For
development, configure a Release build with the slow tests on:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DWISDOM_CHESS_SLOW_TESTS=On
cmake --build build -j8
ctest --test-dir build -j 4          # -L fast / -L slow to pick one
```

The suites, by the labels and name prefixes `ctest -N` shows:

- **Engine** (`wisdom-chess-fast-tests`, label `fast`): doctest cases for
  the board, move generation, rules, search, transposition table and
  logger. Run new engine tests in a Debug build as well, since only Debug
  builds assert that a move is played by the side to move.
- **Engine, slow** (`wisdom-chess-slow-tests`, label `slow`): perft against
  the published node counts, the hash-collision sweep and the search
  scenarios. Needs `WISDOM_CHESS_SLOW_TESTS=On`.
- **`Fatal: ...`**: runs the engine's emergency-logging paths as separate
  processes and checks that they report before aborting.
- **`UCI: ...`** and **`Console: ...`** (`cmake/CliTests.cmake`): script
  the binaries' standard input and check the output.
- **View-model** (`wisdom-chess-viewmodel-tests`): the state shared by
  the GUI frontends.
- **`QML: ...`** (`src/wisdom-chess/ui/qml/test`): Qt Test suites, some
  of which load the real QML off screen. Any QML warning fails them.
  When run by hand, set `QT_QPA_PLATFORM=offscreen` and
  `QT_QUICK_BACKEND=software` (plus `QT_QUICK_CONTROLS_STYLE=Fusion` on
  macOS) as `ctest` does. `cmake --build build --target all_qmllint` runs
  Qt's `qmllint` over the QML module, as CI does.
- **React** (`src/wisdom-chess/ui/react`): `npm test -- --run` runs the
  vitest suites; `npm run check:wasm-types` checks that the TypeScript
  types generated from `src/wisdom-chess/ui/wasm/wisdom-chess.idl` are
  current, and `npm run generate:wasm-types` regenerates them.

The style linter is built with the project and checked in CI:

```bash
./build/scripts/linter/wisdom-linter <cppfile>
cmake --build build --target lint
```

### Sanitizers

CI runs AddressSanitizer and UndefinedBehaviorSanitizer with Clang over
the QML UI and both test suites. To reproduce locally:

```bash
CC=clang-18 CXX=clang++-18 cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWISDOM_CHESS_QML_UI=On -DWISDOM_CHESS_SLOW_TESTS=On -DWISDOM_CHESS_ASAN=On
cmake --build build-asan -j $(nproc)
ASAN_OPTIONS=detect_leaks=1:strict_string_checks=1:check_initialization_order=1 \
UBSAN_OPTIONS=print_stacktrace=1 ctest --test-dir build-asan -j 4 --output-on-failure
```

ThreadSanitizer needs a Qt built with it, which `scripts/build-tsan.sh`
takes care of; its header explains the options.

### Measuring playing strength

`scripts/run-engine-match.sh base=main new=HEAD` builds the UCI engine at
two commits and plays them against each other with
[fastchess](https://github.com/Disservin/fastchess), reporting an Elo
difference. Its header documents the options.
