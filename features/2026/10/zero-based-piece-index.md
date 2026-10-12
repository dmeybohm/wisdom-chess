# Zero-based piece indexing

Follows `optional-color.md`, and is stacked on that branch.

## Motivation

With `Piece::None` gone, `Pawn` is 1, but `Num_Piece_Types` still
counted the slot `None` had: `to_int (Piece::King) + 1`, seven. Every
array sized by it carried a dead slot:

- The Zobrist table had fourteen rows of 64 keys, two per color, and
  `zobristPieceIndex()` subtracted one from `pieceIndex()` but still
  multiplied by seven, so rows 6 and 13 were never read: 128 of 896
  keys.
- `Material::my_piece_count` indexed by `to_int (type)`, so its slot 0
  was always zero.

`shifted-zobrist-table.md` noted the two rows and left them, expecting
that resizing would change each key's random value and so need the seed
checked again.

## Design

- `pieceIndex()` returns 0-5, and `Num_Piece_Types` is
  `to_int (Piece::King) - to_int (Piece::Pawn) + 1`, six.
  `zobristPieceIndex()` drops its `- 1`, and `Material` indexes through
  `pieceIndex()`.
- `pieceToChar()` loses its `default: return '?'`, which no `Piece`
  reaches now, so `-Wswitch` covers it like the other switches over
  `Piece`.

### The keys do not change

`initializeBoardCodes()` draws the keys from the generator in loop
order, White then Black, Pawn to King, then each square, and stores
each where `zobristPieceIndex()` says. The number of draws and their
order are the same, so every piece and square keeps its key, and only
where the key is stored moves. A black pawn on a8 moves from slot 448
to slot 384 with the same value. A scratch program hashed five FEN
positions and nine positions reached through moves, including
captures and castling, against both builds: identical.

### The seed optimizer scored the wrong keys

`tools/seed_optimizer.cpp` builds its trial tables differently: it
fills every slot in order, the dead rows included, then looks keys up
with the same `zobristPieceIndex()`. For White, rows 0-5, the two
agreed. For Black they did not: the engine's black keys were draws
384-767, while the optimizer put draws 384-447 in dead row 6 and gave
Black draws 448-831. The layout was the same in January, when the
current seed was chosen with this tool (`f6a1e7e2`), so the seed was
picked on a table that differed from the engine's for every black
piece.

With twelve rows and no gaps, filling in order is the engine's draw
order, so the two agree without changing the optimizer. A board code
test pins that: filling `Zobrist_Table_Size` keys in order from a fresh
generator reproduces `Hash_Code_Table`.

### Whether the seed should change

The optimizer, built in Release on both the fourteen-row table and this
branch's, scored the current seed and 1000 random seeds each. The score
is the worst ratio of the fullest bucket to the average, over table
sizes 2^17 to 2^21:

| Table | Current seed | Best of 1000 | Worst of 1000 |
|---|---|---|---|
| Fourteen rows, as the optimizer built it | 36.38 | 36.38 | 48.79 |
| Twelve rows, the engine's keys | 38.45 | 36.38 | 53.75 |

At least ten seeds in each run tie at 36.38, a floor: at 2^21 the
fullest bucket holds 88 positions against an average of 2.42. The
optimizer collects every position of a depth-5 perft without removing
duplicates, 5,072,213 of them for about 900,000 distinct ones, so the
fullest bucket is mostly one position reached many ways, and no seed
can spread its copies. On the engine's keys the current seed's fullest
bucket holds 93, five positions above the floor.

The seed stays. The score mostly counts duplicates, which the
transposition table does not mind, since a repeated position finds its
own entry; collisions between distinct positions are what matter, and
the slow hash collision test checks those and passes. And
`tt-index-metadata.md` found that the seed moves search node counts by
up to 19% chaotically, so a new seed would mostly change which
positions the search happens to visit. For the optimizer to choose
seeds usefully, it would need to drop duplicate positions first, which
is a separate change.

## Implementation Progress

### Session #1

- `pieceIndex()` zero-based, `Num_Piece_Types` six, `Material` and
  `zobristPieceIndex()` updated, `pieceToChar()` exhaustive.
- The board code test above.
- Debug build with QML, tools, benchmarks and slow tests: every
  non-slow test passes, and `lint` is clean. Hashes match the
  fourteen-row build.
- The seed check above. The current seed stays.
