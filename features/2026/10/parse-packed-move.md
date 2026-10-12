# Checked parsing of packed moves

## Motivation

`Move::fromPacked()` turns any `int` into a `Move` without a check. The
WASM frontend sends moves between the main thread and the engine
worker, and through JavaScript, as `toPacked()` integers:

- A human move: `WebGame::makeHumanMove()` checks it is legal and
  returns it packed; React passes the number to
  `GameModel::notifyHumanMove()`, which posts it to the worker, and
  `workerReceiveMove()` unpacks it and plays it with `game->move()`.
- An engine move: the worker packs the search's move and posts it to
  `mainThreadReceiveMove()`, which unpacks it to show it.

React only notifies a legal move (`App.test.tsx` pins that it skips an
illegal one), but `notifyHumanMove (long)` is public in the IDL, so any
number could reach `game->move()`, whose checks are Debug-only `ASSERT`s.
A number that is no move would corrupt the board in Release.

## Design

- `Move::isWellFormed()`: two different squares and a kind code that
  names a move, 0 to 11. It is a member because the layout is private.
  It says nothing about legality.
- `parsePackedMove (int) -> optional<Move>`: empty for a number outside
  sixteen bits or a move that is not well formed. It follows the
  `parseX()` convention for values from outside the program, while
  `Move::fromPacked()` stays for the program's own. It is a free
  function after the class, not a static member: a declaration naming
  `optional<Move>` inside the class would see `Move` incomplete and
  choose `std::optional`.
- Both bindings use it with `EXPECTS`. The numbers they receive are ones
  our own code packed, so a bad one is a bug; playing it anyway would
  leave the worker's game and the main thread's apart, so it fails
  loudly instead.
- A well-formed move can still be illegal, such as a3 to a4 from the
  starting position, where a3 is empty. `Game::move()` trusts its move,
  and `Board::makeMove()` checks the source piece only in Debug, so the
  worker also checks `isLegalMove()`, a new public function in
  `generate.hpp` that looks for the move among the legal moves of the
  player to move.
- The engine's move reaches the main thread's game too:
  `mainThreadReceiveMove()` turns it into text for JavaScript, and
  React passes that to `WebGame::makeComputerMove()`, which played it
  through `toMove()` and `Game::move()` without a legality check. The
  check belongs in `makeComputerMove()`, where the move meets the board
  it is played on, rather than in `mainThreadReceiveMove()`, which runs
  before JavaScript and has no access to that game. It is also public in
  the IDL and takes any string. It now parses the move and requires
  `isLegalMove()`, both with `EXPECTS`. `search.cpp` and the
  view model had their own copies of the same check; they call
  `isLegalMove()` now, the view model after its check that a human is to
  move.

## Implementation Progress

### Session #1

- `isWellFormed()`, `parsePackedMove()`, and the bindings.
- A test round-trips every legal move from four positions that reach
  castling, en passant, captures and promotions, and rejects numbers
  outside sixteen bits, moves to their own square and unused kinds.
- Debug with QML: 332 tests pass, and `lint` is clean. Clang builds,
  and the WASM frontend builds.

### Session #2

- Review: the worker checks the move is legal before playing it, with
  the new `isLegalMove (const Board&, Move)`, which has a test.
- Debug with QML: 333 tests pass, and `lint` is clean. Clang builds,
  and the WASM frontend builds.

### Session #3

- `WebGame::makeComputerMove()` checks the move parses and is legal.
  A new fatal test program, `wisdom-chess-web-fatal-tests`, covers an
  unparseable and an illegal computer move; the existing web test
  already plays a legal one.
