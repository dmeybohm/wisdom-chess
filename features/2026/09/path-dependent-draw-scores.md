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

Session #2 of the ownership branch hit this from the UCI side: after

```
position startpos moves g1f3 g8f6 f3g1 f6g8
go depth 4
position startpos
go depth 3
```

the fresh starting position evaluated to 0 on 20 nodes with 20 of 20
table hits, where a fresh process gives 90 on 575 nodes. That was
addressed by clearing the table when a `position` command does not
continue the current game, which quarantines one route to the problem
without removing it. Within a single search the same leak exists between
transposing lines, and no clear can help there.

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
