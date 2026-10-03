# The noexcept variant as a suffix

## Motivation

Each checked operation has a form that terminates instead of throwing, for
an invariant in a `noexcept` function. That form was named with a prefix
(`noexcept_narrow`, `NOEXCEPT_EXPECTS`) while the unchecked form already
took a suffix (`narrow_cast`). With the variant last, each family shares
its operation's name: `narrow`, `narrow_noexcept`, `narrow_cast`, and
`EXPECTS`, `EXPECTS_NOEXCEPT`. The forms sort and complete together, and a
search for the operation finds all of them.

## Design

| Before | After |
|---|---|
| `NOEXCEPT_EXPECTS`, `NOEXCEPT_ENSURES` | `EXPECTS_NOEXCEPT`, `ENSURES_NOEXCEPT` |
| `noexcept_expects`, `noexcept_ensures` | `expects_noexcept`, `ensures_noexcept` |
| `noexcept_narrow`, `noexcept_widen`, `noexcept_to_unsigned` | `narrow_noexcept`, `widen_noexcept`, `to_unsigned_noexcept` |

The fatal test cases and their functions follow, such as
`Fatal: narrow-noexcept-overflow`. The linter's macro spacing rules name
the macros, so they change with them.

`AGENTS.md`, `docs/` and the unmerged `to-unsigned.md` use the new names.
The other feature logs keep the old ones, as records of what the code was
called when they were written.

## Implementation Progress

### Session #1

- Renamed every use in the source, the linter rules and the docs.
- With the new names mapped back to the old, the Release disassembly of
  all 90 project objects is identical before and after.
- Checked that the linter still flags a misspaced `EXPECTS_NOEXCEPT`.
- Verified lint, all 297 Release tests and the 261 fast Debug tests. The
  QML frontend was not configured, so its one renamed call in
  `game_model.cpp` is left to CI.
