
# TODO

- [x] Store castling state in the Board so that a move that makes it impossible to castle is captured (for example moving a rook)
- [x] Check the move MoveHistory for whether a draw happens and return evaluate my_score of zero
- [~] Use Board hashing to implement transposition table
     - need to add absolute depth instead of overwriting all the time
- [x] Add early exit from `iterate()` if checkmate is detected
- [ ] Implement quiescent search before returning a my_score evaluating the Board
- [ ] Add opening book
- [ ] Do some pawn structure / positioning evaluation
- [~] Multithread
- [ ] Finish implementing incremental Board hashing
