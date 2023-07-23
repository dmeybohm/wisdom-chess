#include "material.hpp"
#include "board.hpp"

namespace wisdom
{
    Material::Material (const wisdom::Board& board)
    {
        for (auto&& coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (piece != Piece_And_Color_None)
                this->add (board.pieceAt (coord));
        }
    }

    static auto dualBishopsAreTheSameColor (const Board& board,
                                                 Color first_bishop_color, Color second_bishop_color)
        -> Material::CheckmateIsPossible
    {
        auto first_bishop = ColoredPiece::make (first_bishop_color, Piece::Bishop);
        auto second_bishop = ColoredPiece::make (second_bishop_color, Piece::Bishop);
        auto first_coord = board.findFirstCoordWithPiece (first_bishop);

        auto starting_at = first_bishop_color == second_bishop_color ? nextCoord (*first_coord, +1) : First_Coord;
        assert (starting_at.has_value());

        auto second_coord = board.findFirstCoordWithPiece (second_bishop, *starting_at);

        assert (first_coord.has_value());
        assert (second_coord.has_value());

        return (coordColor (*first_coord) == coordColor (*second_coord)) ?
            Material::CheckmateIsPossible::No :
            Material::CheckmateIsPossible::Yes;
    }

    auto Material::checkInsufficientMaterialScenarios (const Board& board) const -> CheckmateIsPossible
    {
        auto white_knight_count = pieceCount (Color::White, Piece::Knight);
        auto black_knight_count = pieceCount (Color::Black, Piece::Knight);
        auto white_bishop_count = pieceCount (Color::White, Piece::Bishop);
        auto black_bishop_count = pieceCount (Color::Black, Piece::Bishop);

        // King and King:
        if (white_knight_count + black_knight_count +
                white_bishop_count + black_bishop_count == 0)
        {
            return CheckmateIsPossible::No;
        }

        if (white_bishop_count == 0 && black_bishop_count == 0)
        {
            // King and knight vs King:
            if ((black_knight_count == 1 && white_knight_count == 0) ||
                (white_knight_count == 1 && black_knight_count == 0))
            {
                return CheckmateIsPossible::No;
            }
        }

        if (white_knight_count == 0 && black_knight_count == 0)
        {
            // King and bishop vs King:
            if ((black_bishop_count == 0 && white_bishop_count == 1) ||
                (white_bishop_count == 0 && black_bishop_count == 1))
            {
                return CheckmateIsPossible::No;
            }

            // King and bishop vs King and bishop with opposite colored bishops:
            if (black_bishop_count == 1 && white_bishop_count == 1)
            {
                return dualBishopsAreTheSameColor (board, Color::White, Color::Black);
            }

            //
            // King and two bishops vs King;
            //
            if (black_bishop_count == 2 && white_bishop_count == 0)
            {
                return dualBishopsAreTheSameColor (board, Color::Black, Color::Black);
            }
            if (white_bishop_count == 2 && black_bishop_count == 0)
            {
                return dualBishopsAreTheSameColor (board, Color::White, Color::White);
            }
        }

        return CheckmateIsPossible::Yes;
    }
}
