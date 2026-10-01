# Search the root even when it is a draw

## Motivation

UCI played a random move in any position where a draw could be
claimed.

`IterativeSearchImpl::search()` tested `isProbablyDrawingMove()` on
every node, the root included. A root that was a draw returned the draw
score at ply 0 without trying a move, so the search had no move to
report. `UciInterface` then sends `pickRandomLegalMove()`, which is
meant for a search stopped before it finished a depth.

UCI never answers a draw proposal, so the draw statuses stay at
`NotReached` and the root is a draw from the third occurrence of a
position, or from a halfmove clock of 100. Measured with the UCI binary
before the fix:

- `position fen 6k1/5ppp/8/8/8/8/8/R3K3 w - - <clock> 90`, then
  `go depth 3`, three runs each. Ra8 is mate. Clock 98: `a1a8` every
  time. Clock 100: `a1a2`, `a1d1`, `a1a6`. Clock 120: `a1a8`, `a1c1`,
  `e1f1`.
- `position startpos moves g1f3 g8f6 f3g1 f6g8 g1f3 g8f6 f3g1 f6g8`,
  the third occurrence of the starting position: seven different moves
  in eight runs. After one shuffle fewer it is `b1c3` every time.

The other frontends were not affected. The console, QML and WASM
frontends search only when `Game::getStatus()` is `Playing`, and ask
the players about a draw first.

## Fix

`search()` skips the draw test at ply 0. The caller asked for a move,
and a draw that nobody claims leaves the game going.

Below the root nothing changes: a third occurrence, a clock at the
limit and insufficient material are still scored as draws. So the
engine still avoids or seeks them as before; it just no longer refuses
to move once it stands in one.

`Game::findBestMove()` therefore returns a move for a root that is a
draw, where it used to return nothing. It still returns nothing when
there is no legal move, or when the search is cancelled before it
finishes a root move, and UCI keeps its random move for that case.

## Alternative: UCI declines both draws

The first idea was for `UciInterface` to decline the threefold and
fifty-move draws whenever it builds a game. Rejected:

- Declining moves the limits to the fifth occurrence and 150 halfmoves
  inside the tree as well. A GUI that adjudicates the third occurrence
  or the fifty-move rule would then end games the engine thought were
  still alive, such as a repetition it walked into while ahead.
- The random move would only move to the fifth occurrence and to a
  clock of 150.
- Insufficient material at the root has no status to decline.

## Tests

- `game_test.cpp`, "findBestMove finds a move in a position that can be
  claimed as a draw": past fifty moves, where only a capture resets the
  count; the third occurrence of a mate-in-one position; and bare
  kings.
- `UCI: a position past fifty moves without progress is searched` and
  `UCI: the third occurrence of a position is searched`.

The fifty-move position gives Black's king a flight square, so that Ra8
is not mate. Whether a mate that completes the count outranks the draw
is the `seventy-five-move-checkmate` branch's subject, and this test
holds with or without it.

## Implementation Progress

### Session #1

Created `search-root-draw-move` in its own worktree from `origin/main`
at `b64bf5b0`. Found while planning `fen-declined-fifty-move-draw`.

Wrote the tests first. Before the fix `findBestMove()` returned no move
in the first subcase, and both UCI tests failed. Then made the change
in `search()`.

Release `ctest` with slow tests passed 270 of 270. The new tests pass
in a Debug build, and the `lint` target is clean. The QML tests were
not built, because this machine has no Qt. The search speed was not
measured; the change adds one integer comparison per node.
