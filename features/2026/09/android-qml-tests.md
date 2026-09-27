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

### Session #2

Step 5, the two tests that failed:

- **QML: dialogs** loaded the desktop layout, as it does everywhere. On a
  phone the window is the whole screen and the system's status bar lies
  over its top, where that layout's toolbar, 35 pixels high, has no room
  left for its button. The layout does not ship on Android, so rather than
  test it there, `Application` now loads the layout of the platform the
  test runs on unless told which: `mobile_main.qml` on Android. "QML:
  application" follows, and both now test on the device what ships on it.
- The mobile menu has no Quit item. The dialogs test waits for the menu's
  last item to know it is open, which is "About Wisdom Chess" there, and
  skips the two cases about the quit dialog.
- **QML: mobile** found the menu's right edge at 392 of 411. In the
  application the menu is flush with the edge; the Material style, which
  Android uses, scales a menu up from 90% as it opens, and the test
  measured during that. It now waits with `QTRY_COMPARE`.

Also:

- The project sets `QT_USE_TARGET_ANDROID_BUILD_DIR` itself when the tests
  are on, in the cache, which is the only place Qt reads it from. The
  application's package then builds in `android-build-WisdomChessQml`, as
  it already did under Qt Creator.
- The tests share a `RESOURCE_LOCK`, since each takes over the display,
  so `ctest -j` runs them one at a time beside the other tests.
- Each test's package is part of a build of everything. Qt Creator leaves
  packages out of one, and the runner would run a stale package.

Results on the x86_64 emulator (Android 16), `ctest -j 4`, Release:

| Build | Tests | Result |
|---|---|---|
| Android, Qt 6.11.2 | 232 | all passed |
| Android, Qt 6.9.3 (what CI uses) | 232 | all passed |
| Desktop, Qt 6.11.2 | 233 | all passed |

A clean Android build takes three to four minutes, the tests about one.
`all_qmllint` and the style linter pass.

Not done: CI still only builds the Android package with the tests off, so
nothing here runs there. That needs an emulator in CI. Done in session #3.

### Session #3

PR #294 merged, so this branch merged `main` and is now based on it.
Opened as PR #295.

**An emulator in CI.** The new `android-tests` job in `cmake.yml` lets the
runner's user at KVM, boots an x86_64 emulator with
`reactivecircus/android-emulator-runner` and runs
`scripts/android-tests.sh`, which configures, builds and runs `ctest`. The
`android` job stays: it builds arm64-v8a, which is what a phone runs.

- The build is inside the emulator's step because test discovery runs
  the test programs. It is one script because the action runs each line
  of its `script` as a command of its own, so nothing carries over from
  one line to the next.
- The emulator is API 36 with Google APIs, the level the tests were
  developed against locally. From API 35 the system's bars lie over the
  window, which is what the layout has to cope with on a current phone.
- The slow tests are on, as in the other jobs.

The first run passed: 266 tests, fast and slow, with Qt 6.9.3 and NDK
27.3.13750724. Where its ten and a half minutes went:

| Step | Time |
|---|---|
| Installing Qt, not yet cached | 1.5 minutes |
| Installing and booting the emulator | 1.5 minutes |
| Configuring and building | 5.5 minutes |
| Running the tests | 2 minutes |

That makes it the longest job of the workflow. Caching the emulator's
image would save part of the second row; it is not done.

The script was first run locally against the same Qt, where the same 266
tests passed.
