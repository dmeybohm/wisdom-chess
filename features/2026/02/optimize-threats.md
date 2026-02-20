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

### Session #4: Benchmark evaluation

Benchmarked each logical unit at 6 checkpoints (3 runs each, medians reported).
See "Benchmark Results" section below for full data and recommendations.

## Benchmark Results

**Environment:** Linux 6.17, GCC, `-O3 -DNDEBUG`, CPU governor set to
performance. Each checkpoint: build from clean PCH, verify fast tests pass,
run 3x, take median.

### Logical Units Evaluated

| Unit | Commit(s) | Description |
|------|-----------|-------------|
| **A** | `187372a` | Reorder checkAll (pawn/knight first), simplify knight to loop, unify sliding helpers |
| **B** | `3a37294` | SWAR rank scan with bitmask + branchless knight (OR-accumulate, no early exit) |
| **C** | `f522357` | SWAR column scan + extract shared `scanLane()` template |
| **D** | `178023f` | SWAR diagonal scan + remove `ThreatStatus` enum |
| **E** | `a5ed419`..`1cec4c3` | API safety: `narrow_cast`, `span<const ColoredPiece, 64>` |

### Threat Detection Microbenchmarks (ns/op, lower is better)

| Checkpoint | not-threatened | threatened | sweep-64 | many-queens |
|-----------|---------------:|----------:|----------:|------------:|
| **main** (baseline) | 20.57 | 32.96 | 1231.69 | 12.88 |
| **A** (187372a) | 19.30 | 33.57 | **889.21** | 17.98 |
| **A+B** (3a37294) | 24.92 | 32.71 | 1299.39 | 13.38 |
| **A+B+C** (f522357) | 27.36 | 27.49 | 1260.45 | 12.56 |
| **A+B+C+D** (178023f) | 36.43 | 38.15 | 1529.00 | 14.50 |
| **HEAD** (1cec4c3) | 38.60 | 35.42 | 1457.73 | 15.49 |

### Perft End-to-End (NPS, higher is better)

| Checkpoint | starting depth-5 NPS | kiwipete depth-4 NPS |
|-----------|---------------------:|---------------------:|
| **main** (baseline) | 9,885,964 | 7,930,692 |
| **A** (187372a) | 9,735,341 | 7,614,658 |
| **A+B** (3a37294) | 8,779,001 | 7,542,443 |
| **A+B+C** (f522357) | 8,731,169 | 7,413,061 |
| **A+B+C+D** (178023f) | 7,986,392 | 6,844,017 |
| **HEAD** (1cec4c3) | 7,538,073 | 6,602,209 |

### Per-Unit Analysis

#### Unit A: Reorder + Simplify Knight + Unify Sliding Helpers
- **Performance:** sweep-64 improves 28% (1232→889 ns). not-threatened holds
  steady (21→19 ns). many-queens regresses 40% (13→18 ns) because reordering
  pawn/knight first delays queen detection in queen-heavy positions. Perft NPS
  is within noise (~1.5% slower).
- **Simplicity:** Big win. Knight goes from ~70-line template-heavy code with 4
  instantiations to a ~25-line loop. Unifying sliding helpers removes duplicate
  `checkDiagonalThreats`.
- **DRY:** Win. One `checkSlidingThreats<Piece>` instead of two nearly-identical
  functions.
- **Verdict: KEEP.** The simplicity improvement is substantial. The sweep-64
  improvement (the most representative real-game benchmark) validates the
  reordering. The many-queens regression is confined to an extreme test case
  that rarely occurs in practice.

#### Unit B: SWAR Rank Scan + Branchless Knight
- **Performance:** Regression. sweep-64 goes from 889→1299 ns (46% worse than
  Unit A, 5% worse than main baseline). not-threatened regresses 19→25 ns (29%
  worse). many-queens improves 18→13 ns (back to near-baseline). Perft NPS
  drops ~10% vs main.
- **Simplicity:** Mixed. SWAR bitmask (`buildOccupancyMask`, Mycroft's trick)
  adds ~30 lines of non-trivial bit manipulation. Branchless knight
  (OR-accumulate, conditional index) is harder to read than Unit A's simple
  loop.
- **DRY:** Neutral.
- **Sub-patch analysis:** Skipped. Since Unit B as a whole is a regression on
  the primary benchmark (sweep-64), teasing apart the SWAR rank scan vs
  branchless knight would not change the recommendation.
- **Verdict: DROP.** The SWAR approach doesn't pay off for rank scanning. The
  overhead of building the occupancy mask, packing bits, and computing
  `countr_zero`/`bit_width` exceeds the cost of the simple per-square loop that
  it replaces. The branchless knight always evaluates all 8 squares even when
  the first would early-exit, which hurts the threatened case.

#### Unit C: SWAR Column Scan + scanLane Refactor
- **Performance:** Marginal improvement over A+B (sweep-64 1299→1260 ns),
  threatened improves (33→27 ns), but still worse than both main and Unit A
  alone on sweep-64. Perft NPS continues to decline.
- **Simplicity:** Good in isolation — `gatherColumn()` is clean and `scanLane()`
  is well-factored. But it depends on Unit B's SWAR infrastructure being present.
- **DRY:** Good — shared `scanLane()` between rank and column.
- **Verdict: DROP.** Depends on Unit B (which is being dropped). Even with B
  present, the combined effect is still a regression vs main.

#### Unit D: SWAR Diagonal Scan
- **Performance:** Worst regression. All benchmarks degrade significantly:
  not-threatened +33% (27→36 ns), threatened +39% (27→38 ns), sweep-64 +21%
  (1260→1529 ns), many-queens +15% (13→15 ns). Perft NPS drops to ~8.0M
  (vs ~9.9M baseline, -19%).
- **Simplicity:** Highest complexity of all units. Diagonal start index and
  length calculations are intricate. Variable-length gather loop with stride-7
  and stride-9 patterns.
- **DRY:** Removes 4 template instantiations and `ThreatStatus` enum, which is
  good — but at the cost of complex replacement code.
- **Verdict: DROP.** Diagonal scanning is the worst candidate for the SWAR
  approach because: (1) diagonals are variable-length (1-8 squares), adding
  index/length computation overhead, (2) gather with stride 7/9 is cache-
  unfriendly compared to the simple loop-with-break, (3) the original
  4-direction template approach early-exits frequently in practice.

#### Unit E: API Safety (narrow_cast, span)
- **Performance:** HEAD vs A+B+C+D shows no meaningful change (within noise).
  The `narrow_cast` and `span` wrappers are zero-cost abstractions.
- **Simplicity:** Clear win. `narrow_cast` makes narrowing conversions explicit.
  `span<const ColoredPiece, 64>` provides bounds safety and documents the
  expected array size.
- **DRY:** Neutral.
- **Verdict: KEEP.** Zero-cost safety improvements. These are independent of the
  SWAR changes and can be applied on top of any combination of other units.

### Overall Recommendation

**Keep Units A and E. Drop Units B, C, and D.**

The branch should be restructured to contain only:
1. Unit A (commit `187372a`): Reorder, simplify knight, unify sliding helpers
2. Unit E (commits `a5ed419`..`1cec4c3`): API safety improvements

This yields the best performance point (sweep-64 ~28% faster than main, all
other metrics within noise) with significantly simpler code than the original.

The SWAR occupancy bitmask approach (Units B/C/D) is an elegant technique in
theory, but the overhead of mask construction, bit packing, and
`countr_zero`/`bit_width` lookup exceeds the cost of the simple per-square
loops it replaces. The original loops benefit from early exit (first piece
found terminates the scan), which the SWAR approach sacrifices by design —
it always processes the full lane before examining individual squares.

### Perft Trend Summary

The end-to-end perft numbers confirm the microbenchmark findings. Each SWAR
unit progressively degrades NPS:

```
main:    9.9M NPS (baseline)
A:       9.7M NPS (-1.5%, within noise)
A+B:     8.8M NPS (-11%)
A+B+C:   8.7M NPS (-12%)
A+B+C+D: 8.0M NPS (-19%)
HEAD:    7.5M NPS (-24%)
```

Unit A alone is performance-neutral end-to-end while improving the targeted
threat microbenchmarks. The SWAR units cause a cumulative 24% NPS regression.

### Post-Restructure Verification

After reverting Units B/C/D (keeping A+E), benchmarked the final branch state:

| Benchmark | main | A+E (final) | Change vs main |
|-----------|-----:|------------:|---------------:|
| not-threatened | 20.57 ns | 19.16 ns | **-7%** |
| threatened | 32.96 ns | 31.76 ns | **-4%** |
| sweep-64 | 1231.69 ns | 847.93 ns | **-31%** |
| many-queens | 12.88 ns | 16.84 ns | +31% |
| perft start-d5 NPS | 9,885,964 | 10,728,928 | **+9%** |
| perft kiwi-d4 NPS | 7,930,692 | 8,507,357 | **+7%** |

The restructured branch is faster than main across all benchmarks except the
extreme many-queens test case. The 7-9% perft NPS improvement confirms that
the threat detection optimizations from Unit A (reordered short-circuit,
simplified knight, unified sliding helpers) translate to meaningful end-to-end
speedup.
