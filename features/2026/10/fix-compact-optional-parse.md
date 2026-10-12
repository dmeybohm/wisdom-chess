# Reject moves that stay on one square

## Motivation

`optional<Move>` uses the packed a8-to-a8 move as its empty representation.
`parseMove()` accepted a8-to-a8 text and returned that value, which makes a
Debug build abort when the compact optional checks its constructor argument.
Move text can come from the console or a saved game file.

## Design

Reject a move whose source and destination are equal after parsing both
coordinates. Such a move is invalid regardless of its suffix, and this keeps
the compact optional's sentinel out of the parser result.

## Implementation Progress

### Session #1

- Added the same-square check to `parseMove()`.
- Added regression cases for a8-to-a8 and another same-square move.
- The `toMove` case passes in Release and Debug (17 assertions each); the
  Release C++ style lint passes.
