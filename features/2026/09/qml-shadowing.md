# Removing the QML shadowing warnings

## Motivation

After [qml-type-annotations.md](qml-type-annotations.md) `qmllint`'s
compiler category still reports 27 members that "can be shadowed": each
is reached through a property or a cast whose declared type a subclass
could extend, so qmlcachegen leaves the access to the interpreter. The
category is at `info` because of them. Reaching the same members through
an id, or not reaching across the boundary at all, satisfies the
compiler, and in each case that is the simpler structure.

| Group | Count | Fix |
|---|---|---|
| Dialog functions through `GameRoot.dialogs` | 10 | GameRoot handles the menu's signals itself |
| `GameModel` methods (6.11 lint only) | 10 | `GameModel` is `final` |
| Square coordinates through casts, the root's `board`, the window's focus item | 6 | Board tracks selection from its squares' clicks |
| `gameRoot.anyDialogOpen` from `MainWindow` | 1 | the pause logic moves into `GameRoot` |

## Plan

1. **`GameModel` final.** The test fixture's `InspectableGameModel`
   subclasses it to reach `getGame()` and `isHoldingAMove()`. Both become
   public, the subclass goes, and its two helpers, `board()` and
   `makeCurrentPlayerComputer()`, move onto the fixture's `Application`.
   The `FINAL` on the properties, added on the previous branch, is
   redundant on a final class and goes.
2. **Menu and dialogs in `GameRoot`.** `GameRoot` takes the menu as a
   required property and answers its four signals by calling `dialogs`
   by id. The pause-while-a-popup-is-open logic moves there too, since
   it now has both the menu and the dialogs. `MainWindow` keeps the
   title, colour, visibility and closing handler and needs no properties;
   the mains set `menu` on their root instead of on the window, and lose
   their signal-forwarding lines.
3. **Selection by clicks.** `ChessSquare` emits `clicked()` instead of
   toggling its own focus. `Board`'s delegate, which has an id, reports
   the click with the square's coordinates, and `Board` keeps the
   selected square: a click selects, a second click on the same square
   deselects, a click elsewhere moves. The delegate sets the square's
   focus to the outcome, so the highlight still follows the selection
   and a dialog still hands the focus back to the selected square. After
   a move the destination is not focused, as before, so the focus a
   dialog returns cannot select it. The window-level tracking in
   `GameRoot`, the `board` property and the casts in `Board` go.
4. **Lint.** `CompilerWarnings=warning` in `.qmllint.ini`; expect zero
   findings on Qt 6.11.2 and 6.9.3.
5. Verify: `all_qmllint`, `ctest`, the UI tests under `Basic` and
   `Fusion`, on both Qts; the C++ lint target.

## Risks

The selection rewrite touches the interplay with dialogs that
[qml-tests.md](qml-tests.md) found a defect in. The tests for a move, an
illegal move, a capture, promotion, the draw offer's focus return and
the menu dialog's focus return all exercise it.

## Implementation Progress

### Session #1

The QML side went as planned; the C++ side did not do what it was for.

- **`GameRoot`** takes the menu, answers its four signals by calling
  `dialogs` by id, and holds the pause-while-a-popup-is-open logic.
  `MainWindow` is five lines. The mains set `menu` on their root and
  lost their forwarding lines.
- **Selection by clicks.** `ChessSquare` emits `clicked()`; `Board`
  keeps `selectedRow` and `selectedColumn`, and the delegate binds the
  square's `focus` to being the selected one, so the highlight and the
  focus a dialog returns both follow the selection without any item
  being cast. `GameRoot`'s window-level tracking and `board` property
  are gone. The tests for moves, an illegal move, promotion and both
  focus-return cases pass.
- **`GameModel` final, `getGame()` and `isHoldingAMove()` public**, the
  fixture's subclass replaced by two helpers on `Application`. This
  removed nothing: moc records the class as final, but qmltyperegistrar
  writes `isFinal` only on properties (Qt's own qmltypes have it on 466
  properties and on no component or method), so qmllint cannot know a
  type is final and the ten method calls still warn on 6.11's lint. The
  per-property `FINAL`, removed as redundant and then restored, is what
  clears the property warnings. The change is in its own commit and can
  be dropped; it is kept as a true statement about the class, and moc's
  record of it would serve a future qmltyperegistrar that emits it.
- **Lint level.** `CompilerWarnings` stays at `info`. At `warning` CI's
  Qt 6.9 would pass with zero findings, but every local lint with 6.11
  would fail on the ten method calls, which nothing in the code can fix.
- `qmllint` on 6.11.2: 10 informational, all `GameModel` methods; 27
  before this branch. Full `ctest` (192) passes; the four UI tests pass
  under `Basic` and `Fusion`; the C++ `lint` target is clean.

### Checked against CI's Qt 6.9.3

`all_qmllint` reports nothing at any level, with the compiler category
on. The seven QML test binaries pass, and the four UI tests pass under
`Basic` and `Fusion`. So CI's lint is fully compile-clean; only 6.11's
`qmllint` still reports the ten method calls, as information.
