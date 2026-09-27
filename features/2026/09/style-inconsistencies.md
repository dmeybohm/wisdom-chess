# Style inconsistencies

## Motivation

The 2026-09-27 code review
([improvement-suggestions.md](improvement-suggestions.md), on its own
branch) noted that style drifts in ways the linter does not catch. This
document lists the inconsistencies found by searching the whole source
tree, so they can be fixed here a category at a time.

The linter enforces five formatting rules and the pointer rules.
`AGENTS.md` documents those and nothing about naming, so for everything
below the convention is taken to be what most of the code does. Counts
are from commit `131bb67` and exclude the linter's test fixtures in
`scripts/linter/tests`.

## Findings

### A. One obvious direction

These have a clear majority form or a documented rule, and fixing them
cannot change behaviour.

1. **Trailing whitespace.** 440 lines in 53 C++ files; the most are
   `engine/move.cpp` (33), `engine/test/castling_eligibility_test.cpp`
   (28), `ui/qml/main/game_settings.hpp` (24) and `engine/piece.hpp`
   (24). Outside C++: 8 lines in CMake files and 5 in QML. TypeScript,
   the shell scripts and the workflow files have none.
2. **Two blank lines in a row.** Eight places: `engine/castling.hpp:160`,
   `engine/generate.cpp:586`, `engine/move.hpp:23`,
   `engine/test/coord_test.cpp:19`, `engine/test/search_test.cpp:549`,
   `ui/qml/test/dialogs_test.cpp:464`, `ui/wasm/web_logger.cpp:36`,
   `ui/wasm/web_types.hpp:391`.
3. **No newline at the end of the file.**
   `engine/test/castling_eligibility_test.cpp`, and four hand-written
   files in `ui/react/src`: `AboutModal.tsx`, `Board.tsx`,
   `lib/Pieces.ts`, `lib/WisdomChess.ts`.
4. **`.editorconfig` contradicts the linter.**
   `src/wisdom-chess/engine/.editorconfig` sets `indent_style = tab`,
   and the `no-tabs` rule forbids tabs. It also covers only the engine
   directory.
5. **Comments that repeat the declaration.** Thirteen, such as
   `// Copy constructor` and `// Destructor`: `engine/game.cpp` (8),
   `engine/search.cpp` (2), `engine/game.hpp:74`,
   `engine/game_impl.hpp:17`, `engine/search.hpp:76`.
6. **camelCase variables and parameters.** Variables are `snake_case`
   everywhere else.
   - `engine/generate.cpp:276` (`enPassantTarget`),
     `engine/board_code.hpp:195`, `:224` (`metadataBits`).
   - `ui/viewmodel/game_viewmodel_base.hpp:47`, `:51` and `.cpp:91-133`,
     `:285-289`.
   - `ui/qml/main/chess_engine.cpp:39`, `:131`, `:141`, `:200`, `:207`
     and `.hpp:55`, `:65`, `:74`.
   - `ui/qml/main/chess_game.cpp:72-119` and `.hpp:89-92`.
   - `ui/qml/main/game_model.cpp:67`, `:315-318`, `:738` and
     `.hpp:300-303`. The same file spells the same four parameters
     `src_row` in `movePiece()` and `srcRow` in
     `movePieceWithPromotion()`.
   - `ui/qml/main/pieces_model.cpp:83`.

   Not included: the fields of the settings structs
   (`ui/viewmodel/game_settings.hpp:24-29`, `ui/wasm/game_settings.hpp`,
   `ui/wasm/web_game.hpp:18`). Their names appear in the WebIDL file and
   the QML settings dialog, so they belong under B.
7. **Scalars declared without a value at the top of a function**, then
   assigned further down: `engine/generate.cpp:129`, `:309-312`,
   `:400-401`, `engine/move.cpp:336`, `engine/fen_parser.cpp:61`,
   `:260`, `:267`, `engine/board.cpp:53`. Elsewhere a variable is
   declared where it gets its value.
8. **`this->` on member access.** Twenty uses in five files, and no
   other file has any: `engine/position.cpp` (12),
   `ui/qml/main/chess_game.cpp` (5), `engine/generate.cpp:420-421`,
   `engine/material.cpp:12`, `ui/qml/main/chess_engine.cpp:71`.
9. **`std::` on a type `global.hpp` aliases.** In the engine, outside
   its tests: `std::optional` 2 against 55 bare, `std::string` 5 against
   51, `std::vector` 4 against 16, `std::pair` 2 against 1, `std::span`
   1 against 1.
10. **Reference bound to the name.** `const Board &board` at
    `engine/generate.cpp:125`, `engine/test/move_perft_test.cpp:37`,
    `engine/test/fen_parser_test.cpp:176`. Everywhere else it is
    `const Board& board`.
11. **Constructor initializer list with a trailing colon.**
    `engine/global.hpp:243`, `ui/qml/main/chess_engine.cpp:39`,
    `ui/wasm/web_types.hpp:261`. Everywhere else the colon starts the
    next line.
12. **Two statements joined by a comma.** `engine/board.cpp:147`.
13. **A lower-case constant.** `bytes_per_mb` at
    `engine/transposition_table.cpp:17`; constants are
    `Snake_Title_Case`.

### B. Needs a decision

Both forms are common, or the change reaches an interface, so the
direction is a choice rather than a correction.

14. **Layout of a trailing return type.** 441 definitions put the name
    and `-> Type` on their own lines; 109 are on one line. The one-line
    form is concentrated in `engine/game.cpp` (18), `engine/history.cpp`
    (6) and the benchmarks (13). `AGENTS.md` shows the one-line form,
    and the linter accepts both.
15. **Opening brace after a multi-line parameter list.** 29 functions
    put it on the line of the closing parenthesis (`) {`), most in
    `ui/qml/main/game_model.cpp` (8) and `engine/move.cpp` (5); 7 put it
    on its own line.
16. **Space after a lambda's capture list.** `[] (` 48 times, `[](` 12.
17. **Names of lambdas.** About 20 are camelCase, like functions
    (`hasRookAt`, `boardFromFen`), and about 16 are `snake_case`, like
    variables (`shuffle_back_to_start`, `periodic_func`). Most of both
    are in tests.
18. **`std::chrono::` or `chrono::`.** `global.hpp` aliases the
    namespace. The engine uses the long form 24 times and the short form
    18, and `engine/game.cpp`, `game.hpp` and `search.cpp` use both.
19. **`std::stringstream` for output only.** Seven uses, against one
    `std::ostringstream`, which is what all eight need.
20. **Private members without `my_`.** `FenParser` (`builder`,
    `active_player`, `engine/fen_parser.hpp:32-33`), `IterativeSearch`
    (`impl`, `engine/search.hpp:74`, where `Game` has `my_pimpl`), and
    `ConsoleGame` (`quit`, `paused`, `show_final_position`,
    `ui/console/play.cpp:161-163`). The reverse also occurs: `Move` and
    `InlineThreats` are structs whose public fields carry `my_`.
21. **`ParseMoveException`** (`engine/move.hpp:34`) is the one exception
    class not named `...Error`; there are seven of those. Renaming it
    reaches every frontend that catches it.
22. **`Error::extra_info()`** (`engine/global.hpp:256`) is the one
    `snake_case` member function.
23. **Getters with and without `get`.** `Game::getBoard()`,
    `Board::getKingPosition()` against `IterativeSearch::moveTimer()`,
    `ColoredPiece::color()`, `Coord::row()`. Names taken from the
    standard containers (`size()`, `begin()`) are a separate case and
    should stay.
24. **Constants and enumerators in other forms.** `Combined_NormalCapture`
    and the five beside it (`engine/move.hpp:56-61`) join words without
    an underscore. The unscoped `enum MaterialWeight`
    (`engine/global.hpp:62`) has enumerators like `WeightKing`. Other
    unscoped enums: `LogLevel` (`engine/logger.hpp:10`), `MetadataBits`
    (`engine/board_code.hpp:70`), `PlayStatus`
    (`ui/wasm/bindings.cpp:21`). `Roles` in `pieces_model.hpp:54` is the
    usual Qt form and should stay.
25. **File-local functions.** Thirteen source files use `static`; five
    use an unnamed namespace (`engine/global.cpp`, `engine/logger.cpp`,
    `ui/uci/uci_interface.cpp`, `ui/qml/main/pieces_model.cpp`,
    `qml_singletons.cpp`), as does the linter.
26. **`using namespace` at file scope.** Seven frontend files, including
    `using namespace std;` in `ui/qml/main/pieces_model.cpp:9`. The
    engine has none.
27. **Order of includes.** 33 source files include the standard headers
    first and 17 the project's first. The UCI frontend and the
    benchmarks include their own headers by bare name
    (`"uci_interface.hpp"`, `"bench_positions.hpp"`); everything else
    uses the full `wisdom-chess/...` path.
28. **Settings field names.** See the exception under item 6. Renaming
    them changes the WebIDL file and the generated TypeScript types.
29. **The React sources have no formatter or linter.** `package.json`
    has neither ESLint nor Prettier. Of the import lines, 44 end in a
    semicolon and 62 do not; ten files have double-quoted imports and
    seventeen single-quoted; six components are default exports and five
    are named.
    `App.tsx` has one semicolon in 348 lines, which is on its one
    double-quoted import.

### C. Out of scope here

- The linter's own sources (`scripts/linter`). The `linter-lexer` branch
  is rewriting them, and any edit here would conflict.
- `ui/react/src/lib/wisdom-chess-module.d.ts`, which is generated.
- Trailing whitespace in Markdown, where two trailing spaces are a line
  break.

## Plan

1. Record the findings (this document).
2. Fix the items under A, one commit per item, in the order listed.
   Build and run the fast tests and the lint target after each commit
   that touches code rather than whitespace.
3. For item 1, consider a `no-trailing-whitespace` linter rule so it
   stays fixed. It would be added after `linter-lexer` lands, on its own
   branch.
4. Items under B wait for a decision on each. Once decided, the
   convention goes into `AGENTS.md` next to the fix.
5. At the end: full Release build, all tests, a Debug build of the fast
   tests, and the QML tests if Qt is configured in the build tree.

Risk: the whitespace commits touch many files, and these branches are
not merged yet: `qml-pause`, `add-difficulty-levels`,
`mvvm-shared-viewmodel`, `linter-lexer`. They will need a rebase. A
merge with `-Xignore-space-at-eol` avoids most of the conflicts.

## Implementation Progress

### Session #1

- Wrote this document, then fixed every item under A, one commit each.
  Nothing under B was touched.
- Where the fix differs from the finding:
  - **Item 1.** The first counts for the files outside C++ were wrong
    and are corrected above. They came from a `grep` whose `[ \t]` also
    matched a line ending in the letter `t`.
  - **Item 2.** Nine files, not eight: removing the trailing whitespace
    left a second blank line in `engine/search.cpp`.
  - **Item 4.** The file moved to the repository root. It sets four
    spaces for C++, QML, TypeScript and `CMakeLists.txt`, and leaves the
    indent of other file types alone, since the `.cmake`, CSS and IDL
    files mix two and four.
  - **Item 5.** Sixteen comments removed, not thirteen. The three extra
    are `// Game::Impl constructors` and the two "... and assignment"
    comments in `game.hpp`.
  - **Item 6.** A search by identifier found more than the list above:
    `engine/random.hpp` (`oldState`), the `PieceInfo::pieceImage` field,
    and parameters in `ui/wasm/bindings.cpp`, `ui/wasm/web_game.hpp` and
    `ui/wasm/game_model.hpp`. `chess_game.hpp:83` also misspelled one as
    `whitePLayer`. The QML role is still named `"pieceImage"`, and
    `WebGame::moveNumber` is left alone because the WebIDL file names
    it.
  - **Item 7.** `engine/fen_parser.cpp:260` and `:267` are read from the
    stream on the next line, which is declaring at the point of use, so
    they stay. `engine/move.cpp:336` also stays: both variables are set
    by an `if` chain that throws or returns in its last branch.
  - **Item 9.** `bench_main.cpp` keeps `std::`, because its `main()` is
    outside the namespace.
  - **Item 13.** The constant is now `Bytes_Per_Megabyte`.
- Verified on Linux with GCC and Qt 6.11.2:
  - Release, with the QML UI, benchmarks and tools on: no warnings, all
    269 tests pass (235 fast, 34 slow).
  - Debug, without QML: the 227 fast tests pass.
  - The `lint` and `all_qmllint` targets pass.
  - The WASM target `wisdom-chess-web` builds with Emscripten, which is
    the only build that compiles `bindings.cpp` and
    `ui/wasm/game_model.hpp`.
  - Not run: the React tests. The only change to TypeScript is a newline
    at the end of four files.

### Session #2

Three of the items under B were decided and done.

- **Item 14, layout of a trailing return type.** Neither the linter, the
  documents nor the `.clang-format` removed in November 2025 has a rule
  for it, and line length does not explain it: at 100 columns, 503 of
  the 585 split declarations would fit on one line. The number of
  specifiers does, as a tendency:

  | Specifiers | One line | Split | Share on one line |
  |---|---|---|---|
  | 0 | 47 | 97 | 33% |
  | 1 | 42 | 142 | 23% |
  | 2 | 28 | 224 | 11% |
  | 3 | 4 | 85 | 4% |
  | 4 | 0 | 37 | 0% |

  `AGENTS.md` now says to split at three or more and to follow the file
  otherwise. The four one-line declarations with three were split:
  `Game::getBoard()` twice, `Error::message()` and
  `Error::extra_info()`. These counts use a wider search than the 441
  and 109 under item 14, which missed `consteval` and `explicit`.
- **Item 20, the `my_` prefix.** Private members have it and public ones
  do not, which `AGENTS.md` now states.
  - Added: `FenParser::my_builder` and `my_active_player`,
    `IterativeSearch::my_pimpl` (the name `Game` uses), and
    `ConsoleGame::my_quit`, `my_paused` and `my_show_final_position`.
  - Removed: `Move::data`, the five fields of `InlineThreats`, and the
    seven public fields of `Game::Impl`, which the list under item 20
    had missed.
- **Item 21.** `ParseMoveException` is now `ParseMoveError`. Only the
  engine and its tests named it; no frontend catches it by name.
- Verified: Release build with the QML UI on has no warnings, the 235
  fast tests pass, and the `lint` target passes.
- Still open under B: items 15 to 19 and 22 to 28.

### Session #3

Item 29: the React frontend is formatted with Prettier.

- Chosen over ESLint and Biome for its dependencies: `prettier` 3.9.9
  has none, `@biomejs/biome` has none beyond its own prebuilt binary,
  and `eslint` has 30 before `typescript-eslint`. The inconsistencies
  found were all formatting, which is all Prettier does. The version is
  pinned exactly, because a formatter's output can change between
  releases.
- The options follow what most of the code already did: no semicolons
  (926 statement lines without, 74 with), single quotes (427 strings
  against 61), four spaces, and no parentheses around a single arrow
  parameter (21 against 4). The width is 100 columns, the limit the C++
  sources had under `.clang-format`. Of twelve combinations tried, 120
  columns gave the smallest change, 672 lines against 712 at 100; 80
  columns gave 991.
- `npm run format` and `npm run format:check` cover `src`, `scripts` and
  the two configuration files. `.prettierignore` excludes the generated
  `wisdom-chess-module.d.ts`.
- Three commits: the tool, the TypeScript (20 files), and the style
  sheets (8 files). The style sheets mixed two and four spaces, so most
  of their change is indentation: ignoring whitespace it is 17 lines
  added and 26 removed.
- The `web` workflow runs `npm run format:check` before the tests.
  `AGENTS.md` has a short section on it.
- Verified: `tsc --noEmit` is clean, the 51 React tests pass,
  `check:wasm-types` still passes with the generator script
  reformatted, and `format:check` passes. The production build was not
  run, since it needs the WASM artifacts.
