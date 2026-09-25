# New Game and Quit dialog heights

## Motivation

The bug list in [bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md)
has one QML layout item still open:

> The New Game and Quit dialogs are too short for their padding
> (`popups/NewGameDialog.qml`, `popups/ConfirmQuitDialog.qml`: height at
> most 150, padding 40), so their text has no room and is drawn outside
> its box. With taller title and button bars, as in the Basic style, it
> crowds the buttons.

It was found on the `qml-tests` branch while diagnosing a Windows-only
failure of `aNewGameCanBeDeclined` (see "Cause of the Windows failure" in
[qml-tests.md](qml-tests.md)). The New Game dialog leaves 4 px for its
content in Fusion and -35 px in Basic. `showsText()` was relaxed to use
the painted size so the test would pass; the dialog itself was left as it
was. Windows users get the native Windows style, where the problem shows.

## Current state

Both dialogs are sized from outside, in `popups/Dialogs.qml`:

| Dialog | Width | Height | Padding |
|---|---|---|---|
| `NewGameDialog` | `min(400, Screen.width - 50)` | `min(150, Screen.height - 10)` | 40 |
| `ConfirmQuitDialog` | `min(500, Screen.width - 50)` | `min(150, Screen.height - 10)` | style default |

The bug list says both use padding 40; only the New Game dialog does. The
Quit dialog has the style's default padding, so it is less squeezed, but
its fixed 150 px still has to hold the title bar, a 16 pt line of text and
the button box, and in Basic that may not fit either. Measure it before
deciding it needs the same fix.

Inside each dialog the text is `Text { anchors.fill: parent }`, so it takes
whatever height is left, including a negative one, and draws outside it.
The `verticalAlignment` switches to `AlignTop` on mobile, which only
matters because the box is fixed.

`DrawProposalDialog` and `AboutDialog` are not affected: the draw dialogs
are 250 px tall, and the About dialog sets its own implicit size.

## Plan

1. **Test first.** Add a fixture helper to
   `ui/qml/test/application_fixture.hpp` that finds a shown text item and
   reports whether its laid-out box holds what it paints (`height >=
   paintedHeight`, and the same for width), and whether that box lies
   inside the dialog's content area rather than under its footer. Use it
   in `aNewGameCanBeDeclined` and `quittingAsksFirst` in
   `ui/qml/test/dialogs_test.cpp`, or in a new test that opens each dialog.
   Confirm it fails on the current QML under both
   `QT_QUICK_CONTROLS_STYLE=Fusion` and `Basic`.
2. **Size the dialogs from their content.**
   - In `NewGameDialog.qml` and `ConfirmQuitDialog.qml`, replace
     `anchors.fill: parent` on the `Text` with `width: parent.width`, so the
     text has a real implicit height after wrapping, and drop the
     mobile-only `verticalAlignment`.
   - In `Dialogs.qml`, remove the fixed `height` from both, so `Dialog`
     computes its height from the title, content, footer and padding, as
     it already does for `DrawProposalDialog`'s content. Keep the width
     limits. Cap the height at `Screen.height - 10` only if a narrow phone
     screen makes the wrapped text taller than the screen; otherwise leave
     it uncapped.
   - Keep New Game's padding at 40 if it still looks right with the
     natural height; the test will say whether the text fits either way.
3. **Check the styles.** Run the dialogs test under Fusion and Basic
   locally against Qt 6.9 (`./scripts/install-ci-qt.sh`) and the local Qt.
   Take offscreen screenshots of both dialogs in each style before and
   after, as Session #5 of [qml-tests.md](qml-tests.md) did, and look at
   them. The native Windows style is only reachable on CI.
4. **Mobile.** `mobile_test.cpp`'s `aNewGameCanBeStartedFromTheMenu` opens
   the same dialog through `MobileRoot`; run it and check a screenshot at
   phone width, since the text wraps there.
5. Update the bug list entry and add an Implementation Progress section
   here.

Out of scope: the mobile menu button item, which is a separate open entry
in the bug list, and restyling the dialogs beyond their size.

## Risks

- `Dialog`'s implicit height depends on its content item's implicit
  height. A `Text` with no explicit height and a bound width gets its
  implicit height from the wrapped layout, which is what is wanted; a
  binding loop between the width and height would show as a QML warning,
  which the UI tests already turn into a failure.
- Removing the fixed height changes how the dialogs look on every
  platform. The screenshots in step 3 are the check.

## Implementation Progress

### Session #1

- Added `dialogFitsText (dialog, text)` to `application_fixture.hpp`. After
  waiting for layout it checks that the text's box is at least its painted
  size, that the box lies inside the dialog's `contentItem`, and that the
  content ends above the `footer`. New tests
  `theNewGameDialogHasRoomForItsText` and `theQuitDialogHasRoomForItsText`
  in `dialogs_test.cpp` use it, and `aNewGameCanBeStartedFromTheMenu` in
  `mobile_test.cpp` checks it before clicking Yes.
- Before the fix, with Qt 6.11.2 offscreen: New Game failed under Fusion and
  Basic on desktop and mobile; Quit failed under Basic only, as expected
  from its smaller default padding.
- Fix: the `Text` in both dialogs has `width: parent.width` instead of
  `anchors.fill: parent`, and the mobile-only `verticalAlignment` is gone.
  `Dialogs.qml` no longer sets a `height` on either dialog, so `Dialog`
  sizes itself from the title, the wrapped text, the buttons and the
  padding. No screen-height cap was needed: the text is one or two lines.
- Screenshots showed that the Quit dialog, now at its natural height with
  the style's default padding, was shorter than before under Fusion and
  its text sat against the buttons. It now has `padding: 40` like New
  Game, which is what the bug list had assumed it already had. The two
  dialogs now look alike in both styles.
- Verified: all 191 `ctest` tests pass, and the dialogs, mobile and
  application UI tests pass under both `QT_QUICK_CONTROLS_STYLE=Fusion` and
  `Basic`. Linter clean. Not checked locally: Qt 6.9 and the native Windows
  and macOS styles, which CI covers for Windows.
