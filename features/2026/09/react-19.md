# React 19

## Motivation

React only patches its current major version. The last 18.x release was
18.3.1 in April 2024, while fixes through 2026 have shipped for 19.0,
19.1 and 19.2 only. Dependabot's PR #284 bumped `react` and
`@types/react` to 19 but left `react-dom` and `@types/react-dom` on 18,
so `npm ci` failed on the `@types/react-dom` peer range. This branch
moves all four together and replaces that PR.

## Changes

- `react`, `react-dom`, `@types/react` and `@types/react-dom` go to
  `^19.3.0`.
- React 19's types drop the global `JSX` namespace, so the two explicit
  return types (`Modal`, `TopMenu`'s `Menu`) use `React.JSX.Element`.
- React 19's ref callback type allows only a cleanup function or nothing
  as the return value. `react-dnd` 16's connectors return a
  `ReactElement | null`, so passing `drag`, `drop` or `preview` straight to
  `ref=` no longer type-checks. `Square.tsx` wraps each in a callback that
  discards the result. `react-dnd` is no longer maintained, so it is the
  dependency most likely to hold back a later upgrade.

## Implementation Progress

### Session 1 (2026-09-26)

Done as above. `tsc`, the 50 vitest tests and `vite build` pass. The unit
tests only cover starting a drag, so the WASM build was also run in
headless Chromium: dragging e2 to e4 moves the pawn and hands the move to
Black, with no console warnings.
