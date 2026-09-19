# Frontend cleanups

## Plan

Address the low-risk items still open in the Frontends section of
`bug-list-and-engine-warnings.md`:

1. Remove `ChessGame::isLegalMove()` and have the QML `GameModel` use the
   implementation it already inherits from `GameViewModelBase`.
2. Keep `ChessGame::config().players` synchronized when `setPlayers()` changes
   the underlying game's players, with a regression test for both views of the
   state.
3. Remove the unused `ViewModelSettings` type and consolidate the remaining
   user-depth-to-internal-depth conversion shared by the QML and WASM
   frontends.
4. Run the focused view-model, QML, and WASM tests where available, build the
   affected targets, and run the C++ linter.

The engine thread's animation delay is intentionally outside this branch. Its
replacement affects timing and thread coordination and needs a separate design
rather than a mechanical cleanup.

## Implementation Progress

### Session #1

- Created the `frontend-cleanups` branch and recorded this plan.
- Removed `ChessGame::isLegalMove()`. The QML move path now calls
  `GameViewModelBase::isLegalMove()`, whose existing tests cover legal,
  illegal, wrong-color, in-check, and computer-turn cases.
- Changed `ChessGame::setPlayers()` to update the engine's player array and
  `config().players` together. The QML regression test checks both.
- Deleted the unused `ViewModelSettings` header and source and removed them
  from the view-model target.
- Added `fullMovesToPlyDepth()` to the shared view-model types. QML's
  `MaxDepth` and WASM's `GameSettings` now use the same conversion, with a
  direct unit test.
- Verified the native `wisdom-chess-viewmodel-tests`: 10 cases and 65
  assertions pass.
- Built `WisdomChessQml` and ran `QML: ChessGame` and `QML: application`;
  both pass.
- Built the Emscripten `wisdom-chess-web` target successfully.
- Ran the full C++ `lint` target successfully.
- Left the engine thread's animation delay open for separate design work.
