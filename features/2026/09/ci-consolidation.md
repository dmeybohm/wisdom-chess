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
- Release and RelWithDebInfo both define `NDEBUG`. For GCC and Clang,
  `CMakeLists.txt` replaces RelWithDebInfo's default `-O2` with `-O3`,
  matching Release optimization while retaining debug info. MSVC uses
  its own configuration flags. A dedicated Ubuntu Release job still
  tests that configuration, so repeating it on every platform adds
  little coverage. The React WASM build deployed to Netlify uses
  RelWithDebInfo (`scripts/build-react-wasm.sh`), and the `sanitizers`
  job builds it with Clang. Release is still compiled on every PR by
  the Qt WASM build (`scripts/build-qml-wasm.sh`) and by the installers
  when their paths change.
- `web.yml`'s `lint` is a copy of `cmake.yml`'s.

## Decisions

- Do not serialize the native platforms or the sanitizer job. Sanitizers
  stay alone; anything added there lands on the critical path.
- Reduce redundant configurations instead, keeping the wall clock at
  about 6 min and cutting the job count from 17 to about 10.

## Plan

1. **Drop Release from the native matrix** in `cmake.yml`. RelWithDebInfo
   stays on all three platforms, since it is the configuration that is
   deployed. This frees one macOS slot per PR, the scarcest resource.
   Windows RelWithDebInfo is the slower of the two Windows entries, so
   the critical path does not change. One Release build survives in
   step 4.
2. **Stop gating builds on `lint`.** Remove `needs: lint` so lint runs in
   parallel. A lint failure still fails the PR; the compute wasted on a
   lint-failing push is free on a public repository.
3. **Merge `web.yml` into `cmake.yml`** as a `build-wasm` job under the
   single `lint`, removing the duplicate. The Netlify deploy steps and the
   `pull_request_target: closed` trigger for the production deploy move
   with it; check the `concurrency` block still cancels only
   `pull_request` runs. If that trigger makes the merge awkward, the
   fallback is to delete only the duplicate `lint` from `web.yml`.
4. **One Ubuntu job for Release, Debug and FIL-C**, run serially in
   separate build directories, replacing the Debug matrix entry and
   `build-filc`. Release and Debug share the Qt install; FIL-C builds
   with the QML UI off as it does today. Roughly 28 s Qt, 130 s Release
   build and tests, 70 s Debug, 80 s FIL-C: about 5 min, just under the
   WASM job. Keep the Release step with the slow tests on so the
   optimized engine is still tested at `-O3`.
5. Optionally cache or `npx` the Netlify CLI in the WASM job to save
   30 s.

Update `README.md` or `AGENTS.md` only if they describe the workflow
layout.

## Implementation Progress

### Session 1 (2026-09-26)

- `cmake.yml`: the matrix is now three `include` entries, RelWithDebInfo
  on Ubuntu, macOS and Windows. The `Lint QML` step moved to
  `runner.os == 'Linux'` since Release is no longer in the matrix. The
  Debug entry and `build-filc` are replaced by `build-release-debug-filc`,
  which builds Release with the slow tests, Debug, and FIL-C in
  `build-release`, `build-debug` and `build-filc`. No job has
  `needs: lint`.
- `web.yml` stays a separate workflow (step 3's fallback). Its
  `pull_request_target: closed` trigger for the production deploy would
  have run every native job on PR close, or needed an `if` on each. Only
  its duplicate `lint` job and the `needs: lint` were removed.
- Step 5 (caching the Netlify CLI) was not done.
- Jobs per PR: `lint`, three `build`, `build-release-debug-filc`,
  `sanitizers`, web `build`, plus the three path-filtered installers.
  Nine to twelve, from seventeen.
- Job names changed, so any required status checks in branch protection
  that named `build (ubuntu-latest, Release, gcc)` or `build-filc` need
  updating. The token used here could not read the protection settings.
- First run on PR #271 (run 36259034539), all green:

  | Job | Duration |
  |---|---|
  | `build-release-debug-filc` | 5m48s: Release 87 s build + 28 s tests, Debug 77 s + 6 s, FIL-C 103 s + 2 s, Qt 29 s |
  | `sanitizers` | 5m22s |
  | web `build` | 5m05s |
  | Windows RelWithDebInfo | 4m18s |
  | Ubuntu / macOS RelWithDebInfo | 2m40s / 1m46s |

  The combined job overshot the estimate by about 40 s, mostly the FIL-C
  build (103 s against 77 s in the run measured above), and is now the
  longest job by 26 s over `sanitizers`. The workflow's wall clock is
  5m52s, the same as before the change (5m49s and 5m46s). Left as is;
  if it grows, split FIL-C back out into its own job.

### Session 2 (2026-09-26)

- Corrected the build-type rationale to reflect the project's `-O3`
  override for GCC and Clang RelWithDebInfo builds.
