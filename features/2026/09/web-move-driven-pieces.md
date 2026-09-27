# Move-driven piece lists for the frontends

## Motivation

The React frontend animates a piece by keeping its `PieceOverlay` mounted
across renders and changing only its square class, so each piece needs an
id that survives the move. `WebGame::updatePieceList` produced those ids by
comparing the board before and after the move: pieces standing still kept
their ids, and the rest were matched to what was left over by type and
color, with a special case for promotion and a `"Couldn't find id."` error
if nothing matched. It then sorted by id to keep the DOM order stable.

That worked out the move backwards from the board, although `applyMove()`
had the `Move` in hand. React needs stable ids, not board diffing. The QML
`PiecesModel::playerMoved` already applied the `Move` directly, so the two
frontends answered the same question in two ways, each with its own
castling and en-passant rules.

## Plan

1. Add `ui::pieceMovement (Move)` to the viewmodel library: the mover's
   step, the castled rook's step, the square of the captured piece, and the
   promoted piece. Test it in the viewmodel suite.
2. Rebuild `WebGame::updatePieceList` on it: remove the captured entry,
   move the mover and the rook in place, change the type on promotion.
3. Port `PiecesModel::playerMoved` to it.

## Decisions

- **The move's flags, not the board, decide captures.** A move that lands
  on a piece without the capture flag would already corrupt the engine's
  board, so the flag is authoritative. Human moves come through
  `mapCoordinatesToMove`, which sets it.
- **Edit in place instead of sorting.** Erasing a captured entry keeps the
  rest in id order, so the list stays in the order the constructor gave it
  and the sort is no longer needed.
- **Nothing crosses the IDL.** `getPieceList()` returns the same
  `{id, color, piece, row, col}` entries, so the TypeScript side and the
  generated types are unchanged.

## Implementation Progress

### Session #1

- Added `ui/viewmodel/piece_movement.{hpp,cpp}` and
  `test/piece_movement_test.cpp`.
- `WebGame::updatePieceList (Move)` replaces the diff and
  `findAndRemoveId`. `WebColoredPieceList` gained `indexOf` and `removeAt`,
  which are C++-only and not in the IDL.
- There is no C++ harness for `WebGame`, so it was checked in headless
  Chromium against the built module: three scripted games covering castling
  on both sides, en passant by each color, captures, and promotion with and
  without a capture. After every move the ids stayed unique, sorted, one
  per square, bound to the same color, and the mover's id was on its
  destination.
- `PiecesModel::playerMoved` now applies `pieceMovement()` too. It still
  clears the previous castling roles first and emits the same
  `dataChanged` roles in the same order, so the QML delegates and
  `pieces_model_test.cpp` are unchanged. A capture is now removed by the
  move's flag rather than by whatever stood on the destination.

### Session #2

- `WebGame` now has native tests. `web_types.hpp` no longer includes
  Emscripten or `bindings.hpp`, and the `engine_thread` extern moved to
  `game_model.hpp`, its only user, so `web_game.cpp` builds without
  Emscripten. `ui/wasm/test/` builds it into `wisdom-chess-web-tests`, added
  from `ui/CMakeLists.txt` whenever the React UI and fast tests are on, so
  it runs in every native CI job, the sanitizer and Debug builds included.
- The tests replay games through `makeHumanMove` against an engine `Game`
  fed the same moves, and after every move check that ids are unique, in
  order, and match the board square for square. They cover castling on
  each side for each color, en passant by each color, promotion with a
  capture for each color, a computer move, illegal moves and checkmate.
  Disabling the castled rook's update fails two of them.
- The same target builds under Emscripten and passes under node. But
  `ctest` in an Emscripten build cannot run any doctest suite, this one or
  the engine and viewmodel ones: node reports a terminal, so doctest
  colors the test list and `doctest_discover_tests` writes broken
  `add_test` calls. That predates this branch and is left for a separate
  fix. The glue and worker code in `bindings.cpp` and `game_model.hpp`
  still has no tests.
