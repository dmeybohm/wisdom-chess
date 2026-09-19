# CI sanitizer jobs

## Motivation

The bug list
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md))
has this open item under "Build and infrastructure":

> No sanitizer job and no Linux/Clang in `.github/workflows/cmake.yml`.
> `WISDOM_CHESS_ASAN` is unused by CI.

- CI never runs a sanitizer, and never builds with Clang on Linux: the
  build matrix excludes `ubuntu-latest` with `clang`. `WISDOM_CHESS_ASAN`
  exists but nothing uses it.
- The FIL-C job covers memory safety for the engine's fast tests only. It
  builds no slow tests, no console, UCI or QML, and it checks neither
  undefined behaviour nor data races.
- The threaded code has no race detection at all: the UCI search thread,
  the QML `ChessEngine` thread, and the held-move logic in `GameModel`
  (see [qml-engine-move-delay.md](qml-engine-move-delay.md)).

## Decisions

- One new Clang-built sanitizer job closes both halves of the item. The
  `ubuntu-latest`/`clang` exclusion in the build matrix stays; a plain
  Release Clang build is not added.
- The QML UI and its `QML: ...` tests are included from the start, not as
  a follow-up.
- Two builds: AddressSanitizer with UndefinedBehaviorSanitizer, and
  ThreadSanitizer. They cannot be combined in one binary.

## Findings from reading the current setup

- `CMakeLists.txt:66-75`: `WISDOM_CHESS_ASAN` adds
  `-fsanitize=address,undefined` with `add_compile_options`, so it applies
  to every target. That is right, since every object in the process must
  be instrumented. But UBSan only prints by default and the test still
  passes, so a CI job would miss its reports. It needs
  `-fno-sanitize-recover=undefined`, and `-fno-omit-frame-pointer` for
  usable stacks.
- The fatal tests (`engine/test/run_fatal_test.cmake`) judge a case by its
  output and a non-zero result, so an `abort()` under ASan or TSan should
  still pass. To be confirmed locally.
- The CLI tests have `TIMEOUT 180` (`cmake/CliTests.cmake`) and the UCI
  searches are bounded by time, so TSan's slowdown should fit. To be
  confirmed.
- The QML UI tests get `QT_QPA_PLATFORM` and `QT_QUICK_BACKEND` through
  `set_tests_properties(... ENVIRONMENT ...)` in
  `ui/qml/test/CMakeLists.txt`. That adds to the inherited environment, so
  sanitizer options set on the workflow job reach the tests.
- The Qt that `install-qt-action` installs is not instrumented. Expected
  consequences: LeakSanitizer reports from Qt, fontconfig and similar
  libraries, which need a suppressions file; and TSan false positives,
  because `QMutex` locks through futexes that TSan cannot see.
- Local tools: `clang++-18` with the sanitizer runtimes, Qt 6.11.2 under
  `~/Qt`, and `scripts/install-ci-qt.sh` for a Qt 6.9 matching CI.

## Plan

1. **CMake options**, in place in the top-level `CMakeLists.txt`:
   - `WISDOM_CHESS_ASAN`: keep the name, correct the description to say
     ASan and UBSan, and add `-fno-sanitize-recover=undefined
     -fno-omit-frame-pointer` for GCC and Clang. The MSVC branch is left
     alone.
   - New `WISDOM_CHESS_TSAN`: `-fsanitize=thread -fno-omit-frame-pointer`.
     A `FATAL_ERROR` when combined with `WISDOM_CHESS_ASAN` or used with
     MSVC.
2. **Local ASan and UBSan run first**, so that CI does not start red:
   `clang++-18`, `RelWithDebInfo`, QML on, fast and slow tests, in
   `build-asan`. Record how long the slow suite takes; keep it in the job
   unless it exceeds roughly ten minutes.
3. **Local TSan run** in `build-tsan`, fast tests only. The slow suite is
   single-threaded, so TSan would add cost there and no coverage.
4. **Findings policy.** A report in `wisdom::` code is a bug. Fix it on this
   branch with the minimum change, one commit each, and log it under
   Implementation Progress. If a fix needs a design decision, stop and
   discuss it. Reports wholly inside third-party code go in
   `scripts/sanitizers/lsan.supp` or `scripts/sanitizers/tsan.supp`, each
   entry with a comment naming the library and the stack that was seen.
   Never suppress a frame of our own code.
5. **TSan and Qt fallback rule.** Try the `QML: ...` tests under TSan with
   suppressions. If the noise from the uninstrumented Qt cannot be
   suppressed by library without hiding our own frames, build the TSan job
   with `WISDOM_CHESS_QML_UI=Off` and record why. The ASan job keeps QML
   either way.
6. **Workflow.** A new `sanitizers` job in `.github/workflows/cmake.yml`:
   `needs: lint`, `ubuntu-latest`, `fail-fast: false`, and a matrix over
   `address` and `thread`.
   - Steps copied from the `build` job: checkout, `install-qt-action` 6.9
     with its cache, the CPM cache, configure, build, and
     `ctest --output-on-failure -j 4`. No install step.
   - Configure with `clang` and `clang++`, `RelWithDebInfo`,
     `-DWISDOM_CHESS_QML_UI=On`, `-DWISDOM_CHESS_BUILD_LINTER=Off`, the
     matrix entry's sanitizer option, and the slow tests as decided in
     steps 2 and 3.
   - Job `env`: `ASAN_OPTIONS`
     (`detect_leaks=1:strict_string_checks=1:check_initialization_order=1`),
     `UBSAN_OPTIONS=print_stacktrace=1`, and `LSAN_OPTIONS` and
     `TSAN_OPTIONS` naming the suppression files. `TSAN_OPTIONS` also gets
     `halt_on_error=1:second_deadlock_stack=1`.
   - Known runner problem: high-entropy ASLR on the Ubuntu 24.04 images has
     broken sanitizer startup with "unexpected memory mapping". If it
     appears, add a `sudo sysctl vm.mmap_rnd_bits=28` step.
7. **Documentation.** In `AGENTS.md`, the build options table
   (`WISDOM_CHESS_ASAN` wording and a `WISDOM_CHESS_TSAN` row) and a short
   note under Testing on running the sanitizer builds locally with the
   suppression files. The README too, if it lists the options. Tick the
   item in `bug-list-and-engine-warnings.md` with a link to this document.
8. Commit after each step. Open the PR once both builds are green locally,
   checking `gh auth status` first.

## Out of scope

- A plain Release Clang entry in the Linux build matrix.
- MemorySanitizer, which needs an instrumented libstdc++ and Qt.
- The odd `/zi` in the MSVC link options of the existing ASan block.
- Sanitizing the wasm and installer builds.

## Verification

- `ctest` fully green in `build-asan` and `build-tsan`, with the same
  environment variables the workflow sets.
- Prove that each job can fail: a temporary heap overflow, a signed
  overflow, and an unsynchronized write from two threads in a scratch
  test, each seen to fail its build and then removed.
- The ordinary `build/` Release tree still builds without warnings and
  passes, so the option changes do nothing when they are off.
- Both matrix entries green on the PR. Note their wall time here.

## Implementation Progress

### Session #1

- Added `WISDOM_CHESS_TSAN`, fixed `WISDOM_CHESS_ASAN` to add
  `-fno-sanitize-recover=undefined -fno-omit-frame-pointer`, and made the
  two mutually exclusive.
- **Local ASan+UBSan** (`build-asan`, `clang++-18`, `RelWithDebInfo`, QML
  on, fast+slow tests): found and fixed a real leak.
  `GameModel::setupNewEngineThread()` moved the `ChessEngine` onto
  `my_chess_engine_thread` and relied on `QThread::finished ->
  deleteLater()` to clean it up, but that signal only fires once the
  thread has actually run. `ApplicationTest::aModelThatNeverStartedCanBeDestroyed()`
  destroys a `GameModel` without ever calling `start()`, leaking the
  engine, its cloned `Game`, `TranspositionTable` (12 MB) and `History` —
  12,650,232 bytes across 16 allocations. Fixed by tracking whether the
  thread was ever started and deleting the engine directly if not
  (`src/wisdom-chess/ui/qml/main/game_model.{hpp,cpp}`). Full suite
  (fast + slow, 212 tests) is green in ~75s — well under the ~10 minute
  threshold for dropping slow tests from the CI job, so they stay in.
- **Local TSan** (`build-tsan`, fast tests only): all races found trace to
  Qt/glib internals (`QMetaType`'s `std::function` marshaling for a
  cross-thread queued signal, glib's `eventfd` thread wakeup, and
  QtQuick's pooled-thread `QArrayData` refcounting), never to a frame in
  our own code touching the racing memory — consistent with Qt's `QMutex`
  using a raw `futex()` on Linux that TSan cannot see, and the Qt build
  under test not being sanitizer-instrumented. Suppressed with three
  narrow `race:` entries in `scripts/sanitizers/tsan.supp` (function-name
  matches: `QMetaType`, `eventfd`, `QArrayData`). QML stayed on for TSan;
  the `WISDOM_CHESS_QML_UI=Off` fallback was not needed. All 183 fast
  tests green with the suppressions applied.
- `scripts/sanitizers/lsan.supp` created empty — the ASan/LSan run found
  no third-party-only leaks.
- Added the `sanitizers` matrix job to `.github/workflows/cmake.yml` and
  documented both options and the local repro commands in `AGENTS.md`.
