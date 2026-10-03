# `noexcept` on `MoveList`

## Motivation

A review of `move_list.hpp` found four definitions that cannot throw but
were not marked `noexcept`, and two that threw for a programming error
where their neighbours abort.

The copy constructor and copy assignment copy trivially copyable `Move`
values into a fixed array and assign an integer. Because they are
user-declared without `noexcept`, `std::is_nothrow_copy_constructible`
reported `MoveList` as throwing, and generic code that chooses a copy
strategy by that trait treated it so. `operator==` and `operator!=` call
only `noexcept` functions: `size()`, `begin()`, `end()` and `std::equal`
over `Move::operator==`.

`front()` and `back()` checked for an empty list with `EXPECTS`, which
throws `PreconditionError`. In the same class, `append()` on a full list
and `removeLast()` on an empty list check with `NOEXCEPT_EXPECTS`, which
aborts. An empty list is the same kind of programming error in all four,
and `AGENTS.md` reserves a throwing `EXPECTS` for caller input that can
be handled. No code in the engine or any frontend calls `front()` or
`back()`; the only callers were the two test lines asserting the throw,
so no handler of the exception existed to lose.

## Scope

Marked `noexcept`:

- `MoveList (const MoveList&)` and `operator= (const MoveList&)`
- `operator==` and `operator!=`
- `front()` and `back()`, now checked with `NOEXCEPT_EXPECTS`

Left throwing:

- The `initializer_list` constructor. `moveParse()` throws on a bad
  string, which is input.
- `asString()`, the free `asString()` and `operator<<`. String building
  and stream output can throw.

`ASSERT` was considered for `front()` and `back()` and rejected. They are
on no hot path, and matching `append()` and `removeLast()` matters more
than a check nobody pays for.

## Tests

The doctest case "Reading the ends of an empty move list throws" is
removed. The two `Fatal: ...` cases `front-of-empty` and `back-of-empty`
replace it, next to `remove-from-empty` in `fatal_test_main.cpp`.

## Implementation Progress

### Session #1

Made the changes above.

Verification: Release and Debug builds compile without a warning, the
linter passes, the eleven `Fatal: ...` cases pass in Debug, the two new
ones among them, and all 277 tests pass in Release.
