# Claude Instructions for Wisdom Chess

## Code Style Guidelines

### C++ Code Style

Follow these guidelines:

1. **Code Formatting**:
   - Do not run `clang-format` automatically - let the user handle formatting
   - When writing new code, follow the existing style patterns in the codebase
   - Refer to `.clang-format` for guidance of about the rules, but prefer to
     follow the existing style patterns (spaces after non-empty functions,
     spaces around macros in tests, extended functions declaration across
     multiple lines, etc). The style used here is a bit unique, but designed
     for good readability, and not loading up lines with too much noise.
     
     Here are some examples of formatting functions:
```cpp
    [[nodiscard]] auto 
    isCertainlyNthRepetition (const Board& board, int repetition_count) const
        -> bool
    {
        auto repetitions = std::count (my_stored_boards.begin(), my_stored_boards.end(), board);
        return repetitions >= repetition_count;
    }

    [[nodiscard]] bool isProbablyThirdRepetition (const Board& board) const;
    
    void addTentativePosition (const Board& board)
    {
        my_board_codes.emplace_back (board.getBoardCode());
        my_tentative_nesting_count++;
    }

    [[nodiscard]] auto 
    getMoveHistory() const& 
        -> const vector<Move>&
    {
        return my_move_history;
    }
    void getMoveHistory() const&& = delete;


    [[nodiscard]] constexpr auto 
    drawStatusIsReplied (DrawStatus draw_status)
        -> bool
    {
        return draw_status == DrawStatus::Accepted || draw_status == DrawStatus::Declined;
    }
    
    // Whether this move could cause a draw.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    [[nodiscard]] inline auto
    isProbablyDrawingMove (
        const Board& board,
        [[maybe_unused]] Color who,
        [[maybe_unused]] Move move,
        const History& history
    )
        -> DrawCategory
    {
        auto repetition_status = history.getThreefoldRepetitionStatus();
        auto no_progress_status = history.getFiftyMovesWithoutProgressStatus();
        ...
  }
```

     Here are some examples of formatting in tests:
```cpp
TEST_CASE( "Board code stores metadata" )
{
    SUBCASE( "setMetadataBits preserves high bits of hash code" )
    {
        BoardCode code = BoardCode::fromEmptyBoard();

        code.addPiece(
            coordParse("a1"),
            ColoredPiece::make(Color::White, Piece::King)
        );
        code.addPiece(
            coordParse("h8"),
            ColoredPiece::make(Color::Black, Piece::King)
        );

        code.setCastleState(Color::White, CastlingEligibility::Either_Side);
        code.setCastleState(Color::Black, CastlingEligibility::Either_Side);

        // ...
    }
}
```

2. **Code Style Guidelines**:
   - Use trailing return types with auto: `auto functionName() -> ReturnType`
   - Use `[[nodiscard]]` for functions that return values that shouldn't be ignored
   - Everything is in the `wisdom::` namespace
   - Use `wisdom::narrow` and `wisdom::narrow_cast` for narrowing conversions
   - No comments unless explicitly requested

### CMake Style

1. **Options**:
   - Define all options in the top-level CMakeLists.txt
   - Use `option()` for boolean values
   - Use `set(... CACHE STRING ...)` with `set_property()` for multi-value options

2. **Option Naming**:
   - Prefix all options with `WISDOM_CHESS_`
   - Use clear, descriptive names

## Build Instructions

### Prerequisites

- **C++ Compiler**: GCC, Clang, or MSVC with C++20 support
- **CMake**: Version 3.20 or higher
- **Optional**: Qt 6.x for QML UI
- **Optional**: Emscripten SDK for WebAssembly/React build
- **Optional**: Node.js for React frontend development

### Desktop Build (Console + Tests)

```bash
# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j8

# Run tests
./src/wisdom-chess/engine/test/fast_tests

# Run slow tests - only run when making more extensive changes, 
# after fast tests are passing, and only run with optimizations
# enabled (release or release with debug info) because it's slow.
cmake .. -DWISDOM_CHESS_SLOW_TESTS=On
./src/wisdom-chess/engine/test/slow_tests
```

### Qt QML UI Build

```bash
# Configure with Qt path (adjust path as needed)
cmake .. -DWISDOM_CHESS_QT_DIR=~/Qt/6.9.2/gcc_64 -DCMAKE_BUILD_TYPE=Release

# Or force QML UI to be built (fails if Qt not found)
cmake .. -DWISDOM_CHESS_QML_UI=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --target WisdomChessQml
```

### WebAssembly + React Build

```bash
# Setup Emscripten environment (adjust path as needed)
source ~/projects/3rdparty/emsdk/emsdk_env.sh

# Create build directory
mkdir build-wasm && cd build-wasm

# Configure (React build is integrated by default for WASM)
emcmake cmake .. \
  -DWISDOM_CHESS_BUILD_CONSOLE=OFF \
  -DWISDOM_CHESS_BUILD_QT_QML=OFF \
  -DWISDOM_CHESS_BUILD_ENGINE_TESTS=OFF \
  -DWISDOM_CHESS_BUILD_WASM=ON \
  -DWISDOM_CHESS_BUILD_REACT=ON \
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

### QML UI Options

- `AUTO`: Build if Qt6 is found (default)
- `ON`: Require Qt6, fail if not found
- `OFF`: Disable even if Qt6 is available

### Testing

```bash
# Run fast tests (default)
build/src/wisdom-chess/engine/test/fast_tests

# Run with success output
build/src/wisdom-chess/engine/test/fast_tests --success

# Run slow tests (if built)
build/src/wisdom-chess/engine/test/slow_tests
```

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
