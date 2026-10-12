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
- `isColorValid()` is removed. Without `None`, a `Color` written as an
  enumerator is always valid, and an invalid one could only come from
  an integer: `colorFromInt8()` and `colorFromColorIndex()` check their
  range, and the frontends' `mapColor()` reject anything but White or
  Black. Its `EXPECTS` and `ASSERT`s go with it, along with the five
  fatal tests that needed `Color {}` to reach them, and the unused and
  unchecked `colorFromInt()`.

## Implementation Progress

### Session #1

- `Color::None` removed from the engine, WASM conversions and tests.
- Debug build with QML, tools, benchmarks and slow tests: every
  non-slow test passes, and `lint` is clean. Release with the slow
  tests: all 344 tests pass. The WASM frontend builds.

### Session #2

- `Game::findBestMove()` loses its color parameter, which no caller
  passed, and the QML model's `PieceInfo` loses its `piece` field,
  which nothing read.
- `isColorValid()` and `colorFromInt()` removed, as above. Debug: every
  non-slow test passes (326, five fatal cases fewer), and `lint` is
  clean.

### Session #3

- The WASM bindings still passed a color to `findBestMove()`; the call
  spans several lines, so the search for callers missed it.
- `colorIndex()`, `ColoredPiece::make()` and `pawnDirection()` check
  their color with `EXPECTS`, in Release too. They are where a zero
  color does harm: `colorIndex()` returns -1, an index below every
  per-color array; `make()` stores a piece whose color is neither White
  nor Black; and `pawnDirection()` returns -3, moving a pawn off the
  board. Functions that only compare against `Color::White` treat zero
  as Black, a wrong but in-range answer, and are left as they are.
- Debug: 329 tests pass, three new fatal cases. The WASM build passes
  its 261 tests.
- `Color` is a class instead of an enum, so `Color {}` and `Color { 5 }`
  no longer compile. It holds a nested `enum class Value` and converts
  to it implicitly, so `switch (who)` with `case Color::White:` and
  comparisons work as before; `Color::White` and `Color::Black` are
  `Color` constants. Both constructors are private: the one from
  `Value`, so that `Color::Value {}` or a cast cannot become a `Color`,
  and the default one, which the nested `NullRepresentation` uses to
  make the zero value of an empty `optional<Color>`. The only other
  callers are `colorFromInt8()` and `colorFromColorIndex()`, as
  friends; their inputs come from a `ColoredPiece` that `make()`
  checked and from a one-bit index. An explicit conversion template
  serves `to_int()`.
- The frontends' `using enum Color` switches name `Color::White` and
  `Color::Black` instead. A QML test read `Color` signal arguments with
  `QVariant::value()`, which default-constructs; it uses `get_if` now.
- The three fatal tests make their invalid color with a `std::bit_cast`
  of zero, standing in for corrupt memory, the only way left to get one.
- Debug: 329 tests pass, and `lint` is clean.
