# Deciding en passant legality when the target is set

## Motivation

[`en-passant-normalization-cost`](en-passant-normalization-cost.md)
(PR #287) leaves an unusable en passant target out of
`Board::getBoardCode()` and `Board::operator==`. It decides this lazily.
Each `getBoardCode()` call on a board with a target runs
`generateLegalEnPassantMoves()`, and search asks for the code twice per
child: once in `History::addTentativePosition()`, and once for the
transposition table key.

That log's "Rejected: caching the normalized code in a mutable field"
names the alternative: decide once, where the target is set, and record
the answer in `BoardCode`. This branch tries it.

## Design

`BoardCode` stores the target in one of two 5-bit fields, one for a
legal target and one for an illegal one. Both use the same encoding: a
3-bit column, a present bit, and a vulnerable-color bit. At most one
field is set.

| Bits | Field |
| --- | --- |
| 0 | side to move |
| 1–5 | legal en passant target |
| 6–10 | illegal en passant target |
| 11–12 | White castling |
| 13–14 | Black castling |
| 15 | spare |

- `BoardCode::setEnPassantTarget (color, coord, state)` takes an
  `EnPassantTargetState` (`Legal` or `Illegal`).
- `getAnyEnPassantTarget()` replaces `getEnPassantTarget()` and returns
  whichever field is set. FEN output and move generation read it.
- `withoutIllegalEnPassantTarget()` masks the illegal field off.
  `Board::getBoardCode()` returns that, and `Board::operator==` compares
  it, so neither generates moves any more.

`Board` classifies the target wherever the position or the side to move
changes:
- at the end of `makeMove()`, which `updateEnPassantEligibility()` now
  runs after, since legality depends on the finished position
- in the `BoardBuilder` constructor
- in `withCurrentTurn()`

Classifying still calls `generateLegalEnPassantMoves()`. It checks for
the taken pawn and an adjacent capturer before trying any move, so most
double pushes cost a few square reads.

A copy of a `Board` carries the state in its code, and a copy is the
same position. So the copy constructor stays defaulted, and there is no
`mutable` cache to race on or invalidate.

The trade-off is that classifying is now eager. It also runs for
double-push boards nobody asks a code for: moves `generateLegalMoves()`
rejects, search children that fail `isLegalPositionAfterMove()`, and
perft. The perft slow tests are where that would show.

## Plan

1. `BoardCode`: the two fields, `EnPassantTargetState`,
   `getAnyEnPassantTarget()` and `withoutIllegalEnPassantTarget()`, with
   `board_code_test.cpp` cases.
2. `Board`: classify in `makeMove()`, the builder constructor and
   `withCurrentTurn()`. Drop `normalizedBoardCode()`. Rename
   `getEnPassantTarget()` to `getAnyEnPassantTarget()`. Add
   `en_passant_test.cpp` cases, and a check in "Agrees with
   generateLegalMoves" that the stored state matches
   `generateLegalEnPassantMoves()` at every node.
3. Release `ctest` with slow tests, the Debug fast tests, and `lint`.
4. Time search at depth 8 in the three positions from the previous log,
   and the perft slow tests, against `main`. If search gains nothing
   measurable, or perft regresses, record that and stop before a PR.

## Implementation Progress

### Session #1

Implemented the design, with two changes made while working:

- `Board::operator==` compares the board codes before the squares. The
  code is one 64-bit compare now, and rejects nearly every mismatch.
- `BoardCode` and `Board` also have `getLegalEnPassantTarget()`. The
  pseudo-legal generator (`MoveGeneration::pawn()`) skips en passant
  unless the target is legal, because every capture of an illegal
  target fails `isLegalPositionAfterMove()` anyway.
  `eligibleEnPassantColumn()` still reads the raw target, because
  `mapCoordinatesToMove()` also uses it to parse a typed or clicked move.
  There, a pinned en passant capture should parse as en passant and be
  rejected as illegal. It shouldn't become a diagonal pawn move to an
  empty square.

This makes "Agrees with generateLegalMoves" partly circular: the
generator's en passant moves now depend on the classifier it is checked
against. The walk also checks that the stored state matches
`generateLegalEnPassantMoves()` at every node. The perft suites remain
the independent check on en passant move counts.

New tests are in `board_code_test.cpp` ("Board code keeps a legal and an
illegal en passant target apart") and `en_passant_test.cpp` ("An en
passant target's legality is decided when it is set"). Three temporary
mutations each failed at least one of them:

- no re-classification in `withCurrentTurn()`
- no legal-target check in the generator
- every target classified `Illegal`

Release `ctest` with slow tests passed 245 of 245 before the new tests
were added. After them, the fast tests passed 213 of 213 in both Release
and Debug, and `lint` is clean.

#### Timing

Depth 8, Release, user time, three runs each. The branch searched
different node counts from `main`, even though the stored codes should
be equivalent. Moving only `main`'s castling bits to 11 and 13
reproduces the branch's node counts exactly. The difference is the
transposition table index, which comes from the low bits of the code
(`foldHashTo32Bits`), so the metadata layout changes which positions
collide. The layout-matched `main` gives the like-for-like time:

| Build | 1.d4 d5 2.c4 | Start position | Italian (after 3...Bc5) |
| --- | --- | --- | --- |
| `main` | 4,650,526 nodes, 7.55 s | 2,318,521 nodes, 3.38 s | 40,064,400 nodes, 68.90 s |
| `main`, castling bits at 11 and 13 | 4,548,942 nodes, 7.39 s | 2,939,365 nodes, 4.09 s | 37,817,531 nodes, 66.35 s |
| This branch | 4,548,943 nodes, 7.20 s | 2,939,367 nodes, 3.97 s | 37,817,535 nodes, 64.48 s |

Times are medians. Every build chose the same move in each position.

- With nodes matched, deciding legality once is about 3% faster in all
  three positions.
- The layout's effect is larger and goes both ways: 27% more nodes
  from the start position, 2% and 6% fewer in the other two. It isn't
  something this change should be credited or blamed for.
- The perft suites (`ctest -R Perft`, three runs) took a median 26.02 s
  against 25.77 s on `main`, about 1% slower. That's the eager
  classification's cost for boards nobody asks a code for.

### Session #2

- Removed `Board::isEnPassantVulnerable()`. Only tests called it, and
  they now use `getAnyEnPassantTarget()` or `getLegalEnPassantTarget()`,
  so each one says which it checks.
- `Board::makeMove()`, `withMove()`, `updateEnPassantEligibility()` and
  `classifyEnPassantTarget()` are `noexcept`. Everything they reach is
  `noexcept` or checks with `noexcept_expects`, except the `default:`
  case in `position.cpp`'s `change()`. That case is reached only for an
  empty source square, so it now calls `terminateOnPreconditionFailure()`
  instead of throwing `Error`. Every frontend checks a move against the
  legal moves before `Game::move()`, so reaching it would be an engine
  bug.
- `terminateOnPreconditionFailure()` defaults its location to
  `std::source_location::current()`, like `expects`.

Release `ctest` with slow tests passed 247 of 247, Debug passed 213 of
213, and `lint` is clean.
