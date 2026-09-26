# Dragging pieces with a mouse

## Motivation

The QML frontend moves a piece by two clicks: one on the source square,
one on the destination. Dragging the piece is what a mouse user expects,
and the React frontend already offers it through `react-dnd`. Both
frontends should follow one rule: drag with a mouse or touchpad,
tap-to-move with a finger. Click-to-move stays everywhere.

Touch dragging is deliberately left out for now. It needs its own
ergonomics (the finger hides the piece, there is no hover for a
destination highlight) and that can be added later without changing the
structure here.

## Where the platforms differ, and where they do not

QML has two kinds of drag. `Drag.Automatic` hands the drag to the
operating system through `QDrag`; Android supports that only partly and
WebAssembly not at all. A chess piece never leaves the scene, so it only
needs an internal drag, and `DragHandler` does that with the same code
for mouse and touch on every platform the app builds for. So there is
one implementation, not one per platform.

Which pointers may drag is not a platform question either.
`Platform.isMobile` is wrong for a tablet with a mouse and for a laptop
with a touchscreen, and it does not cover a phone browser running the
WebAssembly build at all. `DragHandler.acceptedDevices` decides per
device instead, and widening it to touch later is a change to that one
property.

The React board's drag comes from `react-dnd-html5-backend`, which
listens to the browser's native drag events. Chrome on Android never
sends those from a touch, so dragging is silently off there; iOS Safari
does send them after a long press, with the system ghost image, so
iPhone users get a half-working drag nobody designed. A
`(pointer: fine)` media query in the `canDrag` predicate makes the rule
explicit and the same as the QML one. It asks about the primary pointer,
so a laptop with a touchscreen still drags with the mouse and a tablet
with a mouse attached stays touch-first. `(any-pointer: fine)` would
enable drag on a phone with a stylus, which is not wanted.

## Plan

Built on `qml-shadowing`, whose selection rewrite in `Board.qml` puts
every way of starting a move in one place.

### QML

1. **Which pieces may be lifted.** Nothing in the QML knows which pieces
   belong to the human whose turn it is. With click-to-move an illegal
   source only yields a move status; with dragging every piece would
   lift and snap back. Add `Q_INVOKABLE bool canMoveFrom (int row,
   int column)` to `GameModel`, true when the square holds a piece of
   the side to move and that side's player is human and the game is not
   over. `Game` itself does not change, so the other frontends are not
   affected.
2. **`DragHandler` on `Piece`.** With `target: null`, the handler drives
   the existing `Translate` from its `activeTranslation`; `Piece` grows
   a `dragging` state that disables the two `Behavior` animations while
   the pointer moves it. `acceptedDevices` is mouse and touchpad, so a
   touch press falls through to the square's `MouseArea` and
   click-to-move. `enabled` is bound to `canMoveFrom`.
3. **The drop.** On release, `Piece` maps the pointer's scene position
   into the board (mapping through items accounts for the flipped
   board's rotation), converts to a row and column, and emits
   `dropped(sourceRow, sourceColumn, row, column)`. `Board` clears the
   click selection and routes the drop through
   `animateRowAndColChange`, so promotion, the game-over check and the
   engine's reply behave as for a click. Restoring the translation's
   binding after the drop snaps the piece to wherever the model now
   says it is; for a rejected move that is its old square, and the
   `Behavior` animates it back. A release outside the board does the
   same.
4. **Click-to-move and the tests stay as they are.** The square
   `MouseArea` keeps taking the press, so a click on a piece still
   selects the square beneath. The handler steals the grab only once the
   pointer passes the platform's drag threshold. Pressing a piece while
   another square is selected, then dragging, should clear that
   selection rather than leave it highlighted.
5. **Tests.** The fixture gets a `drag (src, dst)` helper made of a
   press, a few moves past `QStyleHints::startDragDistance`, and a
   release. Cases: a legal drag moves the piece; an illegal drag leaves
   the board unchanged and the piece back on its square; a drag to
   promotion opens the dropdown; a drag of the wrong colour, or of a
   piece whose player is the computer, does nothing; a drag on the
   flipped board lands on the right square; a click after a drag still
   works. Touch cannot be sent through `QTest::mouseClick`, so the
   touch exclusion is pinned with `QTest::touchEvent` if that reaches
   the handler offscreen, otherwise noted as untested.

### React

6. `canDrag` in `Square.tsx` also returns false when
   `window.matchMedia('(pointer: fine)')` does not match. The predicate
   runs at the start of each drag, so a mouse plugged into a tablet is
   seen on the next attempt with nothing to subscribe to. `App.test.tsx`
   runs under jsdom, which has no `matchMedia`, so the check goes
   through a small helper that treats a missing `matchMedia` as a fine
   pointer, and a test stubs it both ways.

### Verify

`ctest` in Release and Debug, the UI tests under `Basic` and `Fusion`,
`all_qmllint`, the C++ `lint` target, `npm test` in the React
directory, and the WebAssembly build of the QML frontend in a browser
with the device toolbar set to a phone, where a touch must still
tap-to-move.

## Risks

- The pieces layer sits above the squares. If the handler takes the
  press instead of letting it reach the square's `MouseArea`, every
  existing UI test that clicks a piece breaks. The grab permissions of
  `DragHandler` default to taking over from items, which is the
  behaviour wanted, but this is the first thing to check.
- A drag during the flip animation, or while the engine's held move is
  being shown, is not a case worth designing for; the drop just goes
  through the same path as a click.

## Implementation Progress

### Session #1

Both frontends are done; two things went differently from the plan.

- **The handler is on the pieces layer, not on each piece.** A
  `DragHandler` on every `Piece` worked until the pressed piece was
  still sliding into its square: while a handler is below the drag
  threshold it only stays interested in a point that is inside its
  parent item, and the animating piece moved out from under the pointer,
  so the handler let go before it could activate. The
  drag-to-promotion test found this, since it drags a pawn two clicks
  after it arrived. One handler on the layer instead reads the square
  under the press, asks `GameModel.canMoveFrom()` about it, and finds
  that square's delegate in the `Repeater`; the piece is then dragged
  by its model square, whatever it is drawn doing. `Piece` keeps only
  `lift()`, `dragTo()` and `drop()`, and its `Behavior` animations are
  off while `dragging` is set. The drop goes through `pieceDropped`
  before `drop()` restores the bindings, so the piece animates from
  where it was let go to its new square, or back home.
- **The square's `MouseArea` is a `TapHandler`.** The plan's first risk
  came true: a `DragHandler` above a `MouseArea` kept the press from
  reaching it, and every click test failed. A `TapHandler` beneath a
  `DragHandler` is the pair Qt designed to cooperate: the tap fires when
  the press does not turn into a drag, and is cancelled when it does.
  All twenty existing tests pass unchanged.
- `canMoveFrom()` lives on `GameViewModelBase`, beside `isLegalMove()`,
  and `GameModel` forwards to it; it is false unless the game is being
  played, the side to move is human, and the square holds that side's
  piece. The held engine move needs no special case: until it is shown,
  the model's turn is still the engine's.
- **Tests.** The fixture gained `drag()`, split into `startDrag()` and
  `finishDrag()` so a test can look at the piece in flight, and
  `touchTap()` and `touchDrag()` on a `QTest::createTouchDevice()`. Ten
  new cases in `application_test.cpp`: a legal drag, the piece following
  the pointer, an illegal drag, a drop off the board, a drag on the
  engine's turn, a drag of the opponent's piece, a drag clearing a click
  selection, a drag to promotion, a drag on the flipped board, and a
  finger that taps but does not drag. The React side has
  `Pointer.test.ts` for the media query, stubbed both ways and absent.
- **Lint.** `all_qmllint` has no warnings on Qt 6.11.2 or 6.9.3. It has
  seven new informational "can be shadowed" notes in `Board.qml`, all
  for members of the dragged `Piece` reached through a property or a
  cast, which is the one place the board has to hold a delegate it did
  not name by id. They are the same class of note as the `GameModel`
  method calls [qml-shadowing.md](qml-shadowing.md) left at `info`, and
  CI fails only on warnings.
- Verified: `ctest` (192) in Release and the seven QML tests in Debug,
  the C++ `lint` target, `npm test` and `tsc` in the React directory,
  all on Qt 6.11.2. Not done: the WebAssembly build of the QML frontend
  in a phone-emulating browser, to see a touch still tap-to-move there;
  the offscreen touch test covers the handler's device filter but not
  the browser's delivery of touches to Qt.

### Session #2

Review of the PR found the React check wrong for hybrid devices: the
`pointer` media feature describes the device's *primary* pointer, and
whether an attached mouse becomes primary is up to the browser, so a
tablet with a mouse could still refuse to drag. The rule is about the
input that starts the drag, not the device, and a native `dragstart`
always follows a `pointerdown` on the same element, which carries that
input as `pointerType`. `PieceOverlay` records it in a ref from
`onPointerDown` and `canDrag` refuses `touch`. That mirrors the QML
handler's per-device filter, closes iOS Safari's long-press drag on
purpose rather than by accident, and lets a pen drag. The media-query
helper and its test are gone; `Square.test.tsx` drives the real HTML5
backend in jsdom, with a stand-in `DataTransfer` and a plain event
carrying `pointerType`, since jsdom 23 has neither `DragEvent` nor
`PointerEvent`: a mouse press starts a drag, a touch press does not,
and each drag is judged by its own press.
