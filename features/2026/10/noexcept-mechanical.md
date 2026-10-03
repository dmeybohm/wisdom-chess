# Mark the functions that cannot throw `noexcept`

## Motivation

The review behind `noexcept-fixes.md` read every engine header and
listed the functions that cannot throw and lack `noexcept`: about 190,
nearly all `constexpr` one-liners over integers, enums and `std::array`,
or getters returning a member. This branch marks them. It follows
`noexcept-fixes` and `logger-noexcept`, which removed the throw sites
that had formally tainted the chains above them, so the move-generation
and draw-detection functions can be marked too.

The gain is documentary and in the type traits, not in speed: the
compiler already inlines these. What it buys is that a `noexcept`
function can call them without the call being the thing that makes its
`noexcept` a lie, and that `std::is_nothrow_*` tells the truth for the
value types.

## Scope

Functions declared in an engine header, with their definitions in the
`.cpp` where there is one. Bottom-up:

1. `coord.hpp`, `piece.hpp`, `castling.hpp`, `move.hpp`: the value
   types everything else is built on.
2. `board.hpp`, `board_code.hpp`, `board_builder.hpp`, `material.hpp`,
   `position.hpp`.
3. `history.hpp`, `threats.hpp`, `generate.hpp`, `evaluate.hpp`,
   `search.hpp`, `transposition_table.hpp`.
4. `random.hpp`, `logger.hpp`, `game.hpp`, `game_impl.hpp`,
   `move_timer.hpp`, `fen_parser.hpp`.

Now eligible because of the two earlier branches: `BoardCode::getCastleState()`
and the `BoardCode` constructor from a `Board`, `Material::checkmateIsPossible()`,
the move generators, `hasLegalMove()`, `isCheckmated()`, `isStalemated()`,
`probableDrawCategory()`, `isProbablyDrawingMove()`,
`mapCoordinatesToMove()` and `Game::getStatus()`.

Out of scope:

- Functions local to a source file, and lambdas. The
  `[[nodiscard]]` pass took the same line.
- The `GameStatusUpdate` virtuals, whose overrides are the frontends'.
- Anything with an `EXPECTS`, `ENSURES`, `narrow`, a `throw`, string
  building, stream output or allocation on its path. The review's
  "correctly throwing" lists.
- `BoardBuilder::fromDefaultPosition()` and `BoardCode::fromDefaultPosition()`,
  whose `addPiece()` calls throw `BoardBuilderError` on input that the
  default position never supplies. Marking them would need the builder's
  input checks to abort instead, which is a design change.
- `MoveTimer::isTriggered()`, which runs a caller-supplied function.
- `= default` special members and destructors, which are already
  implicitly `noexcept` when their members are. `CompileTimeRandom()`
  becomes so once `randomSeed()` and `randomInitialState()` are marked.

## Method

Each header's list from the review was applied by hand, declaration and
definition together, and every translation unit was syntax-checked
before the full build. Adding `noexcept` cannot change behaviour on a
path that does not throw, so the existing tests are the verification;
no test is added.

## Implementation Progress

### Session #1

Branched from `logger-noexcept`. The four header groups were marked in
parallel, one editor per group with disjoint files, each syntax-checking
its translation units with the flags from `compile_commands.json`
before the build. `coordColor()` and `pawnDirection()` had moved from
`board.hpp` to `coord.hpp` in the layering restructuring and were
marked there. 317 declarations and definitions in 32 files; every
changed line adds `noexcept` and nothing else.

Verification: Release and Debug builds compile without a warning, the
linter passes, all 282 Release tests and all 246 Debug tests pass. A
scratch translation unit confirmed the chains report it end to end:
`std::is_nothrow_copy_constructible` holds for `MoveList` and `Board`,
and `noexcept (...)` is true for `makeCoord()`, `Board::withMove()`,
`generateLegalMoves()`, `isCheckmated()`, `probableDrawCategory()`,
`Game::getStatus()` and `Game::mapCoordinatesToMove()`, and false for
`evaluate()`, which keeps its `ENSURES`. The frontends were not rebuilt:
adding `noexcept` to a non-virtual function cannot break a caller.
