# Review fixes

## Motivation

A whole-project review on 2026-09-25 scored the code 7/10 and listed the
defects and smells that keep it there. This branch works through that
list. Quiescence search is the one big item left out: the
`quiescence-search` branch already adds it, and this branch must not
touch the parts of `search.cpp` that branch rewrites.

Two other open branches overlap with the review's frontend findings and
are treated as done here rather than redone:

- `react-view-cleanups` reworks `App.tsx` state, `StatusBar.tsx`,
  `Square.tsx`, `TopMenu.tsx` and `PawnPromotionDialog.tsx`, which covers
  the unused imports, the double drag ref, the `"false"` class name, the
  regex-parsed bold markup, and the engine values read during render.
- `qml-type-annotations` (worktree `qml-cleanups`) folds the three QML
  roots into shared `MainWindow`/`GameRoot`, drops `PromotedPieceModel`,
  and rewrites `SettingsDialog`, `Piece` and `GameMenu`, which covers the
  duplicated root windows, the reversed `ListModel`, the binding
  break-and-rebind, and the timer that applies settings.

## Findings

Every item was verified by reading the code on `main` at `185c0d5`.

### Engine

1. **Metadata mask drops four hash bits.** `board_code.hpp:265` masks with
   `0xfffffffFFFF0000ULL`, which is 15 hex digits, so bits 60-63 are
   cleared on every `setMetadataBits()`. Every construction path and
   every move ends in that call, so the result is deterministic and
   nothing breaks, but the hash is 44 bits wide, not the 48 the comment
   at line 269 claims. `board_code_test.cpp:356,365` copy the same
   mask, so the test cannot see it.
2. **Piece-square table transcription slips.** `position.cpp` follows
   Michniewski's simplified evaluation tables divided by five, except:
   pawn row 5 has `+2` at column 5 where the mirror of column 2 is `-2`;
   bishop row 3 has `0` at column 1 where column 6 has `+1`; the king's
   first three rows have `-4, -4` at columns 5-6 against `-8, -8` at
   columns 1-2, and row 4 ends in `-2` against `-4` at column 0. The
   queen's asymmetries are in the source table and stay.
3. **`<cctype>` on plain `char`.** `toupper`/`tolower`/`isspace` are
   called with `char` arguments at `move.cpp:345,371,376,382,472`,
   `board.cpp:139,178,180` and `uci_interface.cpp:534`. A byte at or
   above `0x80` in user input is a negative `int`, which is undefined
   behaviour for these functions.
4. **`Game::setCurrentTurn()` desynchronises history.** `game.cpp:282`
   replaces the board but `History` keeps the code stored by
   `fromInitialBoard()`, so the start position's repetition count is off
   by one afterwards.
5. **`colorIndex (Color::None)` indexes at -1 in Release.**
   `piece.hpp:117` asserts only; `Board::getKingPosition()` and
   `Game::computerWantsDraw()` are reachable from frontends with a
   `Color` argument.
6. **Dead code.** `Board::withRandomPosition()` (`board.cpp:229-293`,
   test-only), `BoardCode::numberOfSetBits()`, `BoardCode::fromEmptyBoard()`,
   `History::removeLastPosition()`, `TranspositionTable::getHitCount()`
   and `getProbeCount()` (superseded by `getStats()`),
   `Board::isEnPassantVulnerable()`, and the alias pairs
   `MoveList::push_back/append`, `pop_back/removeLast`, `empty/isEmpty`.
7. **Duplicated rank magic numbers.** `generate.cpp:116,118` (6/1),
   `:293` (3/4) and `:619,621` (0/7) hard-code ranks that `First_Row`,
   `Last_Row` and the en passant row constants in `global.hpp:79-91`
   already name. `InlineThreats::row()`/`column()` repeat one ray loop
   four times (`threats.hpp:71-116`); `MoveGeneration::rook()`/`bishop()`
   repeat the slider loop (`generate.cpp:212-264`).
8. **Unreachable and redundant code.** `default:` throws after
   exhaustive switches (`move.cpp:287-291`, `position.cpp:181-182`); a
   pawn check nested inside `case Piece::Pawn:` (`move.cpp:562-565`);
   `dst_coord.empty()` after a length check (`move.cpp:409-410`);
   `row < 8 &&` beside `isValidRow (row)` (`generate.cpp:179-185`).
9. **Stale or wrong comments.** "48-bits Zobrist hash" (`board_code.hpp:269`);
   "more performant than branching" above a ternary (`board.hpp:142-145`);
   "Whether this move could cause a draw" on a function that takes a board
   (`evaluate.hpp:552`); "Whether this move was a legal move for the
   computer_player" on `isLegalPositionAfterMove` (`evaluate.hpp:515`);
   `// todo convert enum to integer index` (`position.cpp:94`).
10. **Convention gaps.** Leading return types in `threats.hpp:36,71,95,118,145,193,265`,
    `history.hpp:85-92`, `game.hpp:68,72`, `search.hpp:47,50`. Missing
    `[[nodiscard]]` on `nextRow`/`nextColumn`/`makeCoord` (`coord.hpp`),
    `coordColor`/`pawnDirection` (`board.hpp`), `checkmateScoreInMoves`/
    `isCheckmatingOpponentScore` (`evaluate.hpp`), `toInt`/
    `makeCastlingEligibilityFromInt` (`castling.hpp`), and `random.hpp`.
    `global.hpp:31-34` forward-declares `doctest::String` in production
    code. `charToRow`/`charToCol` are namespace-scope `static constexpr`
    in a header (`coord.hpp:164-176`).
11. **`DrawCategory`** wraps an enum in a struct with an implicit
    constructor and `bool` conversion (`evaluate.hpp:481-513`) but its
    only consumer treats it as a `bool` (`search.cpp:230`).

### UCI

12. **`position ... moves` applies illegal moves.** `uci_interface.cpp:288`
    maps each token through `mapCoordinatesToMove()`, which checks
    geometry only, then calls `Game::move()`, which does not validate.
    `e2e5` or a move into check corrupts the position. A token that fails
    to parse is skipped without a message, so every later move lands on
    the wrong board. Nothing is reported unless `debug on`.
13. **`position fen` requires six fields** (`uci_interface.cpp:397-405`).
    A four-field FEN is ignored silently.
14. **`bestmove (none)`** (`uci_interface.cpp:675`) where GUIs expect
    `0000`.
15. **`handleQuit` calls `std::exit(0)`** after the join
    (`uci_interface.cpp:587-591`), bypassing the destructor.

### Console

16. **`main()` never calls `setEmergencyLogger()`** (`console/main.cpp:13`),
    which AGENTS.md requires of every frontend.
17. **`save` never reports failure.** `output_format.cpp:21-25,34-40`
    open an `ofstream` without checking; `play.cpp:616-618` has
    `// todo: handle errors here` and prints "Game saved" regardless.
18. **`maxdepth 0` and `timeout 0` are accepted** (`play.cpp:493,502`);
    `Game::setMaxDepth()` stores the value unchecked (`game.cpp:400-403`).
19. **`humanWantsDraw()` indexes `input[0]`** before anything is read
    (`play.cpp:247-260`). Defined on `std::string` but reads as a bug.
20. **`readCommand()` ends with an unreachable `throw`** (`play.cpp:572`)
    because the dispatcher is a `holds_alternative` chain over a
    sixteen-alternative `std::variant` (`play.cpp:575-682`).

### QML

21. **Held-move slot can be overwritten in computer-vs-computer.**
    `game_model.cpp:227` stores one `HeldMove`. The design note in
    `qml-engine-move-delay.md:201-206` assumes only one engine move can be
    outstanding because the engine searches again only on
    `receiveEngineMoved`. That premise is false: `ChessEngine::init()`
    (which calls `findMove()`) also runs on `QThread::started`
    (`game_model.cpp:95-96`), on every `updateConfig` (`chess_engine.cpp:339-352`,
    emitted by `start()`, `restart()` and every settings change), on
    `reloadGame` and on `resumeSearching`. When both players are engines
    every `findMove()` yields a move, so `start()` alone puts two
    searches in flight. If search time is shorter than the animation
    delay, a third move arrives before the hold timer fires, overwrites
    the held one, and `showHeldMove()` applies it to a GUI board that
    never saw the second. `Game::move()` does not validate, so the GUI
    board silently diverges from the engine's. No test sets both players
    to Computer.
22. **Emscripten destructor deletes a running thread.** `stopEngineThread()`
    skips `wait()` on Emscripten (`game_model.cpp:383-386`) but
    `~GameModel` still deletes the `QThread` (`game_model.cpp:39`).
23. **Dead and misleading code.** `delete my_chess_engine_thread` at
    `game_model.cpp:70` always deletes `nullptr`. The comment at
    `game_model.hpp:157-159` says the `gameUpdated` pointer "transfers
    ownership" of a `shared_ptr` the GUI keeps a copy of. Cancelled
    searches are reported as `noMovesAvailable` (`chess_engine.cpp:264-268`)
    and the `TODO` at `:257-259` admits the ambiguity.
24. **QML C++ classes are outside `wisdom::`.** `ChessEngine`, `ChessGame`,
    `MaxDepth`, `GameModel`, `PiecesModel`, `PieceInfo`, `GameSettings`
    and `UISettings` are at global scope. Typo `confilg` at
    `chess_game.hpp:442`; stray `};` after namespaces at `ui_types.hpp:509`,
    `web_types.hpp:343`, `game_settings.hpp:60`, `web_game.hpp:163`;
    unused `pieceStr` at `pieces_model.cpp:162`; `mobile_main.qml:46`
    leaves a `console.log`.
25. **`assert` where AGENTS.md says `expects`.** `game_viewmodel_base.cpp:403`,
    `game_model.cpp:700`, `ui_types.hpp:400-505`. These vanish in Release.

### Shared view-model and WASM

26. **Draw negotiation is written three times.**
    `GameViewModelBase::setProposedDrawStatus()` (`game_viewmodel_base.cpp:395-415`)
    has no production caller; `GameModel::handleDrawStatusChange()`
    (`game_model.cpp:692-714`) and React's `setHumanDrawStatus`
    (`App.tsx:272-290`) reimplement it. On the engine side,
    `ChessEngine::handlePotentialDrawPosition()` plus
    `QmlEngineGameStatusUpdate` (`chess_engine.cpp:193-219,271-309`) and
    `worker::GameState::handlePotentialDrawPosition()` plus
    `WebEngineGameStatusUpdate` (`bindings.cpp:72-128`) are the same
    algorithm with a different transport. `gameStatusTransition()` and
    `statusTransition()` are the same wrapper.
27. **Settings exist in four shapes.** `qml/main/game_settings.hpp:518-573`,
    `wasm/game_settings.hpp:10-59`, `react/src/lib/WisdomChess.ts:29-35`
    and `ChessGame::Config` (`chess_game.hpp:413-421`), converted by hand
    at `chess_game.cpp:623-636` and again at `game_model.cpp:512-522`. The
    React thinking-time slider stops at 10 s (`SettingsModal.tsx:129`)
    while QML allows 30 s (`SettingsDialog.qml:195`).
28. **WASM `setCurrentGameSettings` pauses for nothing.**
    `game_model.hpp:97-103` posts pause, settings, unpause back to back,
    so the worker only observes the final `Playing` value and a running
    search is not interrupted.
29. **WASM enumerators pollute `wisdom::`.** `web_types.hpp` uses
    unscoped enums whose members (`Human`, `White`, `Pawn`, `Playing`,
    `NotReached`) land in the namespace. A comment at `web_game.cpp:162-163`
    describes behaviour the function does not have.

### Build, CI and docs

30. **No `-Werror`.** `cmake/Warnings.cmake` applies `-Wall -Wextra`
    and `/W4` but nothing fails on a warning, so the policy is advisory.
31. **README documents 5 of 14 CMake options.** Missing: `ASAN`, `TSAN`,
    `CONSOLE_UI`, `REACT_UI`, `PCH_ENABLED`, `TOOLS`, `BENCHMARKS`,
    `BUILD_LINTER`, `QT_DIR`; `FILC_COMPAT` appears only in prose. The
    "Running Tests" section runs the two doctest binaries by hand and
    never mentions `ctest`, the CLI, QML, view-model or React tests, or
    the sanitizer build. Qt is stated as 6.8+ while CI uses 6.9. Let's
    document these in a separate document linked to from README.md.
32. **Workflow drift.** `cmake.yml` carries a stale comment about
    MSVC 2022 above `windows-latest`; `web.yml`'s `lint` job duplicates
    `cmake.yml`'s but skips the linter self-tests; no
    `.github/dependabot.yml` although `web.yml` references the dependabot
    actor; CPM dependencies are tag-pinned with no committed lock file.
33. **Linter false positives.** `scripts/linter` flags `foo(x)` inside a
    trailing `// comment` and inside a `/* */` block, and flags
    `Q_UNUSED(x)`. Its own sources use the opposite call spacing and
    are not linted.

### Deferred to after `quiescence-search` merges

These touch the code that branch rewrites, so they wait:

- A timed-out iteration discards a fully searched root move
  (`search.cpp:289-290` returns before the `ply == 0` block).
- A stalemated leaf receives its material score because `evaluate()`
  tests checkmate only (`evaluate.cpp:674-678`).
- Draw contempt reuses `Min_Draw_Score` for both acceptance and search
  (`global.hpp:133-134`, `search.cpp:216`).
- The phantom en passant target set on every double push
  (`move.cpp:201-216`) hashes otherwise-identical positions apart.

## Plan

Each step is its own commit, with a Debug and a Release run of the
affected tests, and the linter, before moving on. Engine steps come
first because they are self-contained and their tests are the fastest.

### 1. Hash mask

Replace the mask in `setMetadataBits()` with one derived from the
metadata width, `~std::uint64_t { 0xffff }`, and fix the comment.
Change the test at `board_code_test.cpp:356,365` to assert that the top
bit of a code survives a metadata update, which fails on the old mask.
Widening the hash changes every code, so the hash-collision slow test
and the perft tests are the regression check.

### 2. Piece-square tables

Correct the four cells listed in finding 2 against the source tables
and add a test that every table except the queen's is left-right
symmetric. This changes evaluation, so the search tests that assert a
specific move ("Bishop is not sacrificed", "Finding moves regression")
are re-run in Release and any that change are examined by hand before
their expected move is updated.

### 3. Character classification

Add `wisdom::toUpper`/`toLower`/`isSpace` helpers in `global.hpp` that
cast through `unsigned char`, and use them at every site in finding 3.
A test feeds a move string with a byte above `0x7f` to
`moveParseOptional()` under UBSan.

### 4. Turn and colour guards

`Game::setCurrentTurn()` rebuilds the history's initial code from the
new board. `colorIndex()` gains `expects (who != Color::None)`, and
`Game::computerWantsDraw()` and the two `Board` getters that take a
colour get the same check at the API boundary. `Game::setMaxDepth()` and
`setSearchTimeout()` reject zero and negatives with `expects`.

### 5. Engine dead code and duplication

Delete everything in finding 6, moving `withRandomPosition()` into the
test that uses it. Replace the rank literals in finding 7 with the
constants from `global.hpp`. Collapse the four ray loops in
`InlineThreats` and the rook/bishop slider loops into one helper each.
Remove the unreachable code in finding 8 and rewrite the comments in
finding 9. Add the missing `[[nodiscard]]` and trailing return types
from finding 10, drop the `doctest::String` forward declaration from
`global.hpp` if the tests still compile without it, and make
`charToRow`/`charToCol` plain `constexpr`. Reduce `DrawCategory` to its
enum or to `bool`, whichever its one caller wants.

### 6. UCI input

Validate each `position` move against `generateLegalMoves()` before
applying it; on the first illegal or unparseable token, stop, keep the
position as it was before the command, and print an `info string`
naming the token. Accept a four-field FEN by defaulting the clocks.
Print `bestmove 0000` when there is no legal move. Return from
`handleQuit` instead of `std::exit`. Extend `cmake/CliTests.cmake` with
scripts for an illegal move, an unparseable move, a four-field FEN and
the no-legal-move case.

### 7. Console

Call `setEmergencyLogger()` in `console/main.cpp`. Make
`OutputFormat::save()` throw `Error` when the stream fails and report it
in `play.cpp`. Reject zero for `maxdepth` and `timeout` with a message.
Fix `humanWantsDraw()` to read before it tests. Replace the
`holds_alternative` chain with `std::visit` over an overload set, which
removes the trailing `throw`. Add `Console:` scripts for the rejected
values and for a save to an unwritable path.

### 8. QML engine thread

Fix finding 21 at the source: `ChessEngine` tracks whether a search is
in flight or a move is awaiting the GUI, and `init()` returns early in
either state, so at most one engine move is outstanding. `GameModel`
guards the held slot with `expects (!my_held_move)` so a regression is
loud. Add a QML UI test with both players set to Computer, `maxDepth 1`
and a long animation delay, and assert the GUI board matches the engine
board after several moves. Correct the invariant statement in
`qml-engine-move-delay.md`. Report cancelled searches through their own
signal instead of `noMovesAvailable`. On Emscripten, skip the
`QThread` delete when the thread is still running, and delete the dead
`delete` at `game_model.cpp:70`.

### 9. QML conventions

Move the QML C++ classes into `wisdom::ui::qml`. Qt 6.9 supports
`Q_OBJECT` and `Q_GADGET` classes in a namespace, and `wisdom::ui`
already holds a `Q_NAMESPACE` whose enums QML uses. QML never sees the
namespace: today the models reach QML as context properties set in
`main.cpp`, and on `qml-type-annotations` as `QML_NAMED_ELEMENT`
singletons whose `QML_FOREIGN` argument becomes the qualified name
while the QML-facing name stays `GameModel`. Two rules keep moc and
`qmltyperegistrar` happy: spell every `Q_PROPERTY`, signal and slot
argument type with its full `wisdom::ui::...` qualification, as
`game_settings.hpp` already does for `wisdom::ui::Player`, and keep
using pointer-to-member `connect()` rather than the `SIGNAL()`/`SLOT()`
string macros, which would need the qualified names too. Nothing in the
tree uses the string macros. Replace the `assert`s in finding 25 with
`expects`. Fix the typo, stray semicolons,
unused variable, stale comments and the leftover `console.log`. This
step is the most likely to conflict with `qml-type-annotations`, so it
is done last among the QML steps and rebased onto that branch if it has
merged by then.

### 10. Shared draw negotiation

Move the engine-side draw negotiation into the view-model library as a
free function `negotiateDraw (Game&, ProposedDrawType, Color, Callback)`
that asks each engine player in turn and hands every answer to the
callback; `ChessEngine` and `worker::GameState` supply the transport.
Make `GameModel::handleDrawStatusChange()` call the base
`setProposedDrawStatus()` and emit after it. Collapse
`gameStatusTransition()`/`statusTransition()` into one base method. The
existing view-model tests cover the base; add a QML test and a WASM
worker test that a draw proposal with two engine players records both
answers.

### 11. Settings

Give `wisdom::ui` one `GameSettings` struct in the view-model library
and have QML's `GameSettings` and the WASM one wrap or alias it, with
`ChessGame::Config` derived from it once. Define the thinking-time
range there and read it from both the QML and React sliders. Make the
WASM `setCurrentGameSettings` bump the game id, or drop the no-op
pause, so the behaviour matches what the code claims. Scope the WASM
enums with `enum class` if Embind allows it; otherwise move them into a
`web::` sub-namespace.

### 12. Build and CI

Add `WISDOM_CHESS_WERROR` (default Off) that appends `-Werror`/`/WX` in
`Warnings.cmake`, turn it on in every `cmake.yml` and `web.yml` build
job, and fix whatever it surfaces per platform in the same commit.
Move the CMake option table and the test instructions out of
`README.md` into a new `docs/building-and-testing.md` that lists all
fourteen options and describes `ctest`, the labels, the CLI, QML,
view-model and React suites and the sanitizer build, matching AGENTS.md;
`README.md` keeps the per-frontend build recipes and links to it. Bump
the stated Qt minimum to 6.9. Delete the stale MSVC comment, make `web.yml`'s lint job run the
linter self-tests, add `.github/dependabot.yml` for GitHub Actions and
npm, and commit `cpm-package-lock.cmake`.

### 13. Linter

Track `/* */` and trailing `//` comments in the line scanner, add
`Q_UNUSED` to the exception list, and add fixtures for each. Reformat
`scripts/linter` to the house style and add it to the `lint` target.

## Risks

- Steps 1 and 2 change engine output. Both are checked against the
  slow suite and the specific-move search tests, and any expected move
  that changes is justified in the session log.
- Step 8 changes when the engine searches. The `QML:` suite and the
  `qml-engine-move-delay.md` timing tests are the guard.
- Steps 9 and 11 touch files that `qml-type-annotations` and
  `react-view-cleanups` also touch. They are done last and rebased if
  either branch merges first.
- Step 12's `-Werror` may fail on MSVC or AppleClang for warnings the
  Linux build never showed. The option stays Off by default so a local
  build is never blocked.

## Implementation Progress
