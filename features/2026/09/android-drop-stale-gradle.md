# Drop the stale Android gradle files

## Motivation

`src/wisdom-chess/ui/qml/android` is the `QT_ANDROID_PACKAGE_SOURCE_DIR`
for the QML app. `androiddeployqt` starts from the templates shipped with
the Qt version in use and copies this directory on top, so every file
committed here shadows Qt's copy on every future Qt upgrade.

Qt Creator's "Create Templates" once generated the whole set into the
repo. The gradle files froze at the Qt 6.4 era (Android Gradle Plugin
7.2.1, Gradle 7.4.2, Java 8 targets), while Qt 6.11.2 ships AGP 9.0.0,
Gradle 9.3.1 and Java 17 targets, plus a plugin type, namespace, Kotlin
source dirs and an androidx dependency its Java code needs. A diff of the
repo's `build.gradle` against the 6.11.2 template showed only Qt-side
changes and no project customization. `gradle.properties` held only JVM
heap settings and the wrapper was a verbatim copy of Qt's.

## Plan

Keep only what the project actually customizes, `AndroidManifest.xml` and
`res/`, and let Qt supply `build.gradle`, `gradle.properties` and the
gradle wrapper for whichever Qt version builds the package. If custom
Gradle configuration is ever needed (extra dependencies, signing), copy
`build.gradle` fresh from the current Qt template, make the minimal edit,
and re-diff it on each Qt upgrade.

The manifest declared Qt's placeholder package `org.qtproject.example`.
Use `com.daveme.wisdomchess`, the identifier the macOS bundle and the
installer already use. Android package segments must be Java identifiers,
so a hyphenated `wisdom-chess` is not possible.

## Implementation Progress

### Session #1

Removed `build.gradle`, `gradle.properties`, `gradlew`, `gradlew.bat` and
`gradle/` from the android package directory. Not yet built for Android;
the SDK and NDK were not installed on the development machine at the time.

### Session #2

Set the manifest package to `com.daveme.wisdomchess`.

Built an x86_64 APK from the command line with Qt 6.11.2, NDK
27.2.12479018 and JDK 17, using Qt's own gradle template (AGP 9.0.0,
Gradle 9.3.1). `aapt2 dump badging` reports the new package name. Not yet
installed or run on a device or emulator.

```bash
export JAVA_HOME=/usr/lib/jvm/java-17-amazon-corretto
~/Qt/6.11.2/android_x86_64/bin/qt-cmake -S . -B build-android-x86_64 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DWISDOM_CHESS_QML_UI=ON \
  -DWISDOM_CHESS_FAST_TESTS=Off \
  -DQT_HOST_PATH=$HOME/Qt/6.11.2/gcc_64 \
  -DANDROID_SDK_ROOT=$HOME/Android/Sdk \
  -DANDROID_NDK_ROOT=$HOME/Android/Sdk/ndk/27.2.12479018
cmake --build build-android-x86_64 -j8
```

Found along the way:

- The QML app's `install(TARGETS)` had no `LIBRARY DESTINATION`, which
  fails to configure on Android, where the app is a module library.
  Fixed.
- With `WISDOM_CHESS_FAST_TESTS` on (the default), the build fails on
  Android: doctest's test discovery runs the test executables on the
  host. Fixed in session #4.
- Gradle warns that the manifest's `package` attribute is ignored in
  favor of the namespace. Harmless: Qt reads the attribute to set that
  namespace, and Qt's own template manifest carries it too.
- The app label was the target name, `WisdomChessQml`. Fixed by writing
  "Wisdom Chess" into the manifest's two `android:label` attributes in
  place of Qt's placeholder, which works on every Qt version the project
  supports. The rebuilt APK reports the new label.

### Session #3

An Android Debug configure reported "Type annotations are not permitted for
the return value of JavaScript functions" for the anonymous callbacks passed
to `Qt.binding()` in `Piece.qml`. Moved their calculations into named QML
methods with `real` return annotations and passed those methods directly to
`Qt.binding()`. The annotations remain on the calculations while the callbacks
use syntax accepted by Qt's JavaScript parser.

Verified with Qt 6.11.2: `all_qmllint` passes, an Android x86_64 Debug
configure and QML code generation for `Piece.qml` pass, and the desktop
`QML: application` test passes (including drag, move, and castling cases).

### Session #4

**Tests on the device.** The test programs are plain executables that need
only the NDK's `libc++_shared.so`, so they run from `adb shell` without
being packaged. `cmake/AndroidTests.cmake` configures
`cmake/android-run-test.sh.in` into the build directory and sets it as
`CMAKE_CROSSCOMPILING_EMULATOR`, which doctest's discovery and `ctest`
both put in front of a test program.

Two ways of registering the doctest suites were considered: keep
discovery, which lists every test case but runs the programs during the
build, or register one test per program, which builds without a device.
Discovery was chosen for the listing, so an Android build with the tests
on needs a device or emulator connected. Without one the wrapper says so
and names the options that turn the tests off.

The wrapper:

- keeps each file on the device in a directory named after its checksum,
  so an unchanged program is not copied again and a rebuilt one replaces
  the old copy;
- copies under a temporary name and renames, because `ctest -j` starts
  several copies of the same program at once;
- gives every `adb` call but the last `/dev/null` for input, or the first
  would swallow the script meant for a command-line test;
- is a single path with no arguments, because the fatal and command-line
  tests pass it through `add_test()`, which splits arguments at `;`.

The fatal tests take the emulator the way the `wasm-doctest-discovery`
branch passes it, with the same edit to `run_fatal_test.cmake`, so the two
branches merge cleanly. All seven cases behave on Android as on Linux. The
command-line tests take it the same way. The saved-game test uses a
relative path on Android, since the device has no build directory, and the
Python script test is left out of an Android build: CMake would have sent
the host's interpreter to the device.

Results on the x86_64 emulator (Android 16, API 36):

| Build | Tests | Result |
|---|---|---|
| Qt 6.11.2, Release, fast and slow | 218 | all passed |
| Qt 6.11.2, Debug, fast, Qt Creator's build directory | 186 | all passed |
| Desktop, Release, fast (unchanged behavior) | 187 | all passed |

Not covered: the QML tests, which need Qt and so an APK and Qt's
`androidtestrunner`; a Windows host, where the wrapper's shell script does
not run; an ARM device.

**CI.** `cmake.yml` gains an `android` job that builds the arm64-v8a
package with the tests off and runs nothing. The same configure and build
commands were run locally against Qt 6.9.3 for Android, the version CI
pins, and produced the APK. The job itself has not run on GitHub yet, so
what the runner provides is still to be confirmed: that the Qt action
installs the desktop Qt beside the Android one, and the `ANDROID_NDK_ROOT`
and `JAVA_HOME_17_X64` variables.
