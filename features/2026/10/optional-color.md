# Removing Color::None

Follows `optional-colored-piece.md`, and is stacked on that branch.

## Motivation

With empty squares as an empty `optional<ColoredPiece>`, `Color::None`
was left meaning "not given": the default for `Game::findBestMove()`'s
color and `toMove()`'s, the color `parseMove()` was given when castling
text could not be read, and a search's color before it was set. An
`optional<Color>` says that directly, and a `switch` over `Color` no
longer needs a case for it.

## Design

- `Color::None` is removed. `White` and `Black` keep 1 and 2, so the
  packing in `ColoredPiece` is unchanged, and `Color` opts in to a
  one-byte `optional<Color>` through `nullRepresentation()`, as `Piece`
  does.
- `Game::findBestMove()` and `toMove()` take `optional<Color>`
  defaulting to `nullopt`, which means the side to move and "no color"
  respectively. `parseMove()` takes `optional<Color>`; castling text
  without a color does not parse, as before.
- The search's `my_searching_color` is `optional<Color>` until
  `iterativelyDeepen()` sets it.
- The web frontend keeps `WebColor::NoColor`, which React uses for "no
  human player". Every function that receives a `WebColor` needs a real
  color, and a `NoColor` already failed `isColorValid()` there, so
  `mapColor (WebColor)` still returns a `Color` and reports `NoColor`
  as a precondition failure, as QML's `mapColor()` does. The IDL is
  unchanged.
- The fatal tests that pass an invalid color now use `Color {}`, a
  value no enumerator names, so they still cover `isColorValid()`.

## Implementation Progress

### Session #1

- `Color::None` removed from the engine, WASM conversions and tests.
- Debug build with QML, tools, benchmarks and slow tests: every
  non-slow test passes, and `lint` is clean. Release with the slow
  tests: all 344 tests pass. The WASM frontend builds.
