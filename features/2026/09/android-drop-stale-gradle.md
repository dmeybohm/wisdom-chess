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

Still open: the manifest declares `package="org.qtproject.example"`.
Newer Qt derives the Gradle namespace from the package name it passes in,
so this should become a real package name, and whether Qt 6.11 still
accepts the `package` attribute at all is to be checked on the first
build.

## Implementation Progress

### Session #1

Removed `build.gradle`, `gradle.properties`, `gradlew`, `gradlew.bat` and
`gradle/` from the android package directory. Not yet built for Android;
the SDK and NDK were not installed on the development machine at the time.
