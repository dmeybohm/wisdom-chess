# Parser naming: parseX and toX

## Motivation

`features/2026/10/to-int-to-uint.md` renamed the string parser `toInt()`
in `str.hpp` to `parseInt()`, which returns `optional<int>`. The coordinate
and move parsers spell the same distinction with a suffix instead:

| Now | Contract |
|---|---|
| `coordParseOptional()`, `moveParseOptional()` | text from outside the program; returns `optional` |
| `coordParse()`, `moveParse()` | a string the program wrote; a bad one is a precondition failure |

One rule covers all of them: `parseX()` checks text from outside and
returns `optional` or `expected`, and `toX()` takes a string the program
wrote and fails its precondition on a bad one.

| Now | New |
|---|---|
| `coordParseOptional` | `parseCoord` |
| `coordParse` | `toCoord` |
| `moveParseOptional` | `parseMove` |
| `moveParse` | `toMove` |
| `castleParse` (`move.cpp`, file-local, returns `optional`) | `parseCastlingMove` |

`castleParse` turns `O-O` or `O-O-O` into a castling `Move` for the
given color, so `parseCastlingMove` names what it returns.

`toCoord` and `toMove` are camelCase domain functions; they sit beside
the snake_case generic conversions in `cast.hpp` (`to_int`, `to_uint`,
`to_bool`) without colliding.

Out of scope:

- `Game::tryCreateGameFromFen()` / `createGameFromFen()` and
  `FenParser::parse()` already read as a parser pair through their
  factory naming.
- Older logs in `features/` that name the old functions are history and
  stay as written.

## Plan

1. Rename the four public functions and `castleParse`, with every caller
   across the engine, the tests and the frontends (console, UCI, QML,
   WASM). This is mechanical; `moveParse` alone has about 290
   occurrences, mostly in tests.
2. Update the comments that name the other form ("Text from outside the
   program goes through ...").
3. Update the parser paragraph in `AGENTS.md`.
4. Build every frontend that calls them, run lint, the Release and Debug
   tests, and the WASM build.

## Implementation Progress

### Session #1

- Created the branch, stacked on `to-int-to-uint` (#345), and this plan.
  Nothing is renamed yet.

### Session #2

- Renamed `coordParseOptional`, `coordParse`, `moveParseOptional`,
  `moveParse` and `castleParse` across 41 files, including the comments
  that name the other form.
- `AGENTS.md` lists `parseInt()` with the parsers and states the naming
  rule for a new pair.
- Verified lint, all 339 Release tests with the QML UI, all 318 Debug
  tests and the WASM build.
