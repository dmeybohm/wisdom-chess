#include <doctest/doctest.h>

#include "material.hpp"

using std::vector;
using namespace wisdom;

TEST_CASE( "Adding material works" )
{
    vector piece_types  { Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn };
    vector colors { Color::White, Color::Black };

    for (auto piece_type : piece_types)
    {
        for (auto color : colors)
        {
            Material material;

            material.add (make_piece (color, piece_type));

            CHECK( (color_index(color) == 0 || color_index(color) == 1) );
            CHECK( material.score(color) > 0 );
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
            Material material;

            material.remove (make_piece (color, piece_type));

            CHECK( (color_index (color) == 0 || color_index (color) == 1) );
            CHECK( material.score (color) < 0 );
        }
    }
}