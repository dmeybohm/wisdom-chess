# Board::fromFen

Branch: `board-from-fen`, based on `engine-surface` (PR #390).

## Motivation

A survey of the test and benchmark sources for helpers that belong in
the engine found one that was written over and over: a function that
takes a FEN string and gives back a `Board`. The two-line body,
`FenParser parser { fen }; return parser.buildBoard();`, appeared as a
named function in `evaluate_test.cpp` and in four benchmarks, as a
`board_from_fen` lambda in three cases of `generate_test.cpp` and one of
`search_test.cpp`, and inline as a parser-then-build pair in about twenty
more places. A dozen search tests built a whole `Game` from the FEN only
to take its board, copying it once more on the way.

`Board` already has the inverse, `toFenString()`, and `Game` has
`createGameFromFen()`, so the FEN-to-board direction was the one gap.

Other helpers in the tests were looked at and left where they are:

- `shuffleKnights()` is defined three times, in `game_status_test.cpp`,
  `evaluate_test.cpp` and the view model test. It is a fixture, not
  engine behaviour, and belongs in a shared test header if anywhere.
- `normalizePositionFen()` in the hash collision test strips the move
  counters from a FEN string for that test's bookkeeping only.
- `fenError()`, `copyMoves()`, `find_move` and `search_with` each wrap one
  test's own setup.

## Design

`Board::fromFen (const string&)` is a static factory beside the
`BoardBuilder` constructor, implemented through `FenParser`. It follows
the naming rule for strings the program wrote: a FEN that does not parse
is a precondition failure, and text from outside the program keeps going
through `FenParser::parse()`. `board.cpp` includes `fen_parser.hpp` for
it; the header chain is unchanged, since `board.hpp` only declares the
function.

Every test and benchmark site now calls `Board::fromFen()`. The sites
that went on to ask the parser for its active player ask the board for
its turn instead, which is the same value: `buildBoard()` sets it. The
search tests that only wanted the board no longer build a `Game`. The
`fen_parser.hpp` include went from every file that no longer names the
parser, including two that had never used it.

## Implementation Progress

### Session #1

- `Board::fromFen()` added.
- Nine local helpers and the inline parser-then-build pairs replaced
  across the engine tests and benchmarks; `fen_parser_test.cpp` and
  `fatal_parse_test.cpp` keep theirs, since they test the parser.
