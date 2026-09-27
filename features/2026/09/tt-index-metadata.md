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
