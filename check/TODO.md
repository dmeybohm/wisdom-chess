# TODO

- [ ] Store castling state in the Board so that a move that makes it impossible to castle is captured (for example moving a rook)
- [ ] Check the move MoveHistory for whether a draw happens and return evaluate score of zero
- [ ] Finish implementing incremental Board hashing
- [ ] Add early exit from `iterate()` if checkmate is detected
- [ ] Use Board hashing to implement transposition table
- [ ] Implement quiescent search before returning a score evaluating the Board
- [ ] Add opening book
- [ ] Do some pawn structure / positioning evaluation
- [ ] Multithread
