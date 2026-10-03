# Layering improvements

## Motivation

Items 8 to 11 of
[improvement-suggestions.md](../09/improvement-suggestions.md) are
about what depends on what in the engine. Item 8, the split of
`global.hpp`, was done on other branches. This branch takes the other
three:

- **Item 9.** `isLegalPositionAfterMove()`, `isCheckmated()`,
  `isStalemated()` and the inline `isKingThreatened()` live in
  `evaluate.hpp`. They are rules of the game, not evaluation, and the
  move generator needs them, so `generate.cpp` includes `evaluate.hpp`
  and the rules layer depends on the layer above it.
- **Item 10.** `board.hpp` includes `generate.hpp` and `evaluate.cpp`
  includes `search.hpp`, and neither uses anything from the header it
  includes. The first one gives every includer of `board.hpp` the
  generator for free, which hides which files use it.
- **Item 11.** `Game::save()` and `Game::load()` pick a file format
  from the file name through two mutable file-scope objects in
  `game.cpp`. Reading and writing files is not what a `Game` is for,
  and every frontend depends on the class.

## Design

### Item 9: the legality functions move down

No new files. The functions go where their callers already are:

- The two inline `isKingThreatened()` overloads go to `threats.hpp`,
  next to `InlineThreats`, which they wrap.
- `isLegalPositionAfterMove()`, `isCheckmated()` and `isStalemated()`
  go to `generate.hpp` and `generate.cpp`, beside `hasLegalMove()`,
  which the last two are built on. The header's contract is then
  "moves and whether they are legal".

`evaluate.hpp` keeps `probableDrawCategory()`, which calls
`isCheckmated()`, so it includes `generate.hpp`; that is the right
direction, evaluation above rules. `generate.cpp` includes
`threats.hpp` in place of `evaluate.hpp`. Files that call the moved
functions include the header that now declares them rather than
relying on `evaluate.hpp` to bring it in.

### Item 10: the two includes come out

`board.hpp` drops `generate.hpp` and `evaluate.cpp` drops
`search.hpp`. Whatever stops compiling includes what it uses. The
sweep for item 9 found a third include of the same kind, `board.cpp`'s
`evaluate.hpp`, which is treated the same way.

### Item 11: files become free functions

A new pair, `game_file.hpp` and `game_file.cpp`, holds

```cpp
// Saves the game as FEN when the file name contains ".fen", and as
// the engine's move list otherwise. Throws Error when the file cannot
// be written.
void saveGame (const Game& game, const string& filename);

// Loads a game saved as a move list. Empty when the file cannot be
// read or holds a move that does not parse.
[[nodiscard]] auto
loadGame (const string& filename, const Players& players)
    -> optional<Game>;
```

`saveGame()` constructs the `OutputFormat` it needs on the stack, so
the file-scope objects go. `loadGame()` is `Game::load()` moved, built
on the public `createGame()` and `move()`. `Game::save()`,
`Game::loadGame()` and `Game::load()` are removed, along with
`game.cpp`'s includes of `<fstream>` and `output_format.hpp`.

The console frontend is the only caller of either. The QML and
WebAssembly frontends do not save or load files, so the API change
reaches them only as a removal they never used.

## Plan

1. Item 9, then item 10, then item 11, each as its own commit, with
   the fast and slow suites passing in Release and the new tests in
   Debug as well.
2. Tick items 9, 10 and 11 in the suggestion list.

The search is not touched, so there is nothing to measure.
