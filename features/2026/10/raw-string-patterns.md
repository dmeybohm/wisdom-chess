# Raw string literals for the fatal test patterns

## Motivation

Every `FATAL_CASE` names the message it expects as a regular expression,
and until now each one was an ordinary string literal, so every dot,
pipe and parenthesis in the pattern was escaped twice: once for the
regex and once for C++.

```cpp
"Precondition failed at .*coord\\.hpp:[0-9]+: result\\.has_value\\(\\)"
```

A survey of the tree for multiline or escape-heavy literals found these
patterns to be the only place where a raw string literal would pay off.
The console help text and the linter's usage text are built with `<<`
chains whose indentation a raw literal would keep, and the linter's JSON
writer interleaves a value on every line.

## Design

Each pattern becomes a raw string literal with an `re` delimiter:

```cpp
R"re(Precondition failed at .*coord\.hpp:[0-9]+: result\.has_value\(\))re"
```

The delimiter is used on every pattern, not only on the ones that end in
an escaped parenthesis. A plain `R"(...\(\))"` is legal, since the
literal ends only at the first `)"`, but the delimiter shows where the
pattern stops, and one spelling throughout is easier to read than two.

`logger_test.cpp` already spelled its timestamp regex this way.

## Implementation Progress

### Session #1

Converted all 52 patterns across the nine `fatal_*_test.cpp` files. The
two patterns without an escape, in `fatal_uncaught_test.cpp`, stay
ordinary literals. The lint target and the 50 listed `Fatal:` tests pass
in a Release build; the four `_debug` cases run only in Debug.
