# Mobile menu button toggles the menu

## Motivation

Open item from [bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md),
found on the `qml-tests` branch: the mobile menu button cannot close the
menu. In `ui/qml/main/mobile_main.qml` the header's `ImageToolButton` runs
`gameMenu.visible ? gameMenu.close() : gameMenu.open()` on click. A `Menu`
closes on a press outside itself, and the button is outside it, so the
press closes the menu; the click then fires on release, sees it closed, and
opens it again.

## Plan

1. Add `theMenuButtonClosesTheMenu` to `ui/qml/test/mobile_test.cpp`:
   click the button to open the menu, click it again, expect the menu to
   be gone. Confirm it fails before the fix.
2. Keep a press on the button from closing the menu, so the button's
   toggle is the only thing acting on it. Give `GameMenu` in
   `mobile_main.qml` the header `ToolBar` as its `parent`, which contains
   the button, and set
   `closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent`.
   Its `x` and `y` become relative to the toolbar:
   `toolbar.width - width` and `toolbar.height`, which is where the menu
   is drawn today, since `root` fills the area just below the header.
3. A press on the board still closes the menu, as does Escape. A press on
   the toolbar's title or icon no longer does. Parenting the menu to the
   button instead would avoid that, but would move the menu away from
   the window's right edge.

`GameMenu.qml` and `desktop_main.qml` are unchanged: the desktop button
only opens the menu. The `Flickable` around `GameMenu` in
`mobile_main.qml` does nothing, since a popup is drawn in the window's
overlay and not inside it; removing it is a separate cleanup.
