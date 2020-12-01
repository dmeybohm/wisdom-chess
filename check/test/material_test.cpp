#include "catch.hpp"

#include "material.h"

TEST_CASE( "Adding material works", "[material]" )
{
    auto piece_type = GENERATE (PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN, PIECE_PAWN);
    auto color = GENERATE (COLOR_WHITE, COLOR_BLACK);

    struct material my_material;

    my_material.add (MAKE_PIECE (color, piece_type));

    CHECK( (color_index(color) == 0 || color_index(color) == 1) );
    CHECK( my_material.score(color) > 0 );
}

TEST_CASE( "Deleting material works", "[material]" )
{
    auto piece_type = GENERATE (PIECE_BISHOP, PIECE_ROOK, PIECE_QUEEN, PIECE_PAWN);
    auto color = GENERATE (COLOR_WHITE, COLOR_BLACK);

    struct material my_material;

    my_material.remove (MAKE_PIECE (color, piece_type));

    CHECK( (color_index(color) == 0 || color_index(color) == 1) );
    CHECK( my_material.score(color) < 0 );
}