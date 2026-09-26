# CI consolidation

## Motivation

Every pull request starts three workflows and about 17 jobs:

- `cmake.yml`: `lint`, seven `build` matrix entries (Release and
  RelWithDebInfo on Ubuntu, macOS and Windows, plus Debug on Ubuntu),
  `build-filc` and `sanitizers`.
- `web.yml`: its own identical `lint`, then one `build` that does the
  React WASM build, the front-end tests and the Qt WASM build.
- `installers.yml`: one job per platform, path-filtered.

The idea was that these could be serialized onto fewer runners: one per
native platform, one for WASM, one for Debug + FIL-C + sanitizers.

## Findings

Timings from runs on 2026-09-26 (`gh api .../actions/runs/<id>/jobs`),
measured from job start:

| Job | Duration | Of which |
|---|---|---|
| web `build` | 5.4 min | Qt WASM build 164 s, emsdk 39 s, Netlify CLI install 30 s |
| `sanitizers` | 4.5 min | build 139 s, tests 90 s |
| Windows RelWithDebInfo | 4.8 min | build 168 s, tests 48 s |
| Windows Release | 4.1 min | |
| Ubuntu Release / RelWithDebInfo | 2.7 / 3.2 min | build 96 s, tests 29 s |
| macOS Release / RelWithDebInfo | 2.2 / 2.0 min | |
| Ubuntu Debug | 1.7 min | |
| `build-filc` | 1.5 min | |
| Installers, per platform | 2.5 to 4 min | |
| `lint` | 12 s | |

Setup steps are small: the Qt install is 25 to 45 s per job with the
cache warm, checkout and CPM cache are a few seconds.

- The workflows are independent, so nothing waits on the WASM job. Each
  workflow takes 5.5 to 6 min because each contains a job of about that
  length: WASM, sanitizers, and Windows RelWithDebInfo are all within a
  minute of each other.
- Serializing makes the PR slower. Windows Release then RelWithDebInfo on
  one runner is about 7.5 min plus setup; Debug, FIL-C and sanitizers on
  one runner is about 7 min. Both exceed the WASM job. Builds dominate
  every job, so sharing a runner's setup saves under a minute.
- Job count matters during bursts. The repository is public, so runners
  are free but capped at 20 concurrent jobs and 5 on macOS. When six
  branches were pushed at 17:10, `lint` jobs queued for 3 to 4 min before
  their dependent builds could start, and installer jobs waited 1 to
  3 min. `needs: lint` turns that queueing into a delay on every build.
- RelWithDebInfo compiles the same `NDEBUG` code as Release at `-O2`
  with debug info. The `sanitizers` job already builds RelWithDebInfo
  with Clang. The three native RelWithDebInfo entries add no coverage.
- `web.yml`'s `lint` is a copy of `cmake.yml`'s.

## Decisions

- Do not serialize the native platforms or the sanitizer job. Sanitizers
  stay alone; anything added there lands on the critical path.
- Reduce redundant configurations instead, keeping the wall clock at
  about 6 min and cutting the job count from 17 to about 11.

## Plan

1. **Drop RelWithDebInfo from the native matrix** in `cmake.yml`. Release
   stays on all three platforms, Debug stays on Ubuntu. This frees one
   macOS slot per PR, the scarcest resource.
2. **Stop gating builds on `lint`.** Remove `needs: lint` so lint runs in
   parallel. A lint failure still fails the PR; the compute wasted on a
   lint-failing push is free on a public repository.
3. **Merge `web.yml` into `cmake.yml`** as a `build-wasm` job under the
   single `lint`, removing the duplicate. The Netlify deploy steps and the
   `pull_request_target: closed` trigger for the production deploy move
   with it; check the `concurrency` block still cancels only
   `pull_request` runs. If that trigger makes the merge awkward, the
   fallback is to delete only the duplicate `lint` from `web.yml`.
4. **Merge Debug and FIL-C into one Ubuntu job**, run serially. About
   3 min total, well under the critical path. Marginal; do last, or skip
   if it makes the matrix harder to read.
5. Optionally cache or `npx` the Netlify CLI in the WASM job to save
   30 s.

Update `README.md` or `AGENTS.md` only if they describe the workflow
layout.

## Implementation Progress
