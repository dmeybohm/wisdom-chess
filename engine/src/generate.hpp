#pragma once

#include "global.hpp"
#include "move.hpp"
#include "board_code.hpp"
#include "move_list.hpp"

namespace wisdom
{
    struct MoveGeneration;

    struct KnightMoveList
    {
        size_t size;
        array<Move, 8> moves;
    };

    using KnightMoveLists = array<KnightMoveList, Num_Squares>;

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
            -> KnightMoveLists
        {
            KnightMoveLists result {};

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

                        auto& size_ref = result[index].size;
                        auto& array_ref = result[index].moves;
                        array_ref[size_ref] = knight_move;
                        size_ref++;
                    }
                }
            }
            return result;
        }
    }

    class MoveGenerator final
    {
        MoveGenerator() = default;

    public:
        [[nodiscard]] static auto
        generateAllPotentialMoves (const Board& board, Color who)
            -> MoveList;

        [[nodiscard]] static auto
        generateLegalMoves (const Board& board, Color who)
            -> MoveList;

        // Store a list of knight moves and their sizes, generated at
        // compile-time:
        static constexpr KnightMoveLists Knight_Moves =
            knightMoveListInit();

        // Get a std::span of the knight move list and the compile-time
        // calculated length:
        [[nodiscard]] static auto
        getKnightMoveList (int row, int col)
            -> span<const Move>
        {
            auto coord = Coord::make (row, col);
            const auto square = coord.index();

            const auto& list = Knight_Moves[square];
            return { list.moves.data(), list.size };
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

