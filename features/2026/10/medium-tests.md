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

### Session #2

The Debug fast suite still took 28 s. Run one at a time, 27.1 of its
33.5 s were `generateLegalEnPassantMoves`, which takes 0.55 s in
Release. Its "Agrees with generateLegalMoves" subcase walked seven move
trees to depth 3. Counting nodes and en passant targets per tree:

| Start | Debug | Nodes at depth 3 | En passant positions, depth 0/1/2/3 |
|---|---|---|---|
| kiwipete | 14.5 s | 97,862 | 0/1/45/1929 |
| f6 target | 2.0 s | 21,637 | 1/0/0/56 |
| position 4 | 1.6 s | 9,467 | 0/0/4/0 |
| starting position | 0.8 s | 8,902 | 0/0/0/0 |
| pawn wall | 0.5 s | 6,489 | 0/0/0/14 |
| position 3, each side | 0.3 s | ~3,000 | 0/0/2/123, 0/0/3/74 |

- The starting position reaches no target in three plies and is gone.
  Kiwipete and position 4 go to depth 2, which keeps 46 and all 4 of
  their positions; the rest find theirs at the leaves and stay at 3.
- The deep check moves to the slow suite, as "Perft:
  generateLegalEnPassantMoves agrees with generateLegalMoves at every
  node", beside the `generateCaptures` and `hasLegalMove` ones. It goes
  deeper than the old subcase (position 3 to 5, the pawn wall to 4),
  requires each tree to reach an en passant position, and takes 2.5 s in
  Release.

| Build | `fast` | `medium` |
|---|---|---|
| Release | 0.5 s | |
| Debug | 3.3 s, was 28.4 | 12.2 s |

- Verified lint, all 295 Release tests with the slow suite, and the
  Debug fast and medium suites.
