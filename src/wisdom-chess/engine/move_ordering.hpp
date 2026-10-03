#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/piece.hpp"

namespace wisdom
{
    // The killer moves of one ply, most recent first.
    using KillerMoves = array<optional<Move>, 2>;

    // The quiet moves that caused a beta cutoff at each ply, so that the
    // sibling positions of the same ply can try them early.
    class KillerTable
    {
    public:
        // Record a cutoff by `move` at `ply`. A capture or a promotion is
        // not kept, since the sort already ranks it ahead. A move already
        // in a slot stays where it is.
        void store (int ply, Move move) noexcept;

        [[nodiscard]] auto
        getKillers (int ply) const noexcept
            -> KillerMoves;

    private:
        array<KillerMoves, Max_Search_Depth> my_killers {};
    };

    // How often each quiet move caused a beta cutoff, by side, source and
    // destination square, weighted by the depth left at the cutoff.
    class CutoffHistory
    {
    public:
        // A counter stops growing here instead of overflowing.
        static constexpr int32_t Max_Score = 1 << 30;

        // Record a cutoff by `move` with `depth` plies left. A capture or a
        // promotion is not counted.
        void store (Color who, Move move, int depth) noexcept;

        [[nodiscard]] auto
        getScore (Color who, Move move) const noexcept
            -> int32_t;

    private:
        using SquareScores = array<array<int32_t, Num_Squares>, Num_Squares>;

        array<SquareScores, Num_Players> my_scores {};
    };

    // What generateAllPotentialMoves() ranks ahead of its plain order.
    struct MoveOrdering
    {
        // The move to try first, usually the transposition table's.
        optional<Move> priority_move;

        // Tried after the captures and promotions, in slot order.
        KillerMoves killers;

        // Orders the other quiet moves, highest score first.
        nullable<const CutoffHistory> history;
    };
}
