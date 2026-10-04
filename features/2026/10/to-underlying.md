# Named conversions for enums, doubles and bools

## Motivation

A review of the remaining `static_cast`s found groups that bypass the
named conversions of `engine/numeric_cast.hpp`, now `engine/cast.hpp`:

- Enum to integer. `static_cast<int> (piece)` names a target type that
  happens to match, or widen, the enum's underlying type. If the enum's
  underlying type changes, the cast still compiles, and it converts silently.
  `std::to_underlying` states the intent and gives the exact type.
- Integer to enum. `pieceFromInt (int)` and `getMoveCategory()` convert
  an `int` to an `int8_t` enum, which wraps a value out of range.
- Integer to `double` in statistics code, and `bool` through an explicit
  `operator bool` in tests.
- Integer to integer conversions that an existing helper (`to_unsigned`,
  `narrow`, `numeric_limits`) already covers.

`std::to_underlying` is C++23, and the project builds as C++20. Raising the
standard touches every toolchain at once (GCC/Clang, Emscripten, the Android
NDK, MSVC, Qt's moc), which is its own decision. So the header gets
`wisdom::to_underlying` with the same signature. Once the project moves to
C++23, it becomes `using std::to_underlying;`.

The new conversions:

- `to_enum` and `to_enum_debug` check that the integer fits the enum's
  underlying type, with the usual checked and `_debug` forms. Whether
  the value names an enumerator is the caller's to check; the WASM
  bindings do so in their mapping `switch`. The engine's hot paths use
  `to_enum_debug`; the WASM bindings, whose integers come from
  JavaScript, use `to_enum`.
- `to_double` is unchecked: the counters it converts can pass 2^53, and
  rounding them is fine for statistics. It exists so that the cast reads
  as intended and a search for `static_cast` finds only the unusual ones.
- `to_bool` converts through an explicit `operator bool`. Passing the
  value straight to doctest's `CHECK` does the same cast, but a compound
  expression such as `a | b` does not compile there, and MSVC may warn
  on the implicit conversions.

With conversions other than integer ones in it, `numeric_cast.hpp` is
renamed to `cast.hpp`.

## Scope

Kept as `static_cast`:

- `str_test.cpp`, which casts bytes 0x80-0xff to negative `char`s on
  purpose, and `castling_eligibility_test.cpp`'s `static_cast<uint8_t>(~0)`,
  which shows what that cast does.
- `engine/transposition_table.{hpp,cpp}`, which the
  `transposition-table-improvements` branch is rewriting.

## Implementation Progress

### Session #1

- Added `to_underlying`, `to_enum`, `to_enum_debug`, `to_double` and
  `to_bool`, with compile-time tests in `global_test.cpp` and fatal cases
  for an integer that does not fit an enum.
- Converted the enum to integer, integer to enum and integer to `double`
  casts. The two `log2` round trips that found a table's size exponent
  became `std::bit_width`.
- Converted the `static_cast<bool>` checks in
  `castling_eligibility_test.cpp` to `to_bool`.
- Converted the integer to integer casts that a helper covers: the
  all-ones castling mask is `numeric_limits<uint8_t>::max()`,
  `str.hpp`'s case conversions go through `narrow_debug<char>`, and the
  rest through `narrow` or `to_unsigned`.
- The WASM bindings pass their `bool` flags to the worker as `int`
  through `widen`, which accepts `bool`.
- `QmlDrawByRepetitionStatus` converts to and from
  `DrawByRepetitionStatus` through a pair of `mapDrawByRepetitionStatus`
  functions in `ui_types.hpp`, like the other mirrored enums there,
  instead of a cast. A status added to one enum and not the other fails
  there instead of reaching QML unnamed.
- Renamed `numeric_cast.hpp` to `cast.hpp` and its fatal tests to
  `fatal_cast_test.cpp`.
- Verified lint, Release with the QML UI, tools, benchmarks and slow
  tests, a Debug engine build, and the Emscripten build and tests.
