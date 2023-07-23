#ifndef WISDOM_CHESS_GENERATE_HPP
#define WISDOM_CHESS_GENERATE_HPP

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    struct ScoredMove
    {
        Move move;
        int score;
    };

    static_assert(std::is_trivial<ScoredMove>::value);

    struct MoveGeneration;

    class MoveGenerator final
    {
    private:
        mutable unique_ptr<MoveListAllocator> my_move_list_allocator = MoveListAllocator::makeUnique();
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

#endif // WISDOM_CHESS_GENERATE_HPP
