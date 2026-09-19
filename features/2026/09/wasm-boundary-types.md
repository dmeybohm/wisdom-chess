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
- Generator: `ui/react/scripts/generate-wasm-types.mjs`, plain Node with no
  dependencies, run as `npm run generate:wasm-types`. It runs the pinned
  generator into a temporary directory, brands the enums, and writes
  `src/lib/wisdom-chess-module.d.ts`. `npm run check:wasm-types` compares
  without writing and exits 1 when a file is stale.
- The generator handled the whole IDL, including the `[Value]` array
  attribute and the `[Prefix]` interfaces, and already emitted enum types
  for returns and attributes. After the IDL change the parameters are typed
  as well, for example
  `makeHumanMove(src: string, dst: string, pieceType: wisdom_WebPiece)`.
- `lib/WisdomChess.ts` now aliases the generated types under the names the
  app already imported (`Game`, `GameModel`, `PieceColor`, `PieceType`,
  `GameStatus` and so on), so the components did not change. The
  hand-written module, model and settings types and every `any` alias are
  gone. `WasmObject` names the classes JavaScript has to destroy, and
  `withWasmObjects` takes those.
- `mapPieceToIcon` and `fromNumberToColor` compare against the module's
  constants, not the numbers 1 to 6 and 1 and 2.
- With real types in place the application code compiled unchanged. It had
  been using the enums consistently; only the declarations were wrong.
  `App.tsx` lost its two remaining `any`s in the local `throttle` helper and
  the literal `0 as GameStatus`.
- Test doubles: the mocks had been numbered by hand and were wrong twice
  before. The generator also writes `src/test/wasm-enum-values.ts`, the enum
  values in IDL order, covered by the same check, and `src/test/wasmEnums.ts`
  exposes them with the branded types. A first attempt imported the IDL into
  the tests with `?raw`, but Vite refuses files outside the project root,
  and loosening that was not worth it. The values assume plain C++ enums
  numbered from zero, which holds for all six.
- New tests: answering the threefold repetition and the fifty move dialogs
  calls `setHumanDrawStatus` with the right, defined, draw type.
- CI: `web.yml` runs `npm run check:wasm-types` before the front-end tests.
  `AGENTS.md` documents the workflow.
- Verified:
  - The threefold test fails when `App.tsx` is pointed back at the old
    misspelled name, which passes `undefined`.
  - A scratch file passing `White` as a piece type, `Pawn` as a color, and
    the bare number `1` as a color produced three compile errors.
  - Renaming an enum value in the IDL made the check exit 1 and name both
    generated files; restoring it made the check pass.
  - `tsc` is clean, 41 React tests and 114 fast C++ tests pass, the wasm
    target, the desktop build and the production bundle build without
    warnings, and `lib/WisdomChess.ts` contains no `any`.
  - In headless Chromium against the dev server:
    `typeof wisdomChessWeb.ThreefoldRepetition` is `number` and the old
    spelling is `undefined`; all 32 piece icons render through the module
    constants; a clicked move, the engine's reply, Settings then Apply, and
    a new game all work, with no console errors.
- Not verified: the new CI step has not run on GitHub yet, and no draw
  offer was reached in the browser, so the corrected draw type is covered
  by the unit tests only.
- Left as it is: the generated file itself contains `any` in Emscripten's
  helper signatures (`destroy`, `wrapPointer`, `castObject`), and the worker
  message still arrives as JSON whose fields are cast to the enum types at
  that one point in `App.tsx`.
