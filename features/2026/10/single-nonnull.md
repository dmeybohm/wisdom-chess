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

- Constructing from a null `T*` reports through `logEmergency()` and
  aborts. The constructor is `noexcept` and checks with
  `NOEXCEPT_EXPECTS`. `gsl::not_null` called `std::terminate` there, with
  no message. See "Abort or throw" below.
- It converts from a `nonnull` of a derived or less-const type, without a
  check, and compares by address, as `nullable` does.
- There is no implicit conversion to `T*`. An API that takes a raw
  pointer gets `get()`. `gsl::not_null` converted silently, which also
  let a `nonnull` be tested like a `bool`.
- It is not constructible from a `nullable`. `unchecked_nonnull` was,
  with a check, but nothing used it. `nullable::value()` stays the one
  way across, and it throws `PreconditionError` on null: that is where a
  pointer that may be null is turned into one that may not.

Where WebAssembly differs: address 0 is ordinary memory there, so a null
dereference reads garbage instead of trapping. The check on each
dereference would have turned that into an abort, but only for a pointer
zeroed after it was constructed.

### Abort or throw

The first version checked with `EXPECTS`, which throws. It was changed to
abort for three reasons:

- A null handed to a `nonnull` is a bug in the program, not input.
- Every function Qt calls is `noexcept` and many build a `nonnull` from a
  raw pointer, so a throw there ends in `std::terminate` anyway. Under
  Emscripten an exception through `noexcept` reaches the terminate
  handler with nothing to report, and the message is lost.
- The constructor can be `noexcept`.

What it gives up: `ChessEngine`'s `guarded()` cannot turn a null into
`engineFailed`. The application aborts instead.

### No handler that throws

An aborting check cannot be tested inside doctest. A handler that tests
could install to make the check throw instead was considered and
dropped. `NOEXCEPT_EXPECTS` is used in `noexcept` functions, and an
exception from the handler would call `std::terminate` at that boundary.
It would work only for checks in functions that are not `noexcept`, which
can already use `EXPECTS`.

The tests for aborting checks are the `Fatal: ...` tests, each a process
of its own. What made them awkward was that a case had to be registered
in three places: its function, the dispatch in `main()` and a line in
`CMakeLists.txt` with the expected message. `fatal_test_main.cpp` now
has a table, `Fatal_Cases`, of name, expected message and function.
After each build of the program, `discover_fatal_tests.cmake` runs it
with `--list` and writes the tests for CTest, as doctest's discovery
does. A case prints the message it expects before it triggers the error,
and `run_fatal_test.cmake` reads it from the output. The expected
message is a regular expression, and passing it through a CMake list
would break on its brackets.

A case that a build cannot run is left out of the list by a flag in the
table. `assert-failure` follows `Debugging`, where it followed
`CMAKE_BUILD_TYPE`, so it now also runs in the Debug configuration of a
multi-configuration generator.

## Implementation Progress

### Session #1

Replaced the class and removed `unchecked_nonnull`. One place relied on
the implicit conversion to `T*`: `instanceFor()` in `qml_singletons.cpp`,
which now passes `instance.get()` to `QJSEngine::setObjectOwnership()`.
The two `unchecked_nonnull` members, in `generate.cpp` and `search.cpp`,
became `nonnull`. `small-fixups` gained "Move the standard library names
to types.hpp" meanwhile, which was merged in without conflicts.

The tests of `unchecked_nonnull` in `global_test.cpp` became a `nonnull`
test case, with the conversions and the comparison added. The null case
is `Fatal: null-nonnull`.

**Code size.** The first version made the binaries larger, not smaller.
Dereferences lost their check, but every `nonnull` built from a pointer
the compiler cannot prove non-null gained a larger failing path than
GSL's bare call to `std::terminate`. `GameModel::getGame()` went from 6
instructions to 20, with a stack frame and a stack-protector canary on
the path where the pointer is not null.

The cause was not the throw. `expects()` and the functions under it took
`const std::source_location&`, so each caller kept the location in a
temporary on its stack. Switching to `NOEXCEPT_EXPECTS` alone left the
same frame. Taking the location by value, which is one pointer with
libstdc++ and libc++, moved all of it to the failing path, and
`getGame()` is back to a load, a test, a branch and a return. This
applies to every `EXPECTS`, `ENSURES`, `NOEXCEPT_EXPECTS` and `ASSERT` in
the code, not only to `nonnull`.

Bytes of `.text`, GCC 13, Release:

| Binary | Before | `EXPECTS` | `NOEXCEPT_EXPECTS` | Location by value |
|---|---|---|---|---|
| `WisdomChessQml` | 493,024 | 494,208 | 493,952 | 492,434 |
| `wisdom-chess-uci` | 239,742 | 239,790 | 239,774 | 239,502 |
| `wisdom-chess-qml-pieces-model-test` | 345,418 | 348,554 | 346,458 | 344,826 |

No benchmark was run. The two hot sites already used `unchecked_nonnull`,
and the collapse left the search and the move generator alone: in the
first version only eight functions of `wisdom-chess-uci` differed from
before, none of them in either. Taking the location by value does change
them, since `MoveList::append()` checks with `NOEXCEPT_EXPECTS`. Its
effect on speed was not measured.

**Verification.** All of it on the final commit unless noted.

- GCC 13, Release with the QML frontend and the slow tests: 276 of 276
  tests pass, and `lint` is clean.
- GCC 13, Debug with the tools and the benchmarks built: the 234 fast
  tests pass, `Fatal: assert-failure` among them.
- Clang 18 with AddressSanitizer and UndefinedBehaviorSanitizer,
  warnings as errors, QML frontend: 276 of 276.
- Emscripten: `wisdom-chess-web` builds and the 160 fast tests pass under
  Node. Five fatal cases are listed there; the three that need an
  uncaught exception reported are not.
- `run_fatal_test.cmake` was run against a stand-in program for each way
  a case can fail: the wrong message, carrying on past the error, a
  normal exit, and an unknown case. Each was rejected.

Not run: Android, Windows and macOS builds, ThreadSanitizer, and the
React frontend in a browser.
