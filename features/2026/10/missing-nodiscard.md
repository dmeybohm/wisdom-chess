# Add the missing `[[nodiscard]]`s

## Motivation

`AGENTS.md` asks for `[[nodiscard]]` on factory functions and getters. The
headers mostly follow it: of the 417 functions declared in a header that
return a value, operators aside, 384 have the attribute. This branch adds
it to the ones the rule covers that were missed.

The branch was started on `remove-gsl`, which rewrites `ptr.hpp` and
`narrow.hpp`, so that the two would not conflict, and was moved onto
`main` once that had merged.

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
- `std::hash<BoardCode>::operator()`. The other function call operator,
  `CompileTimeRandom`'s, has it.

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
- `nonnull`'s `operator->` and `operator*`.
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

### Session #2

Moved the branch onto `main` and scanned again, this time with the
operators included. `main` had added no declaration to a header since.
The scan found one that the first had skipped as an operator,
`std::hash<BoardCode>::operator()`, and it now has the attribute. Of the
466 declarations in a header that return a value, operators included, 428
have it. Of the 38 without, 36 are under Out of scope and two are the
definitions of `CastlingEligibility::canCastleKingside()` and
`canCastleQueenside()`, which carry it on their declarations in the class.

Verification: a Release build with the slow tests and the QML frontend
compiles without a warning, the linter passes, and all 282 tests pass,
the `QML: ...` ones among them. The WASM frontend was not rebuilt.

### Session #3

CI failed to compile on macOS, on Windows and under the sanitizers: one
caller did discard a result. `chess_game_test.cpp` calls
`ChessGame::fromFen()` inside `QVERIFY_THROWS_EXCEPTION` to see it throw,
and that macro expands its argument as a statement. Clang and MSVC report
the discard. GCC does not, because the statement comes from a macro in a
system header, which is why the local builds and the Linux job passed.
The call is now cast to `void`, as `global_test.cpp` does for `narrow()`.

Verification: a Clang 18 build with the QML frontend compiles without a
warning and all 282 tests pass. Build with Clang as well as GCC when
adding `[[nodiscard]]`.
