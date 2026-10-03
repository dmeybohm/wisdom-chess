# Wider integer conversions

## Motivation

Before this change, `narrow<Target>()` checked signed-to-unsigned conversions
even when the target type was wider. The name `narrow` obscured that use.
Wider integer conversions get a separate family with the same failure
policies: `widen` throws `PreconditionError`, `noexcept_widen` terminates
on a failed invariant, and `widen_cast` is unchecked at runtime. All three
require an integral target strictly wider than the source. A checked
signed-to-unsigned conversion accepts nonnegative values and rejects
negative ones.

The existing `narrow`, `noexcept_narrow`, and `narrow_cast` now require a
target no wider than the source. This makes the two families' width rules
explicit at compile time.

The header is renamed from `narrow.hpp` to `numeric_cast.hpp` because it
also contains `truncate()`. `truncate()` intentionally discards high bits
from unsigned integers; its same-width case remains valid and tested.

## Implementation Progress

### Session #1

- Added the three widening helpers and tests for safe widening, negative
  signed-to-unsigned input, unchecked runtime wrapping, and termination on
  a failed `noexcept_widen` invariant.
- Renamed the conversion header and updated its include in `global.hpp`.
- Reviewed `truncate()` and kept its existing contract.
- Put template declarations on separate lines from function specifiers in
  `numeric_cast.hpp` and documented that preference in `coding-style.md`.
- Constrained the narrow family to targets no wider than their sources.
- Migrated existing wider conversions in `Coord`, castling rights, the WASM
  view, and tests to the widening family.
- Verified `lint`, the full Release build and all 286 Release tests. Built
  the Debug engine tests and ran all eight conversion tests there.
