# One `nonnull` pointer type

Branch: `single-nonnull`, based on `small-fixups`.

## Motivation

`engine/ptr.hpp` had two pointers that are never null. `nonnull<T>` was
`gsl::not_null<T*>`, which checks for null when constructed and again on
every `get()`, `->` and `*`. `unchecked_nonnull<T>` was our own class,
which checks only when constructed. AGENTS.md said to use the second only
where a benchmark showed the check on each dereference costs something.

The check on each dereference guards nothing a program can do:

- A `nonnull` cannot become null after it is constructed. Construction is
  checked and assigning `nullptr` does not compile.
- `gsl::not_null` has no moved-from state either. GSL 4.0.0 declares only
  its copy operations, so a move copies and the source keeps its pointer.
  This holds for a `not_null` of a smart pointer as well as of a `T*`.
- What is left is memory corruption and lifetime bugs. A corrupted pointer
  is rarely exactly zero, a null dereference already faults on the native
  targets, and the sanitizer runs in CI find lifetime bugs far better.

It was not free. `nonnull-observer-params.md` (2026/09) measured a
`test`/`je` at every dereference, and `search()` growing from 428 to 3,550
instructions with GCC when its table pointer was a `nonnull`.

The rule for choosing between the two could not be followed. The same log
records that the benchmarks could not resolve the difference, and that
the assembly decided the two sites that used `unchecked_nonnull`.

## Design

`nonnull<T>` is now the class that `unchecked_nonnull<T>` was, and the
second name is gone. The 91 places that spell `nonnull<...>` keep their
spelling.

- Constructing from a null `T*` throws `PreconditionError`, through
  `EXPECTS`, like every other check of caller input. `gsl::not_null`
  called `std::terminate` there, with no message.
- It converts from a `nonnull` of a derived or less-const type, without a
  check, and compares by address, as `nullable` does.
- There is no implicit conversion to `T*`. An API that takes a raw
  pointer gets `get()`. `gsl::not_null` converted silently, which also
  let a `nonnull` be tested like a `bool`.
- It is not constructible from a `nullable`. `unchecked_nonnull` was,
  with a check, but nothing used it. `nullable::value()` stays the one
  way across, so the place where a null would throw is visible.
  `value()` no longer checks before it builds the `nonnull`, which it did
  only so that a null would throw instead of terminate.

Where WebAssembly differs: address 0 is ordinary memory there, so a null
dereference reads garbage instead of trapping. The check on each
dereference would have turned that into an abort, but only for a pointer
zeroed after it was constructed.

## Implementation Progress

### Session #1
