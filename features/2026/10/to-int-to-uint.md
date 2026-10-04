# to_int and to_uint

## Motivation

Conversions to an integer were spelled per type: `toInt (Piece)`,
`toInt (Color)`, three `toInt8` overloads, and the castling member
`toUint<IntegerType>()` with a free wrapper. `cast.hpp` already had
`to_bool`, which converts anything with a `noexcept` explicit conversion to
`bool`. `to_int` and `to_uint` follow it:

- An enum converts through `widen` of its underlying type, so the
  conversion compiles only when the target holds every value.
- A class converts through its explicit conversion operator, which must be
  `noexcept`. The operator stays explicit so the value cannot leak into
  arithmetic, and `static_cast` to another type does not find it.

`to_uint` takes its target as a template argument. `CastlingEligibility`
gives it a conversion operator template constrained to
`std::unsigned_integral`, so every unsigned width works and a signed one
does not compile. `bool` satisfies that concept, but the class's own
non-template `operator bool` wins overload resolution, which `AGENTS.md`
notes for any other class that adopts the pattern.

`to_int` always returns `int`, so `ColoredPiece` has a plain
`explicit operator int`. The QML piece model keyed its image table on
`int8_t`; it now keys on `int`, so no implicit narrowing appears.

The string `toInt` in `str.hpp` is a parser, not a conversion, and
`Move::toInt()`/`fromInt()` are an encoding pair; neither changes.
`pieceIndex` and `colorIndex` are named indices with range checks and stay
too.

## Implementation Progress

### Session #1

- Added `to_int` and `to_uint` to `cast.hpp`, with tests for enums,
  rejected targets and `ColoredPiece`.
- Replaced `CastlingEligibility::toUint()` and its free wrapper with an
  explicit conversion operator template.
- Replaced `toInt (Piece/Color)` and the `toInt8` overloads with `to_int`,
  including in the QML piece model and the WASM frontend's `to_underlying`
  conversions to `int`.
- Added range `ASSERT`s to `pieceFromInt8()` and `colorFromInt8()`.
- Verified lint, `all_qmllint`, all 339 Release tests with the QML UI,
  all 318 Debug tests and the WASM build.
