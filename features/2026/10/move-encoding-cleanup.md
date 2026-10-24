# Move encoding cleanup

Branch: `move-encoding-cleanup`, based on `main`.

## Motivation

A review of `main` for simplicity and code quality turned up three small
things, each a leftover rather than a defect:

- `move.hpp` named the four kinds of a move twice. `MoveCategory` is the
  enum the switches in `Board::makeMove()` and `Position::applyMove()`
  use, and `Combined_Default`, `Combined_Normal_Capture`,
  `Combined_En_Passant` and `Combined_Castling` were the same four values
  again as `int8_t` constants, which `Move`'s own functions used to set
  and test the four-bit kind field. `getMoveCategory()` converted from
  one to the other.
- `game.hpp` declared `PeriodicFunction` under a comment saying the alias
  avoids needing the full `MoveTimer` definition in the header, but the
  header includes `move_timer.hpp`, and `MoveTimer::PeriodicFunction` is
  the same type.
- `drawDesiresToRepetitionStatus()` in `game.cpp` reduces both players'
  replies to a draw proposal to one accepted or declined status. It is
  used for the fifty-move draw as well as the threefold repetition, so
  its name was narrower than its use.

## Design

- The four `Combined_*` kind constants are gone. `MoveCategory` is the
  one spelling of a move's kind, and the two promotion bases stay as
  constants, since a promotion is not a category: it reads as `Default`
  or `NormalCapturing`. `Move` gets a private `setCategory()` beside
  `setCombined()`, and the tests of the kind field compare against
  `to_underlying (MoveCategory::...)`. The encoding and every function's
  behaviour are unchanged.
- `wisdom::PeriodicFunction` is an alias of `MoveTimer::PeriodicFunction`.
- The helper is `drawDesiresToDrawStatus()`.

## Implementation Progress

### Session #1 (2026-10-24)

- Made the three changes above. `move_parse_test.cpp` spelled two of the
  removed constants and now uses the enum.
- Built and ran the move, game, perft and fatal tests in Release and
  Debug, and the lint target.
