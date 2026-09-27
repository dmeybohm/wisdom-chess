# A lexer for the style linter

## Motivation

Each linter rule works out what is code from raw lines on its own.
`stripComments()` blanks comments and `raw-pointer`'s `blankLiterals()`
blanks string literals, and the two disagree: `stripComments()` takes a
digit separator (`100'000`) for the start of a character literal, so a
comment after one is never stripped, for every rule. Four review findings
on the `raw-pointer` rule in
[nonnull-observer-params.md](nonnull-observer-params.md) came from this,
and probing turned up more: a raw string over several lines, and
declarations or template argument lists split across lines.

## Design

A minimal lexer that splits a file into tokens, without parsing:

- Comments, `//` and `/* */`, across lines.
- String and character literals with escapes and encoding prefixes
  (`u8`, `u`, `U`, `L`), and raw strings (`R"delim( ... )delim"`) with
  any delimiter, across lines. A user-defined literal suffix belongs to
  its literal.
- Numbers as preprocessing numbers: digits, letters, `.`, digit
  separators and exponent signs (`1e+5`, `0x1p-3`).
- Identifiers, keywords included; rules compare the text.
- Operators and punctuation matched longest first (`<=>`, `->*`, `...`,
  `->`, `>>`, `::`, `*=`, ...), so `->` is never read as `>`. A rule that
  matches template brackets counts `>>` as two.
- A preprocessor directive, from `#` at the start of a line through its
  `\` continuation lines, as one token.

Each token keeps its line and column (1-based, in bytes) and whether
whitespace, a comment or the start of a line comes before it, which the
spacing rules need. The lexer never fails: a character it does not
recognize becomes a one-character token. An unterminated string or
character literal ends at the end of its line, since an ordinary literal
cannot hold a newline, so a stray apostrophe cannot swallow the rest of
the file; an unterminated block comment or raw string runs to the end.

`wisdom-linter --dump-tokens <file>` prints the tokens, one per line, and
`tests/run-tests.sh` compares a fixture's `.tokens` file with that output,
as it compares `.expected` files with lint output.

It will not settle whether `A * b` declares a pointer, which still needs
`raw-pointer`'s naming heuristic, but that heuristic gets clean input.

## Plan

1. The lexer, `--dump-tokens` and lexer fixtures.
2. `raw-pointer` ported to tokens, with its fixtures as the regression
   suite plus the cases the line-based version gets wrong.
3. The other rules ported one at a time, each keeping its fixtures, and
   `stripComments()` removed once nothing uses it.

## Implementation Progress

### Session #1

**Step 1: the lexer.** `scripts/linter/lexer.hpp` and `lexer.cpp`, with
`--dump-tokens` in `main.cpp` and `.tokens` support in `run-tests.sh`.
No rule uses it yet. Fixtures in `tests/lexer/`:

- `comments.cpp`: a comment after `100'000`, a block comment over two
  lines, a line comment continued by a backslash, quotes in a comment.
- `literals.cpp`: escapes, encoding prefixes, a raw string holding `)"`
  and a newline, user-defined suffixes, a string continued by a
  backslash, an unterminated character literal.
- `numbers.cpp`: hexadecimal floats, exponents, digit separators.
- `punctuators.cpp`: `->`, `->*`, `.*`, `<=>`, `>>=`, `>>` closing two
  template argument lists, `::`, `...`.
- `preprocessor.cpp`: a directive with a comment after it, one continued
  over two lines, one indented with a digit separator, one after a block
  comment, and a `#` in the middle of a line.
- `spacing.cpp`: `spaced` and `joined` around calls and `*`.

A directive keeps its block comments and literals but not a trailing line
comment, which becomes a token of its own, so a `lint-allow` marker there
still reads as a comment.

**Step 2: `raw-pointer` on tokens.** `LintContext` now carries the
file's tokens, lexed once in `Linter::lintFile()`. The rule reads them
with comments and directives left out and keeps its three steps (what
follows the `*`, its spacing, the name before it). The `using` aliases
come from tokens too, so a comment, a split line or a string can no
longer feed them, the three review findings from the first version. A
`*` at the end of a line is now one whose next token is on a later line,
and a type split across lines is just tokens.

All 14 existing `raw-pointer` fixtures pass unchanged. A new one,
`tokens.cpp`, holds the cases the line-based rule got wrong: a comment
after `100'000` and a raw string over three lines (false positives), a
`*` on the line after `-> Board` and a template argument list split
across lines (missed), and `const char* const*`, where it reported a
"raw pointer to `const`".

Over doctest's and GSL's headers the line-based rule reports 265
pointers and the token-based one 244, with nothing new. The 21 it no
longer reports are:

- 4 inside `#define` bodies. A directive is one token now and the rule
  skips it whole; the line-based rule skipped only a directive's first
  line. A macro body is text for substitution rather than a declaration.
- 17 second stars of `T* const*`. The declaration is reported once, at
  its first `*`, as `char**` already was.

**Step 3: the other rules.** `codeTokens()`, `isIdentifier()`,
`isPunctuator()` and `startsLine()` moved to `lexer.hpp` for all rules.
`no-tabs` stays on lines, since a tab is wrong anywhere in a file,
comments included. Each port was checked against the line-based rule on
real code, and every existing fixture passes unchanged:

- `namespace-braces` read raw lines, so "namespace x {" inside a block
  comment was reported. New fixture: `namespace-braces/comments.cpp`.
- `test-macro-spacing` skipped comments but not string literals, so a
  macro's name in a string was checked as a call. With the spacing
  inside every test macro removed from 43 test files, both versions
  report the same 4,182 violations. New fixture:
  `test-macro-spacing/literals.cpp`.
- `function-call-spacing` kept its own string tracker, which took a digit
  separator for a quote; it was right about the comment after one only by
  that accident. With `foo (x)` and `bar()` broken both ways across a
  copy of `src`, both versions report the same 9,245 violations. A
  comment alone between the parentheses still counts as an argument, as
  it did, so `reset (/* unused */)` is not reported. New fixture:
  `function-call-spacing/tokens.cpp`.
- `trailing-return-type` read raw lines without skipping comments. Over
  doctest, GSL and `src` it reports what the line-based rule did, except
  3 lines inside `#define` bodies, two of which were false positives
  (`ResultBuilder rb(at, ...);` declares a variable).

No rule reads a directive now, so the bodies of multi-line macros are no
longer checked; nothing in `src` has one. `stripComments()` is gone. The
linter takes 0.044 s over `src`, doctest and GSL, against 0.052 s for the
line-based one.
