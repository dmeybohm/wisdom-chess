# Remove the remaining `any`s from the React frontend

## Motivation

`wasm-boundary-types` removed every `any` from `lib/WisdomChess.ts` and
`App.tsx`. What was left in `ui/react/src`:

- `App.test.tsx`: the `GameSettings` mock used `function (this: any)` and
  `as any`.
- `lib/wisdom-chess-module.d.ts` (generated): Emscripten's helper signatures,
  `destroy(obj: any)` and `new (...args: any) => any` in `wrapPointer` and
  `castObject`.

Also, `window` was reached through `(window as unknown) as ReactWindow` or
`WisdomWindow` in eleven places. Not an `any`, but the same kind of hole.

## Plan

1. The `GameSettings` mock: type `this` as `GameSettings` and drop the inner
   `as any`; the whole mock is already cast to `WisdomChess`.
2. `scripts/generate-wasm-types.mjs`: after branding the enums, rewrite
   `destroy(obj: any)` to take `unknown`, and the constructor constraint to
   `abstract new (...args: never) => unknown`, which still satisfies
   `InstanceType`. Fail if any `any` is left in the output, so a generator
   upgrade that adds one is caught by `check:wasm-types` in CI. Regenerate.
3. Replace `ReactWindow` and `WisdomWindow` with a `declare global` augmentation
   of `Window` in `lib/WisdomChess.ts`, with the same members and
   optionality, and use `window` directly everywhere.

Out of scope: `JSON.parse` in `App.tsx`'s draw status handler returns `any`
and is cast to the message type. The message comes from our own worker, so
the cast stays.

## Verification

- `npm run check:wasm-types`, `npx tsc --noEmit`, `npx vitest run`,
  `npm run build`.
- `grep -rnw any src` and `grep -rn 'window as' src` find nothing.
- Make the generator skip one replacement and confirm it fails.
- In a browser, the board loads and a move and the engine's reply work.
