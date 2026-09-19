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
