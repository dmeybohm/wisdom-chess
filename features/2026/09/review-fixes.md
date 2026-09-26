# Review fixes

## Motivation

A whole-project review on 2026-09-25 scored the code 7/10 and listed the
defects and smells that keep it there. This branch works through that
list.

The plan was written against `main` at `185c0d5` and rebased onto
`c14f0e9` on 2026-09-26, after eight branches merged in between. Three
of them change what this branch has to do:

- `quiescence-search` added quiescence and dropped the even-depth-only
  rule. The four search items the first draft deferred to avoid that
  branch are now in scope (findings 34-37, step 6).
- `react-view-cleanups` reworked `App.tsx` state, `StatusBar.tsx`,
  `Square.tsx`, `TopMenu.tsx` and `PawnPromotionDialog.tsx`. The unused
  imports, the double drag ref, the `"false"` class name, the
  regex-parsed bold markup and the engine values read during render are
  gone. React's draw answer is still a third copy of the base logic
  (finding 26).
- `qml-cleanups` folded the three QML roots into shared
  `MainWindow`/`GameRoot`, dropped `PromotedPieceModel`, rewrote
  `SettingsDialog`, `Piece` and `GameMenu`, exposed the models to QML as
  `QML_NAMED_ELEMENT` singletons and registered the enums declaratively
  under `wisdom::ui::qml::*` namespaces. The duplicated roots, the
  reversed `ListModel`, the binding break-and-rebind, the settings timer
  and the stray `console.log` are gone.

`ci-consolidation` removed `web.yml`'s duplicate lint job and the stale
MSVC comment (finding 32 shrinks accordingly). `engine-match-script`
added `scripts/run-engine-match.sh`, which steps 1, 2 and 6 use to
measure their effect on strength.

## Findings

Every item was verified by reading the code on `main` at `185c0d5` and
re-verified on `c14f0e9` after the rebase. Line numbers are from
`c14f0e9`.

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
   `board.cpp:139,178,180`, `fen_parser.cpp:21`, `str.cpp:14`,
   `uci_interface.cpp:541,575` and `console/play.cpp:225,233`. A byte at
   or above `0x80` in user input is a negative `int`, which is undefined
   behaviour for these functions. `uci_interface.cpp:129` already casts
   through `unsigned char` and is the model.
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
   (`evaluate.hpp`); "Whether this move was a legal move for the
   computer_player" on `isLegalPositionAfterMove` (`evaluate.hpp`);
   `// todo convert enum to integer index` (`position.cpp:94`).
10. **Convention gaps.** Leading return types in `threats.hpp:36,71,95,118,145,193,265`,
    `history.hpp:85-92`, `game.hpp:68,72`, `search.hpp`. Missing
    `[[nodiscard]]` on `nextRow`/`nextColumn`/`makeCoord` (`coord.hpp`),
    `coordColor`/`pawnDirection` (`board.hpp`), `checkmateScoreInMoves`/
    `isCheckmatingOpponentScore` (`evaluate.hpp`), `toInt`/
    `makeCastlingEligibilityFromInt` (`castling.hpp`), and `random.hpp`.
    `global.hpp:31-34` forward-declares `doctest::String` in production
    code. `charToRow`/`charToCol` are namespace-scope `static constexpr`
    in a header (`coord.hpp:164-176`).
11. **`DrawCategory`** wraps an enum in a struct with an implicit
    constructor and `bool` conversion (`evaluate.hpp`) but its only
    consumer treats it as a `bool` (`search.cpp:170`).

### UCI

12. **`position ... moves` applies illegal moves.** `uci_interface.cpp:288-291`
    and `:324-327` map each token through `mapCoordinatesToMove()`, which
    checks geometry only, then call `Game::move()`, which does not
    validate. `e2e5` or a move into check corrupts the position. A token
    that fails to parse is skipped without a message, so every later
    move lands on the wrong board. Nothing is reported unless `debug on`.
13. **`position fen` requires six fields** (`uci_interface.cpp:298`).
    A four-field FEN is ignored silently.
14. **`bestmove (none)`** (`uci_interface.cpp:584`) where GUIs expect
    `0000`.
15. **`handleQuit` calls `std::exit(0)`** after the join
    (`uci_interface.cpp:496`), bypassing the destructor.

### Console

16. **`main()` never calls `setEmergencyLogger()`** (`console/main.cpp:13`),
    which AGENTS.md requires of every frontend.
17. **`save` never reports failure.** `output_format.cpp:21-25,34-40`
    open an `ofstream` without checking; `play.cpp:590-592` has
    `// todo: handle errors here` and prints "Game saved" regardless.
18. **`maxdepth 0` and `timeout 0` are accepted** (`play.cpp:467` tests
    `>= 0`); `Game::setMaxDepth()` stores the value unchecked
    (`game.cpp:400-403`).
19. **`humanWantsDraw()` indexes `input[0]`** before anything is read
    (`play.cpp:225-233`). Defined on `std::string` but reads as a bug.
20. **`readCommand()` ends with an unreachable `throw`** (`play.cpp:546`)
    because the dispatcher is a `holds_alternative` chain over a
    sixteen-alternative `std::variant` (`play.cpp:551` onward).

### QML

21. **Held-move slot can be overwritten in computer-vs-computer.**
    `game_model.cpp:227` stores one `HeldMove`. The design note in
    `qml-engine-move-delay.md:201-206` assumes only one engine move can be
    outstanding because the engine searches again only on
    `receiveEngineMoved`. That premise is false: `ChessEngine::init()`
    (`chess_engine.cpp:46`, which calls `findMove()`) also runs on
    `QThread::started` (`game_model.cpp:95-96`), on every `updateConfig`
    (`chess_engine.cpp:218-229`, emitted by `updateEngineConfig()` at
    `game_model.cpp:400` from `start()`, `restart()` and every settings
    change), on `reloadGame` (`chess_engine.cpp:205-215`) and on
    `resumeSearching` (`game_model.cpp:124-125`). When both players are
    engines every `findMove()` yields a move, so `start()` alone puts
    two searches in flight. If search time is shorter than the animation
    delay, a third move arrives before the hold timer fires, overwrites
    the held one, and `showHeldMove()` applies it to a GUI board that
    never saw the second. `Game::move()` does not validate, so the GUI
    board silently diverges from the engine's. `isHoldingAMove()` is now
    public (`game_model.hpp:268`) but no test sets both players to
    Computer; `chess_game_test.cpp:149,174` set engine-versus-human only.
22. **Emscripten destructor deletes a running thread.** `stopEngineThread()`
    skips `wait()` on Emscripten (`game_model.cpp:390-393`) but
    `~GameModel` still deletes the `QThread` (`game_model.cpp:39`).
23. **Dead and misleading code.** `delete my_chess_engine_thread` at
    `game_model.cpp:70` always deletes `nullptr`. The comment at
    `game_model.hpp:179` says the `gameUpdated` pointer "transfers
    ownership" of a `shared_ptr` the GUI keeps a copy of. Cancelled
    searches are reported as `noMovesAvailable` (`chess_engine.cpp:145`)
    and the `TODO` at `:135` admits the ambiguity.
24. **QML C++ classes are outside `wisdom::`.** `ChessEngine`, `ChessGame`,
    `MaxDepth`, `GameModel`, `PiecesModel`, `PieceInfo`, `GameSettings`,
    `UISettings`, `GameModelSingleton` and `PiecesModelSingleton` are at
    global scope, while `ui_types.hpp:184-209` already declares
    `wisdom::ui::qml::color` and its siblings for the enum registration.
    Typo `confilg` at `chess_game.hpp:85`; stray `};` after namespaces at
    `ui_types.hpp:178`, `game_settings.hpp:65`, `web_game.hpp:163`;
    unused `pieceStr` at `pieces_model.cpp:77`.
25. **`assert` where AGENTS.md says `expects`.** `game_viewmodel_base.cpp`,
    `game_model.cpp` and `ui_types.hpp` hold eight `assert` calls on
    caller input. These vanish in Release.

### Shared view-model and WASM

26. **Draw negotiation is written three times.**
    `GameViewModelBase::setProposedDrawStatus()` (`game_viewmodel_base.cpp:279-297`)
    has no production caller; `GameModel::handleDrawStatusChange()`
    (`game_model.cpp:712-733`) and React's `answerDraw`
    (`App.tsx:259-274`, now asking the WASM model for the first and
    second human colour) reimplement it. On the engine side,
    `ChessEngine::handlePotentialDrawPosition()` (`chess_engine.cpp:150-185`)
    and `worker::GameState::handlePotentialDrawPosition()`
    (`bindings.cpp:114-128`) are the same algorithm with a different
    transport. `gameStatusTransition()` (`chess_engine.cpp:100`) and
    `statusTransition()` (`bindings.cpp:62`) are the same wrapper.
27. **Settings exist in four shapes.** `qml/main/game_settings.hpp`,
    `wasm/game_settings.hpp`, `react/src/lib/WisdomChess.ts` and
    `ChessGame::Config` (`chess_game.hpp`), converted by hand in
    `chess_game.cpp` and again in `game_model.cpp`. The React
    thinking-time slider stops at 10 s (`SettingsModal.tsx:92`) while
    QML allows 30 s (`SettingsDialog.qml:190`).
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
    the sanitizer build. Qt is stated as 6.8+ while CI uses 6.9. More
    broadly, the README is written for developers: of its 273 lines,
    the first 17 and the last 20 are for someone who wants to play, and
    the installer download instructions sit in the middle of the build
    recipes. With installers attached to every tagged release, the
    README's first reader is an end user, and the build documentation
    belongs in a separate document linked from it.
32. **Dependency hygiene.** No `.github/dependabot.yml`, although
    `web.yml` references the dependabot actor; CPM dependencies are
    tag-pinned with no committed lock file. (The stale MSVC comment and
    `web.yml`'s duplicate lint job were removed by `ci-consolidation`.)
33. **Linter false positives.** `scripts/linter` flags `foo(x)` inside a
    trailing `// comment` and inside a `/* */` block, and flags
    `Q_UNUSED(x)`. Its own sources use the opposite call spacing and
    are not linted.

### Search, unblocked by the quiescence merge

34. **A timed-out iteration discards a fully searched root move.**
    `search.cpp:200-203` and `:227-228` return before the `ply == 0`
    block at `:245`, so `my_current_result.move` stays empty and
    `iterativelyDeepen()` falls back to the previous depth even when the
    first root move, which move ordering makes the previous best, had
    already been searched to the new depth.
35. **Stalemate is invisible at quiet leaves.** The leaf is now
    `quiesce()`'s stand-pat at `search.cpp:313`, which scores the
    position without asking whether the side to move has a legal move.
    `evaluate()`'s own mate test (`evaluate.cpp:60`) no longer runs in
    search; its only caller is `Game::computerWantsDraw()`
    (`game.cpp:299`). `quiescence-search.md:157` lists this as out of
    scope there.
36. **Draw contempt reuses the acceptance threshold.** `Min_Draw_Score`
    (`global.hpp:134`) is both the score below which the engine accepts
    a draw offer and the contempt `drawingScore()` returns for its own
    repetitions (`search.cpp:146-155`). Tuning one silently moves the
    other.
37. **A double push's FEN target and repetition identity differ.**
    The original review treated a target without an adjacent enemy pawn
    as phantom. The PGN/FEN specification requires the passed square
    after every double pawn push. Repetition identity, however, should
    ignore that target when no legal en passant capture exists. The
    history-key change is planned on a separate branch.

## Plan

Each step is its own commit, with a Debug and a Release run of the
affected tests, and the linter, before moving on. Engine steps come
first because they are self-contained and their tests are the fastest.
Steps 1, 2 and 6 change what the engine plays, so each ends with
`scripts/run-engine-match.sh base=main new=HEAD` and records the result.

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

### 6. Search follow-ups

Four separate commits, each measured with the benchmark search report
and an engine match.

- **Timeout at the root.** When the timer fires at ply 0 after at least
  one root move has been searched to full depth, record the best move
  so far in `my_current_result` before returning. `iterativelyDeepen()`
  takes the partial iteration's move over the previous depth's only when
  it is the same move or scores higher; otherwise the previous depth
  stands. A test with a budget that expires after the first root move
  pins the choice.
- **Stalemate at quiet leaves.** Check `hasLegalMove()` before stand-pat
  evaluation at every non-check horizon node. A material count cannot
  exclude stalemate: even a rook or queen can have no legal move when
  pinned or blocked. Add the KQ-vs-K stalemate trap position as a search
  test and measure the effect on search speed.
- **Separate contempt from acceptance.** Introduce
  `Search_Draw_Contempt` for `drawingScore()`, initially equal to
  `Min_Draw_Score` so nothing changes, and document both in
  `global.hpp`.
- **En passant target after every double push.** Keep the passed square
  for FEN and move generation even without an adjacent enemy pawn.
  Test FEN output for both colors. A separate feature will normalize
  repetition comparisons when the target offers no legal capture.

### 7. UCI input

Validate each `position` move against `generateLegalMoves()` before
applying it; on the first illegal or unparseable token, stop, keep the
position as it was before the command, and print an `info string`
naming the token. Accept a four-field FEN by defaulting the clocks.
Print `bestmove 0000` when there is no legal move. Return from
`handleQuit` instead of `std::exit`. Extend `cmake/CliTests.cmake` with
scripts for an illegal move, an unparseable move, a four-field FEN and
the no-legal-move case.

### 8. Console

Call `setEmergencyLogger()` in `console/main.cpp`. Make
`OutputFormat::save()` throw `Error` when the stream fails and report it
in `play.cpp`. Reject zero for `maxdepth` and `timeout` with a message.
Fix `humanWantsDraw()` to read before it tests. Replace the
`holds_alternative` chain with `std::visit` over an overload set, which
removes the trailing `throw`. Add `Console:` scripts for the rejected
values and for a save to an unwritable path.

### 9. QML engine thread

Fix finding 21 at the source: `ChessEngine` tracks whether a search is
in flight or a move is awaiting the GUI, and `init()` returns early in
either state, so at most one engine move is outstanding. `GameModel`
guards the held slot with `expects (!isHoldingAMove())` so a regression
is loud. Add a QML UI test with both players set to Computer,
`maxDepth 1` and a long animation delay, and assert the GUI board
matches the engine board after several moves. Correct the invariant
statement in `qml-engine-move-delay.md`. Report cancelled searches
through their own signal instead of `noMovesAvailable`. On Emscripten,
skip the `QThread` delete when the thread is still running, and delete
the dead `delete` at `game_model.cpp:70`.

### 10. QML conventions

Move the QML C++ classes into `wisdom::ui::qml`, beside the enum
registration namespaces that `qml-cleanups` already put there. Qt 6.9
supports `Q_OBJECT` and `Q_GADGET` classes in a namespace, and
`wisdom::ui` already holds a `Q_NAMESPACE` whose enums QML uses. QML
never sees the namespace: the models reach it through the
`QML_NAMED_ELEMENT` singletons in `qml_singletons.hpp`, whose
`QML_FOREIGN` argument becomes the qualified name while the QML-facing
name stays `GameModel`. Two rules keep moc and `qmltyperegistrar`
happy: spell every `Q_PROPERTY`, signal and slot argument type with its
full `wisdom::ui::...` qualification, as `game_settings.hpp` already
does for `wisdom::ui::Player`, and keep using pointer-to-member
`connect()` rather than the `SIGNAL()`/`SLOT()` string macros, which
would need the qualified names too. Nothing in the tree uses the string
macros. Replace the `assert`s in finding 25 with `expects`. Fix the
typo, stray semicolons, unused variable and stale comments.

### 11. Shared draw negotiation

Move the engine-side draw negotiation into the view-model library as a
free function `negotiateDraw (nonnull_observer_ptr<Game>, ProposedDrawType, Color, Callback)`
that asks each engine player in turn and hands every answer to the
callback; `ChessEngine` and `worker::GameState` supply the transport.
Make `GameModel::handleDrawStatusChange()` call the base
`setProposedDrawStatus()` and emit after it. Collapse
`gameStatusTransition()`/`statusTransition()` into one base method. The
existing view-model tests cover the base; add a QML test and a WASM
worker test that a draw proposal with two engine players records both
answers.

### 12. Settings

Give `wisdom::ui` one `GameSettings` struct in the view-model library
and have QML's `GameSettings` and the WASM one wrap or alias it, with
`ChessGame::Config` derived from it once. Define the thinking-time
range there and read it from both the QML and React sliders. Make the
WASM `setCurrentGameSettings` bump the game id, or drop the no-op
pause, so the behaviour matches what the code claims. Scope the WASM
enums with `enum class` if Embind allows it; otherwise move them into a
`web::` sub-namespace.

### 13. Build and CI

Add `WISDOM_CHESS_WERROR` (default Off) that appends `-Werror`/`/WX` in
`Warnings.cmake`, turn it on in every `cmake.yml` and `web.yml` build
job, and fix whatever it surfaces per platform in the same commit. Add
`.github/dependabot.yml` for GitHub Actions and npm, and commit
`cpm-package-lock.cmake`.

### 14. End-user README and developer docs

Split `README.md` in two.

`README.md` is for someone who wants to play. In order: the title and
animation, a two-sentence description, the play-online link, a
**Download** section, **Features**, **Screenshots**, a short
**Building from source** paragraph that links to the developer
document, **Contributing** and **License** with the third-party assets.
The Download section names the three installers, links the GitHub
Releases page, and keeps the per-OS "not code-signed" instructions
(SmartScreen, Gatekeeper, `chmod +x`) and the uninstall note that today
sit in the middle of the build recipes, since those are the first thing
a downloader hits. The Linux runtime-library list and the glibc floor
stay with it. The Features list is rewritten to say what a player gets:
play against the engine or another person, choose colour and strength,
draw claims for repetition and the fifty-move rule, and every platform
the app ships on.

`docs/building.md` is for developers and takes everything else:
prerequisites, the per-frontend recipes (console, React and
WebAssembly, Qt desktop, QML and WebAssembly, Android, FIL-C), building
and smoke-testing an installer locally, a table of all fourteen CMake
options, and a **Running tests** section that describes `ctest`, the
`fast` and `slow` labels, the `UCI:`, `Console:`, `QML:`, view-model
and React suites, `qmllint`, and the sanitizer build, matching
AGENTS.md. It states Qt 6.9 as the version CI uses.

`AGENTS.md` currently says the build recipes are in `README.md` and
that a new option is documented "in both"; both sentences change to
name `docs/building.md`. The `Contributing` section of the README
points at `docs/building.md` first and `AGENTS.md` second.

### 15. Linter

Track `/* */` and trailing `//` comments in the line scanner, add
`Q_UNUSED` to the exception list, and add fixtures for each. Reformat
`scripts/linter` to the house style and add it to the `lint` target.

## Risks

- Steps 1, 2 and 6 change engine output. Both the slow suite and the
  specific-move search tests are the guard, any expected move that
  changes is justified in the session log, and an engine match against
  `main` records the strength effect.
- Step 6's stalemate check adds work at leaves. The material gate keeps
  it off the common path; the bench report confirms that.
- Step 9 changes when the engine searches. The `QML:` suite and the
  `qml-engine-move-delay.md` timing tests are the guard.
- Step 13's `-Werror` may fail on MSVC or AppleClang for warnings the
  Linux build never showed. The option stays Off by default so a local
  build is never blocked.
- Step 14 moves text that external links may point at. The README
  keeps a "Building from source" heading so an old anchor lands on the
  pointer to the new location.

## Implementation Progress

### Session #1

- Rebased onto `main` at `c14f0e9` (2026-09-26) before any code
  changed. The rebase touched only this file.
- Re-verified every finding against the new tree and refreshed the line
  numbers. `quiescence-search`, `react-view-cleanups`, `qml-cleanups`,
  `qml-shadowing`, `ci-consolidation`, `uci-millisecond-timing`,
  `engine-match-script` and `board-drag-and-drop` had merged.
- Brought the four deferred search items into scope as findings 34-37
  and step 6, now that quiescence is on `main`. Noted that
  `evaluate()`'s mate test no longer runs in search.
- Dropped from finding 32 the stale MSVC comment and `web.yml`'s
  duplicate lint job, both removed by `ci-consolidation`. Dropped the
  React and QML view items that the merged cleanups fixed, and rewrote
  step 10 around the singleton registration now on `main`.
- Added the engine match script to the verification of every
  strength-affecting step.

### Session #2

Implemented every step, one commit each, on top of `c14f0e9`. Each
engine step was run in Debug and Release, and every step ended with the
full `ctest` run, the style linter and, where QML was touched, qmllint.
The final tree passes 243 tests, builds with `WISDOM_CHESS_WERROR=On`
under GCC and under Emscripten's Clang, and passes the React type check,
the generated-types check and all 50 vitest cases.

Engine:

- **Step 1** (`7ffeb5d`): the mask is now `Piece_Hash_Mask`, derived from
  `Total_Metadata_Bits`. The new test sets every piece on every square,
  changes the turn and checks the hash bits survive; it fails 728
  assertions on the old mask. A 200-game match against `main` scored
  -3 Elo (-36 to +29): no measurable change, as expected.
- **Step 2** (`4f3af65`): eight cells corrected, one more than the review
  found (bishop row 0, column 7, and a queen cell that was also a slip
  against the source table). The symmetry test reads each table back
  through a board. None of the specific-move search tests changed.
  Matches against the hash commit: 200 games -26 Elo (-62 to +9), then
  500 games -5 Elo (-26 to +16). No measurable change; the tables now
  match Michniewski's.
- **Step 3** (`5a63319`): the helpers went into `str.hpp` rather than
  `global.hpp`, at the author's request, as constexpr ASCII functions;
  they also replaced the one-off constexpr `toLower (int)` that
  `coord.hpp` used. The console's draw prompt and `fen_parser.cpp` were
  further call sites the review had missed.
- **Step 4** (`9a5d881`): `History::replaceLastPosition()` keeps the
  history in step with `setCurrentTurn()`; the mutation check fails
  without it. The guards went on the `Game` methods that take a colour
  (`getPlayer`, `computerWantsDraw`, `setCurrentTurn`,
  `setProposedDrawStatus`) rather than on `colorIndex()`, which is on
  the hot path and keeps its `assert`. The console's zero check moved
  here from step 8 so no step leaves `maxdepth 0` throwing.
- **Step 5** (`6974125`): as planned, except that `BoardCode::fromEmptyBoard()`
  and `Board::isEnPassantVulnerable()` stay: both are used by many
  tests as the natural way to build or query a position, and neither
  duplicates anything. `Board::withRandomPosition()` needed the board's
  private members, so it and its test were deleted rather than moved.
  The benchmark shows no slowdown from the single ray template: perft
  16.1M nodes/s against main's 15.5M on the same machine.
- **Step 6** (`e2459f9`, `112134b`, `9680d10`, `203dc38`): all four.
  The stalemate test position (`k7/P7/1K6/8/8/8/8/8 w`) makes the engine
  play the stalemating Ka6 without the gate. The en passant change moved
  two repetition tests: one gains a white pawn so its double push is
  capturable and its point survives, the other now expects the
  repetition one cycle earlier, which is the correct count. The
  en passant portion of this step was superseded in Session #4 after
  checking the FEN specification.
  root-timeout test runs the same search twice and cancels on the last
  depth's final periodic timer call; it fails without the change. The
  depth-6 search report against main: every position equal or fewer
  nodes, the middlegame position 30% fewer.

Frontends:

- **Step 7** (`4771024`): as planned, plus an invalid FEN is now reported
  whether or not debug mode is on; the test that pinned silence was
  changed to expect the report, since UCI GUIs show `info string` lines.
  Five new `UCI:` scripts.
- **Step 8** (`a5c478d`): as planned. Three new `Console:` scripts,
  including an empty answer to a draw offer.
- **Step 9** (`db35acf`): `ChessEngine::my_move_awaiting_gui` is the
  guard; `engineThreadMoved()` checks the slot with `expects`. The new
  application test sets both players to Computer, changes a setting
  while a move is held, and replays every shown move from the start;
  with both guards disabled it fails on a White move played for Black.
  Cancelled searches report `searchInterrupted`.
- **Step 10** (`981ec6e`): every QML C++ class is in `wisdom::ui::qml`.
  Two things the plan did not foresee: inside that namespace an
  unqualified `Color` or `Player` finds the `wisdom::ui` mirror enums,
  so each source file declares `using wisdom::Color; using wisdom::Player;`;
  and qmllint could not resolve the `Q_INVOKABLE` clone methods' gadget
  return types until they were spelled in full.
- **Step 11** (`af1375d`): `negotiateDraw()` and `transitionGameStatus()`
  in the view-model library, used by `ChessEngine` and the WASM worker;
  `GameModel::handleDrawStatusChange()` calls the base. View-model tests
  cover the negotiation and a new `QML: ChessEngine` unit test covers
  its use. The React side still asks the WASM model for both human
  colours and calls `setHumanDrawStatus` twice; folding that into one
  `answerDraw` binding is left for a later branch.
- **Step 12** (`7889811`): `wisdom::ui::GameSettings` with the ranges and
  `applyTo()`. `ChessGame::Config` is an alias of it and `MaxDepth` is
  gone; out-of-range values are rejected when applied. The QML sliders
  read `GameModel.minThinkingTime` and friends; the React slider reads
  the same numbers through four new `GameModel` bindings, so both stop
  at 30 seconds. The WASM enums are enum classes: webidl_binder allows
  one `::` in a value and emits it verbatim, so the IDL spells them
  through the global aliases (`wisdom_WebPlayer::Human`), which keeps
  the JavaScript names, and the bound methods take the enum types
  instead of `int`. The settings restart is a `restart_requested` flag
  the worker's timer checks, cleared when the settings arrive.

Build, CI and docs:

- **Step 13** (`21fa3e3`): `WISDOM_CHESS_WERROR`, on in every CI build
  except FIL-C, whose compiler could not be checked here; the web build
  gets it through an environment variable its script reads. GCC and
  Emscripten's Clang were verified warning-free locally; MSVC and
  AppleClang will be seen on the first CI run.
- **Step 14** (`9a2e7d8`): as planned. The options table gained
  `WISDOM_CHESS_WERROR`, `BUILD_LINTER` and `VCREDIST`.
- **Step 15** (`3391b16`): `stripComments()` in the linter library blanks
  comments column for column; both scanning rules use it. The reformat
  of the linter's sources was mechanical and whitespace-only
  (`git diff -w` shows no other change), and the lint target names the
  sources rather than the directory so the fixtures stay out.

Left for later branches: the React draw answer (step 11 above); the
`web::` namespace was not needed once the enums became enum classes.

### Session #3

The first PR CI run found two build issues with warnings treated as errors.
AppleClang and sanitizer Clang reject two unused `this` captures in the
console command visitor; both lambdas call the static `printHelp()`, so the
captures were removed. MSVC reports C4702 from Qt 6.9 headers while
compiling generated `Board.qml` cache code, despite `/external:W0`. The
QML executable now suppresses that warning on MSVC while retaining `/W4`
and `/WX` for other warnings.

The local Release build with `WISDOM_CHESS_WERROR=On` passes, as do all
243 CTest cases and the style linter. The console target also builds
with Clang 18 and `WISDOM_CHESS_WERROR=On`. MSVC verification requires
the PR's Windows CI job.

The next Windows run compiled the QML cache but reached a new `/WX`
failure in the view-model tests: MSVC 14.51 reports C5285 for a
`std::tuple` specialization in doctest 2.4.12. The test target now
suppresses C5285 on MSVC; warnings remain errors for other diagnostics.

That Windows run then reached the QML settings test, where Qt's
`QVariant` headers emitted the same C4702 warning as the generated QML
cache. The QML test helper now applies the suppression to its test
executables on MSVC.

### Session #4

The PR review clarified that the PGN/FEN specification records the
passed square after every double pawn push. Removed the adjacent-pawn
condition and its helper. Added FEN checks for White's and Black's
opening double pushes, neither of which has an adjacent enemy pawn.
Removed two history subcases that relied on the old target policy;
legal-capture normalization and its draw tests belong to the separate
repetition-key feature.

The new FEN test and all 137 fast engine tests pass in Release and
Debug. The full Release build, all 244 CTest cases, and the lint target
pass.

### Session #5

The material gate on the quiet-horizon stalemate check was not a sound
correctness condition. For example,
`8/8/8/8/2q5/2b5/1R6/K1k5 w - - 0 1` is stalemate despite White having a
rook: the rook is pinned to the king, and the king has no safe square.
The project's depth-one perft reports zero legal moves. Removed
`stalemateIsPlausible()` and checked `hasLegalMove()` at every non-check
quiescence node before stand-pat evaluation. The feature plan above now
records this rationale.

The full Release build and all 244 CTest cases pass. The existing
horizon-stalemate search test and all 137 fast engine tests pass in
Debug. The full Release CTest run took 24.05 seconds, versus 23.71
seconds before this change; that is a suite measurement, not a search
throughput benchmark.
