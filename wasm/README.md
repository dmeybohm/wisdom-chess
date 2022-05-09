# How to Build with Web Assembly

## Configuring Emscripten

Follow the instructions according to Qt 6 here:

https://doc.qt.io/qt-6/wasm.html

Note that you need a "host" Qt installed. The following example assumes you're
using macOS, and you're using the source package installed in
`${QT_PATH}/${QT_VERSION}/Src`.

## Build Qt with Threads support

The binary distribution of Qt/Web Assembly doesn't include Threads support,
which this project needs. So it's necessary to build Qt from source
to build the web assembly version of the project.

You have to have a number of the Qt libraries installed in the "host Qt" in
order to cross-compile to web assembly, including "Qt Remote Objects" support,
and I think a few others.

1. Configure step
```sh
./configure -qt-host-path ${QT_PATH}/${QT_VERSION}/macos/ -platform wasm-emscripten -prefix $PWD/qtbase -feature-thread -feature-regularexpression
```
1. Build step
```.sh
cmake --build . --parallel
```

## Building Qt with Threads support on Windows

1. Use command prompt
1. Ensure you have build tools in your path
1. Add mingw64 tools to path
1. Configure step
```cmd
configure -qt-host-path C:\Qt\6.2.4\mingw_64 -platform wasm-emscripten -prefix %cd%\qtbase -feature-thread -feature-regularexpression  -feature-imageformatplugin -cmake-generator "MinGW Makefiles"
```

## Building Wisdom Chess with Web Assembly 

```sh
${QT_PATH}/${QT_VERSION}/Src/qtbase/bin/qt-cmake ..
cmake --build . --parallel
```

## Building Wisdom Chess with Web Assembly on Windows

I think the MinGW Makefiles generator is the only one that works.

```cmd
mkdir cmake-build-wasm
cd cmake-build-wasm
%QT_PATH%/%QT_VERSION%/Src/qtbase/bin/qt-cmake .. -G "MinGW MakeFiles"
cmake --build . --parallel
```
