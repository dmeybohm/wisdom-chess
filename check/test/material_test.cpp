#include "doctest/doctest.h"

#include "material.h"

#include <vector>
using std::vector;

TEST_CASE( "Adding material works" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    for (auto piece_type : piece_types)
    {
        for (auto color : colors)
        {
            Material my_material;

            my_material.add (make_piece (color, piece_type));

            CHECK( (color_index(color) == 0 || color_index(color) == 1) );
            CHECK( my_material.score(color) > 0 );
        }
    }
}

TEST_CASE( "Deleting material works" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    for (auto piece_type : piece_types)
    {
        for (auto color : colors)
        {
            struct Material my_material;

            my_material.remove (make_piece (color, piece_type));

            CHECK((color_index (color) == 0 || color_index (color) == 1));
            CHECK(my_material.score (color) < 0);
        }
    }
}