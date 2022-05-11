#ifndef WISDOM_CHESS_GENERATE_HPP
#define WISDOM_CHESS_GENERATE_HPP

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"

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
        MoveListAllocator my_move_list_allocator;
        unique_ptr<MoveList[Num_Rows][Num_Columns]> my_knight_moves;

        auto generate_knight_moves (int row, int col) -> const MoveList&;

        auto get_knight_moves (int row, int col) -> const MoveList&;

    public:
        MoveGenerator ()
            : my_knight_moves { nullptr }
            , my_move_list_allocator {}
        {}

        auto generate_all_potential_moves (const Board& board, Color who) -> MoveList;

        auto generate_legal_moves (Board &board, Color who) -> MoveList;

        auto generate_captures (const Board& board, Color who) -> MoveList;

        friend class MoveGeneration;
    };

    auto need_pawn_promotion (int row, Color who) -> bool;

    auto eligible_en_passant_column (const Board& board, int row, int column, Color who)
        -> int;
}

#endif // WISDOM_CHESS_GENERATE_HPP
