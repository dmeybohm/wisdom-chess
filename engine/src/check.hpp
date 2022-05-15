#ifndef WISDOM_CHESS_CHECK_HPP
#define WISDOM_CHESS_CHECK_HPP

#include "global.hpp"
#include "move.hpp"
#include "history.hpp"
#include "threats.hpp"
#include "logger.hpp"

namespace wisdom
{
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

    // Whether this move was a legal move for the computer_player.
    auto was_legal_move (Board& board, Color who, Move mv) -> bool;

    [[nodiscard]] inline auto is_king_threatened (Board& board, Color who, Coord king_coord) -> bool
    {
        InlineThreats threats { board, who, king_coord };
        return threats.check_all ();
    }

    [[nodiscard]] inline auto is_king_threatened (Board& board,
                                    Color who,
                                    int8_t king_row,
                                    int8_t king_col) -> bool
    {
        return is_king_threatened (board, who, make_coord (king_row, king_col));
    }

    // In the search, there is one place we don't call this too often and don't want to bloat
    // the search with it. So call it out of line.
    auto is_king_threatened_not_inlined (Board& board, Color who, Coord king_coord) -> bool;

    // Whether the board is in a checkmated position for the computer_player.
    auto is_checkmated (Board& board, Color who, MoveGenerator& generator) -> bool;

    // Whether in a stalemate position for white or black.
    auto is_stalemated (Board& board, Color who, MoveGenerator& generator) -> bool;

    // Whether this move could cause a draw.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    inline auto is_drawing_move (Board& board, [[maybe_unused]] Color who,
                                 [[maybe_unused]] Move move, const History& history) -> bool
    {
        return history.is_third_repetition (board) ||
               History::is_fifty_move_repetition (board);
    }
}

#endif // WISDOM_CHESS_CHECK_HPP
