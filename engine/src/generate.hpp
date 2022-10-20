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
        unique_ptr<MoveListAllocator> my_move_list_allocator = MoveListAllocator::make_unique ();
        array<unique_ptr<MoveList>, Num_Squares> my_knight_moves {};

        void knight_move_list_init ();

        [[nodiscard]] auto generate_knight_moves (int row, int col) -> const MoveList&;

    public:
        MoveGenerator () = default;

        [[nodiscard]] auto generate_all_potential_moves (const Board& board, Color who)
            -> MoveList;

        [[nodiscard]] auto generate_legal_moves (Board& board, Color who)
            -> MoveList;

        friend class MoveGeneration;
    };

    [[nodiscard]] auto need_pawn_promotion (int row, Color who)
        -> bool;

    [[nodiscard]] auto eligible_en_passant_column (const Board& board,
                                                   int row, int column, Color who)
        -> int;
}

#endif // WISDOM_CHESS_GENERATE_HPP
