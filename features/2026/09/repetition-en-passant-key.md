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
so this branch will target `main` (rebased onto it) rather than
`review-fixes`.

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

Step 1 (dedicated history/repetition tests for the double-push-with-no-
adjacent-pawn, pinned-pawn, and legal-capture cases) is still open, along
with step 4 (search/transposition table check) and step 5 (full
Release + lint run). Fast test suite passes; a Release build and the
slow/CTest suite are running to confirm no other regressions before those
steps are picked up in Session #3.
