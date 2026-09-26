#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/threats.hpp"

namespace wisdom
{
    class Board;

    enum class DrawCategory
    {
        NoDraw,
        InsufficientMaterial,
        ByRepetition,
        ByNoProgress
    };

    // Whether the position reached by `who` playing `mv` is legal: the
    // mover's king is not attacked, and a castling king did not start
    // in, or pass through, check.
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

    // Whether the player to move is checkmated.
    [[nodiscard]] auto
    isCheckmated (const Board& board)
        -> bool;

    // Whether the player to move is stalemated.
    [[nodiscard]] auto
    isStalemated (const Board& board)
        -> bool;

    // Whether the position is, or can be claimed as, a draw by repetition,
    // by the fifty-move rule or by insufficient material.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    [[nodiscard]] inline auto
    isProbablyDrawingMove (const Board& board, const History& history)
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

    // Evaluate the board without testing whether the side to move is
    // checkmated, for callers that already know it is not.
    [[nodiscard]] auto
    evaluateWithoutMateTest (const Board& board, Color who)
        -> int;

    // When there are no legal moves present, return the score of this move, which
    // checks for either a stalemate or checkmate position.
    [[nodiscard]] auto 
    evaluateWithoutLegalMoves (const Board& board, Color who, int moves_away) 
        -> int;

    // Get the score for a checkmate discovered X moves away.
    // Checkmates closer to the current position are more valuable than those
    // further away. Uses linear scoring for correct transposition table adjustment.
    [[nodiscard]] constexpr auto
    checkmateScoreInMoves (int moves)
        -> int
    {
        return Checkmate_Score - moves;
    }

    // Whether the score indicates a checkmate of the opponent has been found.
    [[nodiscard]] constexpr auto
    isCheckmatingOpponentScore (int score)
        -> bool
    {
        return score > Max_Non_Checkmate_Score && score <= Checkmate_Score;
    }
}
