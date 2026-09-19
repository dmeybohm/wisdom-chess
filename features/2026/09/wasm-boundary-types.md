# Typed WASM boundary for the React frontend

## Motivation

`ui/react/src/lib/WisdomChess.ts` describes the Emscripten WebIDL module by
hand and declares `Game`, `PieceColor`, `PieceType`, `GameStatus`,
`WebPlayer`, `DrawProposed`, `DrawByRepetitionType` and
`WorkerGameSettings` as `any`. This is the "`any` at the WASM boundary" item
in `bug-list-and-engine-warnings.md`.

The hand-written types had drifted from `ui/wasm/wisdom-chess.idl`, and
`any` hid it:

- The IDL spelled an enum value `ThreeFoldRepeition`, so the module exported
  `Module.ThreeFoldRepeition`, while the TypeScript type declared
  `ThreefoldRepetition`. `App.tsx` therefore passed `undefined` to
  `setHumanDrawStatus`. It only worked because `undefined` coerces to 0,
  which happens to be that enum value.
- `GameModel.getFirstHumanPlayerColor` was typed as a property but called as
  a method, `getCurrentGame()` was declared but is not in the IDL, and `Pawn`
  and `King` were missing from the module type.
- `mapPieceToIcon` and `fromNumberToColor` hard-coded 1..6 and 1/2.
- Earlier slips that compiled for the same reason: `PieceColor` used where
  `PieceType` was meant, and the test mock's wrong piece and color numbers.

## Plan

1. Fix and tighten the IDL: rename the misspelled enum value, and give
   enum-valued members their enum types instead of `long`, so a generator
   can type them.
2. Generate the TypeScript declarations from the IDL with `webidl-dts-gen`
   and check the result in. A wrapper script post-processes the output so
   that each enum is a branded number. A `--check` mode fails on a diff.
3. Rebuild `lib/WisdomChess.ts` on the generated types, with no `any`.
4. Fix what the compiler then reports, and add a regression test for the
   draw-status bug.
5. Run the check in CI and document the workflow.

## Decisions

- **Generated, not hand-written.** Nothing can drift from the IDL without
  the check failing, including method signatures.
- **`webidl-dts-gen` (pmndrs), v1.12.0, not `webidl2ts`.** Reading
  `webidl2ts`'s source showed it emits Emscripten enums as string unions
  (`"wisdom::NoColor" | ...`) and declares no module constants, which is
  wrong for our glue, where enums are numbers. It was last published in
  April 2023 and depends on jsdom 16. The fork emits `const White: number`
  plus the enum type, strips the `wisdom::` prefix, and is maintained.
- **Pinned `npx`, not a dependency.** The script runs
  `npx --yes webidl-dts-gen@1.12.0`, so `package.json` and the lockfile do
  not grow, in line with `less-deps.md`. The cost is a download on first
  local use and on each CI run. Because the generated file is checked in, a
  failed download never blocks building or testing, only the drift check.
- **Branded numbers for enums.** The generator types every enum as
  `number`, which would let a `PieceColor` be passed as a `PieceType`. The
  wrapper rewrites each enum type to
  `number & { readonly __enum: '<name>' }` and gives the constants that
  type. It fails if an expected pattern is missing, so a generator upgrade
  cannot silently drop the branding.

## Implementation Progress

### Session #1

- IDL and C++: renamed `ThreeFoldRepeition` to `ThreefoldRepetition` in the
  IDL and in `WebDrawByRepetitionType`. Enum types replace `long` for
  `getPlayerOfColor`, `makeHumanMove`'s piece, both draw-status setters, the
  `GameSettings` constructor, and `WebColoredPiece`'s constructor and its
  `color` and `piece` attributes.
- The binder passes an enum type name through as the C type, and
  `web_types.hpp` already aliases those names, so arguments needed no C++
  signature changes: the unscoped enums convert to the existing `int`
  parameters. `WebColoredPiece`'s fields changed from `int` to `WebColor` /
  `WebPiece` because the glue returns the field itself.
- `WebGame` filled those fields with `toInt (piece.color())`, the engine's
  own numbering, which only matched the web enums by coincidence. It now
  uses `mapColor` and `mapPiece`, as one of the three sites already did.
- The wasm target builds without warnings and the regenerated glue exports
  `ThreefoldRepetition`.
