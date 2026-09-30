# Contract macros and the split of `global.hpp`

Branch: `contract-macros`.

## Motivation

The engine mixed two kinds of checks. `expects()`, `ensures()` and
`noexcept_expects()` were functions that reported the file, line and
function of a failure through `std::source_location`, but not what was
checked. `assert()` reported the condition's text, but wrote it to stderr
and bypassed `logEmergency()`, which the QML and WebAssembly builds need
for the message to be seen at all.

Nothing in C++20 lets a function see the text of its argument, so a
macro is the only way to get it. C++26 contracts will provide it
natively, but no compiler this project targets ships them.

## Design

`engine/error.hpp` holds the `Error` classes, the check functions and
four macros that wrap them, each passing the stringified condition:

- `EXPECTS( cond )` throws `PreconditionError`.
- `ENSURES( cond )` throws `PostconditionError`.
- `NOEXCEPT_EXPECTS( cond )` reports through `logEmergency()` and aborts,
  for `noexcept` functions.
- `ASSERT( cond )` replaces `assert()`. It reports and aborts like
  `NOEXCEPT_EXPECTS` but only when `Debugging` is on.

`Debugging` is a `constexpr bool` that follows `NDEBUG`, so a Release or
RelWithDebInfo build has it off, exactly where `assert()` was off before.
That is the one macro-conditional in the file.

`ASSERT` expands to a conditional expression,
`Debugging || std::is_constant_evaluated() ? debug_expects (...) : void()`,
so that without `Debugging` the condition is type-checked but not
evaluated, as `assert()` under `NDEBUG` does not evaluate it. Two
improvements over `assert()` follow: a stale condition fails to compile
instead of rotting, and a variable used only in a check is not reported
unused. In a constant expression the check runs in every build, so a
false condition there is a compile error even in Release.

The macros are spelled with spaces inside the parentheses, like the
test macros, and the linter's `test-macro-spacing` and
`function-call-spacing` rules know them. The functions under the macros
(`expects()`, ...) are not called directly any more, since that would
lose the condition text.

The failure message is the old one with the condition appended:
`Precondition failed at move_list.hpp:61: my_size < Max_Move_List_Size`.
The three `switch` defaults that called `throwPreconditionError` or
`terminateOnPreconditionFailure` directly pass a description of what was
expected instead of a condition. `terminateOnPreconditionFailure` became
`terminateOnCheckFailure`, which takes the kind of check so that
`ASSERT` reports "Assertion failed".

`engine/ptr.hpp` holds `nonnull`, `nullable`, `unchecked_nonnull` and
`owning`. It includes `error.hpp` for `EXPECTS`, so `error.hpp` cannot
include it and writes `gsl::czstring` in full. `global.hpp` includes
both, so nothing else changes its includes.

Not possible: a check that detects a `noexcept` caller on its own. A
callee cannot see its caller's exception specification, so
`NOEXCEPT_EXPECTS` stays an explicit choice.

## Implementation Progress

### Session #1

- `error.hpp`, `ptr.hpp`, and `global.hpp` reduced to the standard
  names, the constants and the narrowing helpers. `<cassert>` is no
  longer included.
- All 57 `assert()` calls, 23 `expects()`, 4 `noexcept_expects()` and
  1 `ensures()` converted to the macros.
- Tests in `global_test.cpp`: `EXPECTS` and `ENSURES` quote the
  condition and the location; `ASSERT` evaluates its condition only
  when `Debugging` is on, and works in a constant expression. The
  `append-overflow` fatal test now matches the quoted condition, and a
  Debug-only fatal test `assert-failure` checks that `ASSERT` reports
  and aborts. It is registered only when `CMAKE_BUILD_TYPE` is `Debug`,
  so a multi-config generator does not get it.
- Verified: GCC Release build with the QML UI, lint clean, 230 fast
  tests pass. Clang 18 Debug build of the engine, no warnings, 220
  tests pass including all eight fatal cases. The slow tests and the
  WebAssembly build were not run; the only change there is the spelling
  of two `EXPECTS` calls in `web_game.cpp`.

### Session #2

The abort path no longer touches the heap. `terminateOnCheckFailure`
builds its message in a fixed 1024-byte buffer on the stack, with
`std::to_chars` for the line number, dropping text that does not fit,
and hands it to `logEmergency()` as a `string_view`. The `try` around
the old `string` concatenation, and its `std::cerr` fallback, went with
it: nothing on the path can throw now.

For that, `logEmergency()` and the virtual `Logger::emergency()` take
`string_view` instead of `const string&`. The eleven implementations
changed signature; two needed a body change. The QML logger converts
the view with `QString::fromUtf8 (data, size)`, and the WebAssembly
logger calls a new `consoleErrorBytes (str, length)` `EM_JS` function,
since `UTF8ToString` needs either a terminator or a length. Both sinks
still allocate on their own side, which is theirs to decide; the
engine side is allocation-free up to the sink.

The throw path (`EXPECTS`, `ENSURES`) still formats a `string` and
`Error` still stores it in a `shared_ptr`, as before this branch.
Making exceptions carry the location and condition as constants would
change `Error::message()` for every caller, and is left for a branch
of its own.

- Verified: GCC Release build with the QML UI and Clang 18 Debug build,
  no warnings, lint clean, all tests pass in both, including the nine
  emergency and fatal cases. The React WASM target builds with
  Emscripten; the browser console output was not checked by hand.
