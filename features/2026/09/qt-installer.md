# Binary installers with the Qt Installer Framework

## Problem

There is no way to ship the desktop Qt QML app to end users. The tree has
no packaging at all: no CPack, no `windeployqt`/`macdeployqt`/Linux deploy
step. The existing `install(TARGETS WisdomChessQml ...)` copies a bare
binary with its Qt RPATH stripped, so an installed program cannot even
find Qt. The macOS rule installs the bundle to an absolute `/Applications`,
which breaks any staged packaging.

Smaller gaps in the same area: the only version number (`0.1`) lives in
the QML sub-project while the top-level `project()` has none (CPack would
silently package as 0.1.1); `Chess.ico` has only 16/32 px entries and
`wisdom-chess.icns` a single 64 px entry; the Windows `.rc` has no
`VERSIONINFO`; and the `.rc` was referenced as `Chess-icons.rc` while the
file was `chess-icons.rc` (only worked on case-insensitive filesystems).

## Decisions

- **Qt Installer Framework** via CPack's built-in IFW generator, driven by
  the `install()` rules plus Qt's `qt_generate_deploy_qml_app_script`,
  rather than hand-written `config.xml`/`package.xml` and manual
  `binarycreator` calls. CPack IFW already emits `.run`/`.exe`/`.dmg`, and
  the Qt deploy script honours CPack's per-component staging prefix.
- **Local builds only.** No CI or release workflow changes.
- **Unsigned installers**; the README documents the SmartScreen and
  Gatekeeper workarounds.
- **Binary stays `WisdomChessQml`**; the user-facing name is "Wisdom
  Chess" everywhere (installer title, Start Menu, shortcuts, `.desktop`,
  macOS `CFBundleName`).
- **Linux installs system-wide to `/opt/WisdomChess`** (installer
  elevates), `.desktop` entry in `/usr/share/applications`; `--root` under
  `$HOME` gets a per-user entry instead.

## Verified against the local Qt 6.11.2 / CMake 3.28 module sources

- `qt_generate_deploy_qml_app_script` must be called from the directory
  that called `find_package(Qt6)` (`QT_DEPLOY_SUPPORT` is directory
  scoped), so it lives in the qml `CMakeLists.txt`, not a top-level module.
- The Linux deploy step rewrites plugin RPATHs but never touches the
  executable, so `INSTALL_RPATH "$ORIGIN/../lib"` is set explicitly.
- The plugin-selection keywords (`EXCLUDE_PLUGIN_TYPES` etc.) only exist
  since Qt 6.10 and unknown keywords are fatal, so they are guarded on
  `Qt6_VERSION`; CI's Qt 6.9 keeps working (the option is OFF there anyway).
- Windows: windeployqt copies the CRT by default but may fall back to loose
  DLLs, so `vc_redist.x64.exe` is bundled explicitly and run by the
  installer, with `NO_COMPILER_RUNTIME` passed to the deploy script.
- CMake 3.28's `CPackIFW.cmake` only auto-searches
  `Tools/QtInstallerFramework/<ver>` for versions up to 4.5, so
  `cmake/Installer.cmake` globs for the newest one under `~/Qt`/`C:/Qt`.
- `configure_file` blanks undefined `@VAR@` tokens, so the component script
  (which uses `@TargetDir@` etc.) is kept static.

## Plan

1. **Hoist the version** to `project(WisdomChess VERSION 0.1.0 ...)` in the
   top-level `CMakeLists.txt`; the qml sub-project uses
   `${WisdomChess_VERSION}`, gains `MACOSX_BUNDLE_BUNDLE_NAME "Wisdom Chess"`
   and a `WISDOM_CHESS_VERSION` define; `main.cpp` sets the application
   name/version/organisation.
2. **Windows resources**: rename to `wisdom-chess-icon.rc` /
   `wisdom-chess.ico` (fixes the case mismatch) and add a `configure_file`d
   `wisdom-chess-version.rc.in` with a `VERSIONINFO` block.
3. **Fix the install rule** in `src/wisdom-chess/ui/qml/CMakeLists.txt`:
   `BUNDLE DESTINATION .`, `COMPONENT Application`, `INSTALL_RPATH` on Linux.
4. **Deploy script** (same file, gated on `WISDOM_CHESS_INSTALLER`):
   `qt_generate_deploy_qml_app_script` + `install(SCRIPT ...)`, the Linux
   menu icon, and `vc_redist.x64.exe` on Windows.
5. **Top-level option** `WISDOM_CHESS_INSTALLER` (default OFF), the MSVC /
   vc_redist check, and `include(cmake/Installer.cmake)` after
   `add_subdirectory(src)`.
6. **`cmake/Installer.cmake`**: QtIFW lookup, CPack identity, IFW look
   and per-platform target dirs / run-program, `include(CPackIFW)` +
   `include(CPack)`, the `Application` component with the MIT license and
   the component script, and the `installer` custom target.
7. **`installer/installscript.qs`**: Start Menu + desktop shortcuts and
   vc_redist on Windows; `.desktop` entry on Linux (system-wide via the
   elevated helper, per-user when the target is under `$HOME`).
8. **Icons**: `scripts/generate-icons.py` (Pillow only) regenerates the
   `.ico` (16..256), `.icns` (16..1024) and the installer PNGs from the
   512 px app mark.
9. **Docs**: README "Installers" section with per-platform commands and
   the unsigned-installer notes; CLAUDE.md option row and build snippet.

## CI integration

`.github/workflows/installers.yml` builds the three installers on
GitHub-hosted runners. Decisions:

- **Triggers**: `v*` tags create a GitHub Release with the installers
  attached (`softprops/action-gh-release@v3`, `contents: write` only on the
  release job); `workflow_dispatch` and pull requests that touch
  installer-related paths upload artifacts only, so the PR that adds the
  workflow validates itself. A tag must match the top-level project
  `VERSION`.
- **Runners**: `ubuntu-latest` (24.04, GCC 13), `windows-latest`
  (MSVC via `ilammy/msvc-dev-cmd`, which also exports `VCToolsRedistDir`
  for `vc_redist.x64.exe`), `macos-latest` (macOS 26, arm64).
  `ubuntu-22.04` was considered for an older glibc baseline but GitHub
  starts brownouts on 2026-09-17 and retires it in April 2027; an
  older-baseline build via a container stays a follow-up.
- **Qt / QtIFW**: `jurplel/install-qt-action@v4` with Qt `6.9.*` and
  `tools: 'tools_ifw,qt.tools.ifw.47'`. The variant is pinned because the
  action passes `tools` straight to `aqt install-tool`, which installs
  every variant when none is given; 4.7 is the only variant published on
  the mirror for all three hosts. The action exports `IQTA_TOOLS`, which
  `cmake/Installer.cmake` now searches first.
- **Smoke test** on all platforms: `scripts/smoke-test-installer.sh`
  installs with `--platform minimal --root <tmp> --accept-licenses
  --confirm-command install`, checks the executable, `qt.conf`, the
  platform plugin, the QtQuick.Controls module (and `ldd` / the `.desktop`
  entry on Linux, `CFBundleName` on macOS, `vc_redist.x64.exe` on Windows),
  then purges with the maintenance tool. On macOS the `.dmg` is mounted
  with `hdiutil` first.

## Implementation Progress

### Session #1

- Implemented all nine plan steps on the `qt-installer` branch (developed
  on Linux, Ubuntu 24.04, Qt 6.11.2, QtIFW 4.11, CMake 3.28.3).
- Icons: no SVG rasterizer or ImageMagick was available locally, so
  `scripts/generate-icons.py` uses Pillow only and takes the 512 px
  `wasm/android-chrome-512x512.png` app mark as its source. The `.ico`
  now has 16/24/32/48/64/128/256 entries, the `.icns` 16 to 1024 (1x/2x).
- Verified on Linux:
  - `cmake --install --component Application` stages `bin/WisdomChessQml`
    with RUNPATH `$ORIGIN/../lib`, `bin/qt.conf` (`Prefix = ..`), Qt and
    ICU libraries in `lib/`, `plugins/platforms/libqxcb.so` (RUNPATH
    `$ORIGIN/../../lib`), and the QtQml/QtQuick/Controls/Layouts QML
    modules; `ldd` resolves everything from the staged tree; the console
    binary is excluded; qmltooling and the Wayland plugin types are not
    shipped. The staged tree is about 115 MB.
  - The staged app runs, and `/proc/<pid>/maps` shows only plugins and
    QML modules from the staged tree, nothing from the system Qt.
  - `cmake --build build-qt --target installer` produces
    `wisdom-chess-0.1.0-Linux-x86_64.run` (about 74 MB) with the expected
    `config.xml` (TargetDir `/opt/WisdomChess`, RunProgram, Modern style)
    and `package.xml` (MIT license, `installscript.qs`, forced install).
  - Unattended per-user install works:
    `./wisdom-chess-0.1.0-Linux-x86_64.run --platform minimal --root ~/wc-test
    --accept-licenses --confirm-command install` installs the tree, writes
    `~/.local/share/applications/wisdom-chess.desktop`, and the installed
    app launches. `WisdomChessMaintenanceTool --confirm-command purge`
    removes both the directory and the menu entry.
  - The first run showed `CreateDesktopEntry` prepends `[Desktop Entry]`
    itself, so the header was dropped from the script's content string.
  - Symbol floors of the built binary: `GLIBC_2.34`, `GLIBCXX_3.4.32`
    (GCC 13), noted in the README.
  - The default configuration (option OFF) still configures, builds and
    passes all 88 fast tests; the style linter passes on `main.cpp`.
- Not verified here (needs the respective machines): the Windows build
  (`VERSIONINFO` resource, `vc_redist` bundling and the `Execute`
  operation, shortcuts) and the macOS build (`macdeployqt` via the deploy
  script, `.dmg` output, `CFBundleName`). The elevated `/opt/WisdomChess`
  install path on Linux was also not run because it needs a password
  prompt; only the `--root` path under `$HOME` was exercised.

### Session #2

- Added `.github/workflows/installers.yml`, `scripts/smoke-test-installer.sh`
  and the `IQTA_TOOLS` lookup in `cmake/Installer.cmake`; documented the
  CI flow. The smoke-test script passed locally against the Linux
  installer before pushing.
- First CI runs on the PR: Linux passed; macOS failed in the smoke test;
  Windows failed in `cpack`; and the regular CMake workflow's macOS
  "Install (CMake)" step failed.
  - **macOS**: in `install(TARGETS)`, `COMPONENT` only applies to the
    artifact group it follows, so `COMPONENT Application` was attached to
    `RUNTIME` only and the `BUNDLE` fell into "Unspecified", which
    `CPACK_COMPONENTS_ALL` excludes. The bundle was never staged;
    macdeployqt then reported "Could not find bundle binary" against the
    directory the QML deploy step had created. Fixed by repeating
    `COMPONENT Application` after `BUNDLE DESTINATION .`.
  - **Windows**: `IQTA_TOOLS` (and `VCToolsRedistDir`) are backslash paths;
    written into `CPackConfig.cmake` they became invalid escapes
    (`Invalid character escape '\a'`). Fixed with `file(TO_CMAKE_PATH)`
    on both.
  - **CMake workflow**: the install smoke test used the default prefix
    `/usr/local`, whose top level is not writable on the macOS runner now
    that the bundle installs relative to the prefix. It now installs into
    `<workspace>/install-test`.
  - Added a "Show the staged package layout" step (`if: always()`) and a
    tree dump in the smoke script on failure so the next failure is
    diagnosable from the log.
- Second run (`f688e58`): all three jobs pass, as does the CMake
  workflow. Results:
  - Linux: `wisdom-chess-0.1.0-Linux-x86_64.run`, headless install, `ldd`
    clean, `.desktop` entry, purge OK.
  - Windows: `wisdom-chess-0.1.0-Windows-AMD64.exe` (76 MB), installed
    tree 146 MB; `vc_redist.x64.exe` was bundled and the elevated
    `Execute` ran; shortcuts, `qwindows.dll`, QtQuick.Controls present;
    purge OK.
  - macOS: `wisdom-chess-0.1.0-Darwin-arm64.dmg` (40 MB), installed
    bundle 154 MB with `QtCore.framework`, `libqcocoa.dylib`,
    QtQuick.Controls, `CFBundleName` "Wisdom Chess"; purge via the
    maintenance tool `.app` OK.
- The `release` job is skipped on PRs; it runs on the first `v0.1.0` tag
  pushed to `main` after merge.

## Out of scope / follow-ups

- Code signing and notarization (secrets-driven `signtool` / `codesign` +
  `notarytool` steps in the workflow).
- macOS universal (arm64 + x86_64) binary: Qt's `clang_64` is universal,
  so `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"` should work once PCH is
  checked for multi-arch builds.
- Windows arm64 and Linux arm64 installers.
- Wayland: opt in via `INCLUDE_PLUGINS qwayland` once the deploy keywords
  are available in the CI Qt version.
- Optional "create desktop shortcut" wizard page (`USER_INTERFACES`).
- A drag-and-drop DMG (CPack `DragNDrop`) as the more idiomatic macOS
  delivery.
- Building the Linux installer in an older-glibc container
  (`container: ubuntu:22.04` on `ubuntu-latest`) for wider compatibility.
- An attribution page in the installer for the CC BY-SA piece images.
- Show the version in the QML About dialog now that
  `WISDOM_CHESS_VERSION` exists.
