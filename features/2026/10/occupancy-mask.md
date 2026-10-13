# Occupancy mask

## Motivation

Move generation finds the side's pieces by testing all 64 squares, and
so does `hasLegalMove()`. [piece-list.md](piece-list.md) replaced that
scan with a list of each side's pieces, grouped by type, and gained 1.3%
at depth 8 against a ceiling of 12.6%. This branch tries the fallback
named there: one `uint64_t` per side marking its pieces' squares.

## Design

- `Board::my_occupied`, one mask per side, added after the other members.
  `sizeof (Board)` goes from 112 to 128 bytes.
- `setPiece()`, through which every square change in `makeMove()` goes,
  clears the square's bit in both masks and sets it in the new piece's.
  That is three bit operations and a branch for each square written, with
  no search and no shifting, and no limit on the number of pieces.
- `CoordSet` (`engine/coord_set.hpp`) wraps a mask and visits its squares
  in index order with `std::countr_zero`. `Board::getPieceCoords()`
  returns one side's.
- `generateSortedMoves()` and both walks in `hasLegalMove()` iterate it
  instead of the squares.
- `Material` and `my_king_pos` stay, so the board keeps three records of
  the pieces besides the squares.
- `occupiedMatchesSquares()` checks the masks against the squares, with
  `ENSURES` in the constructor and `ASSERT` after every move.

## Implementation Progress

### Session #1

- Implemented the design above, with a `CoordSet` test case and a mask
  check in `board_test.cpp`'s walk that compares incremental state with
  a recompute.
