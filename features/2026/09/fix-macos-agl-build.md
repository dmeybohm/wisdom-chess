# Fix macOS CI build failure (AGL framework removed in macOS 26)

## Problem

The CI job `build (macos-latest, RelWithDebInfo, clang)` fails at the
Build step, while CMake configure succeeds. GitHub's `macos-latest`
runner image is now macOS 26 (Tahoe), whose SDK removed the AGL
framework (the Carbon-era OpenGL glue library).

Qt 6.6's bundled `FindWrapOpenGL.cmake` does `find_library(AGL)` and,
when the framework is not found, falls back to a literal
`-framework AGL` link flag. Configure therefore passes, but linking
`WisdomChessQml` fails with `ld: framework 'AGL' not found`
(exit code 2). See QTBUG-137687.

The repository's own CMake never references OpenGL directly; the flag
arrives transitively via `Qt6::Gui` → `WrapOpenGL::WrapOpenGL`.

## Why not bump the Qt 6.6 patch version

Qt 6.6.3 (March 2024) was the final 6.6 release; the series is EOL.
The AGL fix landed only in:

- 6.5.10 — commercial-license-only release
- 6.8.4 — first open-source release with the fix (6.8 is LTS)
- 6.9.2 and later

So the CI must move to at least the 6.8 line.

## Plan

1. **Bump CI Qt version** in `.github/workflows/cmake.yml` from
   `6.6.*` to `6.8.*` (LTS; aqt resolves to the latest 6.8.x patch,
   which is past 6.8.4). Also remove the dead
   `-DWISDOM_CHESS_QML_UI_REQUIRED=On` cache variable — it does not
   exist anywhere in the CMake tree.
2. **Defensive AGL strip** in `src/wisdom-chess/ui/qml/CMakeLists.txt`:
   after `find_package(Qt6 ...)`, on Apple, if the SDK has no AGL
   framework but `WrapOpenGL::WrapOpenGL` carries a `-framework AGL`
   flag, remove it from the target's `INTERFACE_LINK_LIBRARIES`. This
   lets older Qt versions (including 6.6) build on macOS 26. It is a
   no-op with fixed Qt versions and on other platforms.
3. **Use Qt's default backend (Metal) on macOS**: stop defining
   `USE_OPENGL_GRAPHICS_BACKEND` on Apple so `main.cpp` no longer
   forces the QML scene graph onto OpenGL there. The forcing was a
   flicker workaround for specific Linux hardware; OpenGL is
   deprecated on macOS. Linux and Windows keep the OpenGL workaround.
   (On Linux the default backend is OpenGL anyway, so the define only
   pins the default; on Wayland it runs through EGL and works fine.)
4. **Friendly configure hint on Linux**: when OpenGL development files
   are missing (e.g. `Could NOT find OpenGL (missing:
   OPENGL_opengl_LIBRARY OPENGL_INCLUDE_DIR)`), print a hint to
   install them (`libglvnd-dev` on Debian/Ubuntu, `libglvnd-devel` on
   Fedora). Prebuilt Qt on Linux hard-requires OpenGL at configure
   time, so building without it is not supported — but the failure
   should explain itself.

## Out of scope / follow-ups

- `.github/workflows/web.yml` still installs Qt 6.6 for the
  Emscripten/wasm build on Linux. AGL is macOS-only, and bumping the
  wasm Qt changes the deployed web app, so it is left for a separate
  change.
- Verifying Metal rendering on real macOS hardware (this change was
  developed on Linux).
