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

    [[nodiscard]] static auto 
    dualBishopsAreTheSameColor (const Board& board)
        -> Material::CheckmateIsPossible
    {
        auto first_coord = board.findFirstCoordWithPiece (Piece::Bishop);
        assert (first_coord.has_value());

        auto starting_at = nextCoord (*first_coord);
        assert (starting_at.has_value());

        auto second_coord = board.findFirstCoordWithPiece (Piece::Bishop, *starting_at);
        assert (second_coord.has_value());

        return (coordColor (*first_coord) == coordColor (*second_coord))
            ? Material::CheckmateIsPossible::No
            : Material::CheckmateIsPossible::Yes;
    }

    auto Material::checkInsufficientMaterialScenarios (const Board& board) const
        -> CheckmateIsPossible
    {
        auto white_knight_count = pieceCount (Color::White, Piece::Knight);
        auto black_knight_count = pieceCount (Color::Black, Piece::Knight);
        auto white_bishop_count = pieceCount (Color::White, Piece::Bishop);
        auto black_bishop_count = pieceCount (Color::Black, Piece::Bishop);

        auto knight_sum = black_knight_count + white_knight_count;
        auto bishop_sum = black_bishop_count + white_bishop_count;

        auto minor_piece_sum = knight_sum + bishop_sum;

        // King and King:
        if (minor_piece_sum == 0)
            return CheckmateIsPossible::No;

        if (bishop_sum == 0)
        {
            // King and knight vs King:
            if (knight_sum == 1)
                return CheckmateIsPossible::No;
        }

        if (knight_sum == 0)
        {
            // King and bishop vs King:
            if (bishop_sum == 1)
                return CheckmateIsPossible::No;

            // King and bishop vs King and bishop with opposite colored bishops, 
            // or King and two bishops vs King;
            if (bishop_sum == 2)
                return dualBishopsAreTheSameColor (board);
        }

        return CheckmateIsPossible::Yes;
    }
}
