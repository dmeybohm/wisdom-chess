# QML view cleanups

## Motivation

A review of the QML view code on the `qml-dialog-heights` branch, done
after the minimum Qt was raised to 6.8 (`2e60d03`), found no defects in
the branch itself but a fair amount that the Qt 6.8 module setup lets us
delete, plus duplication, dead code and one likely bug. This document is
the umbrella plan. The small, safe items are done on this branch; the
rest are listed with the branch each deserves, in the order they should
land.

The review ran Qt's own `qmllint` through the build's
`WisdomChessQml_qmllint` target (Qt 6.11.2, Linux):

| Category | Count |
|---|---|
| Unqualified access | 129 |
| Unused import | 9 |

The unqualified warnings break down by name:

| Name | Count | Resolved by |
|---|---|---|
| `topWindow`, `root`, parent-element members | 52 | bound components and ids (this branch, `qml-main-window`) |
| `_myGameModel`, `_myPiecesModel` | 45 | `qml-singletons` |
| `Color`, `Player`, `PieceType`, `DrawByRepetitionStatus` | 31 | `qml-enum-registration` |
| `model` in delegates | 16 | required properties (this branch) |

Lint only sees the desktop module: `mobile_main.qml`, `MobileRoot.qml`
and `wasm_main.qml` are only added to the module on their own platform,
so they have never been linted.

## Findings

### Enabled by the Qt 6.8 module setup

1. **The explicit `import "../Helper.js" as Helper` lines are redundant.**
   The generated module qmldir lists `Helper 1.0 Helper.js`, so `Helper`
   is visible through the implicit module import that QTP0004's
   subdirectory qmldir files provide. Verified by linting a copy of the
   module with the import removed from `Board.qml` and
   `SettingsDialog.qml`: no warning, while a misspelled `Helperx` in the
   same place is flagged. Ten files carry the import; lint already marks
   five of them unused.
2. **`main.cpp` reimplements `objectCreationFailed`.** The `objectCreated`
   lambda that checks `!obj && url == obj_url` is what
   `QQmlApplicationEngine::objectCreationFailed` (Qt 6.4) reports
   directly. The `qDebug() << "Creating URL"` above it is a leftover.
3. **The enums are registered by hand.** `ui_types.cpp` calls
   `qmlRegisterUncreatableMetaObject` four times on the same meta-object
   under four names, and every test that loads QML must call
   `registerQmlTypes()` first. Four `QML_FOREIGN_NAMESPACE` wrappers with
   `QML_NAMED_ELEMENT` and `QML_UNCREATABLE` register them at build time
   through `qt_add_qml_module`, and put them in the module's `qmltypes`
   so lint and qmlcachegen know them.
4. **The platform roots are excluded from the module.** Adding the three
   files unconditionally costs a few kilobytes of resources, lets lint
   see them, and removes the `append_lines.cmake` custom command in
   `test/CMakeLists.txt` that exists only to add `MobileRoot` to a copied
   qmldir.
5. **Delegates read `model.role` and outer ids implicitly.** `Board.qml`
   and `PromoteDropdown.qml`. `pragma ComponentBehavior: Bound` and
   `required property` declarations clear the warnings and let
   qmlcachegen compile the bindings.
6. **The models are context properties.** `_myGameModel` and
   `_myPiecesModel` are untyped to every tool and are 45 of the
   warnings. Singletons created from the existing instances fix that,
   but the test fixture sets the same context properties, so this is its
   own branch.
7. **`GameMenu.qml` hides items with both `visible` and `height: 0`.**
   A current `Menu` may skip invisible items on its own; not confirmed
   from the docs, so test before relying on it.
8. Not planned, noted for completeness: `loadFromModule` (Qt 6.5) could
   replace the hand-built `qrc:` URL and the `MAIN_QML_FILE` definition,
   but only if the main files are renamed to type names, so it is not
   worth doing on its own.

### Duplication

9. The three main files share about forty identical lines: the size and
   platform properties, the pause-on-popup handler, the focus tracking
   and the closing handler.
10. `DesktopRoot.qml` and `MobileRoot.qml` repeat the same forwarding
    functions to `Dialogs`. An alias property removes them.
11. `AboutDialog.qml` has four identical `Text` blocks differing only in
    their string.
12. `wasm_main.qml` overlays a `MouseArea` on each `ImageToolButton`,
    which is already a clickable `ToolButton`.
13. `Piece.qml`: the `Connections` block on the castling animation can be
    an `onStopped` handler on the animation, and
    `myTranslation.x = myTranslation.x` is redundant because the
    assignment on the next line already breaks the binding.
14. `Dialogs.qml` repeats the `Math.min(..., Screen.width - 50)` width
    caps per dialog; they could be defaults inside each dialog.

### Dead code

15. `Board.qml`: `property var animateRowAndColChange`, never read
    outside the file.
16. The three mains declare a `currentFocusedItem` on the window that
    nothing reads; the copy on `DesktopRoot.qml` and `MobileRoot.qml` is
    the one the mains write. (The review had this the wrong way round;
    lint's `missing-property` caught it in Session #1.)
17. `BoardDimensions.qml`: `totalSquares`.
18. `Helper.js`: `computerOrHumanLabel`, `targetRowOrCol`.
19. `images/bx-icon-menu-white.svg` is a resource but nothing uses it;
    mobile uses the PNG.
20. Imports lint flags unused: `QtQuick.Layouts` and `QtQuick.Controls`
    in `Board.qml`, `QtQuick.Controls` in `DesktopRoot.qml`,
    `QtQuick.Layouts` in `GameMenu.qml`, and the `Helper` imports above.
21. The `Flickable` around `GameMenu` in `mobile_main.qml` does nothing
    (a popup is drawn in the window overlay), as
    [mobile-menu-toggle.md](mobile-menu-toggle.md) already notes.

### Likely bug

22. **`PromoteDropdown.qml` reverses its piece order on the wrong
    signal.** `onDestinationColumnChanged` calls
    `setFirstRow(destinationRow)`, but the reversal depends on the row.
    White promotes on e8, then Black on e1: the column is unchanged, no
    handler runs, and the list is drawn with the queen farthest from the
    pawn. A first promotion on the a-file misses the same way, because
    the property's default is column 0. Cosmetic: all four pieces stay
    selectable. Tracked in
    [bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md).

### Polish

23. `SettingsDialog.qml`: `internal.fontSize` holds strings for an
    integer pixel size; the `Component.onCompleted` block sets width and
    padding imperatively where bindings would do.
24. `GameMenu.qml`: `hideFinalItem ? false : true` is `!hideFinalItem`;
    the property should be `readonly`.
25. The About text still says 2022 and "Qt © The Qt Company 2021".

## Plan

### This branch: `qml-cleanups` (lint first)

Get lint covering everything, fix what is cheap, and make CI run it,
so the later branches start from a clean baseline and any new issue in
the unlinted files surfaces now.

1. Add `main/MobileRoot.qml`, `main/mobile_main.qml` and
   `main/wasm_main.qml` to the module's `QML_FILES` unconditionally
   (finding 4). Keep `MAIN_QML_FILE` as the per-platform entry point.
   Delete the mobile qmldir custom command and `append_lines.cmake` in
   `test/CMakeLists.txt`; the mobile test then embeds the same resource
   list as the others. Run `ctest` to confirm the three UI tests still
   load.
2. Run `cmake --build build --target all_qmllint` and record what the
   three newly linted files add to the counts above.
3. Fix the cheap warnings: remove the `Helper` imports and the other
   unused imports (findings 1, 20); qualify parent-element members with
   ids; add `pragma ComponentBehavior: Bound` and required properties to
   the delegates in `Board.qml` and `PromoteDropdown.qml` (finding 5).
4. Remove the dead code (findings 15 to 19, 21) and do the `Piece.qml`
   and polish items (13, 23, 24). Try finding 7; keep the `height: 0`
   hack if a hidden item still takes space.
5. Replace the `objectCreated` lambda in `main.cpp` with
   `objectCreationFailed` and drop the debug line (finding 2).
6. Add a `.qmllint.ini` at the module root that keeps
   `UnqualifiedAccess` at `info` for now. The remaining unqualified
   warnings are the context properties and the enums, which the next
   two branches remove; once both land, delete the override so they
   are warnings again.
7. **CI.** The `lint` job in `.github/workflows/cmake.yml` configures
   with `WISDOM_CHESS_QML_UI=Off` and has no Qt, so the QML lint goes in
   the `build` job after its Build step, on Linux only since the result
   is platform independent:
   `cmake --build build --target all_qmllint`. `qmllint` exits non-zero
   on warnings, so the step fails the job. CI uses Qt 6.9, whose
   `qmllint` may differ from the 6.11 one used locally; reproduce a
   CI-only difference with `./scripts/install-ci-qt.sh`.
8. Commit after each of the steps above. Run the C++ linter and the
   full `ctest` at the end, and the QML UI tests under both
   `QT_QUICK_CONTROLS_STYLE=Fusion` and `Basic`.

### Separate branches, in landing order

- **`qml-enum-registration`** (finding 3). Replace `registerQmlTypes()`
  with `QML_FOREIGN_NAMESPACE` wrappers in `ui_types.hpp`, remove the
  three calls in `test/`, and confirm the module's `qmltypes` lists the
  four names. This is 31 of the unqualified warnings. Small, but it
  changes how every test process starts, so it gets its own branch.
- **`qml-main-window`** (findings 9, 10, 11, 12, 14). A common
  `MainWindow.qml` (an `ApplicationWindow`) holding the shared
  properties and handlers, with the three mains reduced to their header
  and root; a `dialogs` alias on the roots; a `Repeater` in the About
  dialog; the wasm toolbar using the buttons' own `onClicked`. The UI
  tests find items by class, not by file, so they should be unaffected;
  the `QML: mobile` test is the check for the mobile main. If
  `loadFromModule` (finding 8) is ever wanted, this is where the mains
  would be renamed.
- **`qml-promotion-order`** (finding 22). Change the handler to
  `onDestinationRowChanged`, with a UI test that promotes on the same
  file twice and checks the order of the dropdown's images.
- **`qml-singletons`** (finding 6). Register `GameModel` and
  `PiecesModel` as singletons from the instances `main()` creates
  (`QML_SINGLETON` with a static `create()` returning the stored
  instance, or `qmlRegisterSingletonInstance`), rename the 45 QML uses,
  and change `application_fixture.hpp` to register the same way instead
  of setting context properties. Largest change, last in line, and the
  one that turns the remaining unqualified warnings back on.

Out of scope: restyling the dialogs, and anything in the C++ models
beyond what the singleton registration needs.

## Implementation Progress

### Session #1

Steps 1 to 8 of this branch's plan are done, one commit each. Qt 6.11.2,
Linux, offscreen.

- **Module (step 1).** The three platform roots are in the module on every
  platform; `MAIN_QML_FILE` still picks the entry point. The mobile UI
  test embeds the same resources as the others, and `append_lines.cmake`
  is gone.
- **Baseline (step 2).** With the roots included, lint reported 137
  unqualified accesses and 12 unused imports. The three files added 8
  unqualified `_myGameModel` accesses and 3 unused imports, nothing else.
- **Lint fixes (step 3).** Helper and unused imports removed; parent
  members qualified with ids; `Board.qml` and `PromoteDropdown.qml` use
  `pragma ComponentBehavior: Bound` with required properties. `Piece.qml`
  declares its five roles as required properties, so the `Repeater` fills
  them itself and the delegate in `Board.qml` only binds `flipped`. After
  this: 100 unqualified, 0 unused imports. Every remaining one is a
  cross-file id (`topWindow`, `root`), a context property or an enum.
- **Dead code and polish (steps 4, 13, 23, 24).** As listed, except:
  the windows' `currentFocusedItem` was the dead copy, not the roots' (see
  finding 16); the `Flickable` around the mobile menu (finding 21) was deferred
  while `mobile-menu-toggle` still edited the same lines, and removed once
  the branch was rebased on main with that merged; and the About text (finding 25) is content, not code, and is left
  to the author. The settings dialog's bindings reference `topWindow`
  five more times, so the count is 105 unqualified, all cross-file.
- **Finding 7 is settled: keep the `height: 0` hack.** A probe of a
  `Menu` with an invisible `MenuItem` and `MenuSeparator` under Fusion and
  Basic showed the menu keeping its full height (63 px against 31 px for a
  one-item menu in Fusion; 93 against 40 in Basic); with `height: 0` as
  well it matched the one-item menu.
- **`main.cpp` (step 5)** uses `objectCreationFailed`; the debug line is
  gone.
- **Lint configuration (step 6).** The lint target exited zero with 105
  warnings: since Qt 6.7 `qmllint` only fails when `--max-warnings` is
  set. `src/wisdom-chess/ui/qml/.qmllint.ini` sets `MaxWarnings=0` and
  `UnqualifiedAccess=info`. Verified both ways: the target exits 0 with
  114 infos, and exits non-zero when the override is removed. The file
  says to delete the override once `qml-enum-registration` and
  `qml-singletons` land.
- **CI (step 7).** A `Lint QML` step in the `build` job, Linux Release
  only, runs `all_qmllint`. `AGENTS.md` documents the target, the
  configuration and the conventions the fixes introduced. Not yet run on
  CI, and not checked against Qt 6.9's `qmllint`.
- **Verified (step 8).** All 191 `ctest` tests pass; the three QML UI
  tests pass under `Basic` and `Fusion`; the C++ `lint` target is clean.
  The promotion dropdown's click path, which now reads `choice.piece` and
  the `dropDownTop` properties, is covered by
  `aPromotionGoesThroughTheDropdown` in `application_test.cpp`, which
  opens the dropdown, highlights an entry and chooses the knight.
