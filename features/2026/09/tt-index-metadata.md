# Metadata bits in the transposition table index

## Motivation

`en-passant-target-state` repacked the 16 metadata bits of `BoardCode`,
moving the castling state from bits 7 and 10 to bits 11 and 13. Nothing
else about the stored codes changed, yet node counts at depth 8 moved a
lot:

| Position | Castling bits at 7 and 10 | At 11 and 13 |
| --- | --- | --- |
| 1.d4 d5 2.c4 | 4,650,526 | 4,548,942 (−2%) |
| Start position | 2,318,521 | 2,939,365 (+27%) |
| Italian (after 3...Bc5) | 40,064,400 | 37,817,531 (−6%) |

That branch confirmed the cause. Moving only `main`'s castling bits
reproduces its node counts exactly, so the layout alone accounts for the
difference.

The table index is
`foldHashTo32Bits (hash) & my_size_mask` (`transposition_table.hpp:10`,
`transposition_table.cpp:88`, `:129`, `:151`). `foldHashTo32Bits` is
`(hash >> 32) ^ hash`, so the index's low 16 bits are the metadata XORed
with Zobrist bits 32–47. Within one castling state, moving the castling
bits XORs every index by the same constant, which is only a relabeling.
Positions in different castling states collide differently, though.
Search is sensitive enough to what the table holds that the tree
diverges from depth 6 onward.

The open question is whether one layout is actually better, or whether
this is chaotic sensitivity that any change to the table would show.
It decides whether `en-passant-target-state` can ship its layout as is.

## Plan

1. Measure the natural spread. Choose a suite of positions: the three
   above plus about 20 more from different openings and middlegames,
   and some endgames with castling rights still set. Search each at a
   fixed depth with several Zobrist seeds (`randomSeed()` /
   `randomInitialState()` in `random.hpp`). Record node counts, time,
   and `TranspositionTableStats` hits and probes, which search already
   logs (`search.cpp:481`, `:507`).
2. Measure the layouts the same way. If the difference between the two
   layouts is within the seed-to-seed spread, it's noise. Then record
   that, and `en-passant-target-state` needs no change.
3. If it's systematic, make the index independent of the metadata
   layout, and measure over the suite. Options:
   - Give each metadata field its own Zobrist keys, XORed into the piece
     bits, as the pieces already are.
   - Mix the whole 64-bit code before masking, with a multiplicative
     (Fibonacci) hash, instead of folding.

   `tools/seed_optimizer.cpp` builds the same code layout by hand
   (`computeHashWithTable`), so any change has to reach it too.

## Implementation Progress

### Session #1

Ran plan steps 1 and 2. The layout's effect is noise, so step 3 isn't
needed and `en-passant-target-state` can keep its layout.

Method:
- Eight Release builds of `main` (2217329). Layout A keeps the castling
  bits at 7 and 10, and layout B moves them to 11 and 13. Each layout
  was built with four PCG initial states (`randomInitialState()`): the
  default `0x853c49e6748fea9b`, plus `0x2545f4914f6cdd1d`,
  `0x9e3779b97f4a7c15` and `0xd1b54a32d192ed03`. They were set through
  a temporary `WISDOM_EXPERIMENT_INITIAL_STATE` define, which isn't
  committed.
- 24 positions:
  - the start position and eleven openings, played from it
  - the five perft positions, including kiwipete
  - three castling endgames
  - two middlegames
  - a rook ending and a pawn ending
- Each position was searched to depth 7 in a fresh UCI process with
  `debug on`, which reports exact nodes, quiescence nodes, table probes
  and hits for each depth. The counts are summed over all depths.
- The default layout A build reproduced `main`'s node counts exactly.

Results:

| Build | Nodes, all 24 positions | Table hit rate |
| --- | --- | --- |
| A, default seed | 69,841,722 | 19.36% |
| A, seed 2 | 59,157,743 | 19.42% |
| A, seed 3 | 61,834,257 | 19.56% |
| A, seed 4 | 58,489,784 | 19.67% |
| B, default seed | 68,454,150 | 19.29% |
| B, seed 2 | 63,528,921 | 19.29% |
| B, seed 3 | 62,330,636 | 19.46% |
| B, seed 4 | 57,964,885 | 19.64% |

- Per position, B's geometric mean node count over the four seeds is
  0.8% above A's, averaged over the 24 positions. Across positions that
  gives t = 1.62, which isn't significant.
- No single position's layout difference exceeds its own seed-to-seed
  spread: |z| ≤ 0.8 everywhere.
- The seed moves the total by up to 19% within a layout, much more than
  the layout does. Most of that comes from a few volatile positions: the
  Sicilian's node count varies 37% between seeds, and the Italian's 16%.
  Thirteen positions vary less than 2%.
- Hit rates stay between 19.3% and 19.7%.

The default seed gave the highest total in both layouts. On this suite
that's most likely chance too, given how much a few positions dominate.
Picking a seed by this measure would just fit the suite.

The 27% difference `en-passant-target-state` saw from the start position
was at depth 8. At depth 7, the start position varies by 0.3% between
seeds and differs by 0.1% between layouts. So that 27% was one sample of
the same chaotic sensitivity, deeper in the search.
