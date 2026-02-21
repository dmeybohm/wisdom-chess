# Pause Engine When Menu/Dialog is Open (QML Frontends)

## Problem

When a menu or dialog is opened in the QML frontends (desktop, mobile, WebAssembly QML), the chess engine continues thinking and can make a move while the user is interacting with the UI. The React frontend already handles this correctly by pausing/unpausing the WASM worker when modals open/close.

## Solution

Add pause/unpause support to the QML `GameModel` using the same pattern as the existing `buildNotifier()` periodic function mechanism. When any menu or dialog is open, an atomic flag is set that causes the periodic function to cancel the engine's search. When all popups close, the flag is cleared and the engine resumes.

### C++ Changes

- `GameModel`: Added `std::atomic<bool> my_paused`, `Q_INVOKABLE pause()/unpause()`, and `resumeSearching` signal
- `buildNotifier()`: Extended to check `my_paused` and cancel the search if paused
- `setupNewEngineThread()`: Connected `resumeSearching` → `ChessEngine::init`

### QML Changes

- `Dialogs.qml`: Exposed `anyDialogOpen` property tracking visibility of all dialogs
- `DesktopRoot.qml` / `MobileRoot.qml`: Forwarded `anyDialogOpen` upward
- `desktop_main.qml` / `mobile_main.qml` / `wasm_main.qml`: Combined menu + dialog visibility into `anyPopupOpen`, connected to `_myGameModel.pause()`/`unpause()`

### MoveTimer Flag Cleanup

While implementing the pause feature, we simplified the `MoveTimer` flag semantics:

- **`setCancelled(true)` now always sets `triggered = true`**: A cancelled search is always a triggered search. This enforces the invariant and eliminates the need for callers to remember to set both flags.
- **`isTriggered()` simplified**: Since `cancelled` implies `triggered`, the early-exit checks in `isTriggered()` only need to check the `triggered` flag.
- **`setTriggered()` made private**: All external callers should use `setCancelled()` instead. The game-ended case in `buildNotifier()` was changed from `setTriggered` to `setCancelled` since a game change should also discard the search result. `setTriggered` is now only used internally by `isTriggered()` when the timer expires and by `setCancelled`.
- **`isCancelled()`** remains public for callers that need to distinguish "timed out" from "externally cancelled" when deciding whether to use the search result.
