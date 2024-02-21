#pragma once

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    struct MoveGeneration;

    class MoveGenerator final
    {
    private:
        mutable array<unique_ptr<MoveList>, Num_Squares> my_knight_moves {};

        void knightMoveListInit() const;

        [[nodiscard]] auto generateKnightMoves (int row, int col) const -> const MoveList&;

    public:
        MoveGenerator() = default;

        [[nodiscard]] auto generateAllPotentialMoves (const Board& board, Color who) const
            -> MoveList;

        [[nodiscard]] auto generateLegalMoves (const Board& board, Color who) const
            -> MoveList;

        friend class MoveGeneration;
    };

    [[nodiscard]] auto needPawnPromotion (int row, Color who)
        -> bool;

    // Return en passant column if the board is the player is eligible.
    [[nodiscard]] auto eligibleEnPassantColumn (const Board& board,
                                                int row, int column, Color who)
        -> optional<int>;
}

