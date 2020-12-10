#include <catch/catch.hpp>

#include "material.h"

TEST_CASE( "Adding material works", "[material]" )
{
    auto piece_type = GENERATE (Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn);
    auto color = GENERATE (Color::White, Color::Black);

    struct material my_material;

    my_material.add (make_piece (color, piece_type));

    CHECK( (color_index(color) == 0 || color_index(color) == 1) );
    CHECK( my_material.score(color) > 0 );
}

TEST_CASE( "Deleting material works", "[material]" )
{
    auto piece_type = GENERATE (Piece::Bishop, Piece::Rook, Piece::Queen, Piece::Pawn);
    auto color = GENERATE (Color::White, Color::Black);

    struct material my_material;

    my_material.remove (make_piece (color, piece_type));

    CHECK( (color_index(color) == 0 || color_index(color) == 1) );
    CHECK( my_material.score(color) < 0 );
}