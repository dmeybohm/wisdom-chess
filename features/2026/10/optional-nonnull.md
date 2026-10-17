# Optional nonnull

## Motivation

`Board::getPieces()` (PR #369) returns a range whose iterator holds a
`nonnull<const Board>`. A `nonnull` has no default value, so the iterator
cannot be default-constructed, and C++20's `std::forward_iterator`
requires that. On `main` the iterator is therefore an input iterator,
ending at `std::default_sentinel`, which suits every current caller:
they all make one pass.

This branch makes it a forward iterator, so that multi-pass algorithms
such as `std::ranges::max_element` accept `getPieces()`. It was kept out
of PR #369 because it changes `nonnull`, which everything uses, for an
iterator concept that no caller needs yet.

## Design

- `nonnull` nests a `NullRepresentation`, as `Coord` does, so
  `optional<nonnull<T>>` is a `compact_optional` the size of a pointer.
  The empty value is a null pointer, made through a private constructor
  that only the `NullRepresentation` reaches; since only the optional
  holds a `NullRepresentationKey`, nothing else can make a null
  `nonnull`, and the public constructor still aborts on null.
- `BoardPieces::Iterator` holds `optional<nonnull<const Board>>`. Its
  defaulted constructor leaves it empty, `operator*` unwraps it with `*`,
  and two iterators compare by the squares they have left. `end()` stays
  `std::default_sentinel`.

### Alternatives considered

- A placeholder board, built once in a function-local static, for the
  default iterator to point at: every default construction would check
  the static's guard, and `Board` would gain a function only for this.
- A `constexpr` empty board: `Board`'s constructor is not `constexpr`, and
  a board without kings fails its `ENSURES`.
- `nullable<const Board>`: `value()` checks with `EXPECTS` in every build,
  a branch for every piece in the generator's loop.

### Trade-off

`optional<nonnull<T>>` overlaps with `nullable<T>`. The compact optional's
`*` and `->` check with `ASSERT` only, so in Release an empty one gives a
null pointer, where `nullable::value()` reports a contract failure.
AGENTS.md says to prefer `nullable` and to use this only where the check
costs too much and the code knows the optional is not empty.

## Implementation Progress

### Session #1

- Implemented the design above, branched from `occupancy-mask` at
  `db320755`.
- `optional_test.cpp` pins `optional<nonnull<int>>` as the compact
  optional and the size of a pointer, and checks that one starts empty and
  holds a pointer. `board.hpp` asserts `std::forward_iterator` for the
  iterator.
- A compile-only check outside the repository confirmed that
  `BoardPieces` is a `std::ranges::forward_range` and that its iterator
  is 16 bytes. The generator's piece loop compiles to the same
  instructions as on `occupancy-mask`.
- The fast and medium tests passed in Release and Debug, and lint was
  clean, in the `occupancy-mask` worktree before the change was moved
  here.
