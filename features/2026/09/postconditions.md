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
- **`evaluateWithoutMateTest`.** `ENSURES` that the score stays inside
  `Max_Non_Checkmate_Score`. The constants were pinned by
  `static_assert`; the computed sum was not. It is on in Release, at
  every evaluation; "Benchmarks" below has what that costs.
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

## Benchmarks

Whether the checks cost anything in a Release build, for this branch
and for the macros branch before it. `wisdom-chess-benchmarks` was
built from three revisions: `0288c7d`, `main` before the contract
macros; `2c59593`, `main` with them; and `3f6c940`, this branch with
the `ENSURES` in `evaluateWithoutMateTest`. Each ran the whole suite
five times, pinned to one core, in alternating order. The figures are
medians.

The ratios are geometric means over the benchmarks of each group. Below
one is faster.

| Group | Benchmarks | Macros / before | This branch / `main` |
|---|---|---|---|
| Move generation | 12 | 1.022 | 0.956 |
| Threat detection | 4 | 0.971 | 1.008 |
| `withMove` and legality | 6 | 0.991 | 0.995 |
| Perft | 4 | 0.988 | 0.973 |
| Search to depth 6 | 5 | 0.986 | 0.986 |
| Search to depth 8 | 5 | 0.983 | 1.025 |
| Warm-table replay | 2 | 0.957 | 1.035 |
| All | 38 | 0.995 | 0.986 |

| Benchmark | Before macros | `main` | This branch |
|---|---|---|---|
| Search, Italian game, depth 8 | 22.41 s | 21.97 s | 22.43 s |
| Search, Kiwipete, depth 8 | 3.79 s | 3.84 s | 3.79 s |
| Search, 200 plies of history, depth 6 | 572 ms | 564 ms | 555 ms |
| Warm-table replay, cleared every search | 6.37 s | 6.12 s | 6.28 s |
| Perft, Kiwipete, depth 4 | 347 ms | 347 ms | 344 ms |

Neither change is measurable. The same benchmark varied between rounds
by 2% to 16% on this laptop (`powersave` governor, turbo on),
with a median spread of 9%, which is several times any difference
between the builds. The 22-second search, the steadiest measurement, is
within 2% across all three.

`--search-report 6` gave the same move, score, depth and node counts
from all three builds on every position, so the checks change no
search result.

## Implementation Progress

### Session #1

- The checks above, the builder change, and tests: the recompute walk,
  the power-of-two count, and the builder rejecting a king's square.
- Verified: GCC Release build with `-Werror` and the QML UI, lint
  clean, 232 fast and 35 slow tests pass. Clang 18 Debug build with
  `-Werror`, 233 tests pass in 15 s, including the web game tests that
  drive the piece-list check. The React WASM target builds in Debug
  with `-Werror`.

### Session #2

- The evaluation bound became an `ENSURES`, and "Benchmarks" above
  records that neither it nor the contract macros cost anything
  measurable. Release: 267 tests pass. Debug: 233.
