# Empty squares as an empty optional

Follows `compact-optional.md`, and is stacked on that branch.

## Motivation

`Piece::None`, `Piece_And_Color_None` and the "no piece" uses of
`Color::None` stood for an empty square. Any `ColoredPiece` could be
empty, so code that needed a real piece had to remember to check, and a
`switch` over `Piece` needed a `None` case that was never meant to be
reached. With `optional` now as small as the value it holds, an empty
square can be an empty `optional<ColoredPiece>` at no cost in space.

## Design

- `Board::pieceAt()` and `BoardBuilder::pieceAt()` return
  `optional<ColoredPiece>`, and both store
  `array<optional<ColoredPiece>, Num_Squares>`. A `ColoredPiece` always
  holds a real piece and color; `ColoredPiece::make()` asserts it.
- `ColoredPiece` nests a `NullRepresentation` whose empty value is zero,
  the byte an empty square already had, so the board's layout is
  unchanged.
- `Piece::None` is removed and `Pawn` starts at one. The numbering is
  otherwise unchanged, so the packing in `ColoredPiece`, the Zobrist
  tables and the arrays indexed by `pieceIndex()` stay as they were;
  index 0 of those arrays is still unused, and `Num_Piece_Types` still
  counts it.
- An enum cannot nest a type, so `optional.hpp` also accepts a
  `nullRepresentation (std::type_identity<T>)` function found by
  argument-dependent lookup. `Piece` declares one returning `Piece {}`,
  making `optional<Piece>` one byte.
- `Move::getPromotedPiece()` requires `isPromoting()` rather than
  returning `None`. Its callers already checked; `promotionRank()` now
  does too. `PieceMovement::promoted_piece` and the parsed promotion in
  `parseMove()` and perft are `optional<Piece>`.
- `BoardBuilder::PieceRow` is `array<optional<Piece>, Num_Columns>`, so a
  row with gaps is written with `nullopt`.
- The frontends keep their own "no piece" values: QML's
  `PieceType::None` and the web's `WebPiece::NoPiece`, which the
  bindings need for an enum that cannot be null. Only their conversions
  change: `mapPiece()` from the frontend type returns `optional<Piece>`,
  and the reverse loses its `None` case. The IDL and the generated
  TypeScript types are unchanged.
- `Color::None` stays for now; its uses that mean "not given" are a
  separate change.

## A GCC 13 constant-evaluation bug

GCC 13 rejects, in a constant expression, reading an element of an
array of `compact_optional<T>` that was value-initialized with `{}`,
when `T`'s empty value is all zero bits ("accessing value of
'<anonymous>' through a 'compact_optional<...>' glvalue"). Clang
accepts it, and so does GCC when the array is default-initialized
without braces or filled after the `{}`. The only constant evaluation
that hit it is `Default_Board_Builder`, so `emptySquares()`
default-initializes its array.

## Implementation Progress

### Session #1

- Engine, viewmodel, QML, WASM, perft and the seed optimizer moved
  to `optional<ColoredPiece>` squares; `Piece::None`,
  `Piece_And_Color_None` and `Weight_None` are gone.
- `threats.hpp`'s sliding check returns early on an empty square
  instead of deriving `None` from the piece type.
- Tests compare squares with `ColoredPiece::make()` or `nullopt` in
  place of separate type and color checks.
- Debug build with QML, tools and benchmarks: every non-slow test
  passes, and `lint` is clean. Release with the slow tests: all 344
  tests pass. The WASM frontend builds.
