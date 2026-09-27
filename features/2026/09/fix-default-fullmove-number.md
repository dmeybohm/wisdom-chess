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
3. Read a fullmove number of 0 in a FEN as 1, and reject 0 in
   `BoardBuilder::setFullMoves()`.
4. Find the tests that depend on the old default and fix them. A board
   built by hand in a test now starts at 1 unless it calls
   `setFullMoves()`.
5. Run the Release `ctest` with slow tests, the new test in Debug, and
   the `lint` target.

## Reading 0

The specification doesn't allow a fullmove number of 0, but some
programs write it. `FenParser::parseFullMove()` reads it as 1 instead
of rejecting the FEN, so those files still load and are written back
correctly.

`BoardBuilder::setFullMoves()` rejects 0 with `BoardBuilderError`, as
it does any other number out of range. Only our own code calls the
builder, where 0 is a mistake. The parser remaps before it calls the
builder, so no `Board` can hold 0.

## Draw detection

The fifty and seventy-five move rules are not affected. They read the
halfmove clock, as does the repetition probe. The fullmove number is
only incremented after Black's move and written by `toFenString()`.
`Board::operator==` and the board code ignore both clocks.

## Implementation Progress

### Session #1

Created `fix-default-fullmove-number` in its own worktree from `main`
at `65e56e5`. Found while testing `en-passant-normalization-cost`. No
code or tests have been changed yet.

### Session #2

Implemented all five plan steps.

"FEN full move number starts at 1" and the subcase "A full move number
of 0 is read as 1" in `fen_parser_test.cpp` failed before the fix with
`0 == 1`, and pass after it. No existing test depended on the old
default, and no FEN literal in the source tree ends in a fullmove of 0.

Release `ctest` with slow tests passed 244 of 244. The Debug build
passed the 210 fast tests, and the `lint` target is clean. The QML
tests were not built, because this machine has no Qt. Their FEN
literals all give a fullmove number of 1 or more.

### Session #3

`BoardBuilder::setFullMoves()` now rejects 0. "Board builder rejects
move clocks that are out of range" checks that, and its lower limit for
the fullmove number is now 1.

Release `ctest` with slow tests passed 244 of 244. The Debug build
passed the 210 fast tests, and the `lint` target is clean.

Merged `main` after PR #287 landed there. Nothing needed changing.
Release `ctest` with slow tests passed 245 of 245, the Debug build
passed the 211 fast tests, and the `lint` target is clean.
