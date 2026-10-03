# Engine architecture

This page is a map of the chess engine for someone about to change it:
how a position is represented, how moves are generated and tested, how
a position is scored, how the search works, and how the game decides a
draw. Building and testing are in [building.md](building.md) and the
code conventions in [coding-style.md](coding-style.md). The reasons
behind the design decisions are in the feature logs under `features/`,
and each section links to the ones that matter.

The engine is the `wisdom-chess-core` library in
`src/wisdom-chess/engine`, in the `wisdom::` namespace. The chess parts
of it, from the bottom up:

| Layer | Files | What it holds |
|---|---|---|
| Primitives | `coord.hpp`, `piece.hpp`, `move.hpp`, `castling.hpp`, `move_list.hpp` | A square, a piece, a move, castling rights, and a list of moves |
| Board | `board.hpp`, `board_code.hpp`, `material.hpp`, `position.hpp` | The position and what is kept up to date with it: its hash, its material and its piece-square score |
| Rules | `generate.hpp`, `threats.hpp`, `evaluate.hpp` | Move generation, attack detection, legality, checkmate and stalemate, draw detection, and the static evaluation |
| Search | `search.hpp`, `transposition_table.hpp`, `move_timer.hpp`, `history.hpp` | Iterative deepening with alpha-beta and quiescence, the table, the clock, and the position history the draw rules need |
| Game | `game.hpp`, `game_status.hpp` | The game a frontend plays: the players, the draw claims, and the search entry point |

## Representation

- **`Coord`** is one byte holding `row * 8 + column`, a square index
  from 0 to 63: the column is the low three bits and the row the three
  bits above it, with the top two bits unused. Row 0 is the eighth rank
  and column 0 is the a-file, so a8 is index 0 and h1 is 63. White pawns
  move toward lower rows.
- **`ColoredPiece`** is one byte holding `color * 8 + type`: the piece
  type is the low three bits and the color the two bits above it, with
  the top three bits unused. White is 1 and Black is 2, and the empty
  square is all zeros.
- **`Move`** is two bytes: the source and destination squares and a
  four-bit code that tells a plain move from a capture, an en passant
  capture, castling, and a promotion to each piece with or without a
  capture. Castling is stored as the king's move and the rook's half is
  derived from it. The all-zero move is the null move, which the table
  uses for "no move".
- **`MoveList`** is a fixed array of 252 moves with no heap allocation.
  A position has at most 218 legal moves.
- **`Board`** is 120 bytes: the 64 squares, the `BoardCode`, the
  halfmove and fullmove clocks, the `Material` and `Position` summaries
  and the two king positions. Making a move means copying the board:
  `withMove()` returns the new position and the caller keeps the old
  one, so the search never unmakes a move. At this size copy-make is
  cheap and there is no undo path to keep correct.
- **`BoardCode`** is the 64-bit key for the table and for repetition. The
  high 48 bits are a Zobrist hash of the pieces, from a table of random
  numbers generated at compile time; the low 16 bits hold the side to
  move, both sides' castling rights and the en passant target. The
  target is stored in one of two fields, depending on whether the side
  to move has a legal capture of it. `getBoardCode()` drops a target
  without one, because FIDE counts only a possible en passant capture
  when comparing positions, while `getUnnormalizedBoardCode()` keeps it
  as FEN records it. See
  [en-passant-normalization-cost.md](../features/2026/09/en-passant-normalization-cost.md)
  and [tt-index-metadata.md](../features/2026/09/tt-index-metadata.md).
- **`Material`** keeps each side's material score and piece counts as
  pieces are added and removed, and answers whether checkmate is still
  possible with what is left. **`Position`** keeps each side's
  piece-square score the same way, from one table per piece type,
  mirrored for Black.

## Move generation and legality

The generator produces pseudo-legal moves, and legality is tested by
making the move. The generator walks the 64 squares and, for each piece of the side to move, runs that piece's
generator. Sliding pieces step along their rays; knight destinations
come from a table computed at compile time; pawns handle the single and
double push, captures, promotions and en passant; the king adds castling
when the rights, the empty squares and the rook are there.

The entry points are:

- `generateAllPotentialMoves()`, which sorts the whole list: the
  caller's priority move first, then captures by the material they win,
  then promotions, then by coordinates. Quiet moves therefore come out
  in board order. The search passes the table's best move as the
  priority move.
- `generateCaptures()`, the same list restricted to captures and
  promotions to a queen, for the quiescence search.
- `generateLegalMoves()`, which filters the pseudo-legal list.
- `hasLegalMove()`, the stalemate and checkmate test, which stops at the
  first legal move it finds. When the side is not in check, a piece that
  shares no row, column or diagonal with its own king cannot be pinned,
  so any move it has other than en passant is legal without a board
  copy. Only when no such move exists does it fall back to trying the
  king's moves and then each piece's. See
  [cheaper-has-legal-move.md](../features/2026/09/cheaper-has-legal-move.md).

Legality is `isLegalPositionAfterMove()`: the mover's king must not be
attacked in the new position, and a castling king must not have started
in or passed through check. The attack test is `InlineThreats`, which
looks outward from the king's square for a pawn, a knight, a slider
along the row, the column and the diagonals, and the other king. It
answers yes or no for that one square, and both search loops rely on
it: they copy the board for every pseudo-legal move and discard the
illegal ones.

## Evaluation

A position is scored from the point of view of a color as material,
piece-square score and a castling term, each the difference between the
two sides. The castling term penalizes a side that has lost a castling
right and refunds twice the penalty once it has castled.

Scores are integers, with a pawn at 100 before scaling. The static
score is bounded by `Max_Non_Checkmate_Score`, so that mate scores
above it are unambiguous. A mate found `n` plies away scores
`Checkmate_Score - n`, which is linear so the table can shift a stored
mate score by the ply at which it is probed. A node with no legal move
scores as checkmate when the king is attacked and as 0, stalemate,
otherwise.

## Search

`IterativeSearch` is a negamax alpha-beta search with iterative
deepening and a quiescence search at the horizon. It is created with a
board, the game's `History`, the time and depth limits, the caller's
`TranspositionTable` and the `DrawLimits` to apply.

`iterativelyDeepen()` searches depth 1, then 2, up to the limit, and
stops early when it finds a mate or when the clock runs out. A depth cut
short by the clock keeps its partial result only when the root move it
had finished is the previous depth's choice seen deeper, or scores
better; otherwise the previous depth stands. Either way, every node
the cut-short depth completed has stored its result in the
transposition table, where the next search finds it.

`search()` at each node:

1. Returns a draw score when the position is a repetition or has hit
   the halfmove limit under the `DrawLimits`, or has insufficient
   material, except at the root. The score is `Search_Draw_Contempt`
   when the draw falls on the engine's own move and 0 when the opponent
   could claim it, so the engine plays on unless it is losing.
2. Drops into `quiesce()` when the depth is spent.
3. Probes the table, except at the root, and returns a stored score
   whose depth and bound satisfy the window.
4. Generates the pseudo-legal moves with the table's move first, and
   for each one copies the board, discards it if illegal, pushes the
   child onto the `History` as a tentative position, recurses with the
   window negated, and pops it.
5. Scores a node with no legal move as checkmate or stalemate.
6. Stores the result with its bound type, unless the clock stopped the
   search or a descendant returned a draw score. A draw by repetition or
   by the move counter belongs to the path that reached the node, and
   the table is keyed by the board alone, so such a score is never
   stored. See
   [path-dependent-draw-scores.md](../features/2026/09/path-dependent-draw-scores.md).

`quiesce()` searches captures and queen promotions until the position is
quiet. A node not in check first runs the stalemate test with
`hasLegalMove()`, then stands pat on the static score when that already
beats beta, and otherwise tries the captures. A node in check searches
all its evasions instead, for up to `Max_Quiescence_Evasion_Ply` plies
of consecutive checks, after which it is scored statically, so a series
of checks cannot extend the search without end. See
[quiescence-search.md](../features/2026/09/quiescence-search.md).

## Transposition table

`TranspositionTable` is an array of entries, a power of two long,
indexed by the board code folded to 32 bits and masked. An entry holds
the full code, the best move, the score, the depth and whether the score
is exact, a lower bound or an upper bound. There is one slot per index:
a store replaces whatever is there unless it is the same position
searched deeper. Mate scores are stored relative to the node and
corrected by the ply on the way in and out.

The table is search state, not game state: `Game::findBestMove()`
borrows the caller's table, and whoever runs searches owns one, keeps it
across the moves of a game and clears it for a new game. See
[transposition-table-ownership.md](../features/2026/09/transposition-table-ownership.md).

The halfmove clock is not part of the key, and a draw score is never
stored, so a position's entry stays valid whatever path reaches it.

## Game, history and draws

`Game` holds the current `Board`, the `History`, the time and depth
limits, which player is human and which is the engine, and the state of
any draw claim. `move()` applies a move and records the new position;
`findBestMove()` runs a search and returns the move, or nothing when the
search was canceled.

`History` is what the draw rules need: the board code of every position
so far, the full boards for an exact comparison, and the moves. The
search pushes and pops tentative positions on a copy of it as it
descends. Repetition is counted only over the reversible tail of the
history, since no position can recur across a capture or a pawn move,
and comes in a probable form, by code, and a certain form, by comparing
boards, which the game uses before declaring a draw.

`getStatus()` reduces the position to one `GameStatus`, in this order:
checkmate, stalemate, a threefold repetition that is reached, accepted
or declined, a fivefold repetition, fifty moves without progress
reached, accepted or declined, seventy-five moves, and insufficient
material. A draw a player may claim is reported as reached and the
frontend asks the players; their answers come back through
`setProposedDrawStatus()`.

Who decides a draw is the `DrawArbiter`. By default the game does: it
proposes claimable draws, and the search counts a draw at the claimable
limits of three repetitions or a hundred halfmoves, unless the players
declined that claim, in which case the limits rise to the automatic
five and 150. A UCI GUI arbitrates draws itself, so the UCI frontend
sets the arbiter to `External` with the limits the search should apply.
The search knows neither arbiter; it is given `DrawLimits` and nothing
defaults them. See
[draw-arbiter.md](../features/2026/10/draw-arbiter.md).
