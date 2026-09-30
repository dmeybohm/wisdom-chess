# Postconditions in the engine and the web game

Branch: `postconditions`, from `main` after
[contract-macros.md](contract-macros.md) merged.

## Motivation

The codebase had one `ENSURES`. A postcondition earns its place where a
function maintains derived state that cannot be checked by reading the
function alone, or where a result crosses into code that trusts it, and
where a violation would otherwise pass quietly. A check that only
restates the line above it, or whose violation the next call would catch
anyway (the turn not flipping after a move), was left out.

## Design

- **The search result** (`IterativeSearchImpl::iterativelyDeepen`). A
  move that is returned must be legal on the board searched, and its
  score must lie within the checkmate band. Once per search, so it is
  `ENSURES` and stays on in Release. The existing `catch` turns a
  failure into a `SearchError` with the board attached.
- **`Board`'s constructor from a builder.** Each stored king position
  holds that color's king: `ENSURES( hasKingsAtKingPositions() )`. Once
  per board built.
- **`Board::makeMove`.** The same king check, as `ASSERT`, since the
  function is `noexcept` and on the hot path.
- **`evaluateWithoutMateTest`.** `ASSERT` that the score stays inside
  `Max_Non_Checkmate_Score`. The constants were pinned by
  `static_assert`; the computed sum was not.
- **`TranspositionTable::fromEntries`.** A precondition rather than a
  postcondition: `EXPECTS( std::has_single_bit (entry_count) )`. The
  index mask is the count minus one, so a count that is not a power of
  two leaves entries unreachable, with no symptom but a weaker search.
  The megabytes constructor got no matching `ENSURES`: its size comes
  from doubling, so the check would only restate the loop.
- **`WebGame::updatePieceList`.** `ASSERT` that the displayed piece
  list is exactly the pieces on the board. The list is the frontend's
  incremental mirror of the board.
- **`BoardBuilder::addPiece`** throws `BoardBuilderError` when the
  square already holds a king. See "What the checks found".

The two score bounds are written as two-sided comparisons, as
`position.cpp` already does, rather than with `std::abs`. The file-local
`absoluteValue` in `generate.cpp` is `consteval`, so it cannot be used
at run time, and it has no obvious header to move to.

### The recompute check became a test

The first version also asserted at the end of `makeMove` that the hash,
`Material` and `Position` each equal a recompute from the squares. That
is the check with the most value, but it was too slow to leave in the
code: the slowest Debug test, `generateLegalEnPassantMoves`, went from
13 s to 28 s with any one of the three and to 59 s with all of them,
and the Debug suite from 14 s to 63 s.

It is a test instead: "Incrementally updated board state matches a
recompute" in `board_test.cpp` walks every legal move to depth 3 from
four positions that between them reach castling, en passant, captures
and promotions, and compares each board with a recompute. `Material`
and `Position` got defaulted equality operators for it. Castling rights
are not compared: they depend on the game's history, and a FEN may
grant a right the squares do not back.

## What the checks found

The constructor's `ENSURES` failed on its first run, in
`material_test.cpp`. The `checkmateIsPossible()` test case puts the
black king on a8, and four of its subcases then placed a white bishop on
a8. The builder overwrote the king on the squares but kept a8 as its
position, so those four boards had no black king, and the tests passed
by accident. The bishop now goes on c8, the same square color, and the
builder rejects a piece placed on a king's square.

## Implementation Progress

### Session #1

- The checks above, the builder change, and tests: the recompute walk,
  the power-of-two count, and the builder rejecting a king's square.
- Verified: GCC Release build with `-Werror` and the QML UI, lint
  clean, 232 fast and 35 slow tests pass. Clang 18 Debug build with
  `-Werror`, 233 tests pass in 15 s, including the web game tests that
  drive the piece-list check. The React WASM target builds in Debug
  with `-Werror`.
