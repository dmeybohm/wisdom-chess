# Type annotations in the QML

## Motivation

After [qml-singletons.md](qml-singletons.md) `qmllint` reports nothing in
its default categories. Its compiler category, off by default, reports
what qmlcachegen cannot compile to C++ and so leaves to the interpreter:
84 places. The gain from compiled bindings is small on a UI this size;
the gain from the annotations is that mistakes are caught at build time,
as the lint caught a removed property earlier in this series.

| Cause | Count | Fix |
|---|---|---|
| Members of the model singletons "can be shadowed" | ~35 | `final` on `GameModel` and `PiecesModel` |
| Settings copies held as `property var` | 20 | typed value-type properties |
| Functions without parameter or return annotations | 9 | annotate |
| Members of QML-declared properties "can be shadowed" | ~10 | see step 5 |
| The ListModel's `piece` role, `Qt.platform.os`, dynamic access in `focusMoved` | ~10 | see steps 3, 4 and 6 |

## Plan

1. **Annotate every function**: parameters and return types, `: void`
   where nothing is returned, in `Board.qml`, `BoardDimensions.qml`,
   `Dialogs.qml`, `Piece.qml`, `SettingsDialog.qml`, `GameRoot.qml` and
   `Helper.js`.
2. **`final` on the two model classes** in C++, which is what Qt asks
   for when a member accessed through a typed reference could be
   shadowed by a subclass.
3. **Typed settings copies.** `UISettings` and `GameSettings` become
   named value types, `QML_VALUE_TYPE (uiSettings)` and
   `QML_VALUE_TYPE (gameSettings)` instead of `QML_ANONYMOUS`, and the
   settings dialog's four copies are declared with those types. Writes
   such as `internal.myGameSettings.whitePlayer = Player.Human` are then
   typed writes into a value type property, which QML writes back.
4. **The promotion list as a plain array.** `PromotedPieceModel.qml` is a
   `ListModel` with a `reversed` flag and `swap()` and `reverse()`
   functions that reorder it in place so the queen stays on the target
   square for a promotion on the far rank. That mutable state is where
   the [qml-promotion-order.md](qml-promotion-order.md) defect lived, and
   the ListModel's `piece` role is one of the things the compiler cannot
   type. Replace it with a `readonly property list<int>` of the four
   piece types in `PromoteDropdown.qml`, reversed by a pure binding on
   the destination row, and a typed `imageFor (piece: int): string`
   that names the image from the piece and the side to move. The
   delegate takes `required property int modelData`. The file
   `PromotedPieceModel.qml` goes. The two promotion tests cover both
   orientations.
5. **Shadowing through QML-declared properties.** Qt has `final` for QML
   properties, but CI builds with Qt 6.9; if the keyword is newer than
   that it cannot be used, and these warnings stay. Decide from the
   docs.
6. **`focusMoved`** probes its arguments for `boardRow`, which cannot be
   compiled. Give `ChessSquare` the focus on its root item instead of the
   inner rectangle, so the focused item is a `ChessSquare`, and have
   `focusMoved (oldItem: Item, newItem: Item)` test `instanceof
   ChessSquare` and cast with `as`. The fixture finds squares by their
   `boardRow` and `boardColumn` properties, which move to the root item
   with the focus.
7. **`Qt.platform.os`** in `Platform.qml` is Qt's own loosely typed
   object; silence those three lines with `// qmllint disable compiler`
   rather than leave the category off.
8. **Enable the category.** `CompilerWarnings=warning` in `.qmllint.ini`
   if everything above clears it; otherwise `info`, with the remaining
   count recorded here.
9. Verify as the other branches: `all_qmllint`, `ctest`, the UI tests
   under `Basic` and `Fusion`, the C++ lint target.

## Implementation Progress

### Session #1

From 84 compiler findings to 27, all of one kind, with the category
enabled at `info` so the count stays visible without failing the lint.

- **Annotations** on every function in the module, parameters and
  return types. `Helper.js` could not take them, being plain JavaScript,
  so its two functions moved to their only callers (`zeroPad` into the
  settings dialog, `promotedRow` inlined in `Board.qml`) and the file is
  gone. The anonymous function in `Piece.qml`'s `Qt.binding` takes a
  return type too.
- **`final`**: `PiecesModel` is `final`. `GameModel` cannot be, because
  the test fixture's `InspectableGameModel` derives from it to reach
  protected members; its ten properties are `FINAL` instead, which
  clears the property warnings and leaves ten for its methods. Making
  the fixture a friend of `GameModel` rather than a subclass would allow
  `final` and clear those; that puts a test name in a production header,
  so it is left as an option.
- **Typed settings copies.** `UISettings` and `GameSettings` are the
  value types `uiSettings` and `gameSettings`, and the dialog's four
  copies use them. That exposed a binding loop the `var` copies had
  hidden: writing a member of a typed value-type property notifies, and
  the sliders wrote the setting back from `onValueChanged` while their
  `value` was bound to it. They write from `onMoved` now, which only the
  user's drag emits; the tests read slider values and change settings
  through the model, so they were unaffected either way.
- **The promotion list** is a `readonly property list<int>` in
  `PromoteDropdown.qml`, reversed by a pure binding on the destination
  row, with the image named by a typed function from the piece and the
  side to move. The delegate reads its piece from the list by index, so
  the `Repeater`'s model is the count. `PromotedPieceModel.qml`, with its
  `reversed` flag and in-place `swap()`, is gone; the class of defect
  fixed on `qml-promotion-order` cannot recur, and both promotion tests
  pass.
- **`focusMoved`** takes `Item`s and casts with `as`, which yields null
  for a dialog or a promotion choice; `instanceof` was tried first and
  qmlcachegen cannot compile it. `ChessSquare` takes the focus on its
  root item so the focused item is a `ChessSquare`; the fixture finds
  squares by the same properties, now on that item.
- **`Qt.platform.os`** lines in `Platform.qml` are wrapped in
  `qmllint disable compiler` and `enable`.
- **What remains**, 27 informational: members reached through
  QML-declared properties (`GameRoot.dialogs` 10, `ChessSquare` 4,
  `GameRoot.board`, `MainWindow.gameRoot`), which the `final` property
  keyword would settle but that is documented only from Qt 6.11 and CI
  builds with 6.9; `GameModel`'s ten methods, above; and Qt's own
  `Window.window.activeFocusItem`. When CI moves to a Qt with `final`
  properties, add it, raise `CompilerWarnings` to `warning`, and the
  lint enforces compilability.
- Verified: `all_qmllint` exits 0; full `ctest` (192) passes; the four
  QML UI tests pass under `Basic` and `Fusion`; the C++ `lint` target is
  clean. Not run on CI.

### Checked against CI's Qt 6.9.3

`./scripts/install-ci-qt.sh` had Qt 6.9.3 cached; the module was built
and linted against it in `build-qt69`.

- `all_qmllint` exits 0 with 17 informational findings, a subset of the
  27 from 6.11.2: 6.9's `qmllint` does not report `GameModel`'s methods
  as shadowable, and the rest are the same members through QML-declared
  properties plus Qt's `activeFocusItem`. Nothing new.
- The seven QML test binaries pass, and the four UI tests pass under
  `Basic` and `Fusion`.
