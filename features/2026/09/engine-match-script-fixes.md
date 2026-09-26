# Engine match script fixes

## Motivation

A review of `scripts/run-engine-match.sh` and `scripts/engine-match-tally.py`
after they were merged ([engine-match-script.md](engine-match-script.md),
PR #268) found nothing wrong with a normal run, but found problems with
interrupted runs, the cache, bad input, and the tally's statistics on
small samples. The findings were checked by rerunning them or reading the
code. The review mistook a match that was still running for one that had
been interrupted, but the problem with interrupts it inferred is real.

## Plan

1. **Tally statistics.** The score was clamped to at least `0.5 / n` from
   either end before converting to Elo, which printed nonsense on small or
   one-sided samples: one win gave "-0, range -2400 to +2400", ten wins
   "+512", and four draws "-0 to -0". Drop the clamp. Print an Elo only
   with both wins and losses, a range only from 10 games, and say why
   otherwise. Also: clear errors for a missing PGN and for a name with no
   games, a usage line that says two names are needed, a warning for every
   game that did not end normally (from the PGN's `Termination` tag), not
   only for losses on time, and a unit test run through `ctest`.
2. **Interrupts.** fastchess, `grep` and `tee` share a pipeline, so Ctrl-C
   stopped `grep` and `tee` before fastchess's resume message got through,
   and `set -e` then skipped the tally although `games.pgn` held every
   finished game. Keep `grep` and `tee` running through the interrupt,
   always run the tally, and add `--resume DIR` to continue from the run's
   saved `config.json`.
3. **The cache.** Write fastchess and the book through a temporary file and
   rename, as the engine already was, so an interrupted copy or unzip is
   never mistaken for a finished one. Rebuild a cached engine that fails to
   answer `uci`, instead of stopping without an error line.
4. **The refusal message.** `Move Overhead` arrived in `3c48792`, not with
   millisecond timing in `2233100`. Refusing both is right; the message and
   comment named the wrong reason.
5. **Arguments.** An option with no value exited 1 without a message, an
   option could swallow a `NAME=REF`, and `--concurrency abc` became 1
   after the builds. Check every value before building.
6. **Disk.** Delete `build/<commit>` after a successful build; it is about
   180 MB and cannot be reused once its source tree is gone.
7. **Allowed CPUs.** Choose cores only from the process's
   `Cpus_allowed_list`, so a run under `taskset` stays on its CPUs.
8. **Concurrent runs.** Lock the work directory with `flock`, so a second
   run fails at once instead of deleting the first one's builds, pinning to
   the same cores, or sharing a results directory started in the same
   second.
9. **Small fixes.** `Depth` 64 in `--help` and `summary.txt`; `tar`,
   `flock` and `timeout` among the required tools; a proper warning when
   the CPU topology cannot be read; the feature log's link to
   `static-exchange-evaluation.md` marked as on that branch, and its one
   long line rewrapped.
10. **Log size.** fastchess writes its warning that the engines print no
    scores to `fastchess.log` for every move, about 1 MB per 76 games.
    Filter it out when the run ends, as `fastchess.out` already is.

Not done: a line in `AGENTS.md` pointing at the script, and tests for the
shell script itself, which would need fake fastchess, git and builds.

## Implementation Progress

### Session #1

- Step 1. The tally prints "n/a" with a reason instead of an estimate for
  all wins, all losses or all draws, and no range below 10 games. An even
  score prints "0", not "-0". `scripts/tests/engine_match_tally_test.py`
  runs the tally as a program on generated PGNs, in 11 tests; 9 of them
  fail against the tally as merged in #268. It is registered with `ctest`
  as "Scripts: engine match tally", labelled `fast`, when CMake finds a
  Python interpreter. The results of the two earlier matches are
  unchanged.
- Steps 2 to 10 in `run-engine-match.sh`:
  - Interrupts. `grep` and `tee` ignore SIGINT and the script traps it,
    so after Ctrl-C fastchess's "Tournament was interrupted" message gets
    through, the games so far are tallied, and the script prints
    `run-engine-match.sh --resume <results>` and exits with 130.
    `--resume` runs fastchess from the saved `config.json`, which carries
    every setting, and the engine names are kept in `engines.txt` for the
    tally. The summary keeps the partial tally, a "Resumed" line and the
    final one.
  - The cache. fastchess is copied and the book unpacked through temporary
    names. A cached engine that does not answer `uci` within 10 seconds is
    rebuilt; a freshly built one that does not is an error.
  - The refusal names `3c48792`, where `Move Overhead` and the timer fixes
    arrived.
  - Arguments. Every option that takes a value checks that one is there
    and is not the next option, the numbers must be whole numbers in range,
    and `--tc` must look like a fastchess time control, all before any
    building.
  - Disk. `build/<commit>` is removed after a successful build; the build
    log stays.
  - CPUs. Cores are chosen from the process's `Cpus_allowed_list`, one
    logical CPU per physical core. When the topology cannot be read the
    games run unpinned with a warning that says so.
  - The work directory is locked with `flock` on file descriptor 9, held
    until the script and everything it started have exited.
  - `--help` and `summary.txt` mention the depth limit of 64; `tar`,
    `timeout` and `flock` are checked for; `fastchess.log` loses the
    per-move warning when the run ends.
- Tested:
  - Bad arguments: `--tc` with no value, `--tc a=main`, `--concurrency
    abc`, `--rounds 0`, `--tc 8+x`, `--resume` of a directory that is not a
    results directory, and `--resume` with engines. Each fails at once with
    a message.
  - A match from an empty work directory: `build/` held only the build
    logs (16 KB instead of about 180 MB per commit) and `fastchess.log` had
    none of the per-move warnings.
  - Interrupt and resume: SIGINT to the process group after 10 seconds of a
    20-game match. fastchess's resume message came through, the 2 finished
    games were tallied, the resume command was printed, the exit status was
    130, no engine was left running and the sleep inhibitor was released.
    `--resume` then played the rest: fastchess reported 20 games and the
    PGN held 20.
  - The lock: a second match in the same work directory failed at once,
    and the first finished normally.
  - `taskset -c 0,4,1`, where CPUs 0 and 4 share a core: two games at a
    time.
  - A cached engine replaced with a script that exits at once was reported
    and rebuilt.
  - The feature log of the original script had one line of about 110
    columns; rewrapped.

### Session #2

Review comments on PR #272, all three confirmed before fixing:

- **Resume skipped the engine checks.** `--resume` only built fastchess, so
  a cached engine deleted or broken since the interrupt went unnoticed until
  fastchess failed to start it. `engines.txt` now records each engine's
  commit next to its name, and `--resume` passes every commit through
  `build_engine()`, which rebuilds a missing or unresponsive engine. Tested
  by interrupting a match, deleting one cached engine and replacing the
  other with a script that exits at once: `--resume` rebuilt both and
  finished the match.
- **Draws without losses had no Elo.** The tally required both wins and
  losses, so 9 wins and a draw printed "n/a (no losses)" for a score of
  95%. A score has a finite Elo whenever it is strictly between 0% and
  100%, so only all wins or all losses get "n/a" now. 9 wins and a draw
  gives +512, range +311 to +inf.
- **An unfinished game counted as a game.** An engine counted as present
  when it appeared in any game, even one with the result `*`, so a PGN of
  unfinished games printed an empty table and exited 0. An engine now needs
  a finished game, against anyone, or the tally stops with "no finished
  games".

Found while testing the first fix:

- **Resuming replays unfinished rounds.** fastchess's saved state counts
  only completed rounds, so on `--resume` it replays a round that was half
  played when the match was interrupted. The PGN then holds the finished
  game twice: after an interrupt at 13 games, fastchess reported 20 games
  and the PGN held 21. A round and its two colours name one game, and no
  key repeats in the 1,500 and 76 games of the two unresumed matches, so
  the tally now counts only the last game for each and prints a note when
  it drops any. The resumed match then tallied the same 20 games and result
  as fastchess.
- The unit tests gave every game a round of its own for this; four new
  tests cover all losses, wins and draws without losses, unfinished games,
  and a replayed game. 15 tests pass.
