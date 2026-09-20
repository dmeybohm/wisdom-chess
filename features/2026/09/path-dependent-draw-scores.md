# Path-dependent draw scores

## Motivation

The bug list records that draw scores reach the transposition table under
board-only keys, left open when
[transposition-table-ownership.md](transposition-table-ownership.md)
landed.

A transposition table works because what a position is worth is a
property of the board: reach the same position by any move order and the
answer is the same. Draws break that premise. Threefold repetition and
the fifty-move rule are properties of the *path*, so the same board is
worth +90 as the first occurrence and 0 as the third.

`search()` (`engine/search.cpp:152`) already returns a draw score before
it probes or stores, so a node that repeats under the current history is
never itself written:

```cpp
if (isProbablyDrawingMove (parent_board, my_history))
    return drawingScore (my_searching_color, side);

// probe and store are below this line
```

What is written is its **ancestors**. The draw score propagates up
through the child loop into `best_score`, and the single store site at
the end of `search()` writes that score keyed by the ancestor's board
hash alone. The contamination enters the table one level up, laundered
through a node that looks ordinary.

Session #2 of the ownership branch believed it had hit this from the UCI
side. Measurement during implementation showed that its repro does not
demonstrate the bug — see Session #1 below — but the bug itself is real,
and the fifty-move rule is the sharpest way in, because the board hash
covers the pieces, the side to move, castling rights and en passant, but
**not the halfmove clock**. These two positions share every hash in the
search:

```
8/8/4k3/8/8/4K3/8/7R w - - 96 200
8/8/4k3/8/8/4K3/8/7R w - - 0 200
```

Only the first is close enough to the limit for the draw rule to apply.
Search the first and then the second with one table, and the second plays
`e3f2` scoring itself at -500, where a cold table finds the winning
`h1h7` at +1045. It abandons a won rook endgame on the strength of draw
scores that belong to a clock it no longer has.

## Design

A node's value is path-dependent if **anything** in its subtree hit the
draw check. Such a value is not stored.

The rule has to be viral over the whole subtree, not just over the child
that won. Consider node `P` with children:

- `A` worth 50, ordinary
- `B` worth 0, because it repeats under this history

`best_score` is 50 from `A`, which looks untainted. But under a different
history `B` is not a repetition and might be worth 200, making `P` worth
200 rather than 50. A draw score that *lost* still suppressed the true
value, so "the winning child was not a draw" is not a sufficient test.

Because the condition is "did anything below me draw", it needs no change
to `search()`'s return type and no flag threaded through the recursion. A
monotonic counter on the impl is enough, and `search()` has exactly one
store site:

```cpp
if (isProbablyDrawingMove (parent_board, my_history))
{
    my_draw_nodes++;
    return drawingScore (my_searching_color, side);
}
...
auto draw_nodes_before = my_draw_nodes;   // before the child loop
...
if (!my_current_result.timed_out && my_draw_nodes == draw_nodes_before)
    my_transposition_table.store (...);
```

Any descendant's increment is visible to every ancestor, so virality
falls out of the counter being monotonic rather than having to be
maintained.

The mechanism is self-tuning. `isProbablyDrawingMove` fires only on
threefold repetition or the fifty-move counter
(`engine/evaluate.hpp:88`), so in a quiet middlegame almost nothing is
tainted and the table behaves as it does now. In a position reached for
the third time, nearly everything near the root is tainted and the table
stores very little — which is the correct answer for that position.

### What this does not fix

The probe returns before the children are explored:

```cpp
if (auto tt_score = my_transposition_table.probe (hash, depth, alpha, beta, ply))
    return *tt_score;
```

So the reverse direction survives: a value stored under a history with no
repetition available can be read back under a history where one *is*
available. The taint never fires, because the search never descends far
enough to notice the repetition.

This is the milder direction and it is what engines that keep a table
between moves generally live with. It means the UCI continuation-clear
stays: the two are complementary, taint stopping draw scores getting out
and the clear stopping unrelated histories getting in.

### Rejected

- **Key the entry by the repetition context as well.** Correct, and it
  would let tainted entries be stored and reused safely. It needs a
  second hash over the relevant history, widens `TranspositionEntry`, and
  would cut the hit rate for entries that are almost always reused under
  the same context. Not worth it for a case this rare.
- **Do not store only when the winning child drew.** The smaller change,
  and wrong, for the `A`/`B` reason above.
- **Clear the table whenever the history gains a repetition.** Blunt: it
  discards the whole table over one line, and does nothing for
  transpositions within a single search.

## Plan

1. Add the counter and the store condition to `IterativeSearchImpl`.
2. Engine test: search a repetition-heavy position with a table, then
   search the fresh starting position with the same table, and check the
   score matches a search with a cold table. This is the UCI repro at
   engine level, and unlike the UCI test it also covers the within-search
   case.
3. Benchmark with `search/*` (cost within one search) and
   `search/warm-table` (cost across moves). Record both here.
4. Check the item off in the bug list and update the table's entry in
   `AGENTS.md`.

## Implementation Progress

### Session #1

**The review comment's repro does not show what it claimed.** Instrumenting
the search with a count of nodes that hit the draw check showed
`draw_nodes = 0` for

```
position startpos moves g1f3 g8f6 f3g1 f6g8
go depth 4
position startpos
go depth 3
```

at depth 4 and still 0 at depth 6. No draw score is produced anywhere in
that line, so nothing history-dependent was ever stored. Two facts
explain the 90-versus-0 that looked like corruption:

- The engine's score for the starting position alternates with the parity
  of the depth: 90 at odd depths, 0 at even ones. A **fresh** process
  answers `go depth 4` with 0.
- The first search was `depth 4` and the second `depth 3`. A probe
  legitimately returns an entry stored at a greater depth, so the second
  search was answering with the first one's depth-4 analysis. That is the
  table working, and it is *more* accurate, not less.

Run both searches at depth 4 and the warm and cold answers agree exactly.
The "20 nodes, 20 of 20 hits" was the table doing its job.

The UCI continuation-clear committed in response to that repro
(`d4a2e8d`, merged) therefore rested on a misreading. It is kept — see
below — but for a different reason than the one given at the time, and
the account in
[transposition-table-ownership.md](transposition-table-ownership.md) has
been corrected.

**The underlying concern is real.** The route is the fifty-move rule
rather than repetition, for the reason in the motivation above: the
halfmove clock is not in the hash. Measured in the near-limit position,
one depth-4 search hits the draw check 536 times, and this change
suppresses 532 stores that would otherwise have been written under
board-only keys. An engine test pins the consequence: searching the
`96`-clock position and then the `0`-clock position with one table picks
`e3f2` without the fix and `h1h7` with it, matching a cold table. Checked
that the test fails when the condition is disabled.

**Cost.** None that the benchmarks can see, which is the self-tuning
property the design predicted — the draw check does not fire in ordinary
positions, so nothing is suppressed there.

| | before | after |
|---|---|---|
| `search/starting-depth8` | 2.325s | 2.342s |
| `search/kiwipete-depth8` | 1.901s | 1.901s |
| `search/italian-depth8` | 2.497s | 2.471s |
| `search/history-80-plies-depth8` | 0.549s | 0.540s |
| `search/history-200-plies-depth8` | 0.153s | 0.152s |
| `search/warm-table`, cleared each | 6.017s | 6.060s |
| `search/warm-table`, warm | 4.213s | 4.214s |
| `search/warm-table`, moves differing | 2 of 30 | 3 of 30 |

The one extra differing move is expected: suppressing a tainted entry
changes what a later search finds. The nanobench depth-6 rows are not
reported because the baseline run was unstable (22.9% error on one row);
the manual depth-8 timings and the warm-table figures are the reliable
comparison.

**The UCI continuation-clear stays.** With this change, stored scores no
longer carry draw contamination *out* of a history. The reverse direction
is untouched, because the probe returns before the children are explored:
a value stored where no draw was available can still be read where one
is. Clearing on a non-continuation limits how far that can travel, and it
costs nothing in normal play, where every `position` continues the game.
214 tests pass.
