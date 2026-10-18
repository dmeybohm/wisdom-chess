# Elo rating

## Motivation

`scripts/run-engine-match.sh` measures the difference between two
versions of the engine, not where either stands. A rating on a known
scale needs an opponent whose rating is known: an anchor. The question
was raised on 2026-10-18 and answered with the cheapest anchor there is,
Stockfish playing with `UCI_LimitStrength` at a chosen `UCI_Elo`.

## Design

### Anchors

The match script takes three kinds of engine, as `NAME=ENGINE`:

- `REF`, as before: this engine, built from a git commit, branch or tag.
- `stockfish:ELO`: Stockfish 19, the `linux-x86-64-universal` release
  from GitHub, downloaded on first use into the work directory and
  checked against a pinned SHA-256, with `UCI_LimitStrength` on and
  `UCI_Elo` set to `ELO`. Its rating is `ELO`. The script reads the
  option's range from the engine and refuses an `ELO` outside it, 1320
  to 3190 for Stockfish 19.
- `uci:PATH[@RATING]`: any UCI engine on the machine, with a rating when
  one is known, such as its CCRL rating. This is for cross-checking the
  Stockfish scale against engines with published ratings.

An engine with a rating is an anchor. `engines.txt` in the results
directory now holds a tab-separated name, kind, value and rating per
engine, so a match with anchors can be resumed; the old `name commit`
form is still read.

### Options per engine

fastchess passes every option on to every engine it was given for, and
Stockfish has no `Depth` option, so `Move Overhead`, `Hash` and `Depth`
are given per engine, each only to an engine whose `uci` reply lists it.
Only the time control goes through `-each`. Stockfish gets
`UCI_LimitStrength=true` and `UCI_Elo` as well.

### Gauntlet

Games between two anchors say nothing about the other engines' ratings,
so with anchors the tournament is fastchess's gauntlet rather than its
round-robin. The engines without a rating are the seeds, listed first,
and the seeds play each other and every anchor; the anchors do not play
each other. This was checked with four engines and two seeds: fastchess
played every pairing except the one between the two non-seeds. Without
anchors nothing changes.

### Rating

When there is an anchor, the tally is followed by Ordo's rating table.
Ordo is built from a pinned source tarball fetched over https and checked
against its SHA-256, like the book and Stockfish, not cloned. It runs
over `counted.pgn`, which the tally script now writes with `--counted`:
the games it counted, so a game replayed after a resume is not counted
twice. Anchors are given through `-m anchors.csv` when there are two or
more. Ordo ignores that file when it names a single anchor, since only
its multiple-anchor path reads it (`rating.c`, `anchored_n > 1`), so one
anchor is given with `-A NAME -a RATING` instead. Errors come from 1000
simulations, and the white advantage and draw rate are fitted.

### Noise

Stockfish at a limited strength picks, on purpose, a move other than the
one its last PV began with, and fastchess warns about that on every such
move, followed by the PV, the position and the move list. Those lines
are filtered from the output and the log, alongside the warning for this
engine's missing scores.

### What the number means

Stockfish's `UCI_Elo` is calibrated at 120 seconds plus 1 a move, against
CCRL 40/4 (`search.h` in Stockfish 19, and the Stockfish wiki). A match
at the script's default 8 seconds plus 0.08 is on a different clock, and
the calibration is a polynomial fit over a handful of points, so a
rating from it is "Stockfish-19-scale Elo at this time control", not a
CCRL rating. A CCRL number needs anchors with CCRL ratings, through
`uci:PATH@RATING`.

## Implementation Progress

### Session #1 (2026-10-18)

- Extended `scripts/run-engine-match.sh` with the `stockfish:` and `uci:`
  engine kinds, per-engine options, the gauntlet, and the Ordo rating;
  `scripts/engine-match-tally.py` gained `--counted`.
- A clone-and-build of Ordo was not permitted in the session, so the
  script fetches a tarball of a pinned commit over https instead, which
  also gives it a checksum to verify.
- Smoke test, `--tc 2+0.02 --rounds 2 sf1500=stockfish:1500
  sf1700=stockfish:1700 new=HEAD`, before the gauntlet: the engines
  were downloaded, built and checked, the 12 games took 18 seconds, and
  the summary ended with the pairwise table and Ordo's table with both
  anchors fixed.
- A gauntlet interrupted with Ctrl-C after 6 of 12 games was resumed with
  `--resume`; the tally counted each game once and the rating followed.
- Bracket, `--rounds 20 new=HEAD sf1600=stockfish:1600
  sf2000=stockfish:2000 sf2400=stockfish:2400` at 8+0.08 (120 games,
  11 minutes):

  | Pairing       | W / D / L   | Score | Elo  |
  |---------------|-------------|-------|------|
  | new vs sf1600 | 26 / 4 / 10 | 70.0% | +147 |
  | new vs sf2000 | 14 / 5 / 21 | 41.2% | -61  |
  | new vs sf2400 | 4 / 1 / 35  | 11.2% | -359 |

  Ordo put `new` at 1884 ± 84 on that scale.
- Rating run started: `new=HEAD sf1700=stockfish:1700
  sf1900=stockfish:1900 sf2100=stockfish:2100`, 500 games per anchor at
  8+0.08.
