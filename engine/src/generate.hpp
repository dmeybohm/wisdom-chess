#pragma once

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    struct MoveGeneration;

    using KnightMoveList = array<Move, 8>;
    using KnightMoveLists = array<KnightMoveList, Num_Squares>;
    using KnightMoveSizes = array<size_t, Num_Squares>;

    namespace
    {
        consteval auto
        absoluteValue (auto integer)
            -> decltype (integer)
        {
            static_assert (std::is_integral_v<decltype (integer)>);
            return integer < 0 ? integer * -1 : integer;
        }

        consteval auto
        knightMoveListInit()
            -> pair<KnightMoveLists, KnightMoveSizes>
        {
            KnightMoveLists result {};
            KnightMoveSizes sizes {};

            for (auto coord : CoordIterator {})
            {
                auto row = coord.row<int>();
                auto col = coord.column<int>();

                for (int k_row = -2; k_row <= 2; k_row++)
                {
                    if (!k_row)
                        continue;

                    if (!isValidRow (k_row + row))
                        continue;

                    for (auto k_col = 3 - absoluteValue (k_row);
                         k_col >= -2;
                         k_col -= 2 * absoluteValue (k_col))
                    {
                        if (!isValidColumn (k_col + col))
                            continue;

                        Move knight_move = Move::make (
                            k_row + row,
                            k_col + col,
                            row,
                            col
                        );
                        auto index = knight_move.getSrc().index();

                        auto& size_ref  = sizes[index];
                        auto& array_ref = result[index];
                        array_ref[size_ref] = knight_move;
                        size_ref++;
                    }
                }
            }
            return { result, sizes };
        }
    }

    class MoveGenerator final
    {
    public:
        MoveGenerator() = default;

        [[nodiscard]] auto
        generateAllPotentialMoves (const Board& board, Color who) const
            -> MoveList;

        [[nodiscard]] auto
        generateLegalMoves (const Board& board, Color who) const
            -> MoveList;

        friend class MoveGeneration;

    private:
        // Store a list of knight moves and their sizes, generated at
        // compile-time:
        static constexpr pair<KnightMoveLists, KnightMoveSizes> Knight_Moves =
            knightMoveListInit();

        // Get a std::span of the knight move list and the compile-time
        // calculated length:
        [[nodiscard]] static auto
        getKnightMoveList (int row, int col)
            -> span<const Move>
        {
            auto coord = Coord::make (row, col);
            const auto square = coord.index();

            const auto& list = Knight_Moves.first[square];
            const auto size = Knight_Moves.second[square];

            return { list.data(), size };
        }
    };

    [[nodiscard]] auto
    needPawnPromotion (int row, Color who)
        -> bool;

    // Return en passant column if the board is the player is eligible.
    [[nodiscard]] auto
    eligibleEnPassantColumn (const Board& board, int row, int column, Color who)
        -> optional<int>;
}

