# Transposition table ownership

## Motivation

The bug list in
[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md) records
that the transposition table is a value member of `Game::Impl`, so copying a
`Game` copies 16 MB, and the UCI frontend copies the `Game` on every `go`.

Reading the code turned up more than the copy:

- **UCI never keeps a table between moves, copy or no copy.**
  `handlePosition` and `handleNewGame` assign a newly created `Game` to
  `my_game` (`ui/uci/uci_interface.cpp`), and a GUI sends `position` before
  every `go`. The table that `handleGo` copies is already empty. Removing
  the copy alone saves the memcpy and nothing else.
- **The UCI `Hash` option does nothing.** `my_settings.hash_size_mb` is
  written by `setoption` and never read. No frontend can size the table,
  because it is private to `Game::Impl`.
- **Games that never search pay for a table.** `WebGame::my_game` on the
  wasm main thread and the QML main-thread `ChessGame` each allocate 16 MB.
  `ChessGame::clone()` goes through FEN, so the QML engine thread's game
  starts with its own new table as well.
- **`findBestMove` is `const` but writes the table**, which is why the
  member is `mutable`. Two threads calling that `const` method on one
  `Game` would race.

The console, the wasm worker and the QML engine thread keep one `Game` for
the whole game and search with it directly, so their table already survives
between moves. UCI is the only frontend that loses it.

## Design

The table is search state, not game state. It moves out of `Game`, and
whoever runs searches owns one and passes it in:

```cpp
[[nodiscard]] auto findBestMove (
    shared_ptr<Logger> logger,
    TranspositionTable& transposition_table,
    Color whom = Color::None
) const
    -> optional<Move>;
```

`IterativeSearch::create` already takes a `TranspositionTable&`, so the
engine below `Game` does not change.

`TranspositionTable` becomes move-only: copy construction and copy
assignment are deleted. The copy in `Game`'s copy constructor did provide
one real guarantee, that a search on the copy shares nothing with the
original. Move-only keeps that guarantee and makes it a compile-time one.
A table has exactly one owner, it cannot be shared between threads by
accident, and a silent 16 MB copy no longer compiles. `Game` copies shrink
to the board and the history, so UCI keeps its copy-per-`go` design, which
is what isolates the search thread from `position` commands.

Owners:

| Frontend | Owner | Cleared |
|---|---|---|
| UCI | `UciInterface` | `ucinewgame`; rebuilt at the new size by `setoption name Hash` |
| Console | `ConsoleGame` | when a game is loaded or a new one started |
| wasm worker | `worker::GameState` | `workerReinitializeGame` |
| QML | `ChessEngine` | `reloadGame`, which receives the game that replaces the current one |

In UCI the search thread uses the table while it runs, and the main thread
touches it only after `waitForSearchThread()`. `handleSetOption` does not
wait today and must do so before rebuilding the table. The table is not
cleared on `position`: that is the point of the change.

### Reusing entries across moves

Entries are keyed by the position hash and the score is a property of the
position, so an entry written while searching one root is valid under the
next. Two details were checked:

- Mate scores are stored relative to the node (`scoreToTT` /
  `scoreFromTT`), not to the root, so a different root ply does not skew
  them.
- `store()` always replaces an entry with a different hash, so deep entries
  from earlier moves cannot clog the table. No aging scheme is needed.

Scores that depend on the path, through the repetition and fifty-move
rules, are the usual graph-history problem. It exists within a single
search today and is not made worse in kind.

One behaviour change: with a warm table the engine can choose a different
move than it would with a cold one, so UCI results now depend on the
searches before them. `go depth N` on a fresh process is still
deterministic.

### Rejected

- **`shared_ptr` to the table in `Game::Impl`.** The smallest diff, and
  copies would share one table between the UCI main thread's game and the
  search thread's game. That is the hazard the current copy avoids.
- **Allocate lazily in `Game` and do not copy it.** Fixes the memcpy and
  the idle 16 MB, but UCI still learns nothing, `Hash` stays dead, and a
  copy of a `Game` would not be a copy.
- **A `Searcher` class holding the table, timer and depth.** Tidier if
  there were many callers, but `Game::findBestMove` has four, one per
  frontend, and no test calls it. One added parameter is enough. The timer
  and depth can move later if a per-depth result callback (Session #16 of
  the bug list) wants a home.

## Plan

1. `TranspositionTable`: delete the copy operations, default the moves, and
   pin both with `static_assert`s in `transposition_table_test.cpp`.
2. Remove `my_transposition_table` from `Game::Impl` and add the parameter
   to `Game::findBestMove`. Add an engine test that a search through
   `Game::findBestMove` stores entries in the caller's table and returns a
   legal move when called again with the warm table.
3. Give each frontend its table as in the table above. Wire
   `hash_size_mb` to `TranspositionTable::fromMegabytes`.
4. UCI CLI tests: a two-move game (`position`, `go`, `position ... moves`,
   `go`) yields two `bestmove` lines; `setoption name Hash value 1`
   followed by a search still answers; `ucinewgame` between searches.
   Run the UCI tests under the ThreadSanitizer build, since the table is
   now handed between two threads.
5. Benchmark, in `engine/bench/bench_search.cpp`: search the consecutive
   positions of one scripted game to a fixed depth, once clearing the table
   before every search and once only before the first. Report the time for
   each and how many of the chosen moves differ. Record the result here.
6. Update the API notes in `AGENTS.md` and check the item off in the bug
   list.

## Implementation Progress
