# En passant without a color

## Motivation

`toMove()` with no color rejected any move whose category was not
`Default` or `NormalCapturing`. Castling never reached that check, since
`parseCastlingMove()` returns `nullopt` without a color, so in practice it
rejected only en passant.

The check dates from November 2020 (`64bbd91d`), when `parse_move()`
replaced `parse_simple_move()` and parsed a colorless move as white. It
kept the old function's contract of plain moves and captures only. Even
then en passant did not use the color; castling used it for the row and
promotion for the promoted piece's color.

Today `parseMove()` passes the color only to `parseCastlingMove()`:
promotion stores the piece type alone and `Move::makeEnPassant()` takes
two coordinates. So `toMove ("e5 d6 ep")` builds the same move as with a
color, and the check rejected a move that had parsed correctly.

## Implementation Progress

### Session #1

- Removed the category check from `toMove()` and updated its comment:
  only a castling move needs a color.
- Replaced the `move-parse-en-passant-without-color` fatal case with a
  doctest that en passant and promotion parse the same without a color,
  and castling does not parse. The `move-parse-castling-without-color`
  fatal case stays.
- Added a note to `AGENTS.md`: run the tests that cover a change, and
  the full suite only for a large functional change, leaving it to CI.
- Ran the move parsing doctest cases and the `move-parse` fatal cases in
  a Debug build.
