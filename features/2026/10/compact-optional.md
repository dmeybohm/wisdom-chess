# A compact optional

## Motivation

`std::optional<T>` stores a flag next to the value, because it has to
hold every value of `T`. For the engine's small value types this
doubles the size: `optional<Move>` is 4 bytes for a 2-byte move, and
`optional<Coord>` is 2 bytes for a 1-byte square. Both types have bit
patterns their own API never produces. No move goes from a square to
itself, so `Move { .data = 0 }` (a8 to a8) is never a move. A square is
0-63, so a negative `row_and_col` is never a square.

Before this change, the transposition table worked around the flag by
storing a bare `Move` and treating `data == 0` as "no best move" through
`Move::isNullMove()`, with the search passing `value_or (Move {})`. That
convention lived in two places and the type did not show it.

## Design

`engine/optional.hpp` defines `wisdom::optional<T>`, lowercase like
`nonnull` and `nullable`. By default it is `std::optional<T>`, so any
type works as before. A type that nests a public `NullRepresentation`
gets `compact_optional<T>` instead, which stores only a `T` and uses the
null value to mean empty:

```cpp
struct NullRepresentation
{
    static constexpr auto make() noexcept -> T;
    static constexpr auto isNull (const T& value) noexcept -> bool;
};
```

`compact_optional` has the `std::optional` members the code uses
(`has_value`, `operator bool`, `value`, `value_or`, `*`, `->`,
`reset`, `emplace`, comparison with `T`, `nullopt` and itself), so
call sites do not change. `value()` reports the failure and aborts,
through `EXPECTS`, instead of throwing. Storing the null value itself
is a precondition failure, checked by `ASSERT`.

The nested type must be public: the concept checks it from outside
the type, and a private one would silently select `std::optional`. A
type that relies on the compact form pins it after its definition:

```cpp
static_assert (sizeof (optional<Move>) == sizeof (Move));
```

`types.hpp` no longer brings in `std::optional`; `global.hpp` includes
`optional.hpp` instead. `optional.hpp` needs `error.hpp`, which
includes `types.hpp`, so `types.hpp` cannot include it without a cycle.

Code outside the `wisdom` namespace that spells `std::optional<Move>`
gets the flagged form, which does not convert to the compact one. The
QML `ChessGame` header was the one case.

## Scope

- `Move` and `Coord` get a `NullRepresentation`. `Move::isNullMove()`
  is removed.
- `TranspositionEntry::best_move` is `optional<Move>`, still 2 bytes,
  and `TranspositionTable::store()` takes an `optional<Move>`.
- `ColoredPiece` / `Piece_And_Color_None` and `Color::None` are left
  alone. An empty square is a value of the board, not a missing piece,
  and removing `None` from `Piece` and `Color` is a separate refactor.

## Implementation Progress

### Session #1

- Added `engine/optional.hpp` with `HasNullRepresentation`,
  `compact_optional` and the `optional` alias.
- `Move` (a8 to a8) and `Coord` (-1) define their null values, with
  size `static_assert`s.
- The transposition table stores `optional<Move>`; the search passes
  its `optional<Move>` straight through.
- `engine/test/optional_test.cpp` covers the alias selection, the
  compact API and the a8 square, and a transposition table subcase
  covers storing no move.
- The Debug build with QML passes `ctest` (331 tests) and `lint`; the
  WASM frontend builds.
