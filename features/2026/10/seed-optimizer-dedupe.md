# Seed optimizer counts each position once

Follows `zero-based-piece-index.md`, and is stacked on that branch.

## Motivation

`tools/seed_optimizer.cpp` scores a Zobrist seed by how evenly the
hashes of the positions in a depth-5 perft spread over tables of 2^17
to 2^21 buckets: the worst ratio of the fullest bucket to the average.
It kept every position the perft reached, 5,072,213 of them, and a
position reached by several move orders hashed into the same bucket
each time, whatever the seed. The fullest bucket was mostly one popular
position: at 2^21 it held 88 or more against an average of 2.42, and
many seeds tied at that floor. So the score measured how often
positions repeat, which no seed changes, rather than how well the keys
spread distinct positions, which is what the transposition table
needs.

## Design

`collectPositions()` keeps a board only the first time its key is
seen. The key is what the hash covers, and nothing derived from it:
the packed piece on each of the 64 squares and the unnormalized
metadata bits (turn, castling, en passant target), compared through a
defaulted `operator<=>` in a `std::set`. A repeated position is still
searched, since it may be reached in fewer moves than before and so
lead to positions the first visit did not.

The count, 1,351,950, is above the roughly 900,000 distinct positions
usually quoted for five plies, because positions that differ only in
castling rights or a recorded en passant target count separately here,
as they do in the hash.

## Results

1000 random seeds, Release build:

| | Positions | Current seed | Best | Worst |
|---|---|---|---|---|
| Before | 5,072,213 | 38.45 | 36.38 | 53.75 |
| Deduplicated | 1,351,950 | 10.86 | 10.86 | 15.51 |

At 2^21 the current seed's fullest bucket holds 7 against an average
of 0.64, with a standard deviation of 0.80, the square root of the
average, as independent random keys give. With that average about ten
of the 2^21 buckets are expected to reach 7, so 7 or 8 is what a good
seed scores, and at least ten seeds tie at 7. The current seed is among
them, so it stays. The score is coarse at this size, a whole number of
positions over the average, and the next criterion, the mean standard
deviation, separates the leaders by about 1%.

## Implementation Progress

### Session #1

- Positions deduplicated by the hash's inputs; the run above.
