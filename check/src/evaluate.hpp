#ifndef EVOLVE_CHESS_EVALUATE_HPP
#define EVOLVE_CHESS_EVALUATE_HPP

#include "piece.hpp"
#include "move.hpp"
#include "move_tree.hpp"

namespace wisdom
{
    class Board;

    class History;

    // Evaluate the board.
    int evaluate (Board &board, Color who, int moves_away);

    // Evaluate the board and check if it's a draw.
    int evaluate_and_check_draw (Board &board, Color who, int moves_away,
                                 Move move, const History &history);

}

#endif // EVOLVE_CHESS_EVALUATE_HPP
