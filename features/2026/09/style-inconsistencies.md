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

- Wrote this document. No code changed yet.
