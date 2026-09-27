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
