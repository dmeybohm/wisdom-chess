# React view cleanups

## Motivation

A review of the React frontend (`src/wisdom-chess/ui/react/src`) found one
confirmed bug and a number of places where the view code does more work
than it needs to. Most of the extra work comes from `App.tsx` reading engine
state in two different ways, and from values that pass through the UI as
strings or refs and are converted back later.

## Findings

### Bugs

1. **The promotion dialog stays open after promoting.** `handlePromote()`
   makes the move, but nothing clears `pawnPromotionDialogSquare` or
   `focusedSquare`. `ENGINE_SYNC` merges only the engine snapshot, and
   `SET_LAST_DROPPED` touches only `lastDroppedSquare`. The dialog stays
   until the next click on a piece dispatches `FOCUS`. A probe test that
   promotes through the dialog confirmed that `makeHumanMove('e2', 'e4',
   Queen)` is called and the dialog is still rendered afterwards.
2. **A click move leaves the focus on the source square.** The same missing
   reset applies after an ordinary click move. The stale `focusedSquare`
   points at an empty square, so it is invisible. The next click on a piece
   takes the "second click" path in `handlePieceClick()`, fails to find a
   source piece, and falls back to `FOCUS`. It works, but only by accident.
3. **The computer-move throttle is never cancelled.** `throttle()` returns a
   `cancel()` that nothing calls. A pending `notifyComputerMove()` can
   fire after unmount or after a new game starts. This has not been
   reproduced in the app, so it is marked as unverified.

### Simplifications

`App.tsx` and `reducer.ts`:

4. **One sync action.** `BOOTSTRAP` and `ENGINE_SYNC` behave identically.
   `currentTurn` and `inCheck` are read from `gameRef` during render
   instead of from the snapshot, so they are correct only because every
   engine change also dispatches a sync. Put them in `snapshotFromEngine()`,
   together with the settings. Then `SET_SETTINGS` and the special case in
   `startNewGame()` go away, and a single `SYNC` action remains.
5. **`squares` is not state.** It is always `initialSquares`. `Board` can
   import it directly.
6. **`GameState` is UI state.** It lives in `lib/WisdomChess.ts`, the WASM
   boundary module, but only the reducer uses it. Move it to `reducer.ts`.
7. **Draw dialogs.** Two `useState` flags, declared partway down the
   component after the functions that use them, two nearly identical
   handlers, and two nearly identical JSX blocks. Replace them with one
   piece of state, one handler, and a small table of the two draw types.
   `setHumanDrawStatus()` has an unused `who` parameter, and it reads
   `wisdomChess` from a `const` declared later in the component.
8. **Refs for singletons.** `modelRef` and `wisdomChessRef` hold
   page-lifetime globals and never change. Only `gameRef` needs to be a
   ref. Also remove dead locals (`mod` twice, the unused event parameter of
   `startNewGame()`, the redundant `srcPiece &&` check).
9. **Pieces carry a `PieceColor`.** `Piece.color` is `'white' | 'black'`.
   `App` and `Square` convert it back with `fromColorToNumber()` to call
   the engine. Store the enum value and drop both conversion functions.
   `handlePieceClick()` can use `state.pieces` instead of reading the piece
   list from WASM again on each click.

Components:

10. **`Modal` owns its overlay.** `App` renders the overlay separately,
    based on a flag that leaves out the draw dialogs, so those appear with
    no overlay behind them. Render the overlay from `Modal` and type
    `children` as `React.ReactNode`.
11. **`StatusBar` builds markup and then parses it.** It formats
    `<strong>White</strong> to move` as a string and splits it apart with a
    regex to render it. Only `gameOverStatus`, which comes from the engine
    (`game_viewmodel_base.cpp`), needs the parser. Render the locally built
    status as JSX, simplify the `moveStatus` concatenation, and remove the
    unused imports.
12. **`SettingsModal` mixes uncontrolled and controlled inputs.** The radio
    buttons and checkboxes are read through four refs, and the sliders use
    state. Use one controlled settings object and a `flipped` flag. Also
    remove the stray `id="computerBlack"`.
13. **`Square` / `PieceOverlay`.** `useDrop` collects `isOver` and `canDrop`
    and never uses them. Collecting them re-renders every square and piece
    as a drag passes over. `drag` is attached to both the wrapper `div` and
    the `img` inside it. `canDrag` has an unused `monitor` parameter.
14. **`PawnPromotionDialog`.** The `direction` prop is unused, `color` is
    typed `number` rather than `PieceColor`, and the class expression
    `${selected && 'selected'}` emits the class `false`.
15. **`TopMenu`.** `toggleOpen` reads `isMenuOpen` from the closure instead
    of using a functional update. The document click listener is removed
    and added again on every toggle when it could be registered once. The
    menu items are `<a>` elements with no `href`, and the images have no
    `alt`. `settingsClicked` is optional, unlike the other two callbacks.
16. **Small leftovers.** `lib/Squares.ts` uses a class with private fields
    and getters for a constant list that plain objects would cover.
    `startReact()` in `main.tsx` takes an unused parameter named `window`,
    which shadows the global.

## Plan

Fix the bugs separately from the refactors, each with its own commit.

1. **Bugs 1 and 2.** Clear the focus and the promotion square once a move
   is attempted, legal or not. Add App tests: promotion closes the dialog,
   and a click move clears the focus. Cancel the throttle on unmount and
   when a new game starts (bug 3).
2. **State shape (4–9).** A single `SYNC` action with the full engine
   snapshot, `squares` removed, `GameState` moved, the draw dialog state
   merged, `PieceColor` in `Piece`. Update `reducer.test.ts` to match.
3. **Components (10–16).** One commit per component where the change is
   more than a line or two.
4. **Verify.** Run `npx vitest run`, `npx tsc --noEmit` and `npm run build`
   after each step. The tests mock the engine and do not drive react-dnd,
   so drag and drop, promotion, and the draw dialogs also need a manual
   check against the real WASM build.

## Out of scope

- Upgrading React 18 → 19 or TypeScript 4.9 → 5. That would be dependency
  work in the spirit of `less-deps.md`, and it deserves its own branch.
- Replacing react-dnd, or the hidden-`img` drag preview workaround in
  `PieceOverlay`. Neither can be checked without a browser, and the
  workaround's reason is not recorded.
- Pausing the engine while a draw dialog is open. The draw dialogs are
  answered while the engine waits anyway. Any change here would alter
  behavior, not simplify the code.

## Implementation Progress

### Session #1

- Reviewed every file under `ui/react/src`. Baseline: 41 vitest tests pass
  and `tsc --noEmit` is clean.
- Confirmed bug 1 with a temporary App test (not committed).
- Recorded the findings and plan above. No code changes yet.
