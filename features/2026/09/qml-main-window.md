# Shared QML main window

## Motivation

Findings 9 to 14 of [qml-cleanups.md](qml-cleanups.md), plus the cross-file
ids that make up most of what `qmllint` still reports there. The three
platform mains (`desktop_main.qml`, `mobile_main.qml`, `wasm_main.qml`)
repeat about forty lines: the board size and platform properties, the
pause-on-popup handler, the focus tracking that turns two clicked squares
into a move, and the closing handler. `DesktopRoot.qml` and
`MobileRoot.qml` repeat the forwarding functions to `Dialogs`. Every other
file reaches the board size and platform flags through the window's id,
`topWindow`, and the animation timings through the root's id, `root`,
which lint cannot resolve across files: 43 `topWindow` and 6 `root`
accesses after `qml-enum-registration`.

## Plan

1. **Singletons for what the window only forwarded.**
   - `main/BoardDimensions.qml` becomes a `pragma Singleton` (the CMake
     source property `QT_QML_SINGLETON_TYPE`). Files use
     `BoardDimensions.squareSize`, `boardWidth` and `boardHeight`. The
     mobile main still recomputes `squareSize` on an orientation change,
     through the singleton.
   - New `main/Platform.qml` singleton with the `isMobile`,
     `isWebAssembly`, `isDesktop` and `isMacOS` flags, computed from
     `Qt.platform.os` directly; the three Helper functions go.
   - `root.animationDelay` and `root.castlingRookPause` in `Board.qml` and
     `Piece.qml` read `_myGameModel` directly, which is what the root
     forwarded; the root properties go.
   - The test fixture's `squareSize()` reads the singleton through
     `QQmlEngine::singletonInstance` instead of the window.
2. **`main/GameRoot.qml`**, an `Item` that `DesktopRoot` and `MobileRoot`
   extend, holding what they shared: the `Dialogs` item and an alias to it
   (finding 10; the mains call `root.dialogs.showNewGameDialog()`), the
   `anyDialogOpen` property, and the focus tracking. The tracking moves
   here from the window: the `Window.activeFocusItem` attached property
   gives the item that has focus, and `GameRoot` passes the previous and
   the new one to its board, which the derived file supplies as a
   required `board` property.
3. **`main/MainWindow.qml`**, an `ApplicationWindow` the three mains
   extend, with the title, colour, visibility, the pause handler and the
   closing handler. It takes the root and the menu as required
   properties, since the mains place the menu differently.
4. **The mains** keep only their size, header, root and menu.
5. **Smaller items.** `AboutDialog.qml`'s four `Text` blocks become a
   `Repeater` over the lines (11). `wasm_main.qml`'s three `MouseArea`s
   become one over the toolbar row (12). `NewGameDialog`,
   `ConfirmQuitDialog` and `DrawProposalDialog` carry their own width
   caps and padding instead of `Dialogs.qml` setting them (14).
6. **A wasm smoke test.** Nothing loads `wasm_main.qml` today. A
   `wasm_test.cpp` like `mobile_test.cpp`, which loads it, checks the
   starting position and opens the menu from the toolbar, so the
   restructured file is at least loaded and clicked once. On Linux
   `Platform.isWebAssembly` is false, as `isMobile` is for the mobile
   test, so this shows the file works, not the wasm-only paths.
7. **Check.** `qmllint` should report only `_myGameModel` and
   `_myPiecesModel` (40 accesses); `ctest` passes; the UI tests pass
   under `Basic` and `Fusion`; the C++ lint target is clean.

## Risks

- `Window.activeFocusItem` as the focus source instead of the window's
  `focusObjectChanged` signal: the same item, read from the root. The
  application tests that click two squares, promote and answer a draw
  offer cover it, including the case where a dialog gives focus back.
- Singleton creation order: `BoardDimensions` reads `Screen` from a
  `QtObject`, which it did before as an instance inside the window; the
  mobile orientation test covers the recompute.
- Android and wasm are not built here. The files are linted and loaded
  by the tests on Linux, and the CI installers build the desktop app.
