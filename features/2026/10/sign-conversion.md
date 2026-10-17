# Sign-conversion warnings

Branch: `sign-conversion`, stacked on `enable-wconversion`.

## Why

`enable-wconversion` turned on `-Wconversion`. Clang's `-Wconversion`
includes `-Wsign-conversion` and GCC's does not, so that branch turned it
off on Clang to keep the flag meaning the same on both. This branch turns
`-Wsign-conversion` on for both instead.

The warning finds few bugs here on its own: nearly every site it flagged
indexes a `std::array` with a value that is nonnegative by construction,
such as a `Coord`'s square index or a color index. What it buys is
consistency. The engine already wrapped some of those subscripts in
`to_unsigned_debug` (`search.cpp`, `move_ordering.cpp`) and left most of
them bare, so whether an index got the cast depended on who wrote the
line.

## Approach

Fix the types that produce indexes rather than cast at each subscript,
so the subscripts read as they did before, with no cast:

- `Coord::index()` defaults to `std::size_t`. The few arithmetic users
  ask for `index<int>()` (packing a `Move`, `nextCoord()`) or
  `index<std::ptrdiff_t>()` (an iterator offset in `Board`).
- `ColorIndex` is `std::size_t`, and `pieceIndex()` and
  `zobristPieceIndex()` return it too. `colorIndex()` and `pieceIndex()`
  convert through `to_unsigned_debug`, the one checked conversion.
- `MoveList` and `OrderedMoveList` keep their `std::ptrdiff_t` count,
  since iterator arithmetic wants a signed offset, and write their few
  subscripts in iterator form. `size()` returns `std::ptrdiff_t`, as
  `std::ssize()` would.
- `WebColoredPieceList`, the WASM piece list, indexes with
  `std::size_t`; the IDL says `unsigned long`, which still generates
  `number` on the TypeScript side. `indexOf()` returns an
  `optional<std::size_t>` instead of `-1`.
- `BoardBuilder`'s row loops iterate the row directly with an `int`
  column counter for `addPiece()`.
- `BoardCode`'s metadata updates compute in `int` and narrow once, since
  `metadata &= ~mask` promotes the complement to a negative `int`.

The subscripts GCC and Clang flagged were the same 34 sites, so one fix
serves both.

## Implementation Progress

### Session 1 (2026-10-17)

Done as above. Verified: GCC 13 and Clang 18 Release builds with
`WISDOM_CHESS_WERROR` on, the style linter, and the fast and medium
tests in Release and in a Debug engine build.
