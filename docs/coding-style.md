# Coding style

The C++ layout here is not what a formatter produces by default, so this
page lists it. Where a rule of the style linter (`scripts/linter`)
enforces a convention, its name is given in brackets. The React frontend
is formatted by Prettier instead (`src/wisdom-chess/ui/react/.prettierrc.json`).

## Running the linter

The linter is built with the project, and the `lint` target runs it over
`src` and the linter's own sources. CI fails on any error.

```bash
./build/scripts/linter/wisdom-linter <file-or-directory>...
./build/scripts/linter/wisdom-linter --list-rules
cmake --build build --target lint
```

A line containing `lint-allow(<rule>)` is exempt from that rule; give the
reason in the same comment, as in
`// lint-allow(raw-pointer): main's signature`.

## Layout

- Four spaces, never tabs [`no-tabs`]. `.editorconfig` sets this up for
  C++, QML, TypeScript and `CMakeLists.txt`.
- No whitespace at the end of a line [`no-trailing-whitespace`].
- Allman braces [`allman-braces`, `namespace-braces`]: the brace that
  opens a namespace, type, function or control statement goes on a line
  of its own, at the indent of the line before it. This holds after a
  multi-line parameter list too, and a function with a one-line body is
  split across lines. Exempt are an empty block (`{}`,
  `struct None {};`), a brace initializer, and a lambda, whose brace may
  stay on the line of its parameters.

  ```cpp
  if (board.isEmpty (coord))
  {
      return;
  }
  ```

- The contents of a namespace, and the `case` labels of a `switch`, are
  indented one level.
- A space before the parenthesis of a call or declaration that has
  arguments, and none when it is empty: `foo (x)` but `bar()`
  [`function-call-spacing`]. Control statements take the space too:
  `if (x)`, `for (...)`. So does a lambda's capture list:
  `[&board] (int count)` but `[this]()`.
- Test macros put spaces inside the parentheses instead:
  `CHECK( x == y )`, `REQUIRE( ... )`, and Qt Test's `QCOMPARE( a, b )`.
  So do the contract macros of `engine/error.hpp`: `EXPECTS( x > 0 )`,
  `ENSURES( ... )`, `NOEXCEPT_EXPECTS( ... )` and `ASSERT( ... )`
  [`test-macro-spacing`].
- Brace initializers have spaces inside the braces:
  `Coord coord { row, col }`.
- `&` and `*` bind to the type: `const Board& board`.
- A long parameter list puts one parameter on each line, and the closing
  parenthesis at the indent of the declaration:

  ```cpp
  auto IterativeSearch::create (
      const Board& board,
      const History& history,
      shared_ptr<Logger> logger,
      const MoveTimer& timer,
      int max_depth,
      nonnull<TranspositionTable> transposition_table
  ) -> IterativeSearch
  {
      ...
  }
  ```

- A constructor's initializer list starts the next line with the colon,
  and each further member starts its line with a comma:

  ```cpp
  Linter::Linter (const LinterConfig& config)
      : my_config { config }
      , my_rules { registerAllRules() }
  {
  }
  ```

## Functions

- Trailing return types: `auto fn() -> ReturnType`
  [`trailing-return-type`, a warning].
- With three or more specifiers around it (`[[nodiscard]]`, `constexpr`,
  `static`, `const`, `noexcept`, a ref-qualifier), the name and
  `-> ReturnType` go on lines of their own:

  ```cpp
  [[nodiscard]] static constexpr auto
  make (int row, int col)
      -> Coord
  ```

  With fewer, either layout is accepted, and both are in use: follow the
  file being edited.

## Naming

| Kind | Form | Example |
|---|---|---|
| Types, enumerators | `PascalCase` | `ColoredPiece`, `Color::White` |
| Functions | `camelCase` | `generateLegalMoves()` |
| Variables, parameters | `snake_case` | `search_depth` |
| Lambdas held in a variable | `snake_case` | `board_from_fen` |
| Private data members | `my_` and `snake_case` | `my_pimpl` |
| Public data members | `snake_case`, no prefix | `data` |
| Constants | `Snake_Title_Case` | `Max_Search_Depth` |
| Exception classes | ending in `Error` | `ParseMoveError` |

A name that JavaScript or QML also uses is spelled as that side spells
it. The fields of the settings structs are the case: `searchDepth`,
`thinkingTime` and `debugLogging` are named in
`ui/wasm/wisdom-chess.idl` and in the QML settings dialog.

Getters:

- A class of the engine names a getter `getX()`: `Game::getBoard()`,
  `Board::getKingPosition()`.
- A small value type names the part it returns: `Coord::row()`,
  `ColoredPiece::color()`, `Error::message()`. Names the standard
  containers use stay as they are: `size()`, `empty()`, `begin()`.
- A Qt class follows Qt, where the getter of a property has no prefix:
  `GameModel::inCheck()`. The view model the frontends share does the
  same.
- A class bound to JavaScript uses the names in the WebIDL file:
  `WebGame::getCurrentTurn()`.
- A predicate starts with `is`, `has` or `can`, and a conversion with
  `as` or `to`: `isCastling()`, `asString()`.

## Standard library names

`engine/global.hpp` brings the common standard types into the namespace
(`string`, `optional`, `vector`, `unique_ptr`, ...) and aliases
`chrono`. Inside `wisdom`, write them without `std::`:
`optional<Move>`, `chrono::milliseconds`. Code outside the namespace,
such as a `main()` or a test that only uses `wisdom::ui::qml`, keeps
`std::`.

## Source files

- A function that only its own source file uses goes in an unnamed
  namespace, not `static`. So does a variable beside it.
- No `using namespace std;`. `using namespace wisdom;`, or one of its
  nested namespaces, is for the tests and the frontends' sources; the
  engine's sources have none.
- Includes go in groups with a blank line between: the standard library
  and other `<...>` headers first, then the project's headers. A project
  header is named by its full path, `"wisdom-chess/engine/board.hpp"`. A
  test includes the helpers of its own directory by bare name
  (`"wisdom-chess-tests.hpp"`), as the linter does throughout.
