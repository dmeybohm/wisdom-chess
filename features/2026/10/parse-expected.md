# Parsers return their errors

Step 1 of `exception-removal.md`: text from outside the program no longer
reaches a throw or a contract check. Once step 3 makes contract checks
abort, nothing a user types can end the process.

## Design

- `engine/expected.hpp` holds `wisdom::expected` and `wisdom::unexpected`,
  aliases for `tl::expected` (v1.3.1, header only, through CPM), and
  `ParseError`, which holds a message for the user. Moving to C++23's
  `std::expected` later changes the aliases only.
- Each parser has two forms. One returns `optional` or `expected` and is
  for outside text: `Game::tryCreateGameFromFen()`, `FenParser::parse()`,
  `moveParseOptional()`, `coordParseOptional()`. The other is for strings
  the program wrote, such as literals in tests and `MoveList`'s
  initializer list, and treats a bad one as a precondition failure:
  `createGameFromFen()`, the `FenParser` constructor, `moveParse()`,
  `coordParse()`, `pieceFromChar()`.
- The FEN parser's steps return `expected<void, ParseError>` instead of
  throwing, so no exception is involved even inside it.
- Two FEN errors used to escape as `BoardBuilderError` because the parser
  left them to the builder: a ninth piece in a rank, and a side with no
  king, which surfaced when the board asked the builder for the king
  positions. The console caught only `FenParserError`, so either one
  ended it with "Uncaught error: Missing king position in constructing
  board." The parser checks both now, and `BoardBuilder` gains
  `hasKingPositions()` for it.
- `BoardBuilder`'s own checks become `EXPECTS`: outside text reaches the
  builder only through the parser.
- `moveParseOptional()` returns empty for castling without a color,
  where it threw before.
- `parseUciMove()` uses `coordParseOptional()` and drops its
  `catch (...)`; `mapCoordinatesToMove()` is `noexcept`.
- `CoordParseError`, `ParseMoveError`, `FenParserError`,
  `BoardBuilderError` and the unused `PieceError` are gone.
- Left as they are: `ChessGame::fromFen()`, which only rebuilds a game
  from a FEN the engine wrote, and `game_file.cpp`'s I/O errors, since
  only the console saves games and native builds keep exceptions.

## Implementation Progress

### Session #1

- Converted the parsers and their callers in UCI and the console, and
  the tests: rejections now check the returned error or message, and
  the strict forms check for `PreconditionError` until step 3.
- New tests: the two FEN errors above, both forms of a FEN that does not
  parse, `Console: a FEN string without a king` (the binary from before
  the change aborts on it) and `UCI: a FEN without a king is reported`.
- Verified all 298 Release tests including the slow ones, the 284 Debug
  tests, the fast tests with Clang 18 and `-Werror`, the benchmarks'
  build and `lint`. Not built locally: the QML frontend, whose code is
  unchanged, MSVC and Emscripten, left to CI.
