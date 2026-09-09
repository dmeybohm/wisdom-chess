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

- [ ] **Piece-square tables mirrored on both axes for Black.** *(verified)*
  `translatePosition` in `engine/position.cpp:76-90` flips row and column,
  but `pawn_positions` (row 5: `+1 -1 -2 0 0 2 -1 +1`), `king_positions`
  (`-6 -8 -8 -9 -9 -4 -4 -6`) and `bishop_positions` are not left-right
  symmetric. Black therefore prefers the opposite wing from White. Mirror
  only the rank, or make the tables symmetric.
- [ ] **FEN en-passant parser catches the wrong exception.** *(verified)*
  `engine/fen_parser.cpp:113` catches `BoardBuilderError`, but
  `coordParse` throws `CoordParseError` (`engine/coord.hpp:203,209`). The
  rewrap to `FenParserError` never fires.
- [ ] **`moveParse` indexes `str[0]` before the empty check.** *(verified)*
  `engine/move.cpp:510` reads `str[0]`; the empty guard is inside
  `moveParseOptional` (`engine/move.cpp:409`), which runs afterwards.
  Reachable from `Game::load` with a blank line.
- [ ] **`CastlingEligibility` stream operator appends a bool as a char.**
  *(verified)* `engine/castling.cpp:18`:
  `result += value.isSet (CastlingRights::Queenside);` emits `\x00` or
  `\x01` instead of "eligible" / "not eligible".
- [ ] **`TranspositionTable (int size_in_mb)` has no lower bound.**
  *(verified)* `engine/transposition_table.cpp:11-23`: size 0 yields
  `power_of_2 == 0`, so `my_size_mask` becomes `SIZE_MAX` and every probe
  indexes out of bounds. `fromEntries` has `Expects (entry_count >= 2)`;
  this constructor needs the same.
- [ ] **`Board::withRandomPosition` keeps stale castling rights.**
  *(verified)* `engine/board.cpp:291` rebuilds the board code from the
  shuffled squares but the castling eligibility is unchanged. A later
  castling move would hit the assert in `getCastlingRookMove`
  (`engine/move.cpp:104`) or move a non-rook in release. Test-only path.
- [ ] `CompileTimeRandom::max()` returns `numeric_limits::min()`
  (`engine/random.hpp:58-63`). Violates *UniformRandomBitGenerator*.
- [ ] `parseCastling` silently ignores unknown letters
  (`engine/fen_parser.cpp:135-143`, no `default`); `parsePieces` checks
  `row > Num_Rows` instead of `>=` (`engine/fen_parser.cpp:70,83`), so
  `row == 8` reaches `BoardBuilder::addPiece` and throws the wrong type.
- [ ] `Board::Board (const BoardBuilder&)` initializer order does not
  match declaration order and `my_position { Position { *this } }` reads a
  partially constructed object (`engine/board.cpp:19-25`). Works only
  because `my_squares` is declared first. Needs a comment at minimum.
- [ ] `MoveList (Color, std::initializer_list<czstring>) noexcept` calls
  `moveParse`, which throws (`engine/move_list.hpp:22-30`). Drop the
  `noexcept`.
- [ ] `Error`'s copy constructor is `noexcept` but copies two strings
  (`engine/global.hpp:205-207`).

### Engine: search and performance

- [ ] **No quiescence search.** `search()` calls `evaluate()` at
  `depth <= 0` (`engine/search.cpp:159-162`). `iterativelyDeepen` then
  discards every odd-depth result (`engine/search.cpp:295-296`) as a
  substitute, throwing away roughly half the search time.
- [ ] **Repetition check scans the whole history at every node.**
  `isProbablyDrawingMove` calls `History::isProbablyNthRepetition`, which
  does `std::count` over all board codes (`engine/history.hpp:94-101`).
  Bound the scan by the half-move clock.
- [ ] **Search result passed through a member overwritten at every ply.**
  `my_current_result` is written at every node
  (`engine/search.cpp:174-176, 226-234`); correctness depends on the root
  call writing last. Write it only at `ply == 0` or return a struct.
- [ ] **Transposition table deep-copied per search.** The TT is a value
  member of `Game::Impl` (`engine/game_impl.hpp:30`) and `Game`'s copy
  constructor copies it (`engine/game.cpp:77-80`). The UCI frontend copies
  the `Game` on every `go` (`ui/uci/uci_interface.cpp:295-301`), copying
  about 12 MB and discarding the learned table.
- [ ] Leaf evaluation calls full legal-move generation when in check
  (`engine/evaluate.cpp:63, 87-97`).
- [ ] `compareMoves` recomputes `materialDiff` in the return
  (`engine/generate.cpp:538-542`); the sort lambda captures
  `MoveGeneration` by value (`engine/generate.cpp:564-568`).
- [ ] Search timing uses `system_clock` (`engine/search.cpp:334, 340`);
  use `steady_clock` as `MoveTimer` already does.
- [ ] `MoveList::data()` returns the 504-byte array by value
  (`engine/move_list.hpp:172-176`). Unused today.

### Engine: error handling and hygiene

- [ ] `iterativelyDeepen` catches `Error`, prints to `std::cerr` and calls
  `std::terminate()` (`engine/search.cpp:306-312`). Let it propagate.
- [ ] Direct `std::cout`/`std::cerr` in library code bypassing `Logger`:
  `engine/board.cpp:31, 286`, `engine/game.cpp:253`,
  `engine/search.cpp:308-309`.
- [ ] `Game::load` returns `nullopt` on open failure but throws on a bad
  move (`engine/game.cpp:251-255, 266`). Pick one.
- [ ] `Coord::index()`, `row()`, `column()` are not `const`
  (`engine/coord.hpp:58-81`); all `InlineThreats` methods are non-`const`
  (`engine/threats.hpp:36-265`); `operator<< (ostream&, Position&)` takes
  a non-const reference (`engine/position.hpp:32-34`).
- [ ] Duplicated castling rook column logic in `Board::getCastlingRookMove`
  (`engine/move.cpp:73-107`), `BoardCode::applyMove`
  (`engine/board_code.cpp:102-121`) and `Position::applyMove`
  (`engine/position.cpp:194-210`).
- [ ] `castlingRowForColor` (`engine/move.hpp:344-351`) and
  `castlingRowFromColor` (`engine/position.cpp:92-105`) are the same
  function.
- [ ] Dead code: `DrawStatus status;` (`engine/game.cpp:305`),
  `castled_state += "";` (`engine/board.cpp:151`), unused parameters on
  `applyForCastlingMove` and `isProbablyDrawingMove`, unreferenced
  `Board::pieceAtIndex`, `Board::squareData`, `MoveList::fromZeroInitialized`,
  `BoardCode::withMove`, `TranspositionTable::getStoredEntriesCount`,
  `MoveGeneration::none()`.
- [ ] `CoordIterator` (`engine/coord.hpp:216-279`) claims
  `forward_iterator_tag` but has no postfix `++`, its `reference` is
  `Coord&` while `operator*` returns by value, and `begin()` ignores the
  stored coordinate.
- [ ] Mixed tabs and spaces: `engine/evaluate.hpp:78-81`,
  `engine/evaluate.cpp:99-103`, `engine/game.hpp:36-59`,
  `engine/board_code.hpp:20-25`, `engine/search.cpp:292-296`. Candidate
  for a linter rule.

### Frontends

- [ ] **Pointer thrown instead of exception.** *(verified)*
  `ui/wasm/web_game.cpp:82`: `throw new Error { "Failed to map move." };`.
  No C++ `catch (Error&)` will match and the object leaks.
- [ ] **Console draw prompt tests the wrong character.** *(verified)*
  `ui/console/play.cpp:231`: `input[0] == 'y' || input[1] == 'Y'`. A
  capital `Y` is treated as declining.
- [ ] **UCI `stop` suppresses `bestmove`.** *(verified)*
  `handleStop` (`ui/uci/uci_interface.cpp:368-371`) bumps `my_search_id`,
  so the `if` at line 319 skips `sendBestMove`. The UCI protocol requires
  `bestmove` after `stop`.
- [ ] **Draw-answered flags never reset on new game.** *(verified)*
  `thirdRepetitionDrawAnswered` and `fiftyMovesDrawAnswered`
  (`ui/react/src/App.tsx:312-313`) are not cleared in `startNewGame`
  (`App.tsx:281-290`), so the draw dialog appears at most once per session.
- [ ] **`PiecesModel::playerMoved` skips an element after removal.**
  *(verified)* `ui/qml/main/pieces_model.cpp:160-176` calls
  `my_pieces.removeAt (i)` inside a forward loop without adjusting `i`,
  and then reads the `piece_model` reference it just invalidated.
- [ ] **`uiSettings` is undefined in `mobile_main.qml`.** *(verified)*
  `ui/qml/main/mobile_main.qml:46` logs `uiSettings.squareSize`; the
  property lives on `_myGameModel`. ReferenceError on every orientation
  change.
- [ ] **Leaked WebIDL objects.** `getCurrentGameSettings()` returns a
  `new GameSettings` (`ui/wasm/game_model.hpp`) that `App.tsx:106, 287`
  never destroys; `App.tsx:184-185, 195` allocates three objects per move
  and frees none. `WebMove::asString` returns `strdup`
  (`ui/wasm/web_move.hpp:43`). *(strdup verified)*
- [ ] **`any` at the WASM boundary.** *(verified)*
  `ui/react/src/lib/WisdomChess.ts:95-143` declares `Game`, `PieceColor`,
  `PieceType`, `GameStatus`, `WebMove`, `WebCoord` and others as `any`.
  Consequences that compile today: `onDropPiece` is declared
  `(dst, src)` in `Board.tsx:23` but `(src, dst)` in `Square.tsx:16`;
  `PawnPromotionDialog.tsx` types `selectedPiece` as `PieceColor`.
- [ ] `ChessGame::clone()` round-trips through FEN
  (`ui/qml/main/chess_game.cpp:555-567`), dropping move history. Safe only
  because it runs when history is empty; needs a comment or a real copy.
- [ ] `ChessGame::isLegalMove` (`ui/qml/main/chess_game.cpp:73`)
  duplicates `GameViewModelBase::isLegalMove`
  (`ui/viewmodel/game_viewmodel_base.cpp:107`). *(verified)*
- [ ] `GameModel::~GameModel` deletes the engine thread without
  `quit()`/`wait()` (`ui/qml/main/game_model.cpp:336`).
- [ ] Debug `std::cout` in `ui/wasm/web_game.cpp:120-121`. *(verified)*
- [ ] `QThread::usleep (200000)` in the engine slot to wait for animation
  (`ui/qml/main/chess_engine.cpp:126`). *(verified)*
- [ ] `ViewModelSettings` (`ui/viewmodel/viewmodel_settings.hpp`) appears
  unused; the `userDepth * 2` mapping is written three times.

### Tests

- [ ] **Threat test inner loop runs once.** *(verified)*
  `engine/test/check_test.cpp:44`: `for (auto col = 7; col < 8; col++)`.
  Only column h is checked, so 56 of 64 expected values are dead data.
- [ ] Perft asserts node counts for only positions 1 and 2. Positions 3,
  4 and 5 appear in `hash_collision_slow_test.cpp:136-139` for collision
  checks only; position 6 is absent. `MoveCounter` lacks castles,
  promotions and checks.
- [ ] No tests for `evaluate.cpp`, `game_status.cpp`, `move_timer.cpp`,
  `output_format.cpp`, the console UI, UCI, view-model or QML C++.
- [ ] `generate_test.cpp` compares `asString()` output; brittle.
- [ ] React `App.test.tsx:46-85` mock hard-codes `Pawn: 5`; the real enum
  has `Pawn = 1`, `Queen = 5`.

### Build and infrastructure

- [ ] **React integrated-build default read before it is set.**
  *(verified)* `CMakeLists.txt:36` uses
  `WISDOM_CHESS_REACT_BUILD_INTEGRATED_DEFAULT`, which is assigned at
  lines 49-53. The option has always defaulted OFF, contrary to the docs.
- [ ] **Dead `if` hides a bogus target.** *(verified)*
  `ui/console/CMakeLists.txt:5-6` tests `PCH_ENABLED`, a normal variable
  from a sibling scope, and references target `chess`, which does not
  exist. The console binary never gets a PCH.
- [x] Engine compiles with no warning flags. Fixed in this branch.
- [ ] Clear the 60 warning sites the new flags report (see Session #1
  below). `CastlingEligibility` copy constructor first.
- [ ] No sanitizer job, no Debug build and no Linux/Clang in
  `.github/workflows/cmake.yml`. `WISDOM_CHESS_ASAN` is unused by CI.
- [ ] Linter self-tests (`scripts/linter/tests/run-tests.sh`) are not run
  in CI. `LinterConfig::ignore` is populated and never read.
- [ ] `WISDOM_CHESS_SLOW_TESTS` defaults On (`CMakeLists.txt:33`) but
  README and CLAUDE.md say OFF.
- [ ] CLAUDE.md describes `.wisdomstylerc.json` and an automatic feature
  index generator; neither exists. *(verified: no such file)*
- [ ] `target_precompile_headers(... PRIVATE PRIVATE ...)` in
  `engine/CMakeLists.txt:84` and the test and bench CMake files.
- [ ] `-fno-stack-protector` is `PUBLIC` on the engine
  (`engine/CMakeLists.txt:101`) and propagates to all UI code.
- [ ] `cmake_minimum_required` comes after `set(CMAKE_CXX_STANDARD)`
  in the top-level and engine CMake files.

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
