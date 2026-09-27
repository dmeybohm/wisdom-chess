# Run the QML tests on Android

## Motivation

[android-drop-stale-gradle.md](android-drop-stale-gradle.md) made the
tests of an Android build run on a device, through a wrapper that copies a
test program over with `adb` and runs it from a shell. That covers every
test that is a plain program. The eight `QML: ...` tests are not: they
link Qt, and Qt for Android only works inside an application, which the
system starts from a package with a Java activity. So the QML directory
left its tests out of an Android build, and the user interface, including
the mobile layout that only Android ships, was tested on the desktop alone.

This branch is based on `android-drop-stale-gradle` (PR #294), which it
needs.

## Plan

Qt ships the pieces for this, and its own tests use them:

- `qt_add_executable()` on Android builds a package for any executable,
  tests included, with a `<target>_make_apk` target.
- `androidtestrunner`, from the host Qt, installs a package, starts it
  with the test's arguments, brings back what Qt Test printed and exits
  with the number of failures.

So on Android `wisdom_chess_add_qml_test()` registers the test as a call
to `androidtestrunner` instead of to the test program. What that takes:

1. Each test gets its own package directory, which needs
   `QT_USE_TARGET_ANDROID_BUILD_DIR`: Qt otherwise gives every target of a
   directory the same one.
2. The packages are debug packages whatever the build type
   (`QT_ANDROID_DEPLOYMENT_TYPE`). A release package is unsigned and
   cannot be installed, and the runner reads the results with `run-as`,
   which only a debuggable package allows.
3. A test without a GUI links Qt Gui all the same: the activity loads the
   platform plugin before `main()` runs.
4. The UI tests draw on the device's display. The desktop's
   `QT_QPA_PLATFORM=offscreen` must not reach them; the runner forwards
   the environment, and Qt for Android has no such platform.
5. Whatever the tests then assume about a desktop window has to be found
   and dealt with case by case.

## Implementation Progress

### Session #1

Steps 1 to 4 are done, and the tests build and run on the x86_64 emulator
(Android 16, Qt 6.11.2, Release):

| Test | Result |
|---|---|
| QML: PiecesModel | passed |
| QML: ChessGame | passed |
| QML: ChessEngine | passed |
| QML: settings and types | passed |
| QML: application | passed |
| QML: wasm | passed |
| QML: dialogs | 13 of 25 cases fail: no tool button is found |
| QML: mobile | 1 of 9 cases fails: the menu's right edge is at 392 of 411 |

Costs measured: each test is a package of about 45 MB, a clean build of
everything takes about three minutes, and the eight tests take about 45
seconds to run one after another.
