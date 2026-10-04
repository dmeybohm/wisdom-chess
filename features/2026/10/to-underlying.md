# Enum conversions through to_underlying

## Motivation

A review of the remaining `static_cast`s found two groups that bypass
`engine/numeric_cast.hpp`:

- Enum to integer. `static_cast<int> (piece)` names a target type that
  happens to match, or widen, the enum's underlying type. If the enum's
  underlying type changes, the cast still compiles, and it converts silently.
  `std::to_underlying` states the intent and gives the exact type.
- Integer to integer conversions that one of the existing helpers
  (`to_unsigned`, `narrow`, `numeric_limits`) already covers.

`std::to_underlying` is C++23, and the project builds as C++20. Raising the
standard touches every toolchain at once (GCC/Clang, Emscripten, the Android
NDK, MSVC, Qt's moc), which is its own decision. So `numeric_cast.hpp` gets
`wisdom::to_underlying` with the same signature. Once the project moves to
C++23, it becomes `using std::to_underlying;`.

## Scope

Converted:

- Enum to integer casts in the engine, the QML and WASM frontends and
  their tests. Where the target was wider than the underlying type, the
  result goes through `widen`; where narrower, through `narrow`.
- Integer to integer casts that a `numeric_cast.hpp` helper or
  `numeric_limits` expresses directly.

Kept as `static_cast`:

- Integer to enum. There is no standard counterpart, and a wrapper would
  be just as unchecked. The WASM entry points that take an `int` from
  JavaScript already reject bad values in their mapping `switch`.
- Enum to enum (`QmlDrawByRepetitionStatus` and `DrawByRepetitionStatus`),
  whose values match by construction in `ui_types.hpp`.
- Integer to floating point in statistics code; `numeric_cast.hpp` covers
  integers only.
- `static_cast<bool>` in `castling_eligibility_test.cpp`, which tests the
  explicit `operator bool`.
- `engine/transposition_table.{hpp,cpp}`, which the
  `transposition-table-improvements` branch is rewriting.

## Implementation Progress

### Session #1

- Planned the change.
