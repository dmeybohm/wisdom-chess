# Running the doctest suites under Emscripten

## Motivation

`ctest` in an Emscripten build could not run any doctest suite. The engine
and viewmodel tests were built but never registered, and only the fatal
tests, which are added by hand, ran, failing with "Permission denied".
CI's web job builds without tests, so nothing noticed.

## Causes

Once discovery worked, 30 of 199 tests failed, for four reasons:

- **Discovery.** Emscripten reports stdout as a terminal under node even
  when it is a pipe, so doctest colored `--list-test-cases`. Each color
  code holds an unmatched `[`, CMake does not split a list on `;` inside
  an open bracket, and `doctestAddTests.cmake` wrote the whole listing as
  one malformed `add_test`.
- **Exceptions (20 tests).** Emscripten compiles out catch blocks unless
  told otherwise, so every test that expects a throw died with an
  unhandled rejection. The web app shipped the same way: `FenParser`'s
  catch of `CoordParseError` and the terminate handler's reporting did
  nothing in Release. Debug web builds alone had
  `-sNO_DISABLE_EXCEPTION_CATCHING`.
- **Memory (4 tests).** The transposition table and hash collision tests
  outgrow the default fixed heap.
- **Fatal tests (7).** `run_fatal_test.cmake` ran the `.js` output
  directly instead of through node.

## Decisions

- **No colors under Emscripten**, as `DOCTEST_CONFIG_COLORS_NONE` on the
  `doctest` target, so every suite gets it, including ones added later.
- **Native WebAssembly exceptions for all Emscripten code**, chosen over
  skipping the tests or JavaScript-based catching. Every object must use
  one model, and the engine library is shared with the web app, so this is
  a production change: catch blocks now work there too. Measured locally:
  the web `.wasm` grows from 248 KB to 278 KB; perft and the slow suite
  (62.2 s against 63.9 s) show no real change. It needs Chrome 95,
  Firefox 100 or Safari 15.2. The Debug-only flag is gone, as the two
  models cannot be combined.
- **The QML WebAssembly build keeps Qt's model.** Qt's prebuilt
  WebAssembly libraries are built without `wasm-exceptions`, so the flag
  is not set when `WISDOM_CHESS_QML_UI=ON`, which `build-qml-wasm.sh`
  passes. An Emscripten configure that finds Qt with the option left at
  `AUTO` would mix the two, so it stops with an error naming the fix.
- **Three fatal cases are skipped under Emscripten.** An uncaught
  exception leaves `main()` for JavaScript without `std::terminate()`, and
  one thrown through `noexcept` reaches the terminate handler with no
  current exception. Those are properties of the platform, not of our
  code; the four precondition cases still run.
- **Test executables link with `-pthread`.** The engine is compiled with
  `-pthread` under Emscripten, and code compiled that way must be linked
  that way. The tests were linked with no threading flag, which 4.0.7
  tolerated but 3.1.70 did not: `thread_local` variables such as doctest's
  assertion counter pointed at garbage ("memory access out of bounds"
  in 144 of 162 tests). Linking with `-sWASM_WORKERS`, as the web app
  does, set up thread-local storage but still broke typed catches on
  3.1.70. `-pthread` goes on the `doctest` target, so every test
  executable gets it.
- **CI runs the fast suite under Emscripten** in the web job, with the
  same Emscripten 3.1.70 as the deployed build and warnings as errors.

## Implementation Progress

### Session #1

- All of the above. Locally with Emscripten 4.0.7: 196/196 in Release
  with slow tests, 162/162 in Debug, and the CI step's configuration
  passes with `WISDOM_CHESS_WERROR=On`. The native suite is unchanged.
- The React app loads in headless Chromium and the engine answers a move
  from its worker. `build-qml-wasm.sh` against Qt 6.11.2 still builds,
  with no `-fwasm-exceptions` anywhere in its flags.
- Not verified locally: Emscripten 3.1.70 and Qt 6.9, which CI uses.

### Session #2

- CI failed with "memory access out of bounds" in the new WASM test step.
  GitHub's log storage was down, so it was reproduced locally with
  Emscripten 3.1.70 in a separate emsdk: `main` fails the same way, so it
  predated this branch and went unseen only because CI never ran these
  tests. Fixed by linking the tests with `-pthread`, as above.
- 196/196 on both 3.1.70 and 4.0.7, slow tests included. The web app
  built with 3.1.70 loads and its engine answers from the worker.
- Still open: the web app links with `-sWASM_WORKERS` only, which on
  3.1.70 left typed catches unmatched in the tests. Its typed catches are
  on reporting paths (`logger.cpp`) and in `FenParser`, which the web
  frontend does not reach.
