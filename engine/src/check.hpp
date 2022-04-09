#ifndef WISDOM_CHESS_CHECK_HPP
#define WISDOM_CHESS_CHECK_HPP

#include "global.hpp"
#include "move.hpp"
#include "history.hpp"
#include "threats.hpp"
#include "logger.hpp"

namespace wisdom
{
    // Whether this move was a legal move for the computer_player.
    bool was_legal_move (Board& board, Color who, Move mv);

    // check if the the king indicated by the WHO argument is in trouble
    // in this position
    bool is_king_threatened (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_inline (const Board& board, Color who, int8_t row, int8_t col);

    bool is_king_threatened_row (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_column (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_diagonal (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_knight (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_knight_direct (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_pawn (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_pawn_dumb (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_pawn_c (const Board& board, int who, int8_t row, int8_t col);
    bool is_king_threatened_pawn_inline (const Board& board, Color who,
                                         int8_t king_row, int8_t king_col);
    bool is_king_threatened_king (const Board& board, Color who, int8_t row, int8_t col);
    bool is_king_threatened_king_inline (const Board& board, Color who, int row, int col);
    bool is_king_threatened_diagonal_dumb (const Board& board, Color who,
                                           int8_t king_row, int8_t king_col);

    inline bool is_king_threatened (Board& board, Color who, Coord king_coord)
    {
        InlineThreats threats { board, who, king_coord };
        return threats.check_all ();
    }

    inline bool is_king_threatened (Board& board,
                                    Color who,
                                    int8_t king_row,
                                    int8_t king_col)
    {
        return is_king_threatened (board, who, make_coord (king_row, king_col));
    }

    // Whether the board is in a checkmated position for the computer_player.
    bool is_checkmated (Board& board, Color who);

    // Whether in a stalemate position for white or black.
    bool is_stalemated_slow (Board& board, Color who);

    inline bool is_stalemated_fast (Board& board, Color who)
    {
        // todo
        return false;
    }

    // Whether this move could cause a draw.
    inline bool is_drawing_move (Board& board, [[maybe_unused]] Color who,
                                 [[maybe_unused]] Move move, const History& history)
    {
        auto third_rep = history.is_third_repetition (board);
        if (third_rep) {
            Logger& logger = make_standard_logger ();
            logger.println("Third repetition");
        }
        return third_rep ||
               History::is_fifty_move_repetition (board) ||
               is_stalemated_fast (board, who);
    }
}

#endif // WISDOM_CHESS_CHECK_HPP
