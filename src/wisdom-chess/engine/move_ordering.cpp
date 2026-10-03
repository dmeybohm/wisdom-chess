#include <algorithm>

#include "wisdom-chess/engine/move_ordering.hpp"

namespace wisdom
{
    void
    KillerTable::store (int ply, Move move) noexcept
    {
        ASSERT( ply >= 0 && ply < Max_Search_Depth );

        if (move.isAnyCapturing() || move.isPromoting())
            return;

        auto& slots = my_killers[narrow_cast<size_t> (ply)];

        if (slots[0] == move || slots[1] == move)
            return;

        slots[1] = slots[0];
        slots[0] = move;
    }

    void
    CutoffHistory::store (Color who, Move move, int depth) noexcept
    {
        ASSERT( depth > 0 && depth <= Max_Search_Depth );

        if (move.isAnyCapturing() || move.isPromoting())
            return;

        auto& score = my_scores[narrow_cast<size_t> (colorIndex (who))]
            [narrow_cast<size_t> (move.getSrc().index())]
            [narrow_cast<size_t> (move.getDst().index())];

        score = std::min (score + depth * depth, Max_Score);
    }

    auto
    CutoffHistory::getScore (Color who, Move move) const noexcept
        -> int32_t
    {
        return my_scores[narrow_cast<size_t> (colorIndex (who))]
            [narrow_cast<size_t> (move.getSrc().index())]
            [narrow_cast<size_t> (move.getDst().index())];
    }

    auto
    KillerTable::getKillers (int ply) const noexcept
        -> KillerMoves
    {
        ASSERT( ply >= 0 && ply < Max_Search_Depth );
        return my_killers[narrow_cast<size_t> (ply)];
    }
}
