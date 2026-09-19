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
2. Add a `getWisdomWindow()` accessor to `lib/WisdomChess.ts` holding the one
   `window` cast, and use it everywhere else, including `main.tsx` and the
   tests. The `ReactWindow` and `WisdomWindow` interfaces stay.

Out of scope:

- The `any`s in the generated `lib/wisdom-chess-module.d.ts`. They are
  Emscripten's, in `destroy(obj: any)` and in the constructor constraint of
  `wrapPointer` and `castObject`, which we never call. `destroy` is only passed
  typed objects, and an `any` in a generic constraint does not reach a value,
  so rewriting them would catch nothing. A guard against new ones would only
  break CI on a generator upgrade. Unlike the enum branding, there is no bug
  this would prevent.
- A `declare global` augmentation of `Window`. It would make the globals
  type-check in every file; the accessor keeps the dependency visible.
- `JSON.parse` in `App.tsx`'s draw status handler returns `any` and is cast to
  the message type. The message comes from our own worker, so the cast stays.

## Verification

- `npm run check:wasm-types`, `npx tsc --noEmit`, `npx vitest run`,
  `npm run build`.
- `grep -rnw any src --exclude=wisdom-chess-module.d.ts` finds only prose, and
  `grep -rn 'window as' src` finds only `getWisdomWindow()`.
- In a browser, the board loads and a move and the engine's reply work.
