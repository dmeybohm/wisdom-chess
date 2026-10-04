# React WASM build without exceptions

Step 4 of `exception-removal.md`. `-fwasm-exceptions` came in with
`wasm-doctest-discovery.md` (2026-09), so that the tests expecting a throw
and the engine's catch blocks worked under Emscripten. After steps 1 to 3
the engine catches nothing on purpose, so the reason is gone.

## Design

- **Emscripten's default exception model for every WebAssembly build.**
  Exception code compiles, nothing catches, and a throw aborts. It is the
  model the QML WebAssembly build already used: Qt 6.9.3's
  `wasm_multithread` kit, which CI installs, is built with
  `QT_FEATURE_wasm_exceptions` off and no `QT_FEATURE_exceptions`, so
  `QtWasmHelpers.cmake` adds neither `-fwasm-exceptions` nor
  `-fexceptions`. Both builds now share one model, so the configure error
  that kept Qt and `-fwasm-exceptions` apart is gone too.
- **Not `-fno-exceptions`.** It would make a new `throw` a compile error,
  but the emergency logger's `try` blocks in `logger.cpp` would need
  `#if __cpp_exceptions` around them, and it saves only about 1 KB more,
  gzipped. Under the default model those blocks compile and their
  `catch` never runs, which is harmless: they guard the emergency path,
  which ends in an abort.
- **The web app no longer links with `-pthread`.** It was added only
  because `-sWASM_WORKERS` alone left typed catches unmatched on
  Emscripten 3.1.70. The test executables keep it, for thread-local
  storage.
- **One test is skipped under Emscripten:** the subcase of
  `logger_test.cpp` that makes `std::cerr` throw. `Can_Catch_Exceptions`
  in `wisdom-chess-tests.hpp` says whether a test can.
- The module no longer needs native WebAssembly exceptions in the browser
  (Chrome 95, Firefox 100, Safari 15.2).

## Implementation Progress

### Session #1

- Release build of the web app with Emscripten 3.1.70, as CI uses, in
  bytes:

  | | `.wasm` | gzipped | `.js` | gzipped |
  |---|---|---|---|---|
  | Before | 266,955 | 103,365 | 125,113 | 27,094 |
  | After | 239,028 | 91,367 | 113,843 | 23,947 |

  Dropping `-pthread` accounts for most of the glue's share. With
  4.0.7, `-fno-exceptions` measured about 1 KB smaller gzipped than the
  default model.
- CI's WebAssembly test configuration (Release, warnings as errors)
  builds and passes all 246 tests with both 3.1.70 and 4.0.7.
- The RelWithDebInfo web app built with 3.1.70 loads in headless
  Chromium, cross-origin isolated, and the engine answers a move from
  its worker, with no page errors.
- A native Release build passes the 281 fast tests and `lint`. Not run
  locally: the QML WebAssembly build, which `scripts/build-qml-wasm.sh`
  configures with `WISDOM_CHESS_QML_UI=ON` and which loses only the
  configure error.
