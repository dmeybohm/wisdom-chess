#ifndef WISDOM_CHESS_CHECK_HPP
#define WISDOM_CHESS_CHECK_HPP

#include "global.hpp"
#include "move.hpp"
#include "history.hpp"

namespace wisdom
{
    // Whether this move was a legal move for the computer_player.
    bool was_legal_move (Board& board, Color who, Move mv);

    // check if the the king indicated by the WHO argument is in trouble
    // in this position
    bool is_king_threatened (Board& board, Color who, int row, int col);

    static inline bool is_king_threatened ([[maybe_unused]] Board& board,
                                           [[maybe_unused]] Color who,
                                           [[maybe_unused]] Coord pos)
    {
        return is_king_threatened (board, who, Row (pos), Column (pos));
    }

    // Whether the board is in a checkmated position for the computer_player.
    bool is_checkmated (Board& board, Color who);

    // Whether in a stalemate position for white or black.
    static bool is_stalemated ([[maybe_unused]] const Board& board)
    {
        // todo: detect stalemate efficiently
        return false;
    }

    // Whether this move could cause a draw.
    static inline bool is_drawing_move (Board& board, [[maybe_unused]] Color who,
                                        [[maybe_unused]] Move mv, const History& history)
    {
        return history.is_third_repetition (board) ||
               History::is_fifty_move_repetition (board) ||
               is_stalemated (board);
    }
}

#endif // WISDOM_CHESS_CHECK_HPP
