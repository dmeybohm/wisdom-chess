# ThreadSanitizer with instrumented dependencies

## Motivation

[ci-sanitizers.md](ci-sanitizers.md) added a ThreadSanitizer leg to CI, and
its Session #2 had to turn the QML UI off for it. What is left covers the
engine and UCI fast tests. The code that most needs race detection is not
covered by anything: the QML `ChessEngine` thread, the queued signals between
it and `GameModel`, and the held-move logic in `GameModel`.

The cause was not our code. The Qt that `install-qt-action` installs is not
instrumented, and:

- `QMutex` locks through a raw `futex()`, which TSan cannot intercept. Qt
  does tell TSan about those locks, through `QtTsan::mutexPreLock()`,
  `futexAcquire()` and the rest of `qtbase/src/corelib/thread/qtsan_impl.h`,
  but that header compiles to empty stubs unless Qt itself is built with
  `-fsanitize=thread`.
- Qt's glib event dispatcher wakes threads through glib, which is also
  uninstrumented.

So every hand-off that Qt protects correctly looks like a race, the reports
depend on thread timing, and a suppression tuned on one interleaving missed
the next one in CI. Suppressing by function name (`race:QArrayData`) is also
broad enough to hide a real race of ours that goes through a Qt container.

The fix is to run TSan against dependencies that are instrumented too. That
costs a Qt build, which is too slow for every CI run, so this moves TSan out
of GitHub CI and into a script that builds, or downloads, an instrumented Qt.

## Decisions

- Remove the `thread` entry from the `sanitizers` job in
  `.github/workflows/cmake.yml`. The ASan and UBSan leg stays as it is, QML
  included.
- New `scripts/build-tsan.sh`: gets a TSan-instrumented Qt, builds the app
  against it with `WISDOM_CHESS_TSAN=On` and the QML UI on, and runs `ctest`.
- The instrumented Qt is a normal Qt prefix. The app takes it through
  `WISDOM_CHESS_QT_DIR` with no CMake changes.
- Prebuilt prefixes are shared as assets on a GitHub Release of this
  repository, keyed by a hash of everything that went into them. Not Conan,
  for now: see the next section.

## Which dependencies need instrumenting

TSan, unlike MemorySanitizer, does not need every object in the process
instrumented. Its interceptors cover libc and pthreads. A library needs
instrumenting when it synchronises, or passes memory between threads, in a
way the interceptors cannot see: raw futexes, atomics, lock-free structures.

| Dependency | Today | Plan |
|---|---|---|
| GSL, doctest, nanobench | Built from source by CPM with the project's flags | Already instrumented; nothing to do |
| qtbase, qtdeclarative, qtsvg | Prebuilt, uninstrumented | Build from source with `-sanitize thread` |
| qtshadertools | Prebuilt | Build; qtdeclarative needs it at build time |
| zlib, libpng, libjpeg, pcre2, harfbuzz, double-conversion | System or bundled in Qt | Use Qt's bundled copies (`-qt-zlib` and so on), which get Qt's flags |
| glib | Qt's event dispatcher uses it | `-no-glib`. Qt falls back to its own dispatcher, which is instrumented with the rest of Qt. Removes the `race:eventfd` noise without building glib |
| D-Bus, ICU, OpenSSL, SQL drivers | Linked by the prebuilt Qt | Configure them out; nothing in the app uses them |
| fontconfig, freetype | System | Leave uninstrumented. Qt's `fontconfig` feature requires the system freetype, and both are used from the GUI thread only |
| OpenGL, xcb | System | Try `-no-opengl` and no xcb. The `QML: ...` tests run with `QT_QPA_PLATFORM=offscreen` and `QT_QUICK_BACKEND=software`. Fall back to the system libraries if Qt Quick will not configure without them |
| libstdc++, libc | System | Leave. Its inline atomics are instrumented in our objects, and the rest goes through interceptors. An instrumented libc++ is out of scope |

The QML sources import only `QtQuick`, `QtQuick.Controls`, `QtQuick.Layouts`
and `QtQml`, and CMake links `Quick`, `Svg`, `Test` and `QuickTest`, so the
four Qt repositories above are the whole set.

## Conan

Considered for two jobs: building the dependency graph with TSan flags, and
storing the binaries.

- **ConanCenter cannot host our binaries.** Its packages are built only by
  ConanCenter's own CI from `conan-center-index` recipes, in a fixed set of
  configurations with no sanitizers. Uploading binaries from a user's
  repository is no longer possible. We would need our own remote:
  self-hosted Artifactory CE or `conan_server`, or a hosted service. That is
  a server to run, or an account to depend on, for one package.
- **The ConanCenter `qt` recipe does not fit.** It offers 6.8.3, 6.10.3 and
  6.11.1, not the 6.9 that CI pins. It exposes Qt through Conan's `CMakeDeps`
  rather than Qt's own CMake package, and has a run of issues where
  `qt_add_qml_module`, `qt_policy` and QML plugin registration break
  (conan-center-index #7913, #20450, #18960, #30570).
  `ui/qml/CMakeLists.txt` uses all three. Adopting it means a second way of
  finding Qt in our CMake, for the TSan build only.
- **What Conan would give us** is a package id derived from settings, using
  the `compiler.sanitizer` sub-setting that Conan's sanitizer guide adds
  through `settings_user.yml`, and upload and download of the binary. With
  one compiled dependency, that is a hash and a tarball, which the script
  can do in a few lines.

Recommendation: no Conan in this feature. Revisit if the project gains
compiled third-party dependencies beyond Qt, where a real dependency graph
would start to pay for the remote. If we do want Conan anyway, the cheaper
form is a small recipe of our own in the repository that wraps the same Qt
build, plus our own remote, not the ConanCenter recipe.

## Plan

1. **Qt build recipe**, as functions in `scripts/build-tsan.sh`:
   - Resolve the Qt version the way `scripts/install-ci-qt.sh` does, from
     the `version:` in `cmake.yml`, with an override argument. Fetch the
     four source archives with `aqt install-src` from the same virtual
     environment, or from `download.qt.io` with checksum verification.
   - Configure with `clang-18` or newer, Ninja, and roughly:
     `-release -sanitize thread -nomake examples -nomake tests -no-glib
     -no-dbus -no-icu -no-openssl -no-opengl -fontconfig -system-freetype
     -qt-zlib -qt-libpng -qt-libjpeg -qt-pcre -qt-harfbuzz
     -qt-doubleconversion -submodules qtbase,qtshadertools,qtdeclarative,qtsvg`,
     plus line-table debug info (`-g1`) so that reports have file and line
     without a multi-gigabyte prefix. The exact list is settled by what
     configures; record the final one here.
   - Build with `TSAN_OPTIONS=report_bugs=0`. Qt's host tools (`moc`,
     `qmlcachegen`, ...) come out instrumented too, and TSan exits 66 when
     it has reported anything, which would fail the Qt build on a report
     inside a build tool.
   - Install to `$XDG_CACHE_HOME/wisdom-chess/qt-tsan/<key>`, next to where
     `install-ci-qt.sh` puts its Qt.
2. **Cache key.** A SHA-256 over the Qt version, the submodule list, the
   configure arguments, the Clang major version, the machine architecture
   and a recipe revision number in the script. The key names the prefix
   directory and the release asset, so changing any input builds a new Qt
   instead of reusing a stale one.
3. **Binary sharing.** Shared Qt builds are relocatable, so the prefix packs
   into `qt-tsan-<key>.tar.zst` with a `.sha256` beside it.
   - Download: plain `curl` from a fixed release tag (`tsan-deps`) of this
     repository. Public, so no authentication. Verify the checksum before
     extracting. A miss falls through to building.
   - Upload: `--upload`, maintainer only. Checks `gh auth status` first and
     prints the command to run if it fails. `gh release upload` to the same
     tag.
   - Check the archive size against GitHub's 2 GB per-asset limit. If it is
     over, drop to `-g0` for Qt or strip the QML tooling that is not needed
     after the app is built.
4. **The script's flow**: check tools (Clang 18 or newer, CMake, Ninja,
   `curl`, `zstd`, the fontconfig and freetype development files) and, when
   one is missing, print the install command and stop. It never runs `sudo`.
   Then: compute the key; use the cached prefix, else download, else build;
   configure `build-tsan` with `-DWISDOM_CHESS_QT_DIR=<prefix>
   -DWISDOM_CHESS_QML_UI=ON -DWISDOM_CHESS_TSAN=On`, `RelWithDebInfo`, fast
   tests; build; run `ctest` with the `TSAN_OPTIONS` that CI uses today.
   Options: `--qt-version`, `--cache-dir`, `--no-download`, `--deps-only`,
   `--upload`, and everything after `--` goes to `ctest`.
5. **Suppressions.** With the instrumented Qt in place, delete the three
   entries in `scripts/sanitizers/tsan.supp` one at a time and run the QML
   tests repeatedly (`ctest -R QML --repeat until-fail:20`), because the CI
   failure only showed on some interleavings. The target is an empty file.
   Anything that still reports is either a race of ours, fixed under the
   findings policy in `ci-sanitizers.md`, or a real Qt problem, which gets a
   suppression narrowed to the full Qt stack and a note here.
6. **CI.** Drop `thread` from the matrix in the `sanitizers` job. With one
   entry left, remove the matrix and the `matrix.sanitizer == ...`
   expressions, and the `TSAN_OPTIONS` line.
7. **Documentation.** `AGENTS.md`: the sanitizer paragraph under Testing
   (CI runs ASan and UBSan only; TSan runs through the script, QML
   included), replace the `build-tsan` commands with the script, and drop
   the note about expecting suppression tuning with QML on. Add a line on
   `--upload` and the cache key.
8. Commit after each step. Open the PR once the script is green locally with
   QML on, checking `gh auth status` first.

## Risks

- **Qt build time.** Unknown until measured; an hour or so on 8 cores is
  plausible for an instrumented build of four repositories. Only the first
  maintainer build pays it if the release asset works.
- **`-no-opengl` or no xcb may not configure** with Qt Quick, or the
  offscreen platform may warn about fonts. The QML UI tests fail on any
  warning, so this shows up immediately. Fallback is in the table above.
- **Instrumented host tools.** Besides the exit code handled in step 1,
  older TSan runtimes fail at startup under high-entropy ASLR ("unexpected
  memory mapping"). Clang 18.1 re-executes itself to work around it; if it
  still appears, the script reports the `sysctl vm.mmap_rnd_bits=28` fix
  and does not apply it.
- **Compiler mismatch.** The app must be built with the same Clang major
  version as the Qt prefix. The key covers Clang's version, so a different
  compiler gets a different prefix, not a broken link.
- **Portability of the prebuilt prefix.** Built on Ubuntu 24.04; an older
  glibc cannot load it. Record the build host in a file inside the archive
  and fall back to building when the binaries fail a smoke run
  (`<prefix>/bin/qmake -query`).
- **Losing TSan in CI.** Engine and UCI races would no longer be caught on
  every PR, only when someone runs the script. See the first open question.

## Open questions

1. **Remove the CI leg outright, or replace it?** The engine-only TSan leg
   is green and cheap today. Once a prebuilt prefix is on the release, a CI
   leg could download it and run TSan with QML at about the cost of the
   current leg. Suggested order: land the script and remove the leg as
   asked here, then consider the download-based leg as a follow-up once the
   prefix has proven stable.
2. **Conan**: confirm the recommendation above, or say which remote we
   would use.
3. **Release tag for the binaries**: a single moving `tsan-deps` release,
   as planned, or one release per Qt version.

## Out of scope

- An instrumented libc++ or libstdc++, glib, fontconfig or freetype.
- MemorySanitizer, which does need every dependency instrumented.
- An ASan-instrumented Qt. The ASan leg works with the prebuilt Qt.
- macOS and Windows. The script is Linux only, like `install-ci-qt.sh`.

## Verification

- `./scripts/build-tsan.sh` from a clean cache builds Qt, builds the app and
  passes every fast test with the QML UI on. Record the Qt build time, the
  prefix size and the archive size here.
- A second run reuses the prefix and rebuilds nothing of Qt. With the cache
  directory removed and the asset uploaded, a run downloads instead of
  building.
- `tsan.supp` is empty, or each remaining entry is justified here, and
  `ctest -R QML --repeat until-fail:20` passes.
- Prove the build can fail: a temporary unsynchronised write shared between
  `GameModel` and the `ChessEngine` thread in a scratch test, seen to be
  reported with both stacks symbolised through Qt frames, then removed.
- The `sanitizers` job is green on the PR with only the ASan leg.
