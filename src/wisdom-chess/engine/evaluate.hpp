#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/threats.hpp"

namespace wisdom
{
    class Board;

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

        explicit operator bool() const
        {
            return my_value != NoDraw;
        }

        friend auto
        operator== (DrawCategory first, DrawCategory second)
        -> bool
        {
            return first.my_value == second.my_value;
        }

        friend auto
        operator!= (DrawCategory first, DrawCategory second)
        -> bool
        {
            return !operator== (first, second);
        }
    };

    // Whether this move was a legal move for the computer_player.
    [[nodiscard]] auto
    isLegalPositionAfterMove (const Board& board, Color who, Move mv)
        -> bool;

    [[nodiscard]] inline auto isKingThreatened (
        const Board& board,
        Color who,
        Coord king_coord
    )
        -> bool
    {
        InlineThreats threats { board, who, king_coord };
        return threats.checkAll();
    }

    [[nodiscard]] inline auto isKingThreatened (
        const Board& board,
        Color who,
        int8_t king_row,
        int8_t king_col
    )
        -> bool
    {
        return isKingThreatened (board, who, makeCoord (king_row, king_col));
    }

    // Whether the board is in a checkmated position for the specified player.
    [[nodiscard]] auto
    isPlayerCheckmated (const Board& board, Color who)
        -> bool;

    // Whether in a stalemate position for white or black.
    [[nodiscard]] auto
    isStalemated (const Board& board, Color who)
        -> bool;

    // Whether this move could cause a draw.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    [[nodiscard]] inline auto
    isProbablyDrawingMove (
        const Board& board,
        [[maybe_unused]] Color who,
        [[maybe_unused]] Move move,
        const History& history
    )
        -> DrawCategory
    {
        auto repetition_status = history.getThreefoldRepetitionStatus();
        auto no_progress_status = history.getFiftyMovesWithoutProgressStatus();
        int repetition_count =
            repetition_status == DrawStatus::Declined ?
            5 : 3;
        int without_progress_count =
            no_progress_status == DrawStatus::Declined ?
            150 : 100;

        if (history.isProbablyNthRepetition (board, repetition_count))
            return DrawCategory::ByRepetition;

        if (History::hasBeenXHalfMovesWithoutProgress (board, without_progress_count))
            return DrawCategory::ByNoProgress;

        const auto& material_ref = board.getMaterial();

        if (material_ref.checkmateIsPossible (board) == Material::CheckmateIsPossible::No)
            return DrawCategory::InsufficientMaterial;

        return DrawCategory::NoDraw;
    }

    // Evaluate the board.
    [[nodiscard]] auto 
    evaluate (const Board& board, Color who, int moves_away) 
        -> int;

    // When there are no legal moves present, return the score of this move, which
    // checks for either a stalemate or checkmate position.
    [[nodiscard]] auto 
    evaluateWithoutLegalMoves (const Board& board, Color who, int moves_away) 
        -> int;

    // Get the score for a checkmate discovered X moves away.
    // Checkmates closer to the current position are more valuable than those
    // further away. Uses linear scoring for correct transposition table adjustment.
    constexpr auto
    checkmateScoreInMoves (int moves)
        -> int
    {
        return Checkmate_Score - moves;
    }

    // Whether the score indicates a checkmate of the opponent has been found.
    constexpr auto
    isCheckmatingOpponentScore (int score)
        -> bool
    {
        return score > Max_Non_Checkmate_Score && score <= Checkmate_Score;
    }
}
