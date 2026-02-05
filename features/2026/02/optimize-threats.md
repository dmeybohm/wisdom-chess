# Optimize InlineThreats King Threat Detection

## Overview

Refactor `InlineThreats` in `threats.hpp` to improve performance and reduce
code duplication. This is the single most-called function in the engine during
search, invoked for every pseudo-legal move during legal move generation.

## Changes

### 1. Reorder `checkAll()` Priority

**Before:** `row() || column() || diagonal() || knight() || pawn() || king()`
**After:** `pawn() || knight() || row() || column() || diagonal() || king()`

**Rationale:** Since `checkAll()` short-circuits, the order matters for
performance. Pawn and knight checks are O(1) (checking 2 and 8 fixed squares
respectively), while lane/diagonal scans are O(n) in the worst case. Pawn and
knight attacks are also statistically the most common sources of check in chess.
By checking them first, we avoid expensive lane scans when a cheap check
suffices.

### 2. Simplify `knight()` with Offset Array

**Before:** Complex template-based approach using `checkKnight<row_dir, col_dir>()`
with `if constexpr` branching for row-major vs column-major cases, plus a
separate `checkKnightAtSquare()` helper. ~70 lines of template code.

**After:** Simple iteration over a static constexpr array of 8 knight offsets
with bounds checking and a single `ColoredPiece` comparison per square. ~25
lines.

**Rationale:** The template approach saved a few comparisons but at significant
code complexity cost. The offset-array approach is clearer, and modern compilers
will unroll the loop. Using a pre-constructed `ColoredPiece` for comparison
(same approach as `pawn()`) avoids separate `pieceColor`/`pieceType` calls per
square.

### 3. Unify Lane/Diagonal Threat Helpers

**Before:** Two nearly identical functions: `checkLaneThreats()` (checks for
rook/queen) and `checkDiagonalThreats()` (checks for bishop/queen).

**After:** Single `checkSlidingThreats<Piece sliding_piece>()` template,
instantiated as `checkSlidingThreats<Piece::Rook>` for lanes and
`checkSlidingThreats<Piece::Bishop>` for diagonals.

**Rationale:** The two functions differed only in which piece type was the
non-queen threat (rook vs bishop). Parameterizing on the piece type eliminates
the duplication while maintaining the same generated code through template
specialization.

## Testing

- All 85 fast test cases pass (2666 assertions)
- Linter passes clean on `threats.hpp`
