# Default fullmove number

## Motivation

FEN's fullmove number starts at 1 and goes up after each Black move. A
default `Board` starts at 0, so every FEN written from a standard game
is one too low. After 1.a4 the board writes

    rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 0

where the last field should be 1.

`BoardBuilder::my_full_moves` defaults to 0 (`board_builder.hpp:304`),
and `Board`'s constructor copies it. `Board::my_full_move_clock` has a
default of 1, but that constructor always overwrites it.

The number is only read by `Board::toFenString()`. That reaches users in
two places:

- `FenOutputFormat::save()` writes it to the file the console's `save`
  command produces.
- The QML `ChessGame::clone()` passes the game to the engine thread as
  FEN. The number round-trips there, so nothing breaks, but the copy
  carries the same wrong number.

Source: [PGN/FEN section 16.1.3.6](https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm#16.1.3.6).

## Plan

1. Add a failing test first: a default `Board` writes fullmove 1, and
   the number is 2 after 1.e4 e5.
2. Change the `BoardBuilder::my_full_moves` default to 1.
3. Find the tests that depend on the old default and fix them. A board
   built by hand in a test now starts at 1 unless it calls
   `setFullMoves()`.
4. Run the Release `ctest` with slow tests, the new test in Debug, and
   the `lint` target.

## Open question

`FenParser::parseFullMove()` and `BoardBuilder::setFullMoves()` both
accept 0, and `board_builder_test.cpp:94` pins that. The specification
doesn't allow 0, but some programs write it. The plan leaves reading 0
alone and only fixes what we write. Rejecting it, or reading it as 1,
would be a separate change.

## Implementation Progress

### Session #1

Created `fix-default-fullmove-number` in its own worktree from `main`
at `65e56e5`. Found while testing `en-passant-normalization-cost`. No
code or tests have been changed yet.
