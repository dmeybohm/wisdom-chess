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

### 4. Rank Scan with SWAR Occupancy Bitmask

**Before:** Per-square loop scanning right then left from the king's column,
calling `checkSlidingThreats<Piece::Rook>` at each square until a piece or
board edge is found. Up to 7 iterations with branching per square.

**After:** Load the king's rank as a `uint64_t` via `memcpy`, build an 8-bit
occupancy mask using Mycroft's "has zero byte" SWAR technique, then use
`std::countr_zero`/`std::bit_width` to find the nearest piece in each direction.
Only 2 targeted piece checks instead of up to 7 loop iterations.

**Algorithm:**
1. `memcpy` 8 contiguous `ColoredPiece` bytes into a `uint64_t`
2. Detect zero bytes: `(v - 0x0101..01) & ~v & 0x8080..80` → high bit set for
   each empty square. XOR with `0x8080..80` inverts to get occupied squares.
3. Pack high bits into a single byte via multiply-shift
4. Clear king's own bit, split into right/left masks
5. `countr_zero` (right) / `bit_width - 1` (left) find nearest pieces
6. Check only those 1-2 squares for opponent rook/queen

**Rationale:** Replaces O(n) scanning with O(1) bit manipulation. The SWAR
technique avoids cross-byte contamination that naive bit-shift folding would
cause. `countr_zero`/`bit_width` compile to `tzcnt`/`lzcnt` on x86 and are
efficient on ARM and WASM targets.

**New Board accessors added:** `pieceAtIndex(int)` for index-based access
without Coord construction, `squareData()` for raw pointer to the square array.

### 5. Column Scan with Gathered Occupancy Bitmask

**Before:** Per-square loop scanning up and down from the king's row, same
branching pattern as the original rank scan.

**After:** Gather 8 stride-8 column bytes into a packed `uint64_t` via an
unrolled loop (8 loads + 7 shift-ORs), then reuse the same
`buildOccupancyMask` + `scanLane` logic from the rank scan.

**Refactoring:** `buildOccupancyMask` now takes a `uint64_t` directly (not a
pointer), and a shared `scanLane` template takes the occupancy mask, king
position along the lane, and a check callback. Both `row()` and `column()`
are now thin wrappers that load/gather data and call `scanLane`.

**Rationale:** Although the column gather (8 stride-8 loads) is more expensive
than the rank load (single `memcpy`), the back-end savings are the same:
replacing up to 7 branching iterations with 2 targeted piece checks. The
worst case (open file) benefits most; the common case avoids branch
mispredictions on the scan loop.

### 6. Diagonal Scan with Gathered Occupancy Bitmask

**Before:** Four `checkDiagonalThreat<horiz, vert>()` template instantiations,
each a per-square loop with `if constexpr` bounds checking and
`checkSlidingThreats<Piece::Bishop>` calls. ~45 lines of template code.

**After:** Two `gatherDiagonal` + `scanLane` calls — one for the main diagonal
(stride 9), one for the anti-diagonal (stride 7). Variable-length diagonals
(1-8 squares) are handled naturally since unused high bytes in the packed
`uint64_t` are zero, producing zero bits in the occupancy mask that are never
selected as nearest pieces. Removed the now-unused `ThreatStatus` enum and
`checkSlidingThreats` template.

**Algorithm:** Compute the diagonal's start index and length from the king's
position, gather bytes with the appropriate stride, build occupancy mask via
`buildOccupancyMask`, scan with `scanLane`, and check only the 1-2 nearest
pieces for opponent bishop/queen.

**Rationale:** Same win as the column optimization — replaces up to 7 branching
iterations per ray (4 rays total) with 2 gather loops + 2 scanLane calls (at
most 4 targeted piece checks total). The gather loop has no branching per
iteration.

### 7. Branchless Knight Check

**Before:** Early-exit loop with `if (valid && match) return true` branching
per offset. 8 conditional branches in the loop body.

**After:** Branchless accumulation: unsigned comparison for bounds check
(`(unsigned)r < 8u`), conditional index (`valid ? index : 0` → `cmov`),
AND-masked match, OR-accumulated result. No branches in the loop body.

**Rationale:** The no-threat case (most common) already checks all 8 squares.
Removing branches eliminates misprediction cost on the hot path and enables
compiler auto-vectorization of the uniform loop body.

## Testing

- All 85 fast test cases pass (2666 assertions)
- All 19 slow test cases pass (86 assertions, ~12.5M positions via perft)
- Linter passes clean on `threats.hpp` and `board.hpp`

## Implementation Progress

### Session #1: Initial refactoring (reorder, simplify, unify)

Reordered `checkAll()` for short-circuit efficiency, simplified knight check
from template approach to offset array, unified lane/diagonal helpers into a
single template.

### Session #2: SWAR bitmask rank scan and branchless knight

Replaced per-square rank loop with SWAR occupancy bitmask + `countr_zero`/
`bit_width` nearest-piece lookup. Made knight check branchless with unsigned
bounds and OR-accumulated results. Initial OR-fold approach for occupancy mask
had cross-byte contamination bug; fixed by switching to Mycroft's "has zero
byte" SWAR technique which operates on byte boundaries correctly.

### Session #3: Column and diagonal scan with gathered occupancy bitmask

Extended the SWAR bitmask approach to column scanning by gathering 8 stride-8
bytes into a packed `uint64_t`. Refactored `buildOccupancyMask` to accept a
`uint64_t` directly and extracted a shared `scanLane` template so both `row()`
and `column()` use the same scan logic with different load strategies.

Then extended to diagonal scanning: gather bytes along each full diagonal
(stride 9 for main, stride 7 for anti-diagonal) into a packed `uint64_t`,
reusing the same `buildOccupancyMask` + `scanLane` infrastructure. Diagonals
have variable length (1-8 squares), which is handled by zero-padding the
unused high bytes. Removed the 4 `checkDiagonalThreat` template instantiations
and the now-dead `ThreatStatus` enum and `checkSlidingThreats` helper.
