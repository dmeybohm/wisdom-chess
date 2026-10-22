# Engine surface for the frontends

Branch: `engine-surface`, based on `main` after PR #384.

## Motivation

The frontends were meant to see the engine through `Game`,
`GameStatusUpdate` and the value types, and `Game` and `IterativeSearch`
hide their state behind a PIMPL for that reason. But the frontends also
reached past `Game` in a handful of places, and `game.hpp` itself pulled
`Board` in for everyone through `history.hpp`:

- `getBoard()` for `pieceAt()`, `toFenString()` and `getKingPosition()`.
- `generateLegalMoves()` and `isLegalMove()` from `generate.hpp`, which
  brings in `move_list.hpp` and `move_ordering.hpp`. The console and UCI
  frontends passed `getCurrentTurn()` to the first, which is exactly the
  "must be the side to move" contract the Debug build asserts.
- `isKingThreatened()` from `threats.hpp`, 287 lines of inline attack
  code, compiled into the view model for the "in check" display.
- `getHistory().getMoveHistory().size()` for the move number.
- `evaluate.hpp` in five files and `fen_parser.hpp` in three, with nothing
  from them used: the `Checkmate` they named is `GameStatus::Checkmate`.

None of this is wrong on its own. The cost is that the contract between
the engine and three frontends is whatever the frontends happen to
compile against, so an engine change that touches `Board`, `generate` or
`threats` has to be checked against all of them, and nothing stops the
surface from growing again.

A full façade, which would hide `Coord`, `Move` and `Piece` too, was
considered and rejected. Those are the vocabulary of chess rather than
engine internals; a frontend has to name squares and pieces, so hiding
them means a parallel set of types and a conversion layer. The WASM
frontend already pays that at the IDL boundary (`web_types.hpp`), and
there is no plugin, ABI or external consumer to justify paying it
elsewhere.

## Design

`Game` is the whole surface. Everything a frontend asked `Board`,
`generate.hpp` or `threats.hpp` for is now a query on `Game`, each a
one-line forward through the PIMPL, so `game.hpp` gains no includes for
them:

- `pieceAt (Coord)` and `pieceAt (int row, int column)`.
- `isInCheck (Color who)`.
- `isLegalMove (Move)` and `getLegalMoves()`, which takes the side to
  move from the game, so a caller cannot pass the wrong one.
- `getMoveHistory()`, the `vector<Move>` the history keeps.
- `toFenString()` and `boardAsString()`.

`getBoard()` and `getHistory()` stay, for the search and the tests, but
`game.hpp` now forward-declares `History` instead of including
`history.hpp`, since a reference return needs only the declaration. That
cuts the chain `game.hpp` → `history.hpp` → `board.hpp` → `board_code`,
`coord_set`, `material`, `position` for every frontend. `move_list.hpp`
is included instead, for `getLegalMoves()`; it brings only `move.hpp`.

Cutting that chain showed that `DrawStatus`, `DrawLimits` and the
`Claimable_Draw_Limits` and `Automatic_Draw_Limits` constants lived in
`history.hpp` although they are `Game`'s API: `setProposedDrawStatus()`
takes the first and `setExternalDrawArbiter()` the second, and the UCI
frontend names the constant. They are in `game_status.hpp` now, next to
`GameStatus` and `ProposedDrawType`, and `history.hpp` includes it. The
`BothPlayersDrawStatus` pair and the helpers over it stay in
`history.hpp`, where they are used.

The mutable `getHistory() & -> History&` is gone. No caller used it, and
a getter that hands out write access to the game's state is the one
shape the pointer rules do not cover well: the project prefers
`nonnull<T>` to a mutable `T&` for parameters and members because those
outlive the expression that bound them, but a getter's return is bound
by the caller for one expression or a `const&` local, which is the case
the rule leaves to references. So the rule for `Game` is: getters hand
out `const Type&`, and what the engine must write to, such as the
transposition table, comes in as a `nonnull` parameter. Returning
`nonnull<const History>` was considered and rejected: it would add `->`
at every call site to guard against a null that cannot occur, and no
other getter in the engine returns one.

The linter's new `engine-include` rule pins the boundary. In a file
under a `ui` directory and outside its `test` directory, an
`#include "wisdom-chess/engine/..."` must name one of: `global.hpp`,
`game.hpp`, `game_status.hpp`, `game_file.hpp`, `coord.hpp`,
`move.hpp`, `move_list.hpp`, `piece.hpp`, `move_timer.hpp`,
`transposition_table.hpp`. The rule reads the lexer's preprocessor
tokens and goes by the path, so its fixtures sit in `ui/` and
`ui/test/` subdirectories of `tests/engine-include/`, and
`run-tests.sh` now finds fixtures in subdirectories. The frontends'
tests are exempt because they compare the frontend's view with the
engine's state directly, as `web_game_test.cpp` does with the board.

## Implementation Progress

### Session #1

- `Game` gained the queries above and lost the mutable `getHistory()`.
  `game.cpp` includes `threats.hpp` for `isInCheck()`.
- Every frontend call site moved to the queries; the dead
  `evaluate.hpp` and `fen_parser.hpp` includes and the frontends'
  `board.hpp`, `generate.hpp`, `threats.hpp` and `history.hpp` includes
  are gone. `pieces_model.cpp` had copied the whole `Board` into a local
  before reading it; it now asks the game square by square.
- `web_game_test.cpp` includes `board.hpp` itself, since `game.hpp` no
  longer brings it.
- `DrawStatus`, `DrawLimits` and the two limit constants moved to
  `game_status.hpp`.
- `evaluate_test.cpp` had set a declined draw on the history directly
  through the mutable getter. It now answers the proposal through
  `setProposedDrawStatus()` with both players declining, which the game
  records in the history the same way.
- The `engine-include` rule, its fixtures, and the `coding-style.md`
  and AGENTS.md entries.
- The `web-computer-move-illegal` fatal test matches the quoted
  `EXPECTS` condition, which is now `my_game.isLegalMove (*move)`.
- Verified with the Release build, the full `ctest` suite, the `lint`
  target, the linter's self-tests, and an Emscripten build of
  `wisdom-chess-web` for `bindings.cpp`.
