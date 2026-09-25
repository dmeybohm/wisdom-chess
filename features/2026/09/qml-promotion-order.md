# Promotion dropdown order

## Motivation

Finding 22 of [qml-cleanups.md](qml-cleanups.md), open in
[bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md).
`popups/PromoteDropdown.qml` lists the promotion pieces queen first, with
the queen on the target square and the rest hanging down from it. For a
promotion on the far rank (row 7) the list is drawn at rows 4 to 7, so
the model is reversed to keep the queen on the target square. That
reversal is triggered from `onDestinationColumnChanged`, but it depends
on the row. Two cases miss it:

- A first promotion on the a-file: the property's default is column 0,
  so nothing changes and no handler runs.
- Promotions by both sides on the same file: the second changes the row
  but not the column.

In both the list is drawn queen-farthest from the pawn, with the knight
on the target square. All four pieces stay selectable.

## Plan

1. **Test first**, in `ui/qml/test/application_test.cpp` next to
   `aPromotionGoesThroughTheDropdown`: play a game where Black's b-pawn
   captures on a2 and promotes on a1 (White plays Nc3, Rb1 and h-pawn
   moves to stay out of the way), then click a1 twice, which chooses
   whatever is drawn on the target square. Expect a queen. Confirm it
   fails with a knight before the fix.
2. **Fix**: trigger the reversal from `onDestinationRowChanged`.
   `Board.qml` sets the row before the column, and the reversal reads
   only the row, so the order of assignment does not matter.
3. Tick the bug list entry and add an Implementation Progress section.
