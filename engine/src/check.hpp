#ifndef WISDOM_CHESS_CHECK_HPP
#define WISDOM_CHESS_CHECK_HPP

#include "global.hpp"
#include "move.hpp"
#include "history.hpp"
#include "threats.hpp"
#include "logger.hpp"

namespace wisdom
{
    struct DrawCategory
    {
        enum Value
        {
            NoDraw,
            InsufficientMaterial,
            ByRepetition,
            ByNoProgress
        } my_value;

        DrawCategory (Value value) // NOLINT(google-explicit-constructor)
            : my_value { value }
        {}

        explicit operator bool () const
        {
            return my_value != NoDraw;
        }

        friend auto operator == (DrawCategory first, DrawCategory second) -> bool
        {
            return first.my_value == second.my_value;
        }

        friend auto operator != (DrawCategory first, DrawCategory second) -> bool
        {
            return !operator== (first, second);
        }
    };

    // Whether this move was a legal move for the computer_player.
    [[nodiscard]] auto is_legal_position_after_move (const Board& board, Color who, Move mv) -> bool;

    [[nodiscard]] inline auto is_king_threatened (const Board& board, Color who, Coord king_coord) -> bool
    {
        InlineThreats threats { board, who, king_coord };
        return threats.check_all ();
    }

    [[nodiscard]] inline auto is_king_threatened (const Board& board, Color who, int8_t king_row,
                                                  int8_t king_col) -> bool
    {
        return is_king_threatened (board, who, make_coord (king_row, king_col));
    }

    // Whether the board is in a checkmated position for the computer_player.
    [[nodiscard]] auto is_checkmated (const Board& board, Color who, MoveGenerator& generator) -> bool;

    // Whether in a stalemate position for white or black.
    [[nodiscard]] auto is_stalemated (const Board& board, Color who, MoveGenerator& generator) -> bool;

    // Whether this move could cause a draw.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    inline auto is_drawing_move (const Board& board, [[maybe_unused]] Color who,
                                 [[maybe_unused]] Move move, const History& history) -> DrawCategory
    {
        auto repetition_status = history.get_threefold_repetition_status ();
        auto no_progress_status = history.get_fifty_moves_without_progress_status ();
        int repetition_count =
                repetition_status == DrawStatus::Declined ?
                5 : 3;
        int without_progress_count =
                no_progress_status == DrawStatus::Declined ?
                150 : 100;

        if (history.is_nth_repetition (board, repetition_count))
            return DrawCategory::ByRepetition;

        if (History::has_been_n_half_moves_without_progress (board, without_progress_count))
            return DrawCategory::ByNoProgress;

        const auto& material_ref = board.getMaterial ();

        if (material_ref.checkmate_is_possible (board) == Material::CheckmateIsPossible::No)
            return DrawCategory::InsufficientMaterial;

        return DrawCategory::NoDraw;
    }
}

#endif // WISDOM_CHESS_CHECK_HPP
