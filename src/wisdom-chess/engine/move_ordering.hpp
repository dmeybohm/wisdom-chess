#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/move.hpp"

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

    // What generateAllPotentialMoves() ranks ahead of its plain order.
    struct MoveOrdering
    {
        // The move to try first, usually the transposition table's.
        optional<Move> priority_move;

        // Tried after the captures and promotions, in slot order.
        KillerMoves killers;
    };
}
