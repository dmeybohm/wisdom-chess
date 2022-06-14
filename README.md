<h1 align="center">Wisdom Chess</h1>

Wisdom Chess is a simple multiplatform chess engine written in C++.

Currently supported are Windows, macOS, and Android. 

An experimental Web Assembly port also exists <a href="https://wisdom-chess.netlify.app/" target="_blank">here</a>.
*NOTE* it does have some problems, such as not loading on Android Chrome 
and sometimes the interface disappears. I'm thinking of porting this
to [qmlweb](https://github.com/qmlweb/qmlweb) to fix those problems.

## Building

Use [CMake](https://cmake.org/) in order to build. Optionally, you can use the [Conan package manager](https://conan.io/)
to install some supplemental libraries for running tests or analysis.
If you want to build the graphical interface, you also need to specify
the location of the [Qt](https://www.qt.io) libaries or provide them so that CMake can find them.
The program has been tested with Qt 6.2.4. 

```sh
mkdir build
cd build
# .. optionally install packages:
conan install .. -pr:b=default # specify build profile
cmake -DQTDIR=C:\path\to\Qt\6.2.4 ..
cmake --build . -j 8
```

## Building on Android

You can build on Android using Qt Creator. You just
need to setup your Kit to point to Android and install the appropriate
libraries for Qt/QML there.

## Running

There are two different interfaces generated: a command line interactive
interface and a Qt interface. If you CMake doesn't find Qt, or you
don't provide the `QTDIR` variable to it, then only the command line
interface will be built. It's located in an executable called `chess`.
The Qt interface is in an application titled `appWisdomChessQtQml.exe` or
`appWisdomQtQml.app`.


### Copyright Info

Images copyright Colin M.L. Burnett and used under creative commons license.
https://creativecommons.org/licenses/by-sa/3.0/

Some icons are from https://boxicons.com/ and used by Creative Commons 4.0 
license.

The chess program itself is released under the MIT license.
