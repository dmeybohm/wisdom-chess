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
  host. Not fixed; the build above turns the tests off.
- Gradle warns that the manifest's `package` attribute is ignored in
  favor of the namespace. Harmless: Qt reads the attribute to set that
  namespace, and Qt's own template manifest carries it too.
- The app label was the target name, `WisdomChessQml`. Fixed by writing
  "Wisdom Chess" into the manifest's two `android:label` attributes in
  place of Qt's placeholder, which works on every Qt version the project
  supports. The rebuilt APK reports the new label.
