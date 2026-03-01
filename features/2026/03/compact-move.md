# Compact 16-bit Move Representation

## Motivation

The `Move` struct is currently 4 bytes (4 `int8_t` fields), but the actual
value ranges only require 16 bits total. Shrinking the canonical
representation saves memory in move lists, transposition table entries, and
anywhere moves are stored. It also enables packing a full transposition table
entry into two `atomic<uint64_t>` fields for lock-free multithreaded search.

## Current Representation (32 bits)

```cpp
struct Move {
    int8_t src;              // 0-63 (square index)
    int8_t dst;              // 0-63 (square index)
    int8_t promoted_piece;   // (color_index << 4) | piece_type, or 0
    int8_t move_category;    // 0-3 (Default, Capture, EnPassant, Castling)
};
```

`sizeof(Move) == 4`, with `toInt()` / `fromInt()` for 32-bit packing.

## Proposed Representation (16 bits)

### Bit Layout

| Field | Bits | Range |
|-------|------|-------|
| `src` | 6 | 0-63 (square index) |
| `dst` | 6 | 0-63 (square index) |
| combined category + promotion | 4 | 0-11 (see encoding below) |
| **Total** | **16** | |

### Combined Category/Promotion Encoding (4 bits)

Since EnPassant and Castling can never co-occur with pawn promotion, the move
category and promotion type can be merged into a single 4-bit field:

| Value | Meaning |
|-------|---------|
| 0 | Default (non-capture, non-promote) |
| 1 | Normal capture |
| 2 | En passant |
| 3 | Castling |
| 4 | Promote to Queen (non-capture) |
| 5 | Promote to Rook (non-capture) |
| 6 | Promote to Knight (non-capture) |
| 7 | Promote to Bishop (non-capture) |
| 8 | Promote to Queen (capture) |
| 9 | Promote to Rook (capture) |
| 10 | Promote to Knight (capture) |
| 11 | Promote to Bishop (capture) |
| 12-15 | Unused |

### Promoted Piece Color

The promoted piece color is **not stored** in the Move. It is always the color
of the side that made the move, which is known from context during search and
move application.

This means `getPromotedPiece()` should return a `Piece` (piece type only)
instead of a `ColoredPiece`. All callers must be updated to supply the color
from context.

## Implementation Steps

### Step 1: Change `getPromotedPiece()` to return `Piece` instead of `ColoredPiece`

Update the `Move` API so promotion returns only the piece type:

- `getPromotedPiece() -> Piece` (was `-> ColoredPiece`)
- `withPromotion(Piece)` (was `withPromotion(ColoredPiece)`)
- Update all callers to provide the color from the side-to-move context

**Key callers to update** (search for `getPromotedPiece` and `withPromotion`):
- `src/wisdom-chess/engine/board.cpp` - move application
- `src/wisdom-chess/engine/generate.cpp` - move generation
- `src/wisdom-chess/engine/move.hpp` - the Move struct itself
- UI layers that display or apply moves

### Step 2: Implement the combined category/promotion encoding

Change the `promoted_piece` and `move_category` fields into a single
4-bit combined field using the encoding table above.

### Step 3: Shrink Move to 16 bits

Replace the four `int8_t` fields with a single `uint16_t`:

```cpp
struct Move {
    uint16_t data = 0;  // src(6) | dst(6) | combined(4)
};
```

Update `toInt()` / `fromInt()` to work with `uint16_t`.

### Step 4: Update all code that accesses Move fields directly

The `src`, `dst`, `promoted_piece`, and `move_category` fields become
accessor methods that extract from the packed `uint16_t`.

## Files to Modify

| File | Changes |
|------|---------|
| `src/wisdom-chess/engine/move.hpp` | New 16-bit layout, updated accessors |
| `src/wisdom-chess/engine/board.cpp` | Supply color to promotion handling |
| `src/wisdom-chess/engine/generate.cpp` | Use `Piece` for promotion, update `withPromotion` calls |
| `src/wisdom-chess/engine/evaluate.hpp` | If it inspects promoted pieces |
| `src/wisdom-chess/engine/search.cpp` | Supply color context where needed |
| `src/wisdom-chess/ui/*/` | Update UI layers for new promotion API |

## Verification

1. Run fast_tests - all existing move tests must pass
2. Run the linter on modified files
3. Play games via console UI to verify move generation and application
4. Run slow_tests in Release mode to verify no regressions
5. Run `wisdom-chess-benchmarks` (nanobench suite in
   `src/wisdom-chess/engine/bench/`) before and after the change to measure
   the impact on move generation, perft NPS, and legality checking. A smaller
   Move should improve cache utilization in move lists and may show measurable
   speedups in move generation and perft benchmarks.

## Implementation Progress

### Session #1

All three steps implemented and verified:

1. **Step 1**: Changed `getPromotedPiece()` to return `Piece` instead of
   `ColoredPiece`, and `withPromotion()` to accept `Piece`. All callers
   updated to reconstruct `ColoredPiece` from context where needed. Added
   `pieceToChar(Piece)` overload. (commit e7a6090)

2. **Step 2**: Merged `move_category` and `promoted_piece` into a single
   `combined` field using the 4-bit encoding (0-11). All existing accessors
   (`getMoveCategory()`, `isPromoting()`, `getPromotedPiece()`, etc.) decode
   from the combined field. Move shrinks from 4 to 3 bytes. (commit b544ac4)

3. **Step 3**: Replaced three `int8_t` fields with `uint16_t my_data`
   packing `src(6)|dst(6)|combined(4)`. All accessors use bit masking.
   `operator==` compares `my_data` directly. Also fixed a direct field access
   in `transposition_table.cpp`. `sizeof(Move) == 2`. (commit 38ae66b)

**Verification**: Full build clean, linter passes, all 85 fast tests pass.
