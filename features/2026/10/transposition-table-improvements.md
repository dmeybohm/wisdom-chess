# Transposition table improvements

Item 5 of [improvement-suggestions.md](../09/improvement-suggestions.md).
Branched from `small-engine-fixups` (PR #342), which added
`BoundType::Empty`, and rebased onto `main` after that and PR #343
(`to-underlying`) were merged.

## Motivation

The table has one entry per index, and any store replaces it unless the
same position is already there at a greater depth
(`TranspositionTable::store()`). Quiescence neither probes nor stores.
[tt-index-metadata.md](../09/tt-index-metadata.md) measured a hit rate of
19.3 to 19.7% at depth 7.

The table is full long before a deep search ends. At the default 16 MB
it holds 524,288 entries, and the depth-8 Italian search visits 13
million main-search nodes, each of which stores.

## The entry today

| Field | Type | Offset | Size |
|---|---|---|---|
| `hash_code` | `BoardHashCode` | 0 | 8 |
| `best_move` | `Move` (`uint16_t`) | 8 | 2 |
| (padding) | | 10 | 2 |
| `score` | `int` | 12 | 4 |
| `depth` | `int16_t` | 16 | 2 |
| `bound_type` | `BoundType` | 18 | 1 |
| (padding) | | 19 | 5 |

That is 24 bytes, 17 of them used. The constructor rounds the entry
count *down* to a power of two, so a 16 MB table is 699,050 entries
rounded to 524,288, and uses 12 MB. A 16-byte entry gives 1,048,576
entries in exactly 16 MB: twice the entries at the same setting.

The rounding had a defect that the 24-byte entry hid: it doubled up to
the count and then halved, so a count that was already a power of two
was halved too. With 16-byte entries, every whole number of megabytes is
such a count, and a 16 MB table got 8 MB of entries. `std::bit_floor`
rounds down correctly.

## What the fields need

- **Score.** A stored score is within ±`Checkmate_Score` = 1,152,000.
  `scoreToTT()` turns a mate score into a distance from the node, which
  keeps it within that bound, and the timeout's `-Initial_Alpha` is
  never stored. A signed 22-bit field holds ±2,097,151, so the score
  fits in 22 bits without changing the score constants.
- **Depth.** 1 to `Max_Search_Depth` = 64, which `IterativeSearch`
  requires. That is 7 bits, or 6 bits if stored as `depth - 1`.
  Quiescence entries, if added, would need depth 0 as well.
- **Bound.** Four values with `Empty`: 2 bits, or its own byte.
- **Move.** 16 bits, already packed.
- **Hash.** All 64 bits are kept. The index uses the folded hash's low
  bits, so a shorter check value would raise false matches. Not changed
  here.

## Packing to 16 bytes

Two layouts reach 16 bytes:

1. **Plain fields.** Reorder to `hash_code`, `score`, `best_move`, then
   `depth` as `uint8_t` and `bound_type`: 8 + 4 + 2 + 1 + 1 = 16, with
   no bit operations. No byte is left over.
2. **A packed word.** A 32-bit `DepthAndScoreBits` holds the depth in
   7 bits and the score in the other 25. With `hash_code`, `best_move`
   and `bound_type` in its own byte, that is 15 bytes, and one byte is
   free in a 16-byte entry. Moving the bound into the word as well
   would free a second byte, at the cost of 2 score bits and another
   mask.

The packed word costs a shift and a mask on every probe and store. What
it buys is the free byte, and step 2 wants it for a generation number.
Without one, the replacement rule cannot tell an entry from the current
search from a deep entry left by an earlier move, since the table is
kept across moves.

**Chosen: the packed word**, with the bound in its own byte, so the
layout is settled once and step 2 needs no second change to the entry. The score constants stay as
they are. `Max_Non_Checkmate_Score` is 64 queens at the larger score
scale, far above any reachable evaluation. A tighter bound would free
bits, but nothing needs them yet.

## Plan

Each step changes what the search returns, so each is measured on its
own before the next.

1. **16-byte entries.** The packed word, with `EXPECTS` on the score and
   depth ranges when packing. `static_assert` the entry at 16 bytes. The
   first measurement is the doubled entry count at the same megabytes.
   The generation byte is added here, unused, because a padding byte in
   its place made `clear()` slow (Session #2).
2. **Buckets.** Four entries in a cache line, replaced by depth and
   age. See [Step 2: buckets](#step-2-buckets).
3. **Quiescence probe and store.** At the horizon only. See
   [Step 3: the horizon](#step-3-the-horizon).

## Step 2: buckets

### Layout

A `TranspositionBucket` is `alignas (64)` and holds four entries, so a
bucket is one cache line and a probe reads one line, as it does today.
The table is a `vector` of buckets; `std::allocator` honours the
alignment since C++17, Emscripten included. The bucket index is
`foldHashTo32Bits (hash) & my_bucket_mask`. A 16 MB table is 262,144
buckets.

`getSize()` keeps counting entries, so the search's
"entries = stored/size" line keeps its meaning. `fromEntries()` requires
a power of two of at least four, one bucket. The tests that use two
entries move to four, and the fatal case for one entry changes its
message.

### Finding a position

All three operations scan the bucket's four entries for an entry whose
`hash_code` matches and whose bound is not `Empty`. The bound test keeps
an empty entry from matching a position that hashes to zero.

`probe()` and `getBestMove()` each scan the bucket, one after the other,
from `search()`. The second scan hits the same cache line. Merging the
two calls is a separate change.

### Generations

`TranspositionTable::startSearch()` advances an 8-bit generation, and
`IterativeSearchImpl::iterativelyDeepen()` calls it once, before the
first iteration, so the iterations of one search share a generation.
`clear()` sets it back to 0. An entry's age is
`(generation - entry.generation)` in 8 bits, so wrapping is harmless.
An entry 256 searches old looks new, and only competes on depth.

A store sets the entry's generation. So does any scan that finds the
position, in `probe()` or `store()`, even when it returns nothing or
keeps the deeper entry: the position is in this search's tree, so its
entry is current.

### Replacement in `store()`

1. **The same position:** today's rule. Keep a deeper entry, else
   overwrite.
2. **Else an empty entry**, the first one.
3. **Else the entry worth least**, by `depth - Age_Weight * age`, the
   first on a tie.

The alternative to step 3 evicts by age first and depth second. It
throws out a depth-12 entry from the last move before a depth-1 entry
from this one. Two plies later, the last move's deep entries cover much
of the new tree, which is what `runWarmTableBenchmark` measures. The
weighted rule keeps them while they are deep enough. `Age_Weight` starts
at 8, so each search an entry has outlived counts as 8 plies of depth: a
depth-10 entry from the last search ranks with a depth-2 entry from this
one. If the match is close, the other rule and other weights are the first
things to try.

`stored_entries` keeps its meaning: an empty entry filled.

### Tests

One bucket, from `fromEntries (4)`, makes every hash collide:

- four positions fill the four entries, and a probe finds each
- a fifth replaces the shallowest, all of one generation
- after `startSearch()`, a fifth replaces an older entry over a newer
  one of the same depth, and keeps an older one deep enough to outweigh
  its age
- a store of a position already in the bucket replaces it, or keeps it
  when deeper, without touching the others
- a probe that finds a position refreshes its generation, so it
  survives the next replacement
- a zero hash does not match an empty entry
- `clear()` empties every entry and resets the generation

### Measuring step 2

Base is step 1 (`bf326419`).

- `--search-report 7`: every row clears the table, so this measures the
  buckets alone, without ages.
- `runWarmTableBenchmark` (part of the default benchmark run): 30
  plies at depth 6 with the table kept, which is where the generation
  matters.
- The engine match, which decides.

## Step 3: the horizon

Quiescence neither probed nor stored, which
[quiescence-search.md](../09/quiescence-search.md) left as a later
experiment. A quiescence score depends on more than the position: in
check at `Max_Quiescence_Evasion_Ply` (4), `quiesce()` returns the static
evaluation instead of searching the evasions. The same position searched
at quiescence ply 0 and at ply 3 can score differently, so an entry made
deep in quiescence, with its evasion budget spent, would be wrong at a
node that has the whole budget.

Every node at the main search's horizon, where `search()` reaches depth
0, starts quiescence at ply 0 with the whole budget. So `search()`
probes and stores there, around its call to `quiesce()`, and
`quiesce()` itself is unchanged:

- The entry is stored at depth 0. A main-search probe asks for depth 1
  or more, so it never takes one. The horizon probe asks for depth 0,
  so a deeper entry for the same position answers it, with a better
  score.
- A store does not replace a deeper entry for the same position, and a
  depth-0 entry is the first one a full bucket gives up.
- Quiescence has its own draw check, so the rule for path-dependent
  scores applies: a horizon node whose subtree hit a draw is not
  stored. Nor is a node cut short by the clock.
- The probe comes before the stalemate test in `quiesce()`, so a hit
  saves that too.
- The entry has no move: quiescence does not order by the table.

Storing at every quiescence node, keyed by the evasion budget left,
is a separate change, to be measured on its own.

## Measuring

[tt-index-metadata.md](../09/tt-index-metadata.md) found that changing
only how positions map to slots moves node counts by up to 19% between
Zobrist seeds, and one position by 37%. Any change to the table does
that, so node counts from one build are not evidence either way. Use:

- the engine match (`scripts/run-engine-match.sh`, 500 games at 8+0.08)
  as the deciding measure for each step
- `--search-report` time and hit rate as a check, over several seeds if
  a result is close
- unit tests for the packing (round trips at the field limits, mate
  scores at ply offsets) and for replacement within a bucket

## Implementation Progress

### Session #1

- Wrote this plan. No code changed.
- Checked the fields' ranges against `global.hpp` and `search.cpp`: the
  score fits in 22 bits as the constants stand, so reducing the maximum
  score is not needed for the packing.

### Session #2

Step 1, in three commits:

- `DepthAndScoreBits` packs the depth in 7 bits and the score in 25.
  The entry is 16 bytes and `static_assert`ed so. `EXPECTS` checks both
  ranges, and fatal cases cover them. Unit tests round-trip each
  field's limits and mate scores across plies.
- The constructor halved a power-of-two entry count (see "The entry
  today"). The first depth-7 report showed identical node counts in
  both builds, which led to it. `std::bit_floor` fixes it, and a sizing
  test pins it.
- `clear()` was 6 to 10 times slower with the 16-byte entry, 15 to 30
  ms per clear against 2 to 4 ms. GCC filled the table by copying a
  zeroed entry through the stack, and the copy's 8-byte reload
  overlapped the 16-byte store, which stalled store forwarding on every
  entry. A stand-in program with the same layout did not reproduce it.
  Filling the last byte with the step-2 `generation` field did: the
  clear went back to direct stores and the base build's speed.

Measured from copies of the source in the scratchpad: base is
`small-engine-fixups` (`237d98c9`), new is `bf326419`. Release, GCC,
`--search-report 7`, pinned to one core, alternating rounds. Another
session's test runs loaded the machine during rounds 3 to 5, so only
rounds 1 and 2 are usable:

| Position | Move, score | Nodes, base | Nodes, new | Time, round 1 | Time, round 2 |
|---|---|---|---|---|---|
| starting | e2 e4, 63 | 354,803 | 352,753 | +3% | −9% |
| kiwipete | e2xa6, 66 | 1,705,188 | 1,691,380 | −8% | +3% |
| italian | b1 c3, −32 | 1,670,231 | 1,580,483 | −4% | −3% |
| position3 | b4xf4, 81 | 79,449 | 72,647 | +1% | −4% |
| position4 | c4 c5, −928 | 851,246 | 849,235 | +4% | 0% |
| middlegame | f3 g5, 57 | 6,359,105 | 5,107,840 | −24% | −21% |
| all six | | | | −15% | −12% |

- Every position keeps its move and score.
- The middlegame, the largest search, visits 20% fewer nodes and is
  21 to 24% faster in both rounds. The others are within the noise of
  two rounds.
- Engine matches, base against new, 8+0.08, from
  `scripts/run-engine-match.sh` with its defaults otherwise:

  | Hash | Entries, base / new | Games | W / D / L for new | Elo | 95% range |
  |---|---|---|---|---|---|
  | 16 MB | 524,288 / 1,048,576 | 363 of 500, stopped | 97 / 172 / 94 | +3 | −23 to +29 |
  | 1 MB | 32,768 / 65,536 | 500 | 151 / 199 / 150 | +1 | −23 to +24 |

- The 16 MB match was stopped because the table barely fills at this
  time control. A 250 ms search, about one move's time, reaches depth 4
  or 5 and fills 3,800 entries in the middlegame and 14,000 from the
  start position. Only interior nodes probe and store: a depth-1 node
  hands its children to quiescence, which does not touch the table.
- The 1 MB match puts the table under pressure, and still shows no
  difference.
- Fastchess's interim summaries ("Results of base vs new") count wins
  for the first engine named, base. The script's final table is from
  the later engine's side. Each was checked against the game lines.
- Step 1 stays: it gains in deep searches (the middlegame row above)
  and costs nothing measurable. At 8+0.08 the search is too shallow
  for the table to decide games, so a match at this time control will
  likely not separate step 2 either. Deeper searches, from item 4,
  would make the table matter more.

### Session #3

- Planned step 2 while step 1's match ran. No code changed.

### Session #4

- Implemented step 2 as planned, with one change: `getBestMove()` makes
  an entry current too, since all three operations share the scan.
  Three of the bucket tests fail with `Age_Weight` at 0, so they test
  the ageing and not only the depth.
- Rebased onto `main` at `efddf228`. The commits measured above
  (`bf326419` for step 1, `237d98c9` for its base) are from before the
  rebase; the code they measured is unchanged.
- PR #343 left this branch's files as `static_cast` for this branch to
  convert ([to-underlying.md](to-underlying.md)). `foldHashTo32Bits()`
  uses `truncate`, `computeHitRate()` uses `to_double`. In the packed
  word, `getDepth()` uses `narrow_debug`, since it runs on every probe
  and cannot fail, and `make()` uses `to_unsigned_debug` after its
  `EXPECTS` on the depth. The two `std::bit_cast`s stay: they
  reinterpret bits rather than convert a value.
- Step 2 is not measured yet.

### Session #5

Measured step 2 against step 1 and `main`, rebuilt from the rebased
commits in the scratchpad: `main` at `efddf228` (24-byte entries), step
1 at `44215789` and step 2 at `edf38e23`. Release, GCC, pinned to one
core, on a quiet machine. The warm-table benchmark ran through a
scratch-only hook in `runSearchReport()`.

`--search-report 7`, five alternating rounds, medians:

| Position | Nodes: main → step 1 → step 2 | Time: main | Step 1 | Step 2 | Step 2 vs step 1 |
|---|---|---|---|---|---|
| starting | 354,803 → 352,753 → 336,232 | 0.290 s | 0.293 s | 0.284 s | −3.1%, faster 5 of 5 |
| kiwipete | 1,705,188 → 1,691,380 → 1,678,629 | 1.562 s | 1.544 s | 1.531 s | −0.8%, faster 3 of 5 |
| italian | 1,670,231 → 1,580,483 → 1,461,432 | 1.897 s | 1.796 s | 1.621 s | −9.7%, faster 5 of 5 |
| position3 | 79,449 → 72,647 → 72,587 | 0.075 s | 0.071 s | 0.071 s | 0.0%, faster 2 of 5 |
| position4 | 851,246 → 849,235 → 814,124 | 0.806 s | 0.812 s | 0.782 s | −3.7%, faster 5 of 5 |
| middlegame | 6,359,105 → 5,107,840 → 4,196,236 | 5.609 s | 4.323 s | 3.473 s | −19.7%, faster 5 of 5 |
| all six | | 10.24 s | 8.84 s | 7.76 s | −12% |

- Every build chooses the same move with the same score everywhere.
- Step 2 visits fewer nodes than step 1 in every position, and is 24%
  faster than `main` in total and 38% in the middlegame.

Warm-table benchmark, 30 plies at depth 6, three alternating rounds,
medians:

| Build | Cleared before each search | Table kept | Saving |
|---|---|---|---|
| main | 10.685 s | 8.782 s | −17.8% |
| step 1 | 10.557 s | 8.776 s | −16.9% |
| step 2 | 10.476 s | 8.475 s | −19.1% |

- With the table kept, step 2's slowest run (8.604 s) beat step 1's
  fastest (8.738 s). A 30-ply game at depth 6 does not fill 16 MB, so
  this shows the generation causes no harm more than it shows a gain.

Engine match, step 2 against step 1, 8+0.08 with 1 MB of hash, so
both have 65,536 entries and only the buckets and ageing differ:

| Games | W / D / L for step 2 | Score | Elo | 95% range |
|---|---|---|---|---|
| 500 | 137 / 255 / 108 | 52.9% | +20 | −1 to +42 |

- Step 2 led from game 100 on, by +27 to +30 through game 400, and
  the last hundred games narrowed it to +20. The chance that step 2 is
  stronger is 98%.
- This is the first table change to show up in a match. The step 1
  matches showed none, with 16 MB or 1 MB.
- Not measured: a match at the default 16 MB, which at this time
  control would not fill the table either.

### Session #6

- Step 3: `search()` probes and stores at the horizon, around its call
  to `quiesce()`. The bound type for both stores comes from one helper,
  `boundTypeOf()`.
- New test: a depth-1 search from the start position stores 21 entries,
  the root and the 20 positions after White's first moves, and the
  same search again takes 20 hits. Before step 3 it stored only the
  root.
- Release: all 331 tests pass. Debug: the 318 fast and medium tests
  pass. Lint is clean.
- Measured step 3 against step 2 and `main` (`efddf228`), as in
  Session #5, under `systemd-inhibit`, with no other builds or tests
  running. `--search-report 7`, five alternating rounds, medians:

  | Position | Nodes: step 2 → step 3 | Time: step 2 → step 3 | Change | Rounds faster |
  |---|---|---|---|---|
  | starting | 336,232 → 333,381 | 0.268 s → 0.310 s | +15.7% | 0 of 5 |
  | kiwipete | 1,678,629 → 1,628,027 | 1.495 s → 1.613 s | +7.9% | 0 of 5 |
  | italian | 1,461,432 → 1,437,998 | 1.560 s → 1.650 s | +5.8% | 1 of 5 |
  | position3 | 72,587 → 70,416 | 0.072 s → 0.068 s | −5.6% | 5 of 5 |
  | position4 | 814,124 → 750,721 | 0.759 s → 0.719 s | −5.3% | 4 of 5 |
  | middlegame | 4,196,236 → 4,358,692 | 3.280 s → 4.035 s | +23.0% | 0 of 5 |
  | all six | | 7.43 s → 8.39 s | +13% | |

  The middlegame's depth-7 score moved from 57 to 54, with the same
  move; every other move and score is unchanged. Warm-table benchmark,
  table kept, medians of three: `main` 8.45 s, step 2 8.29 s, step 3
  8.86 s.
- Step 3 is slower. Nodes fall in five positions but time rises: each
  horizon node now pays for a hash, a probe into a 16 MB table, which is
  likely a cache miss, and a store, where about half of these nodes
  would otherwise stop at the stand-pat test. Not kept as it stands.
