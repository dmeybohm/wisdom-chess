<h1 align="center">Wisdom Chess</h1>

<p align="center">
  <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/wisdom-chess-animate.gif" />
</p>

----

Wisdom Chess is a multiplatform chess engine written in C++20 with multiple front-ends:
- **Web**: React frontend with WebAssembly chess engine
- **Web (QML)**: Qt QML application compiled to WebAssembly
- **Desktop**: Qt QML application for Windows, macOS, and Linux
- **Mobile**: Qt QML application for Android
- **Console**: Command-line interface

🌐 **[Play online at wisdom-chess.netlify.app](https://wisdom-chess.netlify.app)**

## Features

- Full chess engine with move validation and game rules
- Configurable search depth and time limits for engine strength
- Clean, modern user interfaces across all platforms
- WebAssembly for high-performance web chess
- Cross-platform compatibility

## Building

### Prerequisites

- **C++ Compiler**: GCC, Clang, or MSVC with C++20 support
- **CMake**: Version 3.20 or higher
- **Optional**: Qt 6.8+ for desktop/mobile GUI
- **Optional**: Emscripten SDK for web version
- **Optional**: Node.js for React frontend development

### Quick Start (Console Version)

The simplest way to try Wisdom Chess:

```bash
git clone https://github.com/dmeybohm/wisdom-chess.git
cd wisdom-chess
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j8
./src/wisdom-chess/ui/console/wisdom-chess-console
```

### Web Version (React + WebAssembly)

1. **Install Emscripten SDK**:
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Build WebAssembly + React (Integrated)**:
   ```bash
   mkdir build-web && cd build-web
   emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build . --target wisdom-chess-react
   ```

   This automatically:
   - Builds the WebAssembly chess engine
   - Runs `npm install` for React dependencies
   - Builds the production React frontend
   - Output is in `src/wisdom-chess/ui/react/dist/`

3. **Development server**:
   ```bash
   cmake --build . --target wisdom-chess-react-dev
   ```
   Starts development server at `http://localhost:5173`

### Desktop Version (Qt QML)

1. **Install Qt 6.8 or newer** from [qt.io](https://www.qt.io/)

2. **Build with Qt**:
   ```bash
   mkdir build-desktop && cd build-desktop
   cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DCMAKE_BUILD_TYPE=Release
   cmake --build . --target WisdomChessQml
   ./src/wisdom-chess/ui/qml/WisdomChessQml
   ```

### Installers (Qt Installer Framework)

The desktop app can be packaged into a self-contained installer with the
[Qt Installer Framework](https://doc.qt.io/qtinstallerframework/) (QtIFW).
The installer bundles the Qt runtime, so users do not need Qt installed.

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

The installers are **not code-signed**, so operating systems warn before
running them:

- **Windows**: SmartScreen shows "Windows protected your PC". Click
  *More info*, then *Run anyway*. The installer asks for administrator
  rights to install into `Program Files\Wisdom Chess`, adds Start Menu
  and desktop shortcuts, and installs the MSVC runtime if needed.
- **macOS**: Gatekeeper refuses to open the installer app on the mounted
  disk image. Right-click it and choose *Open*, or allow it under
  *System Settings > Privacy & Security > Open Anyway*. Alternatively
  clear the quarantine flag: `xattr -dr com.apple.quarantine <installer.app>`.
  The app is installed to `/Applications/Wisdom Chess/`.
- **Linux**: make the file executable (`chmod +x wisdom-chess-*.run`) and
  run it. It asks for your password to install into `/opt/WisdomChess`
  and adds a *Wisdom Chess* entry to the application menu. Use
  `./wisdom-chess-*.run --root ~/WisdomChess` to install into your home
  directory instead. The bundled Qt needs the usual X11/xcb libraries
  from your distribution; on Debian/Ubuntu that is
  `libxcb-cursor0 libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0
  libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libgl1 libegl1
  libfontconfig1`. The binary links against the build machine's C and C++
  runtimes, so it needs a distribution at least as new as the one it was
  built on (glibc 2.34+ for the bundled Qt; the `libstdc++` of the GCC
  used for the build).

Uninstall with the *WisdomChessMaintenanceTool* placed next to the app.

### Web Version (Qt QML + WebAssembly)

The Qt QML interface can also be compiled to WebAssembly:

```bash
# Setup Emscripten (see web version instructions above)
source ./emsdk_env.sh

mkdir build-qml-wasm && cd build-qml-wasm
emcmake cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/wasm_multithread -DCMAKE_BUILD_TYPE=Release
cmake --build . --target WisdomChessQml
# Serve the generated files with a web server
```

Note: Requires Qt for WebAssembly, which is a separate Qt installation.

### Android Version

Use Qt Creator with Android NDK configured. See [Qt Android documentation](https://doc.qt.io/qt-6/android-getting-started.html) for setup details.

<p align="center">
    <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/wisdom-chess-android.png" />
</p>

### Building with FIL-C

[FIL-C](https://github.com/pizlonator/fil-c) is auto-detected at configure time via the `__PIZLONATOR_WAS_HERE__` preprocessor macro. When detected, `WISDOM_CHESS_FILC_COMPAT` is automatically enabled, which disables POSIX signals in doctest (unsupported by FIL-C).

```bash
# Set FILC_ROOT to your FIL-C installation (adjust path as needed):
export FILC_ROOT=/usr/local/filc-0.678-linux-x86_64

mkdir build-filc && cd build-filc
cmake .. \
  -DCMAKE_CXX_COMPILER=$FILC_ROOT/build/bin/fil++ \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . -j8
```

If binaries fail to find FIL-C runtime libraries at execution time, add an rpath via linker flags:
```bash
cmake .. \
  -DCMAKE_CXX_COMPILER=$FILC_ROOT/build/bin/fil++ \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,$FILC_ROOT/pizfix/lib"
```

To manually override the FIL-C compatibility setting:
```bash
cmake .. -DWISDOM_CHESS_FILC_COMPAT=OFF  # Force disable
cmake .. -DWISDOM_CHESS_FILC_COMPAT=ON   # Force enable
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `WISDOM_CHESS_QML_UI` | AUTO | Qt QML UI: AUTO/ON/OFF |
| `WISDOM_CHESS_REACT_BUILD_INTEGRATED` | ON (web), OFF (others) | Integrate Node.js build |
| `WISDOM_CHESS_FAST_TESTS` | ON | Build fast test suite |
| `WISDOM_CHESS_SLOW_TESTS` | OFF | Build comprehensive test suite |
| `WISDOM_CHESS_INSTALLER` | OFF | Build a Qt Installer Framework installer (`installer` target) |

### Examples:
```bash
# Force Qt GUI even if not found (fails if Qt missing)
cmake .. -DWISDOM_CHESS_QML_UI=ON

# Disable Qt GUI completely
cmake .. -DWISDOM_CHESS_QML_UI=OFF

# Enable slow tests for thorough validation
cmake .. -DWISDOM_CHESS_SLOW_TESTS=ON
```

## Running Tests

```bash
# Fast tests (runs in seconds)
./src/wisdom-chess/engine/test/wisdom-chess-fast-tests

# Slow tests (comprehensive, takes longer)
cmake .. -DWISDOM_CHESS_SLOW_TESTS=ON
cmake --build .
./src/wisdom-chess/engine/test/wisdom-chess-slow-tests
```

## Screenshots

<p align="center">
    <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/windows-wisdom-chess.png" />
</p>

## Contributing

See `CLAUDE.md` for development guidelines including code style, build instructions, and architecture notes.

## License

Copyright © Dave Meybohm

The chess engine and applications are released under the MIT License.

### Third-Party Assets

- Chess piece images: Copyright Colin M.L. Burnett, used under [Creative Commons BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)
- UI icons: From [Boxicons](https://boxicons.com/), used under [Creative Commons 4.0](https://creativecommons.org/licenses/by/4.0/)
