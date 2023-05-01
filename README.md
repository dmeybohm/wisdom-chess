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
to build. Optionally, you can use the [Conan package manager](https://conan.io/) 
to install some supplemental libraries for running tests or analysis.

If you want to build the graphical interface, you also need to specify
the location of the Qt libaries with the `QTDIR` variable,
or put them on your system path so that CMake can find them with the
`find_package()` command. You need to provide the particular version you
want to link against. For example on my Windows laptop, Qt was installed
to `C:\Qt` and I pass `QTDIR=C:\Qt\6.2.4`.

If you don't already have Qt installed, the easiest way to get this 
working would be to download and install Qt from the [Qt website](https://www.qt.io/), 
but installing Qt with your system's package manager is also an option. The program has 
been tested with Qt 6.2.4. 

```sh
mkdir build
cd build
# .. optionally install packages:
conan install .. -pr:b=default # specify build profile
cmake -DQTDIR=C:\path\to\Qt\6.2.4 ..
cmake --build . -j 8
```

For Qt Creator on desktop, you should just have to setup the appropriate "kit" 
and then click "Build." See below for notes on building for Android, and see
[this document](wasm/README.md) for notes on building the web assembly version.

## Building on Android

You can build on Android using Qt Creator. You may have to specify
`QT_CREATOR_SKIP_CONAN_SETUP=On` in the project's CMake build settings 
(see [Issue #11](https://github.com/dmeybohm/wisdom-chess/issues/11)).

You need to setup your Kit to point to Android and install the appropriate
libraries for Qt/QML there. See [this "Getting Started" document](https://doc.qt.io/qt-6/android-getting-started.html) for how to use Qt on Android.

<p align="center">
    <img
    src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/ui/qml/images/wisdom-chess-android.png" />
</p>

## Running

There are two different interfaces generated: a command line interactive
interface and a Qt interface. If your CMake doesn't find Qt, or you
don't provide the `QTDIR` variable to it, then only the command line
interface will be built. It's located in an executable called `chess`.
The Qt interface is in an application titled `WisdomChessQml.exe`
`WisdomQtQml.app`. The command-line interface is called `wisdom-chess-console`
for debugging, and doesn't have a help command yet.

You can configure things by clicking on the hamburger menu in the
upper right (or on the "Wisdom Chess" text in the menu bar on the
Web Assembly version). In that menu you can configure which colors
the chess engine will play, the max number of moves the engine
searches, or the max time to use to search each move.

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
