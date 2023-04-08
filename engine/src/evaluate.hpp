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
    auto evaluate (const Board& board, Color who, int moves_away, MoveGenerator& generator) -> int;

    // When there are no legal moves present, return the score of this move, which
    // checks for either a stalemate or checkmate position.
    auto evaluate_without_legal_moves (const Board& board, Color who, int moves_away) -> int;

    // Get the score for a checkmate discovered X moves away.
    // Checkmates closer to the current position are more valuable than those
    // further away.
    constexpr auto checkmate_score_in_moves (int moves) -> int
    {
        return Max_Non_Checkmate_Score + Max_Non_Checkmate_Score / (1 + moves);
    }

    // Whether the score indicates a checkmate of the opponent has been found.
    constexpr auto is_checkmating_opponent_score (int score) -> bool
    {
        return score > Max_Non_Checkmate_Score && score < Initial_Alpha;
    }
}

#endif // WISDOM_CHESS_EVALUATE_HPP
