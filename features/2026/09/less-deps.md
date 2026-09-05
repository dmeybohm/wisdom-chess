# Reduce React front-end dependencies

## Problem

Most commits touching `src/wisdom-chess/ui/react/` are Dependabot bumps of
transitive packages (workbox-build, serialize-javascript, rollup,
minimatch, picomatch, form-data, flatted, lodash, babel plugins, ...).
The app itself imports very little. Every declared dependency in
`package.json` is actually used, so the churn is entirely in the
transitive closure.

## Measurements (from `package-lock.json`, 589 entries total)

Closure size and entries unique to that package (removed if only it is
dropped):

| Top-level package             | closure | unique |
|-------------------------------|--------:|-------:|
| vite-plugin-pwa               |     450 |    295 |
| vitest / @vitest/ui           |     192 |      0 |
| @testing-library/react        |      89 |     13 |
| @vitejs/plugin-react / vite   |      88 |      1 |
| jsdom                         |      71 |      0 |
| react-dnd                     |      19 |      3 |
| @testing-library/user-event   |      16 |      2 |
| @testing-library/jest-dom     |      11 |      8 |
| rc-slider                     |      10 |      3 |
| react-dnd-html5-backend       |       6 |      1 |

Scenarios:

- Drop `vite-plugin-pwa`: 589 -> 294 entries.
- Also replace `rc-slider` and `react-dnd*` with native DOM: -> 281.
- Keep only react, react-dom, vite, plugin-react, typescript: 99
  (i.e. the test stack costs ~180 entries; keeping it is fine).

Mapping recent Dependabot bumps to owners: workbox-build,
serialize-javascript, rollup, minimatch, glob, ajv, brace-expansion,
cross-spawn and the babel plugin all come only via `vite-plugin-pwa`
(workbox bundles its own babel/rollup toolchain). form-data and flatted
come via vitest/jsdom. lodash, tar-fs and tmp are already gone.

## Plan

### Step 1: Replace `vite-plugin-pwa` with hand-written PWA files

This is the one change that matters. It removes half the lockfile and
nearly all of the churn while keeping installability and offline play.

- `public/manifest.webmanifest`: the manifest currently generated from
  the `manifest:` block in `vite.config.ts` (name, short_name,
  description, theme_color, three logo icons, display standalone).
  Link it from `index.html`.
- `public/sw.js`: a small service worker. Workbox's `generateSW` does
  hash-based precaching from a build-time file list. We replace that
  with runtime caching: on `fetch`, for same-origin GET requests use
  cache-first for hashed assets (`/*.*.js`, `/*.*.wasm`, `/assets/*`)
  and network-first with cache fallback for navigations and
  `index.html`. Skip anything under `/qml/` (matches the existing
  `navigateFallbackDenylist`). Bump a `CACHE_NAME` constant so old
  caches are purged on `activate`.
  Since `index.html`, the wasm loader and every engine chunk are all
  fetched on first load, the app is fully cached after one visit.
- Register the worker from `main.tsx` (`navigator.serviceWorker
  .register('/sw.js')`) in production only. `autoUpdate` behaviour
  is reproduced with `skipWaiting()` + `clients.claim()`.
- Remove `VitePWA` from `vite.config.ts` and the devDependency.

Trade-off: no precache manifest means a brand-new deploy is picked up
by the network-first `index.html` fetch on next load, exactly like
`autoUpdate` did. We lose Workbox's guarantee that *all* assets are
cached before first offline use, which is acceptable for this app.

### Step 2 (optional, small): drop `rc-slider`

Two sliders in `SettingsModal.tsx` (thinking time 1-10, depth 1-8).
Replace with `<input type="range">` and move the `.rc-slider-*` rules
in `Settings.css` to `input[type=range]` styling. Removes 10 entries and
the `rc-slider/assets/index.css` import.

### Step 3 (optional, moderate): drop `react-dnd`

`Board.tsx` / `Square.tsx` use `useDrag` / `useDrop` with the HTML5
backend (mouse only; touch already unsupported). Replace with native
`draggable` + `onDragStart` / `onDragOver` / `onDrop` handlers carrying
the source square in `dataTransfer`, or with pointer events. Removes 25
entries and the `DndProvider`. This changes user-facing behaviour
(drag preview image) so it needs manual testing in a browser. Defer
unless the churn from react-dnd itself becomes a problem; it has had
no Dependabot bumps.

### Not changing

- vitest, jsdom, testing-library: they are the test stack, share almost
  everything with vite, and their churn (form-data, flatted) is small.
- `@vitest/ui`: contributes zero unique entries, so removing it gains
  nothing.

## Verification

- `npm ci && npm run test -- --run && npm run build` in
  `src/wisdom-chess/ui/react`.
- Check `dist/` contains `manifest.webmanifest` and `sw.js`, and that
  `prepare-deployment.sh` still copies them (it copies `dist/*`).
- Manually: load the deployed preview, go offline, reload, confirm the
  app still starts and the engine wasm loads.
