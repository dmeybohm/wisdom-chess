#ifndef WISDOM_CHESS_EVALUATE_HPP
#define WISDOM_CHESS_EVALUATE_HPP

#include "global.hpp"
#include "move.hpp"
#include "piece.hpp"

namespace wisdom
{
    class Board;

    class History;

    class MoveGenerator;

    // Evaluate the board.
    int evaluate (Board& board, Color who, int moves_away, MoveGenerator& generator);

    // Evaluate the board and check if it's a draw.
    int evaluate_and_check_draw (Board& board, Color who, int moves_away, Move move,
                                 const History& history, MoveGenerator& generator);

}

#endif // WISDOM_CHESS_EVALUATE_HPP
