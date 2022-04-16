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

1. Configure step
```sh
./configure -qt-host-path ${QT_PATH}/${QT_VERSION}/macos/ -platform wasm-emscripten -prefix $PWD/qtbase -feature-thread -feature-regularexpression
```
1. Build step
```.sh
cmake --build . --parallel
```

## Building Wisdom Chess with Web Assembly 

```sh
${QT_PATH}/${QT_VERSION}/Src/qtbase/bin/qt-cmake ..
cmake --build . --parallel
```
