# Shifted Zobrist table

## Motivation

`Hash_Code_Table` held 48-bit random values, and `boardCodeHash()`
shifted each one left by `Total_Metadata_Bits` on every lookup, so the
table did not hold the keys that went into the board code. The seed
optimizer built its own table the same way and repeated the shift.

The shift costs nothing worth measuring: it is one instruction behind a
load whose index is only known at runtime. The change is for clarity.
The table now holds the keys themselves, and the rule that the piece
hash sits above the metadata bits is written once, in `zobristKey()`.
The keys are the same as before, so the tuned seed still applies.

## Implementation Progress

### Session #1

- Added `zobristKey()`, which shifts a 48-bit random value above the
  metadata bits. `initializeBoardCodes()` and the seed optimizer's
  `generateZobristTable()` build their tables with it, and
  `boardCodeHash()` and `computeHashWithTable()` no longer shift.
- Moved `Total_Metadata_Bits` and the masks above the table, which now
  uses them.
- The bit distribution test checks bits 16 to 63, where the keys now
  live, and a board code test checks that no key touches the metadata
  bits.

### Session #2

- `Num_Piece_Types` is an `int`, like `Num_Players` and `Num_Squares`.
  As a `size_t` it made `zobristPieceIndex()` compute in unsigned and
  convert back to `int` without a check. `Zobrist_Table_Size` stays a
  `size_t` and converts through `to_unsigned`.
- Not changed: `Num_Piece_Types` counts `Piece::None`, so two of the
  table's fourteen piece slots, 128 of its 896 entries, are never used.
  Sizing the table for six piece types would change which random value
  each key gets, so the seed would need checking again.
