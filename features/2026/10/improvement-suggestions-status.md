# Improvement suggestions status and the engine architecture page

## Motivation

Two documentation changes that share a branch. First, the suggestion
list in
[improvement-suggestions.md](../09/improvement-suggestions.md) had no
way to show which items were done, and some had been done or tried on
other branches without the list saying so. Second, there was no
description of how the engine fits together: `docs/` had the build
guide and the coding style, and the design rationale was spread over
the feature logs. A new contributor had to read the headers.

## Plan

1. Give every suggestion a tickmark, check each against `main` and
   record the result in the list's own log.
2. Write `docs/engine-architecture.md` from the current sources, and
   link it from the README.

## Design

The architecture page is a map, not a manual, and it covers the chess
parts only: the representation, move generation and legality, the
evaluation, the search, the table, and the game's draw rules. The
foundation headers, logging, the error and contract conventions, the
frontend wiring and the test layout are left to `AGENTS.md`,
`building.md` and `coding-style.md`. It sends the reader to the feature
log for the reasoning behind each decision rather than repeating it,
and describes what the engine does without listing what it lacks: the
open items live in the improvement suggestions, not here.

What the page covers is drawn from the headers and from `search.cpp`,
`evaluate.cpp`, `generate.cpp`, `game.cpp` and
`transposition_table.cpp` as of `main` at `1111e81c`.

## Implementation Progress

### Session #1

- Tickmarks and statuses: Session #5 of
  [improvement-suggestions.md](../09/improvement-suggestions.md)
  records what was checked. Items 3 and 8 are done; item 2 was tried
  on a stale local branch without a measurable gain and stays open.
- Wrote `docs/engine-architecture.md` and linked it from the README's
  Contributing section.
- Cut the page back to the chess parts after review: the foundation,
  input and output, errors and contracts, frontend and test sections
  came out, along with the implementation details that were not about
  chess.
