# Same-width conversions through widen

## Motivation

`widen()` required a target strictly larger than its source. That rejected
conversions that cannot lose a value, such as `int32_t` to `int` or
`int64_t` to `long long`. The widths of the native types vary by platform,
so code converting between them and the fixed-width aliases had to use
`narrow` for a conversion that never fails.

`widen()` now accepts a target with at least as many value bits as the
source (`std::numeric_limits<T>::digits`, which leaves out the sign bit),
and a signed source still needs a signed target. That allows the same
width when the signedness matches, and still needs a wider target to go
from unsigned to signed. A conversion that is lossy on some platform, such
as `int64_t` to `long` on Windows, fails to compile there.

The rule is a `requires` clause on the `HoldsEveryValueOf` concept rather
than `static_assert`s, so the tests can check through doctest that a lossy
conversion is rejected.

`features/2026/10/widen-cast.md` kept generic functions on `narrow_debug`
because the strict rule rejected their same-width cases.
`CastlingEligibility::toInt<IntegerType>()` converts a `uint8_t` to an
unsigned type, which now always holds every value, so it uses `widen` and
is renamed `toUint()` after the type it returns.

## Implementation Progress

### Session #1

- Replaced `widen()`'s width assertions with the `HoldsEveryValueOf`
  concept and tested same-width conversions and rejected targets.
- Renamed `CastlingEligibility::toInt()` and its free function to
  `toUint()` and converted through `widen`.
- Removed an identity `narrow_debug<size_t>` from `generate.cpp`.
- Updated the `widen` description in `AGENTS.md`.
