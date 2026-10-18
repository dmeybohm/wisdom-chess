# Split the general-purpose code into wisdom-chess-lib

The engine directory held two kinds of code: the chess engine, and the
general-purpose headers it is written with, which know nothing about
chess: the standard-type aliases, the contract checks, the named
conversions, the compact optional, the pointer types, the logger and the
string helpers. They depended only on each other, but nothing enforced
that. This moves them to `src/wisdom-chess/lib` as their own library,
`wisdom-chess-lib` (alias `wisdom::lib`), which the engine links. The
engine library is renamed from `wisdom-chess-core` to
`wisdom-chess-engine`; its alias `wisdom::chess` is unchanged.

## What moved

- `types.hpp`, `error.hpp`, `cast.hpp`, `optional.hpp`, `ptr.hpp`,
  `expected.hpp`, `logger.hpp`/`.cpp`, `str.hpp`/`.cpp` and
  `random.hpp`, under the same names.
- `terminateOnCheckFailure()`, which was in `engine/global.cpp` beside
  nothing else, is now `lib/error.cpp`. `global.cpp` is gone.
- The tests of that code: `logger_test`, `str_test`, the fatal cast and
  contract cases, and the old `global_test`, split into `cast_test`,
  `ptr_test`, `contract_test` and `random_test`. `optional_test` is split
  in two: the library's tests the mechanism with a local type and an
  enum, and the engine's checks that the chess types opt in.
- The fatal test harness, `fatal_test.hpp`, `fatal_test_main.cpp` and the
  `wisdom-chess-fatal-main` library, since the library's fatal tests
  need it and it depends only on the logger. The engine's and the
  frontends' fatal tests include it by its full path,
  `"wisdom-chess/lib/test/fatal_test.hpp"`, as the view model already
  did. A fatal test program made with `wisdom_chess_add_fatal_tests()`
  now links `wisdom::chess` itself when it needs it.

## What stayed

- `engine/global.hpp`, which is now only the chess constants and the
  engine's precompiled header. It includes the lib headers, so code
  that includes it sees nothing new.
- The `CompileTimeRandom` default seed and state. They were tuned for
  the Zobrist hash, but they are the generator's defaults and
  `seed_optimizer` reads them from `random.hpp`, so moving them would
  have changed the API for no gain.

## Build

- The `doctest` and `expected` packages are fetched in
  `src/wisdom-chess/CMakeLists.txt`, since both libraries use them and a
  variable set in one subdirectory is not visible in a sibling.
  `doctest_FOUND` turns the tests off for both from there.
- `wisdom-chess-lib` carries the `expected` include directory,
  `Threads::Threads`, the Emscripten `-pthread` and the
  `-fno-stack-protector` the engine had, all propagated to the engine
  through the `PUBLIC` link.
- New test programs: `wisdom-chess-lib-tests` (label `fast`) and
  `wisdom-chess-lib-fatal-tests`.

## Implementation Progress

### Session #1 (2026-10-18)

- Moved the files with `git mv`, rewrote the include paths, narrowed
  the moved headers' includes to the lib headers they use (`str.hpp`
  and `random.hpp` had included `global.hpp`), split the tests, wrote
  the lib's CMake files, hoisted the packages and renamed the engine
  target. Updated `AGENTS.md`, `docs/building.md`,
  `docs/coding-style.md` and `docs/engine-architecture.md`.

### Session #2 (2026-10-18)

- Review found that the engine's `add_subdirectory(bench)` block had
  been dropped with the package blocks, so `WISDOM_CHESS_BENCHMARKS`
  built nothing. Restored it and built `wisdom-chess-benchmarks` to
  confirm.
- The MSVC CI job failed on the lib tests with doctest's C5285 warning,
  which the view model, web and UCI test targets already silence with
  `/wd5285`. The lib tests get the same block.
