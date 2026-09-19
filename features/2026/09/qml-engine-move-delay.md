# QML engine move delay

## Motivation

The bug list
([bug-list-and-engine-warnings.md](bug-list-and-engine-warnings.md))
has this open item under "Frontends":

> `QThread::usleep (200000)` in the engine slot to wait for animation
> (`ui/qml/main/chess_engine.cpp:126`).

The line is now at `ui/qml/main/chess_engine.cpp:131`, in the slot that
starts a search once it is the engine's turn:

```cpp
    // Wait for animation to finish
    QThread::usleep (200000); // 200 ms

    auto who = game_state->getCurrentTurn();
    ...
    auto optionalMove = game_state->findBestMove (my_logger);
```

It blocks the engine thread for 200 ms before every search, so that the
player's piece has finished moving on the board before the engine's reply
starts to move. The thread cannot handle anything else while it sleeps,
and the 200 ms is a copy of `animationDelay` in `DesktopRoot.qml` and
`MobileRoot.qml`, not tied to it. The `QML: dialogs` test
`theEngineAnswersAMove` spends most of its 0.4 seconds in this sleep.

The plan for removing it comes after the preliminary action item below.

## Preliminary action item: a script to install the Qt that CI uses

The `qml-tests` branch found failures that only happened on CI, because
CI builds with a different Qt from the one used locally. The fastest way
to reproduce them was to install CI's Qt locally (see Session #9 of
[qml-tests.md](qml-tests.md)). That was done by hand in a scratch
directory. This change starts by turning it into a script, so the next
CI-only failure, including any in the UI tests this change will touch,
can be reproduced in minutes.

### What CI installs

- `.github/workflows/cmake.yml` uses `jurplel/install-qt-action@v4` with
  `version: '6.9.*'`, `cache: true` and no `modules:` list, so it gets the
  newest 6.9 release with the default modules.
- The newest 6.9 is 6.9.3 (checked on 2026-09-19 with
  `aqt list-qt linux desktop --spec 6.9 --latest-version`). CI's test logs
  confirm it: "Using QtTest library 6.9.3".
- `.github/workflows/installers.yml` also asks for `6.9.*`, and
  `.github/workflows/web.yml` pins `6.9.3` exactly. The script follows
  `cmake.yml`, whose jobs run the tests.
- The local default is `~/Qt/6.11.2/gcc_64`.

### What worked by hand

All without root, on Linux:

```bash
python3 -m venv <dir>/aqt-venv
<dir>/aqt-venv/bin/pip install aqtinstall          # got aqtinstall 3.3.0
<dir>/aqt-venv/bin/aqt list-qt linux desktop --arch 6.9.3
                                                   # -> linux_gcc_64
<dir>/aqt-venv/bin/aqt install-qt linux desktop 6.9.3 linux_gcc_64 \
    --outputdir <dir>/Qt
```

- The download and unpacking took about 20 seconds. The result is 1.7 GB
  at `<dir>/Qt/6.9.3/gcc_64`.
- The default module set includes everything the build and tests need:
  Qt6Quick, Qt6QuickControls2, Qt6Svg, Qt6Test and Qt6QuickTest.
- Building against it:

  ```bash
  cmake -S . -B build-qt69 -DCMAKE_BUILD_TYPE=Debug \
      -DWISDOM_CHESS_QT_DIR=<dir>/Qt/6.9.3/gcc_64 \
      -DWISDOM_CHESS_QML_UI=ON -DWISDOM_CHESS_SLOW_TESTS=OFF
  cmake --build build-qt69 -j8
  ctest --test-dir build-qt69 -R "^QML:" -j 6
  ```

  `ctest` supplies the UI tests' environment (`QT_QPA_PLATFORM=offscreen`,
  `QT_QUICK_BACKEND=software`). A test executable run by hand needs both
  set.
- Debug matched CI best: the Linux Debug job was the one that failed
  every time.

### What the script should do

A new `scripts/install-ci-qt.sh`:

1. Read the version from `.github/workflows/cmake.yml` (`6.9.*`) and let
   `aqt` pick the newest matching release, as CI does, so the script
   follows CI when the workflow changes. Accept an explicit version, such
   as `6.9.3`, as an override.
2. Install into a directory outside the source tree and outside `/tmp`.
   The default is `${XDG_CACHE_HOME:-$HOME/.cache}/wisdom-chess/qt`; an
   argument or environment variable can change it. `/tmp` is a 7.7 GB
   in-memory filesystem here and was 88% full after the manual install.
3. Keep `aqtinstall` in a virtual environment next to the Qt install. Do
   not install it globally, and do not use `sudo`. Run `aqt` from that
   directory: it writes `aqtinstall.log` to the current directory, which
   would otherwise land in the source tree.
4. Do nothing if that version is already installed. Print the
   `-DWISDOM_CHESS_QT_DIR=...` path to use either way.
5. Support Linux (`linux_gcc_64`). For macOS, `aqt` also offers
   `mac desktop ... clang_64`, but that is untested; the script should
   stop with a clear message on anything else.
6. Check for `python3` with the `venv` module and say what is missing if
   it is not there, rather than failing halfway.

Then point `AGENTS.md` at the script where it now spells out the `aqt`
commands, and delete the manual install left in the scratch directory.

### Verification of the script

- A run from nothing installs the newest 6.9 and prints the path.
- A second run is a no-op.
- A Debug build against that path passes all fast tests, including the
  six `QML:` tests.
- `shellcheck`, if available, reports nothing.

## Implementation Progress
