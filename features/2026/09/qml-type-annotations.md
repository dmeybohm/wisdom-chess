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
