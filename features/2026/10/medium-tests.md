# A medium test level

## Motivation

`debug-casts` moved `search_test.cpp` from the slow suite to the fast one
so the Debug CI job would run the search scenarios. That took the fast
suite from 1.0 s to 7.5 s in Release, nearly all of it one test that runs
a series of one-second searches to fill the transposition table as a game
would. The clock is what that test is about, so it stays as it is. The
fast suite is the quick check after a change, and it should stay quick.

## Design

- `search_test.cpp` gets its own executable, `wisdom-chess-medium-tests`,
  labelled `medium`. It is built whenever the fast tests are, not behind
  `WISDOM_CHESS_SLOW_TESTS`.
- Every CI job runs `ctest` without a label, so the Debug, Release,
  sanitizer, Fil-C, WASM and Android jobs all run it, as they did with the
  search tests in the fast suite.
- `ctest -L fast` is the quick check again; `-L medium` picks the search
  scenarios out.

## Implementation Progress

### Session #1

- Added the executable and documented the level in `docs/building.md` and
  `AGENTS.md`.

| Build | `fast` | `medium` |
|---|---|---|
| Release | 257 tests, 1.0 s | 21 tests, 7.1 s |
| Debug | 260 tests, 28.4 s | 21 tests, 13.6 s |

- Verified lint and all tests in both builds.
