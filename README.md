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
- **Optional**: Qt 6.x for desktop/mobile GUI
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

1. **Install Qt 6.x** from [qt.io](https://www.qt.io/)

2. **Build with Qt**:
   ```bash
   mkdir build-desktop && cd build-desktop
   cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DCMAKE_BUILD_TYPE=Release
   cmake --build . --target WisdomChessQml
   ./src/wisdom-chess/ui/qml/WisdomChessQml
   ```

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

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `WISDOM_CHESS_QML_UI` | AUTO | Qt QML UI: AUTO/ON/OFF |
| `WISDOM_CHESS_REACT_BUILD_INTEGRATED` | ON (web), OFF (others) | Integrate Node.js build |
| `WISDOM_CHESS_FAST_TESTS` | ON | Build fast test suite |
| `WISDOM_CHESS_SLOW_TESTS` | OFF | Build comprehensive test suite |

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
./src/wisdom-chess/engine/test/fast_tests

# Slow tests (comprehensive, takes longer)
cmake .. -DWISDOM_CHESS_SLOW_TESTS=ON
cmake --build .
./src/wisdom-chess/engine/test/slow_tests
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