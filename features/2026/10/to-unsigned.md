# Sign conversions apart from widening

## Motivation

`widen_cast` required a strictly wider target, but that guaranteed nothing
for a signed source and an unsigned target: `int` to `uint64_t` wrapped a
negative value as `int` to `uint32_t` would. It also rejected `int` to
`size_t` on wasm32, where the two are the same width, so an array index
conversion compiled on one platform and not the other.

The hazard in a signed-to-unsigned conversion is the sign, not the width.
The move ordering branch hit both cases: its sort key converts `int`
scores to `uint64_t`, and its killer and history tables index arrays with
an `int` ply and square.

## Design

- `widen_cast` now accepts only conversions that hold every value of the
  source: a strictly wider target, and a signed target for a signed
  source. It needs no check, so its constant-expression check is gone.
- `to_unsigned`, `to_unsigned_noexcept` and `to_unsigned_cast` convert a
  nonnegative signed value to an unsigned type at least as wide. They
  follow the narrow family: the first throws `PreconditionError` for a
  negative value, the second terminates, and the third is a `static_cast`
  at runtime and a compile error for a negative constant.
- The checked `widen` and `widen_noexcept` are unchanged, since they
  check the value.

## Implementation Progress

### Session #1

- Restricted `widen_cast` and added `to_unsigned`, with tests and a
  Debug-only fatal case for a negative value.
- Checked that `widen_cast<unsigned long> (int)`, `to_unsigned<unsigned short> (int)`
  and `to_unsigned<unsigned long> (unsigned)` no longer compile.
- Verified lint, all 291 Release tests and the 256 fast Debug tests.

### Session #2

- Replaced the `ASSERT`-only `to_unsigned` with the three-form family, so
  it matches `narrow` and `widen`. The Debug-only fatal case became one
  for `to_unsigned_noexcept` in every build.
- Switched the move ordering code's 11 sign conversions from `narrow_cast`
  to `to_unsigned_cast`: 8 table indexes in `move_ordering.cpp` and 3 sort
  key fields in `generate.cpp`. They are in `noexcept` search code, where
  `ply` is already bounds-checked and the scores are nonnegative by
  construction. The Release disassembly of `generate.cpp.o`,
  `move_ordering.cpp.o` and `search.cpp.o` is identical before and after.
- Listed every conversion family in `AGENTS.md`.
- Verified lint, all 297 Release tests and the 261 fast Debug tests.
