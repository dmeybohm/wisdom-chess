# Bug list and engine compiler warnings

## Motivation

A code-quality review on 2026-09-09 scored the project 6/10 across the
engine, the frontends, and the test/build infrastructure. The review turned
up a set of concrete defects that a reader can point to, plus one systemic
gap: the engine library compiles with no warning flags at all, while the
linter target already uses `-Wall -Wextra -Wpedantic` / `/W4`.

This document records the defects as a checklist so they can be fixed in
follow-up branches, and covers enabling compiler warnings on the engine
target as the first step.

## Plan

1. Record the review's confirmed findings below, grouped by area, with
   file references, so each one can be fixed and checked off independently.
2. Add `-Wall -Wextra` (GCC/Clang) and `/W4` (MSVC) to `wisdom-chess-core`
   as `PRIVATE` compile options, matching the linter's existing pattern.
3. Build and record what the new flags surface. Warnings are fixed in
   follow-up work, not in this branch, so the flag change stays reviewable
   on its own.

## Bug list

Line numbers are as of commit `6a1f9af`. Items marked *(verified)* were
re-checked by reading the cited code; the rest come from the review and
should be confirmed before fixing.

### Engine: correctness

- [x] **Piece-square tables mirrored on both axes for Black.** *(verified)*
  `translatePosition` in `engine/position.cpp:76-90` flips row and column,
  but `pawn_positions` (row 5: `+1 -1 -2 0 0 2 -1 +1`), `king_positions`
  (`-6 -8 -8 -9 -9 -4 -4 -6`) and `bishop_positions` are not left-right
  symmetric. Black therefore prefers the opposite wing from White. Mirror
  only the rank, or make the tables symmetric. Fixed in Session #20.
- [x] **FEN en-passant parser catches the wrong exception.** *(verified)*
  `engine/fen_parser.cpp:113` catches `BoardBuilderError`, but
  `coordParse` throws `CoordParseError` (`engine/coord.hpp:203,209`). The
  rewrap to `FenParserError` never fires. Fixed in Session #5.
- [x] **`moveParse` indexes `str[0]` before the empty check.** *(verified)*
  `engine/move.cpp:510` reads `str[0]`; the empty guard is inside
  `moveParseOptional` (`engine/move.cpp:409`), which runs afterwards.
  Reachable from `Game::load` with a blank line. Fixed in Session #6.
- [x] **`CastlingEligibility` stream operator appends a bool as a char.**
  *(verified)* `engine/castling.cpp:18`:
  `result += value.isSet (CastlingRights::Queenside);` emits `\x00` or
  `\x01` instead of "eligible" / "not eligible". Fixed in Session #4.
- [x] **`TranspositionTable (int size_in_mb)` has no lower bound.**
  *(verified)* `engine/transposition_table.cpp:11-23`: size 0 yields
  `power_of_2 == 0`, so `my_size_mask` becomes `SIZE_MAX` and every probe
  indexes out of bounds. `fromEntries` has `Expects (entry_count >= 2)`;
  this constructor needs the same. Fixed in Session #7.
- [x] **`Board::withRandomPosition` keeps stale castling rights.**
  *(verified)* `engine/board.cpp:291` rebuilds the board code from the
  shuffled squares but the castling eligibility is unchanged. A later
  castling move would hit the assert in `getCastlingRookMove`
  (`engine/move.cpp:104`) or move a non-rook in release. Test-only path.
  Fixed in Session #15.
- [x] `CompileTimeRandom::max()` returns `numeric_limits::min()`
  (`engine/random.hpp:58-63`). Violates *UniformRandomBitGenerator*.
  Fixed in Session #15.
- [x] `parseCastling` silently ignores unknown letters
  (`engine/fen_parser.cpp:135-143`, no `default`); `parsePieces` checks
  `row > Num_Rows` instead of `>=` (`engine/fen_parser.cpp:70,83`), so
  `row == 8` reaches `BoardBuilder::addPiece` and throws the wrong type.
  Fixed in Session #15.
- [x] `Board::Board (const BoardBuilder&)` initializer order does not
  match declaration order and `my_position { Position { *this } }` reads a
  partially constructed object (`engine/board.cpp:19-25`). Fixed in
  Session #3: initializer list reordered and a comment added on
  `my_squares`.
- [x] `MoveList (Color, std::initializer_list<czstring>) noexcept` calls
  `moveParse`, which throws (`engine/move_list.hpp:22-30`). Drop the
  `noexcept`.
  Fixed in Session #15.
- [x] `Error`'s copy constructor is `noexcept` but copies two strings
  (`engine/global.hpp:205-207`). Fixed on the `error-hygiene` branch: the
  text is shared, so a copy allocates nothing. See
  [error-hygiene.md](error-hygiene.md).

### Engine: search and performance

- [ ] **No quiescence search.** `search()` calls `evaluate()` at
  `depth <= 0` (`engine/search.cpp:159-162`). `iterativelyDeepen` then
  discards every odd-depth result (`engine/search.cpp:295-296`) as a
  substitute, throwing away roughly half the search time.
  Measure any change here with the `search/*` benchmarks added in
  Session #20 (`engine/bench/bench_search.cpp`). They search to a fixed
  depth, so they show the cost per depth; quiescence changes what a depth
  means, so compare the moves and scores they reach as well as the time.
  Quiescence replaces the leaf `evaluate()` call where the in-check mate
  test runs. See "Effect on quiescence search" in
  [faster-legal-move-test.md](faster-legal-move-test.md) for how often
  horizon nodes are in check, what searching their evasions would cost,
  and what happens to the mate test.
- [x] **Repetition check scans the whole history at every node.**
  `isProbablyDrawingMove` calls `History::isProbablyNthRepetition`, which
  does `std::count` over all board codes (`engine/history.hpp:94-101`).
  Bound the scan by the half-move clock. Fixed in Session #20.
- [x] **Search result passed through a member overwritten at every ply.**
  `my_current_result` is written at every node
  (`engine/search.cpp:174-176, 226-234`); correctness depends on the root
  call writing last. Write it only at `ply == 0` or return a struct.
  Fixed in Session #20.
- [x] **Transposition table deep-copied per search.** The TT is a value
  member of `Game::Impl` (`engine/game_impl.hpp:30`) and `Game`'s copy
  constructor copies it (`engine/game.cpp:77-80`). The UCI frontend copies
  the `Game` on every `go` (`ui/uci/uci_interface.cpp:295-301`), copying
  about 12 MB and discarding the learned table.
  The `search/*` benchmarks clear the table before every search, so they
  measure a cold table only. Showing the benefit of a table that survives
  between moves needs a benchmark that searches consecutive positions of
  one game without clearing; add that alongside the fix.
  Fixed on the `transposition-table-ownership` branch: the table moved out
  of `Game` and each frontend owns one, with the
  `search/warm-table` benchmark added. See
  [transposition-table-ownership.md](transposition-table-ownership.md).
- [x] **Draw scores stored under board-only keys.** `search()`
  (`engine/search.cpp:152`) returns `drawingScore()` before it probes or
  stores, so a repeating node is never written to the table, but its
  ancestors are, and they carry a score that only holds for the history
  that produced it. Within one game this is the usual graph-history
  interaction and is what keeping a table across moves costs everywhere.
  Across an unrelated history it is worse, which is why UCI clears the
  table when a `position` command does not continue the current game
  (Session #2 of
  [transposition-table-ownership.md](transposition-table-ownership.md)).
  Fixing it at the root means keeping path-dependence out of stored
  scores: either do not store a score that a draw check produced, or key
  such entries by the repetition context as well.
  Fixed on the `path-dependent-draw-scores` branch by the first of those:
  a node whose subtree hit the draw check is not stored. The sharpest
  case turned out to be the fifty-move rule rather than repetition, since
  the halfmove clock is not part of the hash. See
  [path-dependent-draw-scores.md](path-dependent-draw-scores.md).
- [x] Leaf evaluation calls full legal-move generation when in check
  (`engine/evaluate.cpp:63, 87-97`). Measured, with the alternatives, in
  [faster-legal-move-test.md](faster-legal-move-test.md). Fixed on the
  `faster-legal-move-test` branch: the mate and stalemate tests stop at
  the first legal move.
- [x] `compareMoves` recomputes `materialDiff` in the return
  (`engine/generate.cpp:538-542`); the sort lambda captures
  `MoveGeneration` by value (`engine/generate.cpp:564-568`).
  Fixed in Session #20.
- [x] Search timing uses `system_clock` (`engine/search.cpp:334, 340`);
  use `steady_clock` as `MoveTimer` already does.
  Fixed in Session #15.
- [x] `MoveList::data()` returns the 504-byte array by value
  (`engine/move_list.hpp:172-176`). Unused today. Removed in Session #20.

### Engine: error handling and hygiene

- [x] `iterativelyDeepen` catches `Error`, prints to `std::cerr` and calls
  `std::terminate()` (`engine/search.cpp:306-312`). Let it propagate.
  The `emergency-logger` branch made it report through `logEmergency()`
  and abort. The `error-hygiene` branch made it propagate as a
  `SearchError` that carries the board; see
  [error-hygiene.md](error-hygiene.md).
- [x] Direct `std::cout`/`std::cerr` in library code bypassing `Logger`:
  `engine/board.cpp:31, 286`, `engine/game.cpp:253`,
  ~~`engine/search.cpp:308-309`~~. The `search.cpp` site was fixed on the
  `emergency-logger` branch. Session #20 removed the two `std::cout` sites;
  `Board::dump()` still writes to `std::cerr`, which is its purpose as a
  debugger helper. Its declaration says so since the `error-hygiene`
  branch, and the item is closed.
- [x] `Game::load` returns `nullopt` on open failure but throws on a bad
  move (`engine/game.cpp:251-255, 266`). Pick one. Fixed in Session #20.
- [x] `Coord::index()`, `row()`, `column()` are not `const`
  (`engine/coord.hpp:58-81`); all `InlineThreats` methods are non-`const`
  (`engine/threats.hpp:36-265`); `operator<< (ostream&, Position&)` takes
  a non-const reference (`engine/position.hpp:32-34`). Fixed in
  Session #20.
- [x] Duplicated castling rook column logic in `Board::getCastlingRookMove`
  (`engine/move.cpp:73-107`), `BoardCode::applyMove`
  (`engine/board_code.cpp:102-121`) and `Position::applyMove`
  (`engine/position.cpp:194-210`). Fixed in Session #20.
- [x] `castlingRowForColor` (`engine/move.hpp:344-351`) and
  `castlingRowFromColor` (`engine/position.cpp:92-105`) are the same
  function. Fixed in Session #20.
- [x] Dead code: ~~`castled_state += "";` (`engine/board.cpp:151`)~~
  (removed in Session #15), unused
  parameters on `isProbablyDrawingMove`, unreferenced
  `Board::pieceAtIndex`, `Board::squareData`, `MoveList::fromZeroInitialized`,
  `BoardCode::withMove`, `TranspositionTable::getStoredEntriesCount`,
  `MoveGeneration::none()`. Removed in Session #20.
- [x] `CoordIterator` (`engine/coord.hpp:216-279`) claims
  `forward_iterator_tag` but has no postfix `++`, its `reference` is
  `Coord&` while `operator*` returns by value, and `begin()` ignores the
  stored coordinate. Fixed in Session #20.
- [x] Mixed tabs and spaces: `engine/evaluate.hpp:78-81`,
  `engine/evaluate.cpp:99-103`, `engine/game.hpp:36-59`,
  `engine/board_code.hpp:20-25`, `engine/search.cpp:292-296`. Candidate
  for a linter rule. The tabs were replaced in Session #20; the `no-tabs`
  linter rule was added on the `error-hygiene` branch.

### Frontends

The remaining low-risk cleanup items are being addressed on the
`frontend-cleanups` branch; see [frontend-cleanups.md](frontend-cleanups.md).
The engine thread's animation delay was kept separate because replacing
it needed a timing and thread-coordination design; that design and the
replacement are in
[qml-engine-move-delay.md](qml-engine-move-delay.md).

- [x] **Pointer thrown instead of exception.** *(verified)*
  `ui/wasm/web_game.cpp:82`: `throw new Error { "Failed to map move." };`.
  No C++ `catch (Error&)` will match and the object leaks.
  Fixed in Session #16.
- [x] **Console draw prompt tests the wrong character.** *(verified)*
  `ui/console/play.cpp:231`: `input[0] == 'y' || input[1] == 'Y'`. A
  capital `Y` is treated as declining.
  Fixed in Session #16.
- [x] **UCI `stop` suppresses `bestmove`.** *(verified)*
  `handleStop` (`ui/uci/uci_interface.cpp:368-371`) bumps `my_search_id`,
  so the `if` at line 319 skips `sendBestMove`. The UCI protocol requires
  `bestmove` after `stop`.
  Fixed in Session #16.
- [x] **Draw-answered flags never reset on new game.** *(verified)*
  `thirdRepetitionDrawAnswered` and `fiftyMovesDrawAnswered`
  (`ui/react/src/App.tsx:312-313`) are not cleared in `startNewGame`
  (`App.tsx:281-290`), so the draw dialog appears at most once per session.
  Fixed in Session #17.
- [x] **`PiecesModel::playerMoved` skips an element after removal.**
  *(verified)* `ui/qml/main/pieces_model.cpp:160-176` calls
  `my_pieces.removeAt (i)` inside a forward loop without adjusting `i`,
  and then reads the `piece_model` reference it just invalidated.
  Fixed in Session #16. The `qml-tests` branch reverted the fix to test
  it and found the defect narrower than described here: the stale
  reference pointed at the element that had shifted into the slot, so that
  piece was still processed, but it missed the reset of its castling
  roles. A test there now fails without the fix.
- [x] **`uiSettings` is undefined in `mobile_main.qml`.** *(verified)*
  `ui/qml/main/mobile_main.qml:46` logs `uiSettings.squareSize`; the
  property lives on `_myGameModel`. ReferenceError on every orientation
  change.
  Fixed in Session #16.
- [x] **Leaked WebIDL objects.** `getCurrentGameSettings()` returns a
  `new GameSettings` (`ui/wasm/game_model.hpp`) that `App.tsx:106, 287`
  never destroys; `App.tsx:184-185, 195` allocates three objects per move
  and frees none. `WebMove::asString` returns `strdup`
  (`ui/wasm/web_move.hpp:43`). *(strdup verified)* Fixed in Session #21.
- [x] **`any` at the WASM boundary.** *(verified)*
  `ui/react/src/lib/WisdomChess.ts:95-143` declares `Game`, `PieceColor`,
  `PieceType`, `GameStatus`, `WebMove`, `WebCoord` and others as `any`.
  Consequences that compile today: `onDropPiece` is declared
  `(dst, src)` in `Board.tsx:23` but `(src, dst)` in `Square.tsx:16`;
  `PawnPromotionDialog.tsx` types `selectedPiece` as `PieceColor`.
  Session #20 corrected those two declarations. The types are now
  generated from the IDL; see `wasm-boundary-types.md`.
- [x] **Console number prompts abort on out-of-range input.** *(verified)*
  `readInt` in `ui/console/play.cpp` caught `std::invalid_argument` from
  `std::stoi` but not `std::out_of_range`, so a very large number at the
  `maxdepth` or `timeout` prompt terminated the program. Found and fixed
  in Session #14.
- [x] `ChessGame::clone()` round-trips through FEN
  (`ui/qml/main/chess_game.cpp:555-567`), dropping move history. Safe only
  because it runs when history is empty; needs a comment or a real copy.
  Commented in Session #20.
- [x] `ChessGame::isLegalMove` (`ui/qml/main/chess_game.cpp:73`)
  duplicates `GameViewModelBase::isLegalMove`
  (`ui/viewmodel/game_viewmodel_base.cpp:107`). *(verified)*
  Removed on the `frontend-cleanups` branch; `GameModel` now uses the
  inherited implementation. See [frontend-cleanups.md](frontend-cleanups.md).
- [x] `GameModel::~GameModel` deletes the engine thread without
  `quit()`/`wait()` (`ui/qml/main/game_model.cpp:336`). Fixed on the
  `qml-engine-thread-shutdown` branch; see
  [qml-engine-thread-shutdown.md](qml-engine-thread-shutdown.md).
  The `qml-tests` branch has a test that aborts without the fix.
- [x] Debug `std::cout` in `ui/wasm/web_game.cpp:120-121`. *(verified)*
  Fixed in Session #16.
- [x] **A castled rook never animates again.** Found on the `qml-tests`
  branch. `PiecesModel::playerMoved` cleared the rook's castling roles
  without emitting `dataChanged`, so the `Piece.qml` delegate kept
  `isCastlingRook` true and its move animations stayed disabled for the
  rest of the game. Emitting the signal alone made it worse: the
  delegate's handler ran on any change and replayed the castling animation
  from column -1. Fixed there in the model and `Piece.qml` together.
- [x] `UISettings::my_flipped` has no initializer
  (`ui/qml/main/ui_settings.hpp`). Found and fixed on the `qml-tests`
  branch.
- [x] `ChessGame::fromPlayers()` ignores its player arguments, because the
  constructor applies the config's players afterwards
  (`ui/qml/main/chess_game.cpp`). Both callers passed matching players, so
  nothing misbehaved. Found and fixed on the `qml-tests` branch.
- [x] **The draw offer dialogs never open in the QML frontend.** Found on
  the `qml-tests` branch. The shared view-model refactoring (`29208c8`)
  moved `DrawByRepetitionStatus` out of `ui/qml/main/ui_types.hpp` and lost
  its `Q_ENUM_NS`, so `DrawByRepetitionStatus.Proposed` is undefined in
  `popups/Dialogs.qml` and the dialogs' `visible` bindings are never true.
  Fixed there with a mirror enum that supplies the keys.
- [x] The About dialog has no OK button. `popups/AboutDialog.qml` sets
  `standardButtons` on a custom footer, which the `Dialog` overwrites with
  its own unset value. It closes only with Escape or a click outside.
  Found and fixed on the `qml-tests` branch.
- [x] The first click after answering a draw offer is lost. The dialog
  opens during the click that caused it and gives focus back to that
  square when it closes, so `main/Board.qml` takes the next click as a
  move target. Found and fixed on the `qml-tests` branch.
- [x] The mobile menu button cannot close the menu. The press closes the
  open menu, then the button's click on release sees it closed and opens
  it again (`ui/qml/main/mobile_main.qml`). Found on the `qml-tests`
  branch. Fixed on the `mobile-menu-toggle` branch; see
  [mobile-menu-toggle.md](mobile-menu-toggle.md).
- [x] **The promotion dropdown reverses its piece order on the wrong
  signal.** `popups/PromoteDropdown.qml` calls `setFirstRow` from
  `onDestinationColumnChanged`, but the order depends on the row: two
  promotions on the same file, one per side, leave the second list drawn
  queen-farthest from the pawn. Cosmetic. Found reviewing the QML on the
  `qml-dialog-heights` branch; see [qml-cleanups.md](qml-cleanups.md).
  Fixed on the `qml-promotion-order` branch with a test; see
  [qml-promotion-order.md](qml-promotion-order.md).
- [x] The New Game and Quit dialogs are too short for their padding
  (`popups/NewGameDialog.qml`, `popups/ConfirmQuitDialog.qml`: height at
  most 150, padding 40), so their text has no room and is drawn outside
  its box. With taller title and button bars, as in the Basic style, it
  crowds the buttons. Found on the `qml-tests` branch.
  Fixed on the `qml-dialog-heights` branch: the dialogs take their height
  from their content. See [qml-dialog-heights.md](qml-dialog-heights.md).
- [x] `ChessGame::setPlayers()` changes the game's players but not
  `config().players` (`ui/qml/main/chess_game.cpp`). Found on the
  `qml-tests` branch.
  Fixed on the `frontend-cleanups` branch.
- [x] `QThread::usleep (200000)` in the engine slot to wait for animation
  (`ui/qml/main/chess_engine.cpp:126`). *(verified)* Fixed in
  [qml-engine-move-delay.md](qml-engine-move-delay.md): `GameModel` holds
  a reply that arrives while the move before it is still animating, so the
  search and the animation overlap and the engine thread never blocks.
- [x] `ViewModelSettings` (`ui/viewmodel/viewmodel_settings.hpp`) appears
  unused; the `userDepth * 2` mapping is written three times.
  Removed and the remaining conversions consolidated on the
  `frontend-cleanups` branch.

### Tests

- [x] **Threat test inner loop runs once.** *(verified)*
  `engine/test/check_test.cpp:44`: `for (auto col = 7; col < 8; col++)`.
  Only column h is checked, so 56 of 64 expected values are dead data.
  Fixed in Session #14.
- [x] Perft asserts node counts for only positions 1 and 2. Positions 3,
  4 and 5 appear in `hash_collision_slow_test.cpp:136-139` for collision
  checks only; position 6 is absent. `MoveCounter` lacks castles,
  promotions and checks. Fixed on the `more-tests` branch; see
  [more-tests.md](more-tests.md).
- [x] No tests for `evaluate.cpp`, `game_status.cpp`, `move_timer.cpp`,
  `output_format.cpp`, the console UI, UCI, view-model or QML C++.
  All but the QML C++ were added on the `more-tests` branch, along with
  tests for `Game::status()`, which had none. The QML C++ followed on the
  `qml-tests` branch, with Qt Test: the models and settings on their own,
  and the real desktop QML driven offscreen by clicks. They pass on the
  Linux, macOS and Windows CI jobs. The menu, the dialogs, an engine move
  and the mobile QML followed; those have not run on CI yet. See
  [qml-tests.md](qml-tests.md).
- [x] `generate_test.cpp` compares `asString()` output; brittle. Fixed on
  the `more-tests` branch.
- [x] React `App.test.tsx:46-85` mock hard-codes `Pawn: 5`; the real enum
  has `Pawn = 1`, `Queen = 5`.
  Fixed in Session #17.

### Build and infrastructure

- [x] **React integrated-build default read before it is set.**
  *(verified)* `CMakeLists.txt:36` uses
  `WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT`, which is assigned at
  lines 49-53. The option has always defaulted OFF, contrary to the docs.
  Fixed in Session #17.
- [x] **Dead `if` hides a bogus target.** *(verified)*
  `ui/console/CMakeLists.txt:5-6` tests `PCH_ENABLED`, a normal variable
  from a sibling scope, and references target `chess`, which does not
  exist. The console binary never gets a PCH. Fixed in Session #11.
- [x] Engine compiles with no warning flags. Fixed in this branch.
- [x] Clear the warning sites the new flags report. Done in Sessions #2
  and #3; the engine now builds warning-free under `-Wall -Wextra`.
- [x] No sanitizer job and no Linux/Clang in
  `.github/workflows/cmake.yml`. `WISDOM_CHESS_ASAN` is unused by CI.
  The missing Debug build was added in Session #12. Closed by the
  `sanitizers` job, see [ci-sanitizers.md](ci-sanitizers.md).
- [x] Linter self-tests (`scripts/linter/tests/run-tests.sh`) are not run
  in CI. `LinterConfig::ignore` is populated and never read.
  The self-tests were added to the CI lint job in Session #17 and the
  unread `ignore` list was removed in Session #20.
- [x] `WISDOM_CHESS_SLOW_TESTS` defaults On (`CMakeLists.txt:33`) but
  README and CLAUDE.md say OFF.
  Fixed in Session #17.
- [x] CLAUDE.md describes `.wisdomstylerc.json` and an automatic feature
  index generator; neither exists. *(verified: no such file)*
  Fixed in Session #17.
- [x] `target_precompile_headers(... PRIVATE PRIVATE ...)` in
  `engine/CMakeLists.txt:84` and the test and bench CMake files.
  Fixed in Session #17.
- [x] `-fno-stack-protector` is `PUBLIC` on the engine
  (`engine/CMakeLists.txt:101`) and propagates to all UI code.
  Fixed in Session #18.
- [x] `cmake_minimum_required` comes after `set(CMAKE_CXX_STANDARD)`
  in the top-level and engine CMake files.
  Fixed in Session #17.

## Implementation Progress

### Session #1

- Wrote this document.
- Added `-Wall -Wextra` for GCC and Clang and `/W4` for MSVC to
  `wisdom-chess-core` as `PRIVATE` options in `engine/CMakeLists.txt`.
- Rebuilt with GCC (Release). The full tree builds and all 88 fast tests
  pass. The new flags report 60 distinct warning sites in the engine, in
  five groups:

  | Warning | Sites | Root cause |
  |---|---|---|
  | `-Wdeprecated-copy` | 50 | `CastlingEligibility` declares `operator=` (`engine/castling.hpp:105`) but not a copy constructor, so every copy of the type warns. Defaulting the copy constructor, or removing the custom assignment, clears all 50. |
  | `-Wreorder` | 2 ctors | `IterativeSearchImpl` (`engine/search.cpp:19`) and `Board (const BoardBuilder&)` (`engine/board.cpp:18`) initialize members out of declaration order. |
  | `-Wunused-parameter` | 5 | `who` in `engine/move.cpp:76`; `src_piece`, `move`, `dst` in `engine/move.cpp:198-201`; `board` in `engine/board.cpp:220`. |
  | `-Wunused-variable` | 2 | `status` in `engine/game.cpp:305`; `src_piece_type` in `engine/board_code.cpp:96`. |
  | `-Wextra` enum/non-enum conditional | 1 | `color == Color::White ? EN_PASSANT_IS_WHITE : 0` in `engine/board_code.hpp:117`. |

  These are left for a follow-up branch so the flag change stays isolated.
  Fixing the `CastlingEligibility` special members alone removes 50 of
  the 60 sites.

### Session #2

- Removed the hand-written `CastlingEligibility::operator=` in
  `engine/castling.hpp`. It did exactly what the implicit assignment does,
  and its presence made the implicit copy constructor deprecated, which was
  the source of all 50 `-Wdeprecated-copy` sites. The type is now trivially
  copyable. Full build clean of that warning, 88 fast tests pass, linter
  clean. The 10 remaining sites (reorder, unused parameter and variable,
  enum/non-enum conditional) are still open.

### Session #3

- Fixed the ten remaining warning sites so the engine builds clean under
  `-Wall -Wextra` with GCC:
  - `-Wreorder`: reordered the `IterativeSearchImpl` initializer list in
    `engine/search.cpp` and the `Board (const BoardBuilder&)` initializer
    list in `engine/board.cpp` to match declaration order. The member
    declaration order in `board.hpp` was left alone so the 120-byte layout
    is unchanged (verified with a `sizeof` probe against both layouts).
    Added a comment on `my_squares` explaining that `Position` and
    `Material` read it through `*this` during construction.
  - `-Wunused-parameter`: removed `who` from `getCastlingRookMove` and,
    since it was only forwarded there, from `applyForCastlingMove`;
    removed `move` and `dst` from `updateAfterRookMove` and marked
    `src_piece` `[[maybe_unused]]` because it is used only in asserts;
    removed `board` from `removeInvalidPawns`.
  - `-Wunused-variable`: deleted `src_piece_type` in
    `BoardCode::applyMove` and `status` in `drawDesiresToRepetitionStatus`.
  - `-Wextra` enum/non-enum conditional: cast both arms of the
    `EN_PASSANT_IS_WHITE` conditional in `BoardCode::setEnPassantTarget`
    to `std::size_t`.
- Full build has zero warnings, 88 fast tests pass, linter clean on all
  touched files.

### Session #4

- Fixed `operator<< (std::ostream&, const CastlingEligibility&)` in
  `engine/castling.cpp`. The queenside branch now appends "eligible" or
  "not eligible" like the kingside branch, and the output gets its closing
  brace, so the format is
  `{ Kingside: eligible, Queenside: not eligible }`.
- Added a "Stream output" test case to
  `engine/test/castling_eligibility_test.cpp` covering all four
  combinations. Fast suite is now 89 tests, all passing.

### Session #5

- Fixed `FenParser::parseEnPassant` in `engine/fen_parser.cpp` to catch
  `CoordParseError`, the type `coordParse` actually throws, so a malformed
  en-passant square now surfaces as a `FenParserError` carrying the
  original message.
- Added a "FEN notation with an invalid en passant square" test case to
  `engine/test/fen_parser_test.cpp` with two subcases: a square off the
  board (`z9`) and a square missing its rank (`e`). Both failed before the
  fix with "threw a DIFFERENT exception" and pass after. Fast suite is now
  90 tests, all passing.

### Session #6

- Added an explicit empty-string guard at the top of `moveParse` in
  `engine/move.cpp`, before the castling-prefix check reads `str[0]`. An
  empty move string now throws `ParseMoveException` with the message
  "Error parsing move: empty string". Note that `std::string::operator[]`
  at `size()` is defined to return a null character, so the old code was
  not undefined behavior in practice; the guard makes the intent explicit
  and keeps the function safe if the parameter type ever changes to
  `string_view`.
- Added an "Empty and whitespace-only input" subcase to
  `engine/test/move_parse_test.cpp` covering `moveParse` with and without
  a color and `moveParseOptional` returning `nullopt`. Fast suite remains
  90 tests, all passing.

### Session #7

- Added `Expects (size_in_mb >= 1)` to the megabyte constructor in
  `engine/transposition_table.cpp`, matching the existing precondition on
  `fromEntries`, plus `Ensures (power_of_2 >= 2)` on the computed entry
  count. The multiplication now casts the `int` to `size_t` explicitly
  instead of relying on the implicit conversion, which would have wrapped a
  negative size to a huge value.
- GSL contract violations terminate in this build rather than throw, so the
  zero case cannot be asserted from doctest. Verified with a standalone
  probe that `fromMegabytes (0)` now aborts instead of constructing an
  empty table.
- Added a "Transposition table sizing" test case to
  `engine/test/transposition_table_test.cpp` pinning that a 1 MB table is a
  non-empty power of two that fits its budget and that the default
  constructor matches `Default_Size_In_Megabytes`. Fast suite is now 91
  tests, all passing.
- The only production caller that takes user input, the UCI `hash` option,
  already clamps to the range 1 to 1024 MB.

### Session #8

- Replaced all twelve GSL `Expects` / `Ensures` uses in the engine. GSL 4.0
  removed `GSL_THROW_ON_CONTRACT_VIOLATION`, so its contract macros always
  call `std::terminate()` and the failures could not be asserted from
  doctest.
- Added three helpers to `engine/global.hpp`, each taking a defaulted
  `std::source_location`:
  - `expects (cond)` throws `PreconditionError`.
  - `ensures (cond)` throws `PostconditionError`.
  - `noexcept_expects (cond)` prints the failure to stderr and calls
    `std::terminate()`.
  Both error types derive from `wisdom::Error`. The message holds the file
  and line, and `extra_info()` holds the function name. The throwing and
  terminating paths live out of line in the new `engine/global.cpp` so the
  inline callers stay small. All three helpers are `constexpr`, so a failed
  condition during constant evaluation is a compile error.
- `noexcept_expects` is used in the functions that are `noexcept`: the
  `CastlingEligibility (uint8_t)` constructor, `MoveList::append` and
  `removeLast`, and `BoardCode::setEnPassantTarget`. These are hot-path
  internal invariants. Keeping them `noexcept` and terminating matches the
  old GSL behaviour exactly, so there is no performance change. Their failure
  paths are deliberately not tested.
- Considered and rejected: throwing from the hot-path checks in Debug builds
  or in tests only. The checks sit in inline header functions, so a
  test-only switch would compile the same inline functions two ways in one
  program, an ODR violation. The fix would have been a second copy of the
  engine library built for the tests, which was not worth it for conditions
  that should never happen.
- `expects` / `ensures` are used everywhere else: the transposition table
  sizing, `History::addPosition` / `removeLastPosition`, and
  `MoveList::front` / `back`.
- New tests: zero and negative megabyte sizes and entry counts below two are
  rejected by `TranspositionTable`; `front()` / `back()` on an empty
  `MoveList` throw; `History` refuses to commit or remove positions while
  tentative positions are pending. This replaces the standalone probe from
  session 7. Fast suite is now 93 tests, all passing, along with the 23 slow
  tests.

### Session #9

Found while syntax-checking the sources with Emscripten's Clang and with GCC
without `NDEBUG`. CI builds only Release and RelWithDebInfo with GCC-style
warnings on the engine target, so none of these were visible there.

- Debug builds of the engine did not compile. The promotion branch of
  `BoardCode::applyMove` asserted on `src_piece_type`, a local that session
  work in "Fix the remaining engine warnings under -Wall -Wextra" removed
  as unused. It only looked unused because Release compiles the `assert`
  away. The assert now calls `pieceType (src_piece)` directly, so there is
  no local to go unused in Release.
- `Board` declared a defaulted copy constructor but no copy assignment, which
  makes the implicit assignment deprecated (`-Wdeprecated-copy` on Clang).
  Added a defaulted copy assignment operator.
- `MoveTimer` was forward declared as a `struct` and defined as a `class`,
  and the wasm `GameSettings` the other way around. Harmless on the Itanium
  ABI but a possible link error under MSVC. The forward declarations now
  match the definitions.
- Removed three unused locals in `ui/wasm/web_game.cpp`.
- Verified by compiling all 58 engine, test, viewmodel, UCI and console
  sources with GCC without `NDEBUG` (no errors) and the engine plus non-Qt UI
  sources with Clang `-Wall -Wextra` (no diagnostics). Release build and all
  116 tests pass.

### Session #10

Warnings GCC raises with `-Wall -Wextra` on the targets that do not normally
get those flags: the tests, the console UI, and header templates they
instantiate.

- The compile-time range check in `narrow` and `narrow_cast`
  (`engine/global.hpp`) compared the value against the target's limits
  directly. With mixed signedness the usual arithmetic conversions made the
  comparison wrong as well as noisy: a constant `-1` narrowed to an unsigned
  type passed the check. Replaced it with `isLosslessConversion<Target>`,
  the same round-trip plus sign test `gsl::narrow` uses at runtime. It works
  for every arithmetic type, including the `char` targets used in
  `board.cpp` and `coord.hpp`, which rules out `std::in_range`.
- Added `engine/test/global_test.cpp` with `static_assert` coverage of the
  in-range, out-of-range and sign-change cases, plus a runtime check that
  `narrow` throws when the value does not fit.
- Removed unused locals in `board_code_test.cpp`, `board_test.cpp` and
  `history_test.cpp`, and the vestigial pointer parameter of the helper in
  `move_list_test.cpp`.
- `loadFen()` in `ui/console/play.cpp` now returns the factory result
  directly instead of `std::move` on a local.
- Verified: GCC without `NDEBUG` and Emscripten's Clang, both with
  `-Wall -Wextra`, report nothing across the engine, tests, viewmodel, UCI
  and console sources. Fast suite is now 95 tests; all 118 tests pass.

### Session #11

- `-Wall -Wextra` (and `/W4` on MSVC) was only applied to the engine
  library, so the tests and every UI could collect warnings unnoticed. Moved
  the flags into `wisdom_chess_enable_warnings()` in the new
  `cmake/Warnings.cmake` and applied it to every project target: the engine,
  the fast and slow tests, `perft`, the benchmarks, the viewmodel library,
  the console, UCI, QML and wasm front ends, and `seed_optimizer`. The
  flags are `PRIVATE`, so they do not leak into dependencies. The linter
  keeps its own stricter set.
- Warnings that turned up, all fixed:
  - `perft::convertMove` used its `who` parameter only inside an `assert`,
    so it was unused in Release. This is the mirror image of the session 9
    bug, where a variable used only in an `assert` broke Debug. The check is
    a precondition on the caller's input in a test helper, so it is now
    `expects (...)` and active in every build mode.
  - QML: `PieceInfo`'s second constructor initialized members out of
    declaration order; `ChessGame::fromPlayers` wrapped a temporary in
    `std::move`, blocking copy elision; `ChessEngine` had two unused locals
    and slot parameters that must stay to match their signals, now marked
    `[[maybe_unused]]`.
  - Qt's generated moc and QML cache sources and the generated WebIDL glue
    compile clean under the flags, so no per-file suppressions were needed.
- Diagnosed while in the CMake files: the console target's precompiled
  header block tested `PCH_ENABLED`, a plain variable set in the engine's
  directory scope and invisible from `ui/console`, and named a target
  `chess` that does not exist. The block was dead code, and would have been
  a configure error had the variable been visible. It now tests
  `WISDOM_CHESS_PCH_ENABLED` and reuses the engine's PCH, as the UCI target
  already did.
- Lesson recorded: a warning pass needs both modes. Building without
  `NDEBUG` finds code that only compiles inside `assert`; building with it
  finds names that are only used inside `assert`.
- Verified: desktop Release build (GCC), QML build against Qt 6.11.2 (59
  objects) and wasm build under Emscripten's Clang (32 objects) all report
  no warnings; a GCC syntax pass without `NDEBUG` over the engine, tests,
  console, UCI, viewmodel and tools reports none either. All 118 tests
  pass. The benchmarks target was checked afterwards by configuring with
  `-DWISDOM_CHESS_BENCHMARKS=ON`, which has CPM fetch nanobench (there is no
  apt package, and none is needed); it also builds with no warnings. Not
  verified: MSVC `/W4`, which only CI can show.

### Session #12

- Added one Debug job to `.github/workflows/cmake.yml`: Ubuntu, GCC. CI
  previously built only Release and RelWithDebInfo, both of which define
  `NDEBUG`, so the broken `assert` from session 9 could not have been caught
  there. The job is a single extra `include` entry in the build matrix; the
  other six jobs are unchanged.
- The Debug job configures with `WISDOM_CHESS_SLOW_TESTS=Off` through a new
  optional `slow_tests` matrix value that defaults to `On`. The slow suite
  is not usable without optimization, so Debug runs only the fast tests.
  The QML UI stays on in that job so asserts in the UI code compile too.
- Verified locally with a Debug tree built from the same sources: no
  warnings under `-Wall -Wextra`, and all 95 fast tests pass in well under
  a second. The workflow file parses and the new entry expands as intended.
  The job itself has not run on GitHub yet.

### Session #13

Warnings from the first CI run with warnings enabled on every target (run
35442137275, commit `35e42e6`). All eight jobs passed. macOS reported three
warning lines and Windows 108, from seven distinct sites.

- Added `truncate<Target> (value)` to `engine/global.hpp`: a named conversion
  to a narrower unsigned type that deliberately discards the high bits.
  `narrow_cast` cannot be used for that, because its compile-time check
  rejects lossy conversions. `truncate` refuses signed types and widening
  at compile time. Covered in `engine/test/global_test.cpp`.
- `engine/random.hpp` produced 96 of the Windows lines, two warnings
  repeated for every file that includes it: a 64 to 32 bit truncation
  (C4244) and unary minus on an unsigned value (C4146). Both are the
  standard PCG output step. The truncations now use `truncate`, and the
  rotate amount is written `(32u - rot) & 31u`, which is the same value
  modulo 32. Because these numbers seed the hash tables, the change was
  verified with a probe comparing sampled outputs and a checksum over 1.2
  million draws before and after: identical.
- `engine/test/check_test.cpp` passed loop `int`s to `isKingThreatened`,
  which takes `int8_t`. It now converts with `narrow<int8_t>`.
- `tolower` returns `int`: `ui/uci/uci_interface.cpp` now uses
  `narrow_cast<char>` like `board.cpp`, and `scripts/linter/linter.cpp` uses
  a small lambda instead of passing `::tolower` to `std::transform`. Checked
  that the UCI engine still prints a lowercase promotion, `a7a8q`.
- `ui/console/play.cpp`: dropped the unused name from a catch clause (C4101).
- macOS `-Wpessimizing-move`: two `std::move` calls around temporaries in
  `ui/qml/main/game_model.cpp`, which GCC does not flag because they are an
  assignment and a converting initialization rather than returns.
- macOS `ld: ignoring duplicate libraries` on the console: it linked the
  engine directly and again through `wisdom::viewmodel`, which links it
  publicly. The console now names only the viewmodel library.
- Left alone: eight C4702 "unreachable code" warnings that come from inside
  Qt 6.9.3's own headers under `/W4`.
- Verified: desktop (GCC), QML (Qt 6.11.2) and wasm (Emscripten Clang)
  builds report no warnings, all 119 tests pass, and the linter's own 19
  tests pass. The MSVC and AppleClang results can only be confirmed by the
  next CI run.

### Session #14

- Threat test (`engine/test/check_test.cpp`): the inner loop now covers all
  eight columns, using `Num_Rows` and `Num_Columns`. The `INFO` line that
  names the square is enabled, and the checks are `CHECK` so that every
  mismatch is reported, not only the first. All 128 comparisons
  pass, so the 56 previously unchecked table values were correct and the
  engine agrees with them. The defect was only in the test's coverage.
- Console number prompts: reproduced first. Typing
  `99999999999999999999` at the `maxdepth` prompt aborted the console with
  an uncaught `std::out_of_range` from `std::stoi`. `readInt` now uses the
  engine's `toInt`, which reports failure through its `optional` result, so
  the exception handling is gone. The same input now prints "Invalid
  search depth." and the session continues; an empty line is rejected the
  same way and a valid depth is still accepted.
- Extended the `toInt` test in `engine/test/str_test.cpp` with empty input
  and values just past and far past the `int` range, since the console now
  relies on that behaviour.
- The two `std::stoi` calls in `ui/uci/uci_interface.cpp` were checked and
  left alone; both already catch every exception.
- Checklist housekeeping: marked the console precompiled-header item as
  fixed in Session #11 and noted that Session #12 added the Debug build
  named in the CI item, which stays open for the sanitizer and Linux Clang
  jobs.
- Verified: no warnings in the desktop build, all 119 tests pass, linter
  clean on the changed files.

### Session #15

Worked through the contained items left on the checklist, engine first.

- `CompileTimeRandom::max()` returned `numeric_limits::min()`, so the type
  reported an empty range. It now returns `max()`; pinned with
  `static_assert`s in `engine/test/global_test.cpp`.
- `MoveList (Color, initializer_list<czstring>)` was `noexcept` but calls
  `moveParse`, which throws, so a bad move string terminated the program.
  Dropped the `noexcept`; a test now expects `ParseMoveException`.
- FEN parser: a ninth rank slipped past `row > Num_Rows` and failed later
  with the wrong exception type; the check is now `>=`. `parseCastling`
  ignored unknown letters; it now throws `FenParserError`. Tests cover
  both, plus the valid `KQkq` and `-` forms.
- `Board::withRandomPosition` kept the original castling rights. It also
  kept the en passant target and the cached material and position scores,
  which were stale for the same reason. All four are now reset or
  recomputed after the shuffle, and the randomized-board test checks them.
- Search timing in `engine/search.cpp` uses `steady_clock`, matching
  `MoveTimer`, so a wall-clock adjustment can no longer produce a negative
  or inflated search time in the log.
- Removed the no-op `castled_state += "";` in `engine/board.cpp`.
- Not changed: `Error`'s `noexcept` copy constructor. For an exception type
  that is the conventional choice, since a throwing copy during a `throw`
  terminates anyway. The item stays open as a judgement call.

### Session #16

Frontend items from the checklist.

- `ui/wasm/web_game.cpp` threw `new Error`, a pointer no `catch (Error&)`
  matches and which leaks. It now throws by value. Also removed two debug
  `std::cout` lines from `setComputerDrawStatus`.
- Console draw prompt (`ui/console/play.cpp`) tested `input[1] == 'Y'`, so
  a capital `Y` declined the draw. It now compares `toupper (input[0])`.
  The same loop spun forever once stdin reached end of file; it now treats
  that as declining.
- `PiecesModel::playerMoved` removed rows inside a forward loop without
  adjusting the index, then kept using a reference the removal had
  invalidated. After a removal the loop now steps the index back and, for
  the captured piece, continues with the next iteration, so every piece is
  visited once and no stale reference is read.
- `mobile_main.qml` logged `uiSettings.squareSize`, which does not exist
  and raised a ReferenceError on every orientation change. It now reads
  `boardDimensions.squareSize`.
- UCI `stop`. `handleStop` bumped the search id, the same signal used when
  a search is superseded, so the search thread stayed silent and no
  `bestmove` followed, which the protocol requires. `stop` now sets its own
  flag. The periodic callback reacts to it by setting the timer's budget to
  zero through the existing `setSeconds`, so the search ends through its
  normal timeout path and iterative deepening returns the last completed
  depth. Cancelling was not an option, because `Game::findBestMove`
  deliberately discards a cancelled search, and `MoveTimer`'s interface was
  left unchanged. When the search returns no move at all (stop before depth
  1 completes, or a very short time limit) the UCI front end picks a random
  legal move, so `bestmove (none)` is only sent when there are no legal
  moves.
  Checked by driving the binary: stop after two seconds returns a searched
  move; an immediate stop returns a legal move; a superseded search still
  produces exactly one `bestmove`; a checkmated position returns `(none)`;
  a stop does not leak into the following `go`.
- Follow-up worth its own design: the search exposes nothing per depth, so
  no front end sees progress and UCI cannot emit `info depth ... score ...
  pv ...` lines (it currently prints `info Searching depth N`, which is not
  valid UCI syntax). A per-depth result callback would fix that and let
  UCI answer `stop` from its own record of the latest depth.
- Verified: desktop, QML and wasm builds with no warnings, all 122 tests
  pass, linter clean. Not exercised at runtime: the QML list fix, the QML
  log line and the console draw prompt, which need a GUI session or a draw
  offer to reach.

### Session #17

The small React, CMake and documentation items.

- React: `startNewGame` in `App.tsx` now clears `thirdRepetitionDrawAnswered`
  and `fiftyMovesDrawAnswered`, so the draw dialog can appear in every game
  and not only once per page load. A new test in `App.test.tsx` answers
  the dialog, starts a new game and expects the dialog again; it was
  confirmed to fail with the fix removed.
- React: the test mock's piece values now match `WebPiece` in
  `ui/wasm/web_types.hpp` (`NoPiece = 0`, `Pawn = 1` ... `Queen = 5`). The
  mock had `Queen: 0` and `Pawn: 5`.
- CMake: `WISDOM_CHESS_REACT_BUILD_INTEGRATED` read its default variable
  before that variable was set, so the option always defaulted to OFF. The
  default is now computed before the `option()` calls. Checked with fresh
  configures: ON under Emscripten, OFF natively.
- CMake: `WISDOM_CHESS_SLOW_TESTS` now defaults to Off. The README,
  `AGENTS.md`, the build scripts and every CI job already assumed Off and
  pass the option explicitly when they want the slow tests, so the code
  was the odd one out. Existing build trees keep their cached value.
- CMake: `cmake_minimum_required` now comes first in the top-level and
  engine files, and the doubled `PRIVATE PRIVATE` in four
  `target_precompile_headers` calls is a single `PRIVATE`.
- CI: the lint job runs the linter's own test suite before linting.
- `AGENTS.md` (formerly `CLAUDE.md`): removed the claims that the linter
  reads `.wisdomstylerc.json` and that feature indexes are generated
  automatically; neither exists. The WebAssembly configure example used
  five `WISDOM_CHESS_BUILD_*` options that do not exist and were silently
  ignored; it now uses the real option names, matching
  `scripts/build-react-wasm.sh` and `web.yml`.
- Left open: `-fno-stack-protector` being `PUBLIC` on the engine, which is a
  hardening decision for the UI targets, not a cleanup.
- Verified: desktop build with no warnings and all 122 C++ tests passing,
  `tsc` clean, 30 React tests passing, workflow YAML parses, linter
  self-tests pass 19 of 19 when run the way the new CI step runs them.

### Session #18

- `-fno-stack-protector` is now `PRIVATE` on `wisdom-chess-core`. It was
  `PUBLIC`, so every target linking the engine, including the tests and
  all the UIs, also had the stack protector switched off. The engine's own
  sources, where the search and move generation run, keep the flag.
- Checked the generated flags: only the engine target carries it now; the
  tests, console, UCI, viewmodel and QML targets do not. The console and
  UCI targets reuse the engine's precompiled header, and CMake builds with
  `-Winvalid-pch`; no warning appeared, so the header is still accepted
  despite the differing flag.
- The slow suite took 50.95 processor-seconds against 50 to 53 in earlier
  runs this session, so no measurable change. Desktop and QML builds have
  no warnings and all 122 tests pass.

### Session #19

- The macOS build on PR #235 failed with `stack protector mode differs in
  PCH file vs. current file`. The console and UCI targets reused the
  engine's precompiled header (`REUSE_FROM wisdom-chess-core`), which is
  now built with `-fno-stack-protector` while they are not. GCC accepts
  the mismatch, which is why Session #18 saw no problem. Clang rejects it,
  and it only shows up where the stack protector is on by default, as
  with Apple clang.
- Reproduced on Linux with clang 18 and `-fstack-protector-strong`, then
  gave both targets their own precompiled header of `global.hpp` instead
  of reusing the engine's. They keep the stack protector.
- Verified: clang build with `-fstack-protector-strong` and GCC build
  both succeed with no warnings, and all 99 fast tests pass under each.

### Session #20

The earlier work was merged in PRs #233 and #235 and the branch deleted, so
this session restarts the branch from `main`. Scope: the quick items only.

- Piece-square tables: `translatePosition` in `engine/position.cpp` now
  mirrors only the rank for Black. A new test in `position_test.cpp` builds
  a position whose ranks are mirrored between the colors and expects equal
  scores; without the fix it reported 9 against 4.
- Search: `search()` writes `my_current_result` only at `ply == 0`. The
  writes on the transposition-table hit path were dead, since that path
  runs only at `ply > 0`. `my_search_depth - depth` was always equal to
  `ply`, so the member is gone and `ply` is used directly. The result's
  `depth` field, which nothing reads, now holds the searched depth; it was
  always zero before.
- Repetition check: `History::isProbablyNthRepetition` looks back at most
  `half-move clock + 1` positions, because a position cannot recur across
  a capture or pawn move. The existing repetition tests and the slow suite
  pass unchanged.
- Castling: added `castlingRookMove (Move king_move)` to `engine/move.hpp`
  and used it in `Board::applyForCastlingMove`, `BoardCode::applyMove` and
  `Position::applyMove`. `Board::getCastlingRookMove` and
  `castlingRowFromColor` are gone. Tested for all four castling moves.
- `Game::load` returns `nullopt` for an unparseable move as well as for a
  file that cannot be opened, and no longer prints. The console already
  reports "Error loading game." for `nullopt`. New tests cover a missing
  file, the `stop` marker and a bad move. It still does not check that the
  moves are legal.
- `Board::withRandomPosition` puts the board in the `Error`'s extra info
  instead of printing it.
- Const-correctness: `Coord::index()`, `row()`, `column()`, every
  `InlineThreats` method, and `operator<< (ostream&, const Position&)`.
- `CoordIterator`: `reference` is `Coord`, postfix `++` added, `begin()`
  returns the iterator itself so a constructed starting coordinate is
  honoured, and a `static_assert` pins `std::forward_iterator`.
- `compareMoves` reuses the material differences it already computed, and
  the sort lambda captures `MoveGeneration` by reference.
- Removed unreferenced code: `Board::pieceAtIndex`, `Board::squareData`,
  `MoveList::fromZeroInitialized`, `MoveList::data`, `BoardCode::withMove`,
  `TranspositionTable::getStoredEntriesCount`, `MoveGeneration::none()`,
  and the unused parameters of `isProbablyDrawingMove`.
- Tabs replaced with spaces in the five listed engine files and
  `position_test.cpp`.
- React: `Board.tsx` declares `onDropPiece` as `(src, dst)`, matching
  `Square.tsx` and the handler in `App.tsx`; `PawnPromotionDialog.tsx`
  holds its selection as `PieceType`.
- QML: documented on `ChessGame::clone()` that the move history is not
  copied.
- Linter: removed the unread `LinterConfig::ignore`. `run-tests.sh` changed
  directory before using the linter path, so the relative path documented
  in `AGENTS.md` failed all 19 tests; it now resolves the path first.
- Verified: Release and Debug desktop builds and the QML build have no
  warnings; all 136 C++ tests pass (113 fast, 23 slow); `tsc` is clean and
  the 30 React tests pass; the linter and its 19 self-tests pass.
- Search benchmarks: added `engine/bench/bench_search.cpp` to the
  benchmarks target. It searches to depth 6 under nanobench, and once to
  depth 8 with manual timing, from the starting, Kiwipete and Italian
  positions and from two scripted games of 80 and 200 non-capturing plies,
  so the repetition check has a long history to scan. The transposition
  table is cleared before every search. The suite now takes about 16
  seconds, up from about 2.
- Measured the search changes with it: the commit before them against the
  branch, six alternating rounds, comparing within each round because the
  laptop's clock speed drifted 10 to 24% over the run even with the
  `performance` governor. The long-history searches were faster in every
  round: median 8 to 10% at 80 plies and 11% at 200 plies. The three
  short-history positions had medians of 2 to 4% faster with ranges that
  cross zero, so no detectable change. The chosen moves and scores were
  identical. The move generation, threat, legality and perft benchmarks
  compared against `main` all had medians within 4% and ranges that cross
  zero.
- Review finding on the bounded repetition scan: the FEN parser accepted a
  negative half-move clock, which made the scan compute an iterator past
  the end of the history. `FenParser` now throws `FenParserError` for a
  negative half-move clock or full-move number, and
  `BoardBuilder::setHalfMovesClock` / `setFullMoves` throw
  `BoardBuilderError`, so no `Board` can hold a negative clock. Tests
  cover both layers.
- Second review finding on the same scan: adding one to the clock before
  clamping overflowed for a clock of `INT_MAX` where `ptrdiff_t` is 32
  bits, as under Emscripten. The bound is now
  `clock < history_size ? clock + 1 : history_size`, which cannot overflow
  for any `int` and costs the same single comparison. Not reproduced on
  wasm32; the 64-bit test only documents the case.
- The same input exposed an older overflow: `Board::updateMoveClock`
  increments both clocks, so a move from a position loaded with a clock of
  `INT_MAX` was undefined behaviour. Normal play cannot get near that; the
  draw rules end a game at a half-move clock of 150, and the longest legal
  game is under 9,000 moves. Added `Max_Half_Move_Clock` and
  `Max_Full_Move_Number`, both 10,000, to `engine/global.hpp`. The FEN
  parser and the board builder reject larger values the same way they
  reject negative ones, and `updateMoveClock` asserts that neither clock
  is at `INT_MAX`. Considered and rejected: `noexcept_expects` in the
  scan. A pasted FEN is caller input, which should throw rather than
  abort, and the check would run at every search node. A limit of 150 was
  also rejected because `Game::move` does not stop at a draw, so the
  engine can load and analyse positions past it today.
- Left for discussion, as each needs a design decision or runtime
  checking: quiescence search, the per-search transposition table copy,
  full move generation at leaf nodes in check, whether
  `iterativelyDeepen` should propagate errors, `Error`'s `noexcept` copy
  constructor, the WebIDL leaks and `any` types, the QML engine-thread
  shutdown and `usleep`, the duplicated `isLegalMove`, `ViewModelSettings`,
  the missing perft and unit-test coverage, and the sanitizer and Linux
  Clang CI jobs.

### Session #21

Leaked WebIDL objects. The generated glue wraps every pointer a C++
function returns and never frees it; only an explicit `destroy()` does.

- Sites found, one more than the checklist recorded:
  - `getCurrentGameSettings()` allocates a `GameSettings`. `App.tsx` called
    it inside the `useReducer` initial-state argument, which React
    evaluates on every render and then ignores, so the app leaked one
    object per render and not just one at startup. A short test that
    opens the About dialog saw three allocations. It is also called when a
    new game starts.
  - A human move allocates two `WebCoord`s and a `WebMove`, including on
    the paths that return early for a promotion prompt or an illegal move.
  - A computer move allocates a `WebMove` through `WebMove::fromString`.
  - Applying settings allocates a `GameSettings` with `new`.
  - `WebMove::asString` returned `strdup` memory that the glue copies into
    a JavaScript string and never frees. Nothing in the React code called
    it.
- Checked first that C++ keeps none of these pointers: `makeMove`,
  `isLegalMove`, `needsPawnPromotion` and `notifyHumanMove` read the value
  during the call, and both settings setters copy.
- Fix, in `ui/react/src/lib/WisdomChess.ts`: `getCurrentGameSettings()` now
  copies the fields into a plain `WebGameSettings` and destroys the C++
  object, so callers never hold one, and `App.tsx` no longer needs its own
  `toWebSettings`. A new `withWasmObjects (objects, callback)` runs the
  callback and destroys every listed object afterwards, also when the
  callback throws or returns early; the move handlers and the settings
  handler use it. The reducer's initial state uses the lazy initializer
  form, so the settings are read once.
- Removed `WebMove::asString` from the IDL, `web_move.hpp` and the
  TypeScript type instead of fixing it, since it had no caller.
- Tests: the settings are read once and freed across re-renders (fails
  with three calls when the eager initializer is restored), the computer
  move and the applied settings object are freed, and `withWasmObjects`
  frees late additions, frees on a throw and skips empty entries. 35 React
  tests pass and `tsc` is clean. The wasm target builds without warnings
  and the regenerated glue has no `asString`. Not exercised in a browser.
- The interface still made JavaScript own short-lived C++ objects, so
  every new call site would have to remember to free them. Session #22
  removes those objects from the interface.

### Session #22

Removed `WebCoord` and `WebMove` from the WebIDL interface so that
JavaScript owns no short-lived C++ objects.

- Strings need no `destroy()`. For a `DOMString` argument the glue copies
  the text into a scratch buffer it owns and rewinds on the next call. For
  a `DOMString` result it copies the bytes into a JavaScript string and
  never frees the pointer, so C++ must return memory it already owns, as
  `getMoveStatus()` does with a member string. `asString` broke that rule,
  which is why it could only be fixed in C++.
- New `WebGame` interface:
  - `needsPawnPromotion (DOMString src, DOMString dst)`.
  - `makeHumanMove (DOMString src, DOMString dst, long pieceType)` maps the
    squares to a move, checks that it is legal and the human's turn, makes
    it, and returns it packed with `Move::toInt()`, or `-1`
    (`WebGame::Illegal_Move`, `ILLEGAL_MOVE` in TypeScript). It replaces
    `fromTextCoord` twice, `createMoveFromCoordinatesAndPromotedPiece`,
    `isLegalMove` and `makeMove`.
  - `makeComputerMove (DOMString moveText)` takes the text the engine
    worker already sends, replacing `WebMove.fromString` and `makeMove`.
  - `GameModel.notifyHumanMove (long packedMove)`.
- The move goes to the worker as an integer, not as text, for two reasons:
  the worker call `emscripten_wasm_worker_post_function_vi` carries only
  integers and already sent `toInt()`, and parsing move text needs the
  side to move, which `GameModel` does not know.
- `web_move.hpp` and `WebCoord` are deleted. `App.tsx`'s move handler is
  about a third of its previous length and has no cleanup to get wrong.
  `withWasmObjects` remains for the one `GameSettings` object that
  applying settings builds.
- Found while checking in a browser: the wasm target passes
  `-sNO_DISABLE_EXCEPTION_CATCHING` only in Debug, so in Release a C++
  `catch` never runs and every `throw` surfaces in JavaScript as a raw
  pointer. A `try`/`catch` around `coordParse` therefore did nothing. Added
  `coordParseOptional`, a `noexcept` parser returning `optional<Coord>`, to
  `engine/coord.hpp`, with `coordParse` now built on it, mirroring
  `moveParseOptional`. The wasm layer uses it, also guarding a null
  pointer, so a malformed square is reported as an illegal move. Any other
  C++ `catch` in the wasm build is equally inert in Release.
- The old code threw `Error` for a move that could not be mapped and relied
  on a JavaScript `try`/`catch`; that path now returns `-1` without
  throwing. As before, only a mapped but illegal move sets the "Illegal
  move" status.
- The test mock numbered the colors `White: 0, Black: 1, NoColor: 2`; the
  real enum is `NoColor: 0, White: 1, Black: 2`, which `fromNumberToColor`
  depends on. It only surfaced once a test put a piece on the board.
  Corrected, the same kind of error as the piece values in Session #17.
- Tests: `coordParseOptional` in `coord_test.cpp`. In `App.test.tsx`, a
  human move driven through clicks calls `makeHumanMove ('e2', 'e4',
  Queen)` and passes the result to `notifyHumanMove`; an illegal move does
  not notify the engine; a promotion is requested before any move is made;
  a computer move is passed on as text; a message from an earlier game is
  ignored.
- Verified: 137 C++ tests and 39 React tests pass, `tsc` and the linter
  are clean, and the wasm target and the production React bundle build
  without warnings. Also run in headless Chromium against the Vite dev
  server, driven over the DevTools protocol: the old names are gone from
  the module; a malformed square and an illegal move both return `-1`;
  clicking the e2 pawn and then e4 made the move, the engine replied
  1...d5, and the status bar updated; Settings then Apply, and New Game
  then Start New Game, both worked; no console errors or uncaught
  exceptions. Not checked there: promotion, drag and drop, the draw
  dialogs, and other browsers.
- The `any` item was done afterwards on the `wasm-boundary-types` branch.
