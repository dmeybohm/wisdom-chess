#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/generate.hpp"

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

    // Whether the position is a draw by repetition or by the move count,
    // at the limits given, or by insufficient material. A checkmate takes
    // precedence over the move count.
    //
    // NOTE: this doesn't check for stalemate - that is evaluated through coming up empty
    // in the search process to efficiently overlap that processing which needs to occur anyway.
    [[nodiscard]] inline auto
    probableDrawCategory (const Board& board, const History& history, DrawLimits limits)
        -> DrawCategory
    {
        if (history.isProbablyNthRepetition (board, limits.repetitions))
            return DrawCategory::ByRepetition;

        if (History::hasBeenXHalfMovesWithoutProgress (board, limits.half_moves_without_progress))
            return isCheckmated (board) ? DrawCategory::NoDraw : DrawCategory::ByNoProgress;

        const auto& material_ref = board.getMaterial();

        if (material_ref.checkmateIsPossible (board) == Material::CheckmateIsPossible::No)
            return DrawCategory::InsufficientMaterial;

        return DrawCategory::NoDraw;
    }

    [[nodiscard]] inline auto
    isProbablyDrawingMove (const Board& board, const History& history, DrawLimits limits)
        -> bool
    {
        return probableDrawCategory (board, history, limits) != DrawCategory::NoDraw;
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
