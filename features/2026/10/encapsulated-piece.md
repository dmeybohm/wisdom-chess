# Encapsulated Piece and ColoredPiece

Follows `optional-color.md` and `zero-based-piece-index.md`.

## Motivation

`optional-color` made `Color` a class that cannot be built from an
integer. Two holes of the same kind were left:

- `ColoredPiece` was an aggregate with a public packed byte, so any
  byte could be a `ColoredPiece`. Its `type()` decoded the type bits
  with `pieceFromInt8()`, which checked the range only in Debug, so a
  type of 0 or 7 reached `pieceIndex()` and indexed past `Material`'s
  counts or the Zobrist table in Release. Its `color()` read garbage
  color bits as White.
- `Piece` was still an enum, so `Piece {}` and casts gave values that
  are no piece.

## Design

- `ColoredPiece` is a class. Its byte is private, it has no default
  constructor, and only `make()` and its `NullRepresentation` construct
  one, through a private constructor from the packed byte. So every
  `ColoredPiece` holds a valid color and type, and `color()` and
  `type()` decode without a check. Nothing outside `piece.hpp` built
  one by hand. Its `static_assert` checks that it is trivially
  copyable, which the board's copies need, rather than trivial.
- `Piece` is a class like `Color`: a nested `enum class Value`, the six
  constants, an implicit conversion to `Value` for switches and
  comparisons, an explicit conversion template for `to_int()`, private
  constructors, and a nested `NullRepresentation` for the empty
  `optional<Piece>`. `ColoredPiece` is its friend, to decode its type
  bits. `pieceFromInt8()` and the unused `pieceFromInt()` are gone.
- `threats.hpp`'s sliding-piece templates take `Piece::Value`: a class
  with private members is not a structural type, so it cannot be a
  template parameter. The callers' `Piece::Rook` and `Piece::Bishop`
  convert to it at compile time.
- The frontends' `using enum Piece` switches name `Piece::Pawn` and so
  on, as they do for `Color`.
- Neither `Piece` nor `Color` uses `optional.hpp`'s
  `nullRepresentation()` function any more, so its test uses an enum
  of its own.

Unlike `Color`, `Piece` cannot be decoded by choosing between
constants: three bits name six types, so `type()` still converts
through `to_enum_debug`. That is safe because `ColoredPiece` can only
hold what `make()` put there.

## Implementation Progress

### Session #1

- `ColoredPiece` encapsulated, then `Piece` made a class.
- Debug build with QML, tools, benchmarks and slow tests: every
  non-slow test passes, and `lint` is clean.

### Session #2

- Review: `NullRepresentation::make()` was public, so
  `Piece::NullRepresentation::make()` gave the zero `Piece` without a
  cast, and `pieceIndex()` turned it into -1. `make()` now takes a
  `NullRepresentationKey`, which only `compact_optional` and
  `NullRepresentationOf` can create, and `NullRepresentationOf`'s
  members are private to `compact_optional`. The concept still finds
  `make()` through `std::declval`, which needs no access. Every type
  with a null representation takes the key, as does the enum
  `nullRepresentation()` hook, so the empty value of `Move`, `Coord`,
  `ColoredPiece`, `Piece` and `Color` can only come from an empty
  optional. `optional_test.cpp` checks that for each, and a control
  build with the members public confirmed the check can fail. The
  first version also tried `make (NullRepresentationKey {})` inside the
  concept; that expression does not depend on `T`, so Clang rejects it
  as an error rather than an unsatisfied requirement, which broke the
  Clang builds. That path is covered by the key not being
  constructible instead.
- Dereferencing an empty optional with `*` still yields the stored
  value: it asserts only in Debug, like `std::optional`'s undefined
  behavior, and a Release check there would cost a branch on every
  `*board.pieceAt (...)`. `value()` is the checked form.
- `pieceIndex()` asserts its range again, in Debug.
- Debug: every non-slow test passes, and `lint` is clean.
