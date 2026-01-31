# MVVM Shared ViewModel for QML and WASM Frontends

## Summary

Introduce a shared `GameViewModelBase` class that unifies state management between
the QML (`GameModel`) and WASM (`WebGame`) frontends, eliminating duplicate code
and ensuring consistent behavior.

## Motivation

Both QML and WASM frontends duplicated similar state and logic:

**Duplicated State:**
- Game ID tracking
- In-check status
- Move status messages
- Game over status messages
- Current turn
- Draw statuses (third repetition, fifty moves)

**Duplicated Logic:**
- Pawn promotion detection
- Legal move checking
- Game state updates after moves
- Draw proposal handling

This duplication meant bug fixes and feature changes needed to be applied in
multiple places, risking inconsistencies.

## Implementation

### Architecture

The solution uses inheritance with virtual callbacks:

```
GameViewModelBase (shared state + logic)
    ├── GameModel (QML frontend)
    └── WebGame (WASM/React frontend)
```

Key design decisions:

1. **Virtual `getGame()` method**: The base class needs access to the concrete
   `Game` instance, but each frontend manages the game differently. The pure
   virtual `getGame()` method allows the base class to access the game while
   letting subclasses own it.

2. **Virtual callbacks for state changes**: When shared state changes (e.g.,
   `setInCheck()`), the base class calls a virtual callback (e.g.,
   `onInCheckChanged()`). This allows:
   - QML frontend to emit Qt signals
   - WASM frontend to potentially trigger JS callbacks

3. **ViewModelStatusUpdate observer**: A shared observer class handles game
   status events (checkmate, stalemate, draws) and updates the base class state.

### Files Created

| File | Purpose |
|------|---------|
| `src/wisdom-chess/ui/viewmodel/game_viewmodel_base.hpp` | Base class declaration |
| `src/wisdom-chess/ui/viewmodel/game_viewmodel_base.cpp` | Base class implementation |
| `src/wisdom-chess/ui/viewmodel/viewmodel_types.hpp` | Shared types (DrawByRepetitionStatus enum) |
| `src/wisdom-chess/ui/viewmodel/viewmodel_types.cpp` | Type conversions |
| `src/wisdom-chess/ui/viewmodel/viewmodel_settings.hpp` | Shared settings |
| `src/wisdom-chess/ui/viewmodel/viewmodel_settings.cpp` | Settings implementation |
| `src/wisdom-chess/ui/viewmodel/CMakeLists.txt` | Build configuration |

### Files Modified

| File | Changes |
|------|---------|
| `src/wisdom-chess/ui/qml/main/game_model.hpp` | Inherit from GameViewModelBase, remove duplicated state |
| `src/wisdom-chess/ui/qml/main/game_model.cpp` | Implement virtual callbacks to emit Qt signals |
| `src/wisdom-chess/ui/wasm/web_game.hpp` | Inherit from GameViewModelBase, remove duplicated state |
| `src/wisdom-chess/ui/wasm/web_game.cpp` | Use base class methods for state access |
| `src/wisdom-chess/ui/wasm/wisdom-chess.idl` | Update bindings to use getter method naming |

### Shared State and Methods

The base class provides:

**State (with getters and protected setters):**
- `gameId()` - Unique identifier incremented on new game
- `inCheck()` - Whether current player is in check
- `moveStatus()` - Status message for last move
- `gameOverStatus()` - Game over message (checkmate, stalemate, etc.)
- `currentTurn()` - Which player's turn it is
- `thirdRepetitionDrawStatus()` - Three-fold repetition draw status
- `fiftyMovesDrawStatus()` - Fifty moves rule draw status

**Logic:**
- `needsPawnPromotion()` - Check if a move requires pawn promotion
- `isLegalMove()` - Validate a move against current game state
- `updateDisplayedGameState()` - Update all UI state from game
- `setProposedDrawStatus()` - Handle draw proposals
- `resetStateForNewGame()` - Reset state when starting new game

### Build Integration

The viewmodel library is a static library linked by both QML and WASM frontends:

```cmake
add_library(wisdom-chess-viewmodel STATIC
    game_viewmodel_base.cpp
    viewmodel_types.cpp
    viewmodel_settings.cpp
)
```

Both frontends link against it, but the library itself has no Qt or Emscripten
dependencies, keeping it portable.

## Testing

The implementation was verified by:

1. Building and running the QML desktop application
2. Building and running the WASM/React application in a browser
3. Verifying all game features work: new game, moves, pawn promotion, check
   detection, draw proposals, and game over states
