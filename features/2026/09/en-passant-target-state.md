# Deciding en passant legality when the target is set

## Motivation

[`en-passant-normalization-cost`](en-passant-normalization-cost.md)
(PR #287) leaves an unusable en passant target out of
`Board::getBoardCode()` and `Board::operator==`. It decides this lazily.
Each `getBoardCode()` call on a board with a target runs
`generateLegalEnPassantMoves()`, and search asks for the code twice per
child: once in `History::addTentativePosition()`, and once for the
transposition table key.

That log's "Rejected: caching the normalized code in a mutable field"
names the alternative: decide once, where the target is set, and record
the answer in `BoardCode`. This branch tries it.

## Design

`BoardCode` stores the target in one of two 5-bit fields, one for a
legal target and one for an illegal one. Both use the same encoding: a
3-bit column, a present bit, and a vulnerable-color bit. At most one
field is set.

| Bits | Field |
| --- | --- |
| 0 | side to move |
| 1–5 | legal en passant target |
| 6–10 | illegal en passant target |
| 11–12 | White castling |
| 13–14 | Black castling |
| 15 | spare |

- `BoardCode::setEnPassantTarget (color, coord, state)` takes an
  `EnPassantTargetState` (`Legal` or `Illegal`).
- `getAnyEnPassantTarget()` replaces `getEnPassantTarget()` and returns
  whichever field is set. FEN output and move generation read it.
- `withoutIllegalEnPassantTarget()` masks the illegal field off.
  `Board::getBoardCode()` returns that, and `Board::operator==` compares
  it, so neither generates moves any more.

`Board` classifies the target wherever the position or the side to move
changes:
- at the end of `makeMove()`, which `updateEnPassantEligibility()` now
  runs after, since legality depends on the finished position
- in the `BoardBuilder` constructor
- in `withCurrentTurn()`

Classifying still calls `generateLegalEnPassantMoves()`. It checks for
the taken pawn and an adjacent capturer before trying any move, so most
double pushes cost a few square reads.

A copy of a `Board` carries the state in its code, and a copy is the
same position. So the copy constructor stays defaulted, and there is no
`mutable` cache to race on or invalidate.

The trade-off is that classifying is now eager. It also runs for
double-push boards nobody asks a code for: moves `generateLegalMoves()`
rejects, search children that fail `isLegalPositionAfterMove()`, and
perft. The perft slow tests are where that would show.

## Plan

1. `BoardCode`: the two fields, `EnPassantTargetState`,
   `getAnyEnPassantTarget()` and `withoutIllegalEnPassantTarget()`, with
   `board_code_test.cpp` cases.
2. `Board`: classify in `makeMove()`, the builder constructor and
   `withCurrentTurn()`. Drop `normalizedBoardCode()`. Rename
   `getEnPassantTarget()` to `getAnyEnPassantTarget()`. Add
   `en_passant_test.cpp` cases, and a check in "Agrees with
   generateLegalMoves" that the stored state matches
   `generateLegalEnPassantMoves()` at every node.
3. Release `ctest` with slow tests, the Debug fast tests, and `lint`.
4. Time search at depth 8 in the three positions from the previous log,
   and the perft slow tests, against `main`. If search gains nothing
   measurable, or perft regresses, record that and stop before a PR.

## Implementation Progress
