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
- Rating run, `new=HEAD sf1700=stockfish:1700 sf1900=stockfish:1900
  sf2100=stockfish:2100` at 8+0.08, 500 games per anchor, 2 hours 18
  minutes, every game ended normally:

  | Pairing       | W / D / L      | Score | Elo  | 95% range    |
  |---------------|----------------|-------|------|--------------|
  | new vs sf1700 | 302 / 37 / 161 | 64.1% | +101 | +71 to +132  |
  | new vs sf1900 | 257 / 52 / 191 | 56.6% | +46  | +17 to +76   |
  | new vs sf2100 | 154 / 67 / 279 | 37.5% | -89  | -119 to -60  |

  Ordo: **`new` at 1927 ± 19** on Stockfish 19's `UCI_Elo` scale at
  8+0.08, white advantage 10 ± 9, draw rate between equals 11.7%.
  The three pairings alone give 1801, 1946 and 2011, so the anchors
  disagree among themselves by about 200 Elo over a 400 Elo span: the
  limited-strength Stockfish is weaker at the low end than its number
  says, or stronger at the high end. That is the calibration's error,
  not the match's, and is why the rating is quoted on Stockfish's scale
  with the time control, and why CCRL-rated anchors are the next step
  for a CCRL number.
- Results: `~/.cache/wisdom-chess/match/results/20261018-101458-new-sf1700-sf1900-sf2100`.

### Session #2 (2026-10-18)

Two review comments on PR #384:

- A match of anchors alone fell back to a round-robin, against the help
  text. Kept, since two Stockfish levels playing each other is how the
  scale's consistency is checked, but now documented, with a note when
  it happens, and Ordo is neither built nor run, as it would only print
  the fixed ratings back.
- An Ordo failure was swallowed and the script exited 0 without the
  rating. Ordo's output now goes to `ordo.log` in the results directory,
  its tail is printed on failure, and the script exits nonzero, after
  still printing how to resume an interrupted match. Checked by
  replacing the cached Ordo with a script that exits 3.
