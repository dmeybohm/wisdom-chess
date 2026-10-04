# Transposition table improvements

Item 5 of [improvement-suggestions.md](../09/improvement-suggestions.md).
Branched from `small-engine-fixups` (PR #342), which added
`BoundType::Empty`.

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

## What the fields need

- **Score.** A stored score is within ±`Checkmate_Score` = 1,152,000.
  `scoreToTT()` turns a mate score into a distance from the node, which
  keeps it within that bound, and the timeout's `-Initial_Alpha` is
  never stored. A signed 22-bit field holds ±2,097,151, so the score
  fits in 22 bits without changing the score constants.
- **Depth.** 1 to `Max_Search_Depth` = 64, which `IterativeSearch`
  requires. That is 7 bits, or 6 bits if stored as `depth - 1`.
  Quiescence entries, if added, would need depth 0 as well.
- **Bound.** Four values with `Empty`: 2 bits.
- **Move.** 16 bits, already packed.
- **Hash.** All 64 bits are kept. The index uses the folded hash's low
  bits, so a shorter check value would raise false matches. Not changed
  here.

## Packing to 16 bytes

Two layouts reach 16 bytes:

1. **Plain fields.** Reorder to `hash_code`, `score`, `best_move`, then
   `depth` as `uint8_t` and `bound_type`: 8 + 4 + 2 + 1 + 1 = 16, with
   no bit operations. No byte is left over.
2. **A packed word.** A 32-bit `DepthAndScoreBits` holds the score in 22
   bits, the depth in 7 and the bound in 2, with 1 bit spare. With
   `hash_code` and `best_move`, that leaves two free bytes in a 16-byte
   entry.

The packed word costs a shift and a mask on every probe and store. What
it buys is the free bytes, and step 2 wants one for a generation number.
Without one, the replacement rule cannot tell an entry from the current
search from a deep entry left by an earlier move, since the table is
kept across moves.

**Recommendation: the packed word**, so the layout is settled once and
step 2 needs no second change to the entry. The score constants stay as
they are. `Max_Non_Checkmate_Score` is 64 queens at the larger score
scale, far above any reachable evaluation. A tighter bound would free
bits, but nothing needs them yet.

## Plan

Each step changes what the search returns, so each is measured on its
own before the next.

1. **16-byte entries.** The packed word, with `EXPECTS` on the score and
   depth ranges when packing. `static_assert` the entry at 16 bytes. The
   first measurement is the doubled entry count at the same megabytes.
2. **Buckets.** Four 16-byte entries in a 64-byte, cache-line-aligned
   bucket. Probe all four. Store over the same position (keeping the
   rule for a greater depth), else an empty entry, else one from an
   older generation, else the shallowest. A one-byte generation is
   advanced per search, from `IterativeSearch`, and cleared with the
   table.
3. **Quiescence probe and store.** Probing in quiescence was left as a
   later experiment in [quiescence-search.md](../09/quiescence-search.md).
   It stores at depth 0, so a main-search probe at depth ≥ 1 never takes
   one. A quiescence node needs the same path-dependent-score rule as the
   main search, if any draw check can reach it.

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
