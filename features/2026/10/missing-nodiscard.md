# Add the missing `[[nodiscard]]`s

## Motivation

`AGENTS.md` asks for `[[nodiscard]]` on factory functions and getters. The
headers mostly follow it: of the 417 functions declared in a header that
return a value, operators aside, 384 have the attribute. This branch adds
it to the ones the rule covers that were missed.

The branch is based on `remove-gsl`, which rewrites `ptr.hpp` and
`narrow.hpp`, so that the two do not conflict.

## How they were found

A script read every declaration of the form `auto name (...) -> Type` in
`src`, tests left out, and listed those with a return type other than
`void` and no `[[nodiscard]]` before them. A grep for declarations with a
leading return type found only three `Q_INVOKABLE`s. A function whose
return type is deduced, with no `->`, would not be seen.

## Scope

Factories:

- `BoardBuilder::fromDefaultPosition()`, `initDefaultBoardBuilder()`
- `CastlingEligibility::fromInt()`
- `FenParser::build()`, `buildBoard()`
- `Game::load()`
- `History::fromInitialBoard()`
- `makeNullLogger()`, `makeStandardLogger()`, `makeBufferedLogger()`
- `ColoredPiece::make()`
- `ChessGame::fromPlayers()`, `fromFen()`, `fromEngine()` (QML)
- `GameModel::startNewGame()` (WASM)

Getters, predicates and conversions:

- `CastlingEligibility::operator bool()`
- `isValidRow()`, `isValidColumn()`
- `FenParser::parsePiece()`, `parseActivePlayer()`
- `asString (const MoveList&)`
- `MoveTimer::isTriggered()`
- `checkKingThreatRow()` in `threats.hpp`
- `UciInterface::tokenizeCommand()`, `parseUciMove()`, `moveToUci()`
- `pieceAt()` in `ui/wasm/web_types.hpp`

Beyond the letter of the rule, for consistency with their neighbours:

- The comparison operators that lacked it: `operator==` on `Board`, and
  `operator==` and `operator!=` on `Move`. Seventeen of the 28 comparison
  operators already had it.
- `narrow()` and `narrow_cast()`. The other functions in `narrow.hpp` have
  it, and no caller discards the result.

Out of scope:

- `operator==` and `operator!=` on `GameSettings` and `UISettings`. They
  are friend declarations defined in the source file, and an attribute is
  not allowed on a friend declaration that is not a definition. They
  would need a second declaration at namespace scope.
- Assignment, compound assignment, `++` and `<<`, which return the object
  for chaining.
- Functions called for their effect that also return something:
  `Board::applyForEnPassant()`, `transitionGameStatus()`,
  `ChessEngine::gameStatusTransition()`.
- `UciInterface::parsePosition()` and `applyMoves()`, which return a
  success `bool`. Not getters, though ignoring the result would be a bug.
- `unchecked_nonnull`'s `operator->` and `operator*`.
- The three `Q_INVOKABLE`s of `GameModel` that return a value. Only QML
  calls them, where the attribute does nothing.
- Functions local to a source file. The rule is not applied there: 159
  lack the attribute and 45 have it.

## Implementation Progress

### Session #1

Added the attribute to the 32 declarations under Scope, in 18 headers. No
caller needed changing.

Verification: a Release build with the slow tests compiles without a
warning, the linter passes, and all 278 tests pass.
The QML frontend (Qt 6.11) and the WASM frontend
(`wisdom-chess-react`) also compile without a warning; their tests were
not run.
