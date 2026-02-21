# Add `using enum` to high-impact switch statements

## Motivation

Several switch statements use verbose qualified enum names (e.g.
`GameStatus::FiftyMovesWithoutProgressReached`) that hurt readability.
C++20's `using enum` declaration removes the prefix inside the switch
scope, making case labels shorter and easier to scan.

We only applied this to high-impact cases — switches with long enum
type prefixes or many cases where the prefix is noise.

## Changes

### 1. `src/wisdom-chess/engine/game_status.cpp` — `GameStatus` (10 cases)

Added `using enum GameStatus;` inside the `update()` switch. Removes
`GameStatus::` from all 10 case labels.

### 2. `src/wisdom-chess/engine/game.cpp` — `DrawStatus` (2 switches, 3 cases each)

Added `using enum DrawStatus;` to both switches in `status()` that
check threefold repetition and fifty-move rule draw statuses.

### 3. `src/wisdom-chess/ui/wasm/web_types.hpp` — multiple mapping functions

These functions map between scoped `wisdom::` enums and unscoped `Web*`
enums. Since `using enum` brings the scoped enum values into local
scope, they shadow the identically-named unscoped `Web*` values. Return
values that would be shadowed use `::wisdom::` qualification to refer
to the unscoped web enum values.

- **`mapColor(wisdom::Color)`**: `using enum Color;` — 3 case labels
- **`mapPiece(wisdom::Piece)`**: `using enum Piece;` — 7 case labels
- **`mapPlayer(wisdom::Player)`**: `using enum Player;` — 2 case labels
- **`mapGameStatus(GameStatus)`**: `using enum GameStatus;` — 10 case labels
- **`mapDrawByRepetitionType(ProposedDrawType)`**: `using enum ProposedDrawType;` — 2 case labels

The reverse-direction functions (e.g. `mapGameStatus(int)`) switch on
unscoped `Web*` enums that already don't have prefixes, so those were
left unchanged.

### 4. `src/wisdom-chess/ui/qml/main/ui_types.hpp` — mapping functions

These map between `wisdom::` and `ui::` scoped enum types. Added
`using enum` for the switched-on type in each:

- **`mapColor(wisdom::Color)`**: `using enum wisdom::Color;`
- **`mapColor(ui::Color)`**: `using enum ui::Color;`
- **`mapPlayer(wisdom::Player)`**: `using enum wisdom::Player;`
- **`mapPlayer(ui::Player)`**: `using enum ui::Player;`
- **`mapPiece(ui::PieceType)`**: `using enum PieceType;`
- **`mapPiece(wisdom::Piece)`**: `using enum wisdom::Piece;`

## Implementation Progress

### Session #1

All changes applied and verified:
- Build succeeds with no warnings related to these changes
- All 85 fast tests pass
- Linter passes on modified files

### Session #2

Rebased onto main after the MVVM refactoring (PR #196) merged. The
rebase was clean with no conflicts.

Evaluated new files introduced by main for `using enum` candidates:

- **`game_viewmodel_base.cpp`** — Added `using enum GameStatus;` to the
  `onGameEnded()` switch (7 cases) and `using enum ProposedDrawType;` to
  the `onDrawProposed()` switch (2 cases). These switches had long
  qualified enum names that benefited from the same treatment.

Files evaluated but not changed:
- `play.cpp`, `game_model.cpp`, `web_game.cpp` — use if/else chains, not
  switches, so `using enum` would pollute function scope without the
  clear scoping benefit.
- NVI default implementations in `game_status.cpp` — single-line bodies
  with one enum reference each; `using enum` would be noise.

Verified: linter passes, build succeeds, all 85 fast tests pass.
