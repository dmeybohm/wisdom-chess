# Non-owning pointers instead of mutable references

## Motivation

AGENTS.md prefers a non-null observer pointer to a mutable `Type&`, for
parameters and members alike, so that a call that may change its argument
shows it with `&` at the call site. Some code predates the rule. The most
visible case was the search: `Game::findBestMove` already took its
`nonnull_observer_ptr<TranspositionTable>`, then dereferenced it only to
hand `IterativeSearch::create` a reference.

## Scope

Converted, in production code: `IterativeSearch` and its table member,
`MoveGeneration::moves`, UCI's `applyMoves`, the board printing helpers,
`getCompileTimeRandom48` and the seed optimizer's `collectPositions`.
The test and benchmark helpers follow the same rule.

Left as references, because the language or an idiom needs them:
`operator<<` on `std::ostream`, `operator++ (Coord&)`, copy and move
operations, `catch` clauses, and local reference variables.

`writeProperty` in `ui/qml/test/settings_test.cpp` also keeps its
`Gadget&`. It is a template, and `Gadget` cannot be deduced from a `T*`
argument through `nonnull<Gadget>`, so every call would have to
name the type.

`doCheck` in `move_perft_test.cpp` never changed its board, so it takes a
`const Board&` instead of a pointer.

## `__restrict`

We decided against marking the observer pointer `__restrict`. Compilers
use restrict for raw pointer parameters. On the member inside
`gsl::not_null` they mostly ignore it, and a wrong no-alias promise is
silent undefined behavior. It is also not standard C++. References and
pointers are treated the same way for aliasing, so the conversion loses
nothing there.

## `unchecked_nonnull`

`gsl::not_null::get()` checks for null on every dereference. For pointers
dereferenced in a hot loop, `global.hpp` adds `unchecked_nonnull<T>`.
It checks once, in its constructor (`expects`, so a null throws
`PreconditionError`), and converts from `nonnull<T>` without a check.
The name says `unchecked` rather than `unsafe` because the pointer is
still never null; only the check on each dereference is skipped.

The two candidates were `MoveGeneration::moves`, touched for every
generated move, and the search's `my_transposition_table`, touched a few
times per node.

## Names

The types started out as `nonnull_observer_ptr` and `observer_ptr`. Since
raw pointers never own (ownership is `unique_ptr` or `shared_ptr`), the
`_observer_ptr` suffix said nothing a reader needed. What a reader does
need is whether the pointer may be null. The types are now:

- `nonnull<T>`: `gsl::not_null<T*>`, checked on each dereference.
- `nullable<T>`: `T*`.
- `unchecked_nonnull<T>`: non-null, checked only when constructed.

`nullable` replaces `observer_ptr`, not alongside it: two names for the
same `T*` would only raise the question of which to use. `global.hpp` no
longer does `using gsl::not_null`, and the few direct uses in the QML
frontend became `nonnull`. `not_null<Game*>` takes the pointer type and
`nonnull<Game>` the pointed-to type, and having both spellings next to
each other would invite mistakes.

`nullable<int>` can read like `std::optional<int>`, as C#'s `Nullable<T>`
does. The AGENTS.md entry says these are pointers.

## Implementation Progress

### Session #1

Converted every site listed under Scope, then renamed the types.

**Benchmarks.** Three builds of `wisdom-chess-benchmarks` were compared:
the references as before (baseline), `nonnull` at both hot sites
(checked), and `unchecked_nonnull` at both (unchecked). The first attempt
ran while an emulator and two IDEs loaded the machine. The same binary
varied by 2-3x between rounds, and benchmarks that touch neither site
moved by 27%, so those numbers were thrown away. The second attempt ran
each binary five times under `taskset -c 3`, rotating the order each
round, and stopped after the first search benchmarks. Medians relative to
the baseline:

| Benchmark | checked | unchecked |
|---|---|---|
| `pseudolegal/*` (6 positions) | +0.5% to +4.2% | +4.0% to +9.2% |
| `legal/*` (6 positions) | -1.4% to +5.3% | -3.9% to +8.4% |
| `isKingThreatened/*` (control) | -0.1% to +1.1% | -1.9% to +3.3% |
| `perft/*` | -0.7% to +2.3% | +0.4% to +2.2% |
| `search/*-depth6` (3 positions) | +1.9% to +2.2% | +2.3% to +3.0% |

The unchecked build was no faster than the checked one, and on the
pseudo-legal move generator it was consistently a few percent slower.
Removing a check cannot cost time, so differences of this size here are
code layout. The assembly below shows that the unchecked move generator is
identical to the baseline's, so these benchmarks cannot resolve effects of
a few percent. At first we kept `nonnull` at both sites on the strength of
the numbers; the assembly reversed that.

**Assembly.** `generate.cpp` and `search.cpp` were compiled with GCC 13
and Clang 18 at `-O3` in four variants: the references (baseline),
`nonnull`, `unchecked_nonnull`, and `unchecked_nonnull` whose dereference
also told the optimizer the pointer is non-null (`GSL_ASSUME`).

- `nonnull` adds a `test`/`je` to `std::terminate` at every dereference:
  3 instructions in `appendMove`, up to 34 in `pawn()`, where it is inlined
  several times. With GCC the checks also change inlining in the search:
  `search()` grows from 428 to 3,550 instructions and `iterate()` shrinks.
- `unchecked_nonnull` produces the same move generator as the references
  with both compilers, apart from symbol names, and a search within two
  instructions of theirs.
- The assume changes nothing with Clang. With GCC it only rearranges the
  blocks of `pawn()`. Nothing in these paths compares the pointer against
  null, so there is no check for it to remove, and it was left out.

A reference parameter carries `nonnull` and `dereferenceable` guarantees
that a pointer does not, which lets the compiler load through it early.
Neither hot site is a parameter, and the converted parameters are not on
hot paths, so this made no difference here.

Both hot sites now use `unchecked_nonnull`, which gives back the
reference versions' code while keeping the pointer style.

**Tightening `nullable`.** None of the 18 `nullable` uses could be null:
the `getGame()` overrides return a member or `ChessGame::state()`, which
is already `nonnull`; the `my_parent` back-pointers are set by their
owners; and the WASM `GameState::getState()`/`getGame()` return a static
instance. All are `nonnull` now. No caller tested them for null, so no
checks became dead. `nullable` is left with no users outside
`global.hpp`. A `nullable` that cannot be dereferenced until it is
converted to `nonnull`, with a lint rule against raw pointers, is waiting
for the first pointer that can actually be null.
