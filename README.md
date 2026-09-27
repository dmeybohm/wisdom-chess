<h1 align="center">Wisdom Chess</h1>

<p align="center">
  <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/wisdom-chess-animate.gif" />
</p>

----

Wisdom Chess is a chess engine with apps to play it on the web, on
Windows, macOS and Linux desktops, on Android and in a terminal. It is
free software under the MIT License.

🌐 **[Play online at wisdom-chess.netlify.app](https://wisdom-chess.netlify.app)**

## Download

Installers for Windows (x64), macOS (Apple Silicon) and Linux (x86_64)
are attached to each release on the
[GitHub Releases page](https://github.com/dmeybohm/wisdom-chess/releases).
Each bundles everything the app needs, so nothing else has to be
installed.

The installers are **not code-signed**, so operating systems warn before
running them:

- **Windows**: SmartScreen shows "Windows protected your PC". Click
  *More info*, then *Run anyway*. The installer asks for administrator
  rights to install into `Program Files\Wisdom Chess`, adds Start Menu
  and desktop shortcuts, and installs the MSVC runtime if needed.
- **macOS**: Gatekeeper refuses to open the installer app on the mounted
  disk image. Right-click it and choose *Open*, or allow it under
  *System Settings > Privacy & Security > Open Anyway*. Alternatively
  clear the quarantine flag: `xattr -dr com.apple.quarantine <installer.app>`.
  The app is installed to `/Applications/Wisdom Chess/`.
- **Linux**: make the file executable (`chmod +x wisdom-chess-*.run`) and
  run it. It asks for your password to install into `/opt/WisdomChess`
  and adds a *Wisdom Chess* entry to the application menu. Use
  `./wisdom-chess-*.run --root ~/WisdomChess` to install into your home
  directory instead. The bundled Qt needs the usual X11/xcb libraries
  from your distribution; on Debian/Ubuntu that is
  `libxcb-cursor0 libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0
  libxcb-keysyms1 libxcb-render-util0 libxcb-shape0 libgl1 libegl1
  libfontconfig1`. The binary links against the build machine's C and C++
  runtimes, so it needs a distribution at least as new as the one it was
  built on: the release installers are built on Ubuntu 24.04 (glibc 2.39,
  GCC 13).

Uninstall with the *WisdomChessMaintenanceTool* placed next to the app.

## Features

- Play against the engine as either colour, watch it play itself, or
  play another person on the same device.
- Set how deep and how long the engine thinks, from an instant reply to
  a thirty-second search.
- Move pieces by clicking or by dragging them; choose the piece a pawn
  promotes to.
- The full rules: castling, en passant, check and checkmate, stalemate,
  and draws by threefold repetition, the fifty-move rule, insufficient
  material and their automatic forms. A draw you may claim is offered
  to you rather than imposed.
- Flip the board, pause the engine, and start a new game at any time.
- The same engine plays in the browser, on the desktop, on Android and
  in a terminal, and as a UCI engine in any chess GUI.

## Screenshots

<p align="center">
    <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/windows-wisdom-chess.png" />
</p>

<p align="center">
    <img src="https://raw.githubusercontent.com/dmeybohm/wisdom-chess/main/src/wisdom-chess/ui/qml/images/wisdom-chess-android.png" />
</p>

## Building from source

Every frontend, the CMake options and the test suites are described in
[docs/building.md](docs/building.md). The console version needs only a
C++20 compiler and CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
./build/src/wisdom-chess/ui/console/wisdom-chess-console
```

## Contributing

[docs/building.md](docs/building.md) covers building and testing every
part of the project, and `AGENTS.md` records the code conventions and
the things to know before changing each frontend.

## License

Copyright © Dave Meybohm

The chess engine and applications are released under the MIT License.

### Third-Party Assets

- Chess piece images: Copyright Colin M.L. Burnett, used under [Creative Commons BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/)
- UI icons: From [Boxicons](https://boxicons.com/), used under [Creative Commons 4.0](https://creativecommons.org/licenses/by/4.0/)
