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
- `to_unsigned<Target>` converts a nonnegative signed value to an unsigned
  type at least as wide. Its `ASSERT` checks the sign in Debug builds and
  constant expressions, and costs nothing in Release, which suits indexing
  in the search.
- The checked `widen` and `noexcept_widen` are unchanged, since they
  check the value.

## Implementation Progress

### Session #1

- Restricted `widen_cast` and added `to_unsigned`, with tests and a
  Debug-only fatal case for a negative value.
- Checked that `widen_cast<unsigned long> (int)`, `to_unsigned<unsigned short> (int)`
  and `to_unsigned<unsigned long> (unsigned)` no longer compile.
- Verified lint, all 291 Release tests and the 256 fast Debug tests.
