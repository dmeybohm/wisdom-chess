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

    auto
    KillerTable::getKillers (int ply) const noexcept
        -> KillerMoves
    {
        ASSERT( ply >= 0 && ply < Max_Search_Depth );
        return my_killers[narrow_cast<size_t> (ply)];
    }
}
