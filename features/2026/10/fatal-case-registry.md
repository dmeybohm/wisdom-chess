# Fatal cases that register themselves

Step 2 of `exception-removal.md`. Moving the contract checks to abort
roughly doubles the fatal cases, and `fatal_test_main.cpp` kept each
case in two places: its function, and an entry in `Fatal_Cases` about 150
lines further down holding the name and the expected message.

## Design

- `FATAL_CASE( name, expected [, listed] )` in `fatal_test.hpp` defines
  the case's function and a static `FatalCaseRegistrar`, the way
  doctest's `TEST_CASE()` does. The registry is a function-local static
  in `fatal_test_main.cpp`, so a case can register before `main()`
  whatever the order of static initialization.
- The macro is variadic and passes its arguments after the function
  pointer, which `FatalCase` therefore lists first; the optional `listed`
  falls back to its default member initializer. `__VA_OPT__` would be
  the other way to make it optional, but MSVC's traditional preprocessor
  does not support it.
- `--list` sorts the cases by name, since registration order across files
  is unspecified, and fails on a duplicate name, which fails the build
  through `discover_fatal_tests.cmake`.
- The cases are in one file per area: `fatal_board_test.cpp`,
  `fatal_contract_test.cpp`, `fatal_move_list_test.cpp`,
  `fatal_numeric_cast_test.cpp` and `fatal_uncaught_test.cpp`.
  `MarkedLogger` and `Reports_Uncaught_Errors` move to the header. The
  files are compiled into the program rather than linked from a library,
  which could drop the registrars.
- The linter treats `FATAL_CASE` as a test macro: spaces inside the
  parentheses and none before them.

## Implementation Progress

### Session #1

- Moved the 17 cases unchanged. The only difference in their names and
  messages is the file a message names, for the cases whose failing
  check is in the test file itself.
- `--list` gives the same 14 cases in Release, and 17 in Debug.
- Verified the fatal tests in Release and Debug with GCC and in Debug
  with Clang 18 and `-Werror`, the linter's tests and `lint`. Not run:
  MSVC, Emscripten and FIL-C, left to CI.
