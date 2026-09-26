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
