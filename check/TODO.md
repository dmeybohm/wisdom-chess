
# TODO

## Version 1
- [x] Store castling state in the Board so that a move that makes it impossible to castle is captured (for example moving a rook)
- [x] Check the move MoveHistory for whether a draw happens and return evaluate my_score of zero
- [x] Use Board hashing to implement transposition table
     - [x] need to add absolute depth instead of overwriting all the time
- [x] Add early exit from `iterate()` if checkmate is detectedtime
- [ ] Add better stats collection display 
  - [ ] Allow viewing positions in the transposition table from the bitset
  - [ ] Add interactive inspection of game state in console
- [ ] Try switching to to clang to improve build speed on Windows
  - [ ] Maybe try switching linker to LLD (clang linker) in combination with clang
  - [ ] [[Also]] look at other linkers on Windows to link - e.g. IncrediBuild
- [ ] Add profiling and document it
- [ ] Use Zobrist hashing in the bitset class to incrementally build up board hash code
- [ ] Improve move generation so fewer copies are generated and whole board isn't traversed
- [ ] Implement parallelized quiescent search before returning a my_score evaluating the Board
- [ ] Add opening book
- [ ] Do some pawn structure / positioning evaluation
- [x] (partial) Multithread

