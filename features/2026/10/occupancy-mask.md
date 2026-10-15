# Occupancy mask

## Motivation

Move generation finds the side's pieces by testing all 64 squares, and
so does `hasLegalMove()`. The `piece-list` branch
(`features/2026/10/piece-list.md` there) replaced that
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
- `sizeof (Board)` is 128 bytes, from 112 on `main`.
- `--search-report 8` against `main` at `5656ed41` and the piece list's
  step 3 (`e1da3b59`), seven rounds in rotating order pinned to one core,
  each run started under a one-minute load of 1.5; desktop programs kept
  it near 1.9. Medians; every run gave the same counts as `main`.

| Build | Total | starting | kiwipete | italian | position3 | position4 | middlegame |
|---|---|---|---|---|---|---|---|
| `main` | 62.99s | 1.545 | 9.647 | 13.144 | 0.254 | 3.654 | 34.781 |
| piece list, step 3 | 62.31s (−1.1%) | 1.423 | 9.930 | 12.851 | 0.230 | 3.673 | 34.175 |
| occupancy mask | 56.34s (−10.6%) | 1.343 | 8.993 | 11.741 | 0.218 | 3.325 | 30.632 |

  The mask was faster than `main` in all seven paired rounds, by 5.74 to
  7.12 s, and in every position, kiwipete included.
- Session #4 of the `piece-list` branch's log found that reading the list
  instead of the squares saved 8.2%, and that the list's upkeep cost
  about 4% and its other changes about 3%. The mask keeps the saving and
  its upkeep is nearly free, so the scan was worth about 11%, close to
  the ceiling's 12.6%.
- All 337 tests pass in Release, the slow ones included, and the fast
  and medium ones in Debug, where `occupiedMatchesSquares()` is checked
  after every move. The slow ones were not run in Debug.

### Session #2

- `Board::getPieces()` gives one side's pieces with their squares, as
  `for (auto [coord, piece] : board.getPieces (who))`, with a plain
  `Coord` and `ColoredPiece`. The generator's loop uses it, and so do
  both walks in `hasLegalMove()`, whose helpers `generatePieceMoves()` and
  `pieceHasLegalMove()` now take the piece instead of looking it up.
- A `PlacedPiece` that was an aggregate of the two, or a `std::pair`,
  made GCC build the two bytes in one register and read the piece back
  out of its high byte (`mov dh,al`, `movzx esi,dh`) for every piece;
  the depth-7 report was about 1.6% slower. Aligning each member to four
  bytes packed them into a 64-bit register with shifts instead.
  `PlacedPiece` is therefore a class holding the board and the square,
  tuple-like for structured bindings, whose `get<1>()` reads the piece
  from the board: the loop compiles to the same instructions as before
  but a register move. Depth-7 counts match `main`.
