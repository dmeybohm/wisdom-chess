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

    // When there are no legal moves present, return the score of this move, which
    // checks for either a stalemate or checkmate position.
    auto evaluate_without_legal_moves (Board& board, Color who, int moves_away) -> int;

    // Get the score for a checkmate discovered X moves away.
    // Checkmates closer to the current position are more valuable than those
    // further away.
    inline auto checkmate_score_in_moves (int moves) -> int
    {
        return Infinity + Infinity / (1 + moves);
    }

    // Whether the move indicates that it checkmates an opponent.
    inline auto is_checkmating_opponent_score (int score) -> bool
    {
        return score > Infinity;
    }
}

#endif // WISDOM_CHESS_EVALUATE_HPP
