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
