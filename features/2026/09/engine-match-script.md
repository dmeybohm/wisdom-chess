# Engine match script

## Motivation

Scores, node counts and timings show what a search change does, not
whether it makes the engine stronger. On the `static-exchange-evaluation`
branch that question was answered by hand: build the UCI engine at three
commits, build fastchess, download an opening book, and run a round-robin
(Session #3 of
[static-exchange-evaluation.md](static-exchange-evaluation.md), on the
`static-exchange-evaluation` branch). It showed
that quiescence search is worth about 265 Elo, and that static exchange
evaluation made no measurable difference at 8 seconds plus 0.08 per move.

Each step had a pitfall found along the way:

- Engines from before millisecond search timing lose on time
  ([uci-millisecond-timing.md](uci-millisecond-timing.md)).
- Running more games than there are physical cores put two searches on
  one core, which added noise and shortened the effective time control.
- A laptop that suspends mid-match loses the games in progress on time.

`scripts/run-engine-match.sh` does it all in one command, so a later
search change can be measured the same way.

## Design

```
./scripts/run-engine-match.sh [options] NAME=REF NAME=REF [NAME=REF...]
```

- **Engines.** Each `NAME=REF` is exported with `git archive` into the work
  directory and built in Release with only the console UI on, targeting
  `wisdom-chess-uci`. Exporting rather than building the working tree keeps
  uncommitted changes out of a match. Built engines are cached by commit
  hash, and CPM's downloads are cached too, so a rerun only builds what
  changed.
- **Old engines are refused.** An engine without the `Move Overhead` option
  predates millisecond timing and would lose on time, so the script stops.
- **fastchess** is cloned and built at a pinned commit, `60d7a7a26c6b`, so
  results do not shift when fastchess changes.
- **The book** is `8moves_v3.pgn` from `official-stockfish/books`, checked
  against a pinned SHA-256.
- **The match** is a round-robin. Each opening is played once with each
  colour, the openings come in random order from `--seed`, and fastchess
  recovers from an engine crash. The engines get `Move Overhead` 30,
  16 MB of hash, and `Depth` 64 so that the default 16-ply limit never cuts
  a search short at longer time controls.
- **Concurrency** defaults to the number of physical cores. Each game is
  pinned with `-use-affinity` to one logical CPU per physical core, taken
  from `/sys/devices/system/cpu/*/topology/thread_siblings_list`. Asking for
  more games than cores prints a warning and runs without pinning.
- **Sleep.** fastchess runs under
  `systemd-inhibit --what=sleep:idle --mode=block`. KDE Plasma honours it,
  as checked on its power-management agent during the earlier match. It
  does not stop a laptop suspending when its lid is closed.
- **Results** go to `results/<date>-<names>/` in the work directory:
  `games.pgn`, `fastchess.log`, `fastchess.out` (without fastchess's
  warning that the engines print no scores, one per move), `summary.txt`,
  and fastchess's saved state, `config.json`. fastchess writes that to the
  current directory, so it runs from the results directory. The summary records the commits and settings, and ends with
  `scripts/engine-match-tally.py`: wins, draws and losses for each pairing,
  an Elo difference, a 95% range from a normal approximation, and a warning
  for any game lost on time. fastchess's own standings rate each engine
  against the field, not pair by pair.
- The work directory defaults to `$XDG_CACHE_HOME/wisdom-chess/match`.
- Linux only, since it reads the CPU topology and uses `systemd-inhibit`.
  Like `build-tsan.sh`, it never runs `sudo`: a missing tool is reported
  with the command that installs it.

## Implementation Progress

### Session #1

- Wrote `scripts/run-engine-match.sh` and `scripts/engine-match-tally.py`.
- Tested with a fresh work directory:
  - `--build-only old=8b59ede new=main` built the old engine and stopped
    with the message that it predates millisecond timing.
  - `main=main qsearch=quiescence-search see=static-exchange-evaluation`
    at `--tc 2+0.02 --rounds 2` built three engines, fastchess and the book
    in about a minute, played 12 games in 25 seconds, and printed both
    tables.
  - A rerun with `--build-only` took 0.02 s; everything came from the
    cache.
  - During a two-engine run the engines were pinned to CPUs 0 to 3 and the
    sleep inhibitor was held. Afterwards no process was left and the
    inhibitor was gone.
- Invalid arguments fail with a message: one engine, a repeated name, and
  a ref that does not exist.
