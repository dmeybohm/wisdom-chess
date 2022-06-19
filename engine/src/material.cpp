#include "material.hpp"
#include "board.hpp"

namespace wisdom
{
    Material::Material (const wisdom::Board& board)
    {
        for (auto&& coord : board.all_coords ())
        {
            auto piece = board.piece_at (coord);
            if (piece != Piece_And_Color_None)
                this->add (board.piece_at (coord));
        }
    }

    static auto dual_bishops_are_the_same_color (const Board& board,
                                                 Color first_bishop_color, Color second_bishop_color)
        -> bool
    {
        auto first_bishop = make_piece (first_bishop_color, Piece::Bishop);
        auto second_bishop = make_piece (second_bishop_color, Piece::Bishop);
        auto first_coord = board.find_first_coord_with_piece (first_bishop);

        auto starting_at = first_bishop_color == second_bishop_color ?
            next_coord (*first_coord, +1) : First_Coord;
        assert (starting_at.has_value ());

        auto second_coord = board.find_first_coord_with_piece (
            second_bishop, *starting_at);

        assert (first_coord.has_value ());
        assert (second_coord.has_value ());

        return (coord_color (*first_coord) == coord_color (*second_coord));
    }

    auto Material::check_insufficient_material_scenarios (const Board& board) const -> bool
    {
        auto white_knight_count = piece_count (Color::White, Piece::Knight);
        auto black_knight_count = piece_count (Color::Black, Piece::Knight);
        auto white_bishop_count = piece_count (Color::White, Piece::Bishop);
        auto black_bishop_count = piece_count (Color::Black, Piece::Bishop);

        // King and King:
        if (white_knight_count + black_knight_count +
                white_bishop_count + black_bishop_count == 0)
        {
            return true;
        }

        if (white_bishop_count == 0 && black_bishop_count == 0)
        {
            // King and knight vs King:
            if ((black_knight_count == 1 && white_knight_count == 0) ||
                (white_knight_count == 1 && black_knight_count == 0))
            {
                return true;
            }
        }

        if (white_knight_count == 0 && black_knight_count == 0)
        {
            // King and bishop vs King:
            if ((black_bishop_count == 0 && white_bishop_count == 1) ||
                (white_bishop_count == 0 && black_bishop_count == 1))
            {
                return true;
            }

            // King and bishop vs King and bishop with opposite colored bishops:
            if (black_bishop_count == 1 && white_bishop_count == 1)
            {
                return dual_bishops_are_the_same_color (board, Color::White, Color::Black);
            }

            //
            // King and two bishops vs King;
            //
            if (black_bishop_count == 2 && white_bishop_count == 0)
            {
                return dual_bishops_are_the_same_color (board, Color::Black, Color::Black);
            }
            if (white_bishop_count == 2 && black_bishop_count == 0)
            {
                return dual_bishops_are_the_same_color (board, Color::White, Color::White);
            }
        }

        return false;
    }
}
