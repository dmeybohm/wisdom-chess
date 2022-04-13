#include "material.hpp"
#include "board.hpp"

namespace wisdom
{
    Material::Material (const wisdom::Board& board)
    {
        int8_t row, col;

        FOR_EACH_ROW_AND_COL(row, col)
        {
            this->add (board.piece_at (row, col));
        }
    }
}
