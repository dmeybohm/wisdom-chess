# Repetition keys and en passant

## Motivation

The PGN/FEN specification records the passed square after every double
pawn push, even when no enemy pawn can capture. The `review-fixes` branch
now preserves that target in `Board` for FEN output and move generation.
FIDE's repetition rule compares the moves actually possible from each
position: an en passant target distinguishes positions only when a legal
en passant capture exists. A nearby pawn may be pinned, so adjacency
alone cannot decide this.

`History` currently stores raw `BoardCode` values for its fast count and
raw `Board` copies for its exact count. Both comparisons include the
en passant field. This can miss a threefold or fivefold repetition when
the first occurrence follows a double push with no legal capture and
later occurrences have the same pieces and legal moves but no target.

Sources: [PGN/FEN section 16.1.3.4](https://www.saremba.de/chessgml/standards/pgn/pgn-complete.htm#16.1.3.4)
and [FIDE Laws of Chess section 9.2.3](https://handbook.fide.com/chapter/e012023).

## Plan

1. Add focused history tests for a double push with no adjacent enemy
   pawn and one with an adjacent pawn pinned against its king. After the
   positions recur, both should count for threefold and fivefold draws.
   Keep a case with a legal en passant capture that remains distinct
   until the right expires. Run new engine tests in Debug and Release.
2. Normalize a copy of the board for repetition comparison only. If the
   board has an en passant target but `generateLegalMoves()` contains no
   en passant move, clear the target on that copy. Keep the original
   board intact for FEN output and legal move generation.
3. Store the normalized copy's code in `History::my_board_codes` and the
   copy itself in `my_stored_boards`. Normalize query boards before both
   the fast and exact counts. Cover every history entry path, including
   the initial board, tentative positions, committed positions, and
   `replaceLastPosition()`. Keep the existing `std::count` checks.
4. Check search behavior with the corrected history keys. The
   transposition table may continue to use the raw board code: that can
   miss a cache hit but does not change the repetition result. Consider
   transposition key normalization separately if measurements justify it.
5. Run the full Release build, CTest suite, and lint target. Record the
   outcome here before opening a PR.

## Implementation Progress

### Session #1

Created `repetition-en-passant-key` in its own worktree from
`review-fixes` at `0b64075`. No draw-detection code or tests have been
changed yet.

### Session #2

Implemented plan step 2 and 3: `Board::withNormalizedEnPassantTarget()`
returns a copy with the en passant target cleared whenever
`generateLegalMoves()` contains no en passant capture, and `History`
calls it at every entry point (`fromInitialBoard`, `addTentativePosition`,
`addPosition`, `replaceLastPosition`) and on the query board in both
`isProbablyNthRepetition` and `isCertainlyNthRepetition`.

`review-fixes` merged into `main` as PR #275 while this branch existed,
so this branch targets `main` rather than `review-fixes`. It was not
rebased: it still starts at `0b64075`, so it lacks the two later
`review-fixes` commits on `main` (`57e1715`, `229850f`) and merges with
them cleanly.

Normalizing the initial board this way exercises `generateLegalMoves()`
on boards that previously never reached it, which surfaced a latent bug:
`generateAllPotentialMoves()`'s castling code trusts that a castling-
eligibility bit is only set when the corresponding rook is still on its
home square, and never re-checks the square before
`applyForCastlingMove()` applies the move — an inconsistent FEN (rights
declared without the rook present) aborted instead of failing to parse.
Added that missing check to `FenParser::parseCastling()`
(`validateCastlingRookPresent()`), which throws `FenParserError` instead.
Fixed the fen_parser_test.cpp fixtures that depended on the old,
unchecked behavior (they declared castling rights over a board with no
rooks on the corresponding home squares) to use a proper
`r3k2r/.../R3K2R` fixture, and added
"FEN parser rejects castling rights without the rook present" to cover
the new check with `CHECK_THROWS_AS`.

Reworded the `withNormalizedEnPassantTarget()` comment for draw
detection, then reverted it back to the FEN-vs-FIDE framing at the
user's request — it was fine as originally written.

Full Debug `ctest` (fast + slow, including both perft suites) passed
236/236, and `cmake --build build --target lint` is clean. Opened as
draft PR #286 against `main` (not rebased, as above).

### Session #3

Implemented plan step 1. Added `withNormalizedEnPassantTarget` in
`en_passant_test.cpp` with four subcases directly on `Board`: no target
to begin with, no adjacent enemy pawn, an adjacent pawn pinned against
its king (rank pin: rook behind, king on the far side, so the en passant
capture's double removal exposes check), and a genuine legal capture.

Added two `History`-level test cases covering the same scenarios end to
end with `isThirdRepetition`/`isFifthRepetition` (so both the probable
and certain counts are exercised, not just the fast path): "Repetition
counts a position despite an unusable en passant target" (no-adjacent-
pawn and pinned-pawn subcases) and "A legal en passant capture keeps a
position distinct until the right expires". Verified each new test
actually catches the bug by temporarily reverting `History` to use the
raw, unnormalized board in all four call sites and confirming the first
two failed (and the distinctness test still passed, since it doesn't
depend on the fix) before restoring the real implementation.

Step 4 needed no code change: `search()` keys the transposition table
off `parent_board.getCode().getHashCode()` (`search.cpp:186`), never
through `History`, while draw detection goes through the now-normalized
`history.isProbablyNthRepetition()` (`evaluate.hpp:65`,
`isProbablyDrawingMove`). The two are already independent, matching the
plan's accepted trade-off.

Measured the actual node-throughput cost of normalizing, since
`History::addTentativePosition()` runs on every candidate move at every
search node (`search.cpp:213`), not just on real game moves. A depth-7
search from the starting position visits 900,512 nodes; a temporary
counter showed 263,180 of those (29%) see a `Board` with an en passant
target at the point `withNormalizedEnPassantTarget()` runs, because a
pawn double push is a common candidate move at almost every ply. The
first implementation called `generateLegalMoves()` — a full move
generation and legality pass — on every one of those, which measured at
roughly 2.7x slower node throughput than the pre-fix baseline in a clean
run (2.16M nodes/sec baseline vs. 791K nodes/sec).

Added `hasPawnAdjacentToEnPassantTarget()`: capturing en passant requires
a pawn of the right color on the capture row next to the target column,
so its absence rules out a legal capture completely, with no move
generation needed — only the presence of such a pawn (necessary but not
sufficient, since it could be pinned) needs the full
`generateLegalMoves()` check. Re-measured with the same counter: only
3,178 of the 900,512 nodes (0.35% overall, 1.2% of the target-present
nodes) still need the expensive path. Re-timed after the fix: node
throughput lands within noise of the pre-fix baseline (both instrumented
runs measure the same 900,512/242,325 node/quiescence-node totals, so
the comparison is apples to apples). All the en passant repetition
tests, including the pinned-pawn case that exercises the fallback path,
still pass.

Step 5: configured and built a Release tree (`build-release`,
`-DWISDOM_CHESS_SLOW_TESTS=On`). Full `ctest` passed 240/240 (fast +
slow, including both perft suites, ~24s total — much faster than the
Debug run). `cmake --build build-release --target lint` is clean. Fast
suite is now 206 tests (was 202 before this branch; +4 new cases across
`en_passant_test.cpp`, `history_test.cpp`, and the FEN castling
validation from Session #2).

All five plan steps are complete. PR #286 is ready to come out of draft.

### Session #4

Addressed the review of PR #286.

`FenParser` now rejects two more FENs that move generation would
otherwise trust. Normalizing in `History::fromInitialBoard()` runs move
generation while the FEN is loading, so both used to abort inside
`createGameFromFen`, where no frontend can catch a `FenParserError`:

- Castling rights with the king off its home square. Move generation
  checks only the king's column and takes the rook's row from the king's
  row. `validateCastlingRookPresent()` became `validateCastlingPieces()`
  and checks the king too.
- An en passant target that no double push could have left.
  `validateEnPassantTarget()` requires the target on the rank that
  matches the side to move, the vulnerable pawn in front of it, and both
  the target and the pawn's starting square empty. The wrong rank was
  already fatal on `main`, in Release too, through the
  `noexcept_expects` in `BoardCode::setEnPassantTarget()`.

Both repro FENs from the review now answer `Invalid FEN: ...` in the
Debug UCI binary instead of aborting. Two fixtures in
`fen_parser_test.cpp` had an `e6` target with no pawn on e5 and gained
one.

Tests added, each checked by removing the code it covers and watching
it fail:

- `history_test.cpp`: a position with an unusable target entering
  through `fromInitialBoard()`, `replaceLastPosition()` and
  `addTentativePosition()`. Before, only `addPosition()` was covered.
- `fen_parser_test.cpp`: Black-only castling rights, a rook of the
  other color, a non-rook on the rook's square, the king cases, and the
  en passant cases. The rejection tests now check the message, and the
  queenside case uses a full-width rank.
- The two "invalid en passant square" tests claimed `KQkq` on a board
  with one rook, so the castling check threw before the en passant
  field was read. They now use `-` and check the message.

The query-side normalization in `isProbablyNthRepetition()` and
`isCertainlyNthRepetition()` is still untested. It is only observable
with a repetition count of 1, and its cost is part of the follow-up on
the `en-passant-normalization-cost` branch.

Debug: full `ctest` passed 243/243 (209 fast, 34 slow). Release: full
`ctest` passed 243/243. `lint` is clean in both.
