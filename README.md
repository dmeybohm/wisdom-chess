<h1 align="center">Wisdom Chess</h1>

<p align="center">
    <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/ui/qml/images/wisdom-chess-animate.gif" />
</p>

----

Wisdom Chess is a simple multiplatform chess engine written in C++ with a React web front-end and a Qt
mobile and desktop front-end.

View the web version at https://wisdom-chess.netlify.app

## Building desktop version

For the UI, `wisdom-chess` uses [Qt](https://www.qt.io/), specifically Qt version 6.

You can either use [CMake](https://cmake.org/) or Qt Creator (provided by Qt) in order 
to build. You will need to some supplemental libraries, for which you can use the 
[Conan package manager](https://conan.io/) ([vcpkg](https://vcpkg.io/) might work also, 
but hasn't been tested.

Here's an example set of build commands:

```sh
mkdir build
cd build
# Install packages needed to build
conan install .. -pr:b=default # specify build profile
cmake -DQTDIR=C:\path\to\Qt\6.5.0 ..
cmake --build . -j 8
```

For Qt Creator on desktop, you should just have to setup the appropriate "kit" 
and then click "Build." See below for notes on building for Android, and see
[this document](wasm/README.md) for notes on building the web assembly version.

## Building on Android

You can build on Android using Qt Creator. 

You need to set up your Kit to point to Android and install the appropriate
libraries for Qt/QML there. See [this "Getting Started" document](https://doc.qt.io/qt-6/android-getting-started.html) 
for how to use Qt on Android.

<p align="center">
    <img
    src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/ui/qml/images/wisdom-chess-android.png" />
</p>

## Running

<p align="center">
    <img
    src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/ui/qml/images/windows-wisdom-chess.png" />
</p>

## Running Tests

If you want to run the tests, there are two binaries produced called
`fast_tests` and `slow_tests`. You have to pass some flags to CMake 
in order to enable those :

```sh
cmake -DWISDOM_CHESS_FAST_TESTS=On -DWISDOM_CHESS_SLOW_TESTS=On ..
cmake --build . -j 8
```

Make sure to run the `slow_tests` on optimized code, or
you may have to wait a long time.

### Copyright Info

Images copyright Colin M.L. Burnett and used under creative commons license.
https://creativecommons.org/licenses/by-sa/3.0/

Some icons are from https://boxicons.com/ and used by Creative Commons 4.0 
license.

The chess program itself is released under the MIT license.
