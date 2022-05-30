#include "material.hpp"
#include "board.hpp"

namespace wisdom
{
    Material::Material (const wisdom::Board& board)
    {
        int8_t row, col;

        FOR_EACH_ROW_AND_COL(row, col)
        {
            auto piece = board.piece_at (row, col);
            if (piece != Piece_And_Color_None)
                this->add (board.piece_at (row, col));
        }
    }

    auto Material::check_detailed_sufficient_material_scenarios () const -> bool
    {
        return false;
    }
}
