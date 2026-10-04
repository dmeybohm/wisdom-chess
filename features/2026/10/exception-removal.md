# Removing the engine's own exceptions

## Motivation

The engine throws in two situations, and neither needs an exception.

- **Bad input.** The coordinate, move and FEN parsers throw on text that
  does not parse, and nearly every caller catches at the call and turns
  the error into `nullopt` or a message: `moveParseOptional()` catches
  `CoordParseError` twice (`move.cpp`), the console's FEN prompt catches
  `FenParserError` (`play.cpp`), `UciInterface::parseUciMove()` wraps
  `coordParse()` in `catch (...)` and `position fen` reports the message.
  Bad input is an ordinary result, so it belongs in the return type.
  `ChessGame::fromFen()` lets the error propagate, and only `guarded()`
  keeps it out of Qt.
- **Contract checks.** `EXPECTS`, `ENSURES`, and the checked `narrow` and
  `to_unsigned` throw `PreconditionError` or `PostconditionError`. A
  failure is a bug: the callers already validate the input. UCI clamps
  `Hash`, `Depth` and the move overhead and skips a zero `movetime`
  before calling `setSearchTimeout()`. None of the eight
  checked-cast calls outside tests converts outside input, and the four in
  `pieces_model.cpp` are in functions Qt calls, which are `noexcept`, so
  they abort already. Throwing implies a recovery that nothing performs,
  and it is the reason for the paired `EXPECTS`/`EXPECTS_NOEXCEPT` macros
  and the three forms of each cast.

C++26 contracts (P2900) take the same view. `pre`, `post` and
`contract_assert` default to the *enforce* semantic: call the violation
handler, then terminate. *Ignore* is what the `_debug` forms do in
Release. A handler may throw, but a throw out of a `noexcept` function
still terminates, so a throwing build would keep today's split. Aborting
now makes a later move to `pre`/`post` a mechanical rewrite. No toolchain
we build with supports them yet.

## Design

Four branches, each standing alone and each linking back here.

### 1. Parsers return their errors (`parse-expected`)

- `coordParseOptional()` and `moveParseOptional()` already exist. Make
  `moveParseOptional()` call `coordParseOptional()` instead of catching,
  and move `parseUciMove()` to the optional forms. A coordinate or move
  needs no message, so `optional` is enough.
- The FEN parser returns `expected<Game, ParseError>`, or a non-throwing
  `Game::tryCreateGameFromFen()` does, where `ParseError` is a value type
  holding the message. UCI, the console and `ChessGame::fromFen()` use
  it. The parser's internals return the error too, rather than throwing
  and converting at the entry point. That shortcut would leave a throw in
  the WASM build and block step 4.
- The FEN parser builds through `BoardBuilder`, whose range and
  occupied-square checks throw `BoardBuilderError`. The parser validates
  those fields itself before calling the builder, and the builder's
  checks become contract checks: elsewhere they guard positions written
  as literals in tests and setup code.
- `coordParse()`, `moveParse()` and `pieceFromChar()` keep only callers
  that pass literals (`MoveList`'s initializer list, `BoardBuilder`,
  tests), so a bad string is a bug there. They become contract checks
  over the optional forms. `CoordParseError`, `ParseMoveError`,
  `FenParserError`, `BoardBuilderError` and `PieceError` go away.
- `game_file.cpp`'s two I/O throws return an error to the console, which
  then has one error style.
- `expected`: `std::expected` needs C++23, which every toolchain (GCC,
  Clang, MSVC, Apple Clang, Emscripten, the Android NDK) would have to
  support first. `tl::expected` through CPM has nearly the same API, so
  start with it and leave the C++23 move to its own branch.
- About 62 of the 77 `CHECK_THROWS` in the tests are parse errors. They
  become checks on the returned error.

### 2. Fatal harness split (`fatal-case-registry`)

`fatal_test_main.cpp` has 17 cases in 318 lines, and each case takes two
edits that can drift: a function, and an entry about 150 lines down in
`Fatal_Cases` holding its expected regular expression. Step 3 roughly
doubles the count.

- A `FATAL_CASE( name, expected [, listed] )` macro in `fatal_test.hpp`
  defines the function and a static registrar. The registry is a
  function-local static, so initialization order does not matter.
- `fatal_test_main.cpp` keeps `MarkedLogger`, the abort handler, `--list`
  and dispatch. The cases move to one file per area: casts, contracts,
  the move list, uncaught errors and the logger, and search.
- The case files are compiled into the executable, not linked from a
  static library, which could drop the registrars.
- Expectations that match `fatal_test_main\.cpp` follow their case to the
  new file.
- A pure move: the same 17 cases, and the CMake discovery and
  `run_fatal_test.cmake` unchanged. A base class with a virtual `run()`
  would add a class per case and gain nothing.

### 3. Contract checks abort (`contracts-abort`)

- `EXPECTS` and `ENSURES` take the `_noexcept` behaviour: report through
  `logEmergency()`, then terminate. `EXPECTS_NOEXCEPT` and
  `ENSURES_NOEXCEPT` go away. `ASSERT` is unchanged.
- The cast family becomes `narrow` and `to_unsigned`, which always check
  and abort (today's `_noexcept`, renamed); `narrow_debug` and
  `to_unsigned_debug`; and `widen` and `truncate`, unchanged.
  Validating a number from outside is input parsing, as in step 1, and
  uses `std::in_range`.
- `nonnull::value()` aborts on null instead of throwing.
- `PreconditionError` and `PostconditionError` go away, along with the
  `expects-through-noexcept` fatal case, which tests their throwing
  through `noexcept`.
- The `web_types.hpp` enum conversions abort: an invalid value from the
  bindings is a bug.
- About 15 contract `CHECK_THROWS` (in `game_test`, `history_test`,
  `transposition_table_test` and `global_test`) become fatal cases.
- Functions whose only throw was a contract check can become `noexcept`.
- Behaviour change: a bug on the QML engine thread ends the process
  instead of reaching `engineFailed`. The practical difference is small:
  once `my_has_failed` is set the engine refuses further work, so the game
  is over either way, and `logEmergency()` reports it in both cases.
- `guarded()` and the rule that whatever Qt calls is `noexcept` stay:
  the standard library still throws (`bad_alloc`, `std::thread`,
  `std::stoi` in UCI option parsing).
- Update AGENTS.md (`numeric_cast.hpp`, the contract macros, `nonnull`)
  and the feature logs it cites.

### 4. React WASM build without exceptions (optional)

The top-level `CMakeLists.txt` compiles the React/WASM build with
`-fwasm-exceptions`, and `ui/wasm/CMakeLists.txt` links with `-pthread`
so typed catches work on Emscripten 3.1.70. After steps 1 and 3 nothing
on its path throws on purpose.

- Drop `-fwasm-exceptions` for that build, which also retires the
  `-pthread` workaround and the dependence on browser support for Wasm
  exceptions. A standard library throw then aborts, which is acceptable
  in a browser tab.
- Measure the binary size before and after. The saving is not yet known.
- Check what still throws on that path first (`std::stoi`, `.at()`,
  `std::thread`).
- Native builds and the QML WebAssembly build keep exceptions, because of
  Qt.

`-fno-exceptions` for native builds is out of scope. Qt and the standard
library still throw, and there is no size or speed reason to pay for
replacing them.

## Implementation Progress

### Session #1

- Surveyed the throw and catch sites and wrote this plan. No code
  changes yet.
