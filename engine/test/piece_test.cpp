#include <doctest/doctest.h>

#include "piece.hpp"

#include <iostream>

using std::vector;
using namespace wisdom;

TEST_CASE( "A piece can be converted" )
{
    vector<Color> colors {
        Color::White, Color::Black
    };
    vector<Piece> pieces {
        Piece::Pawn, Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen, Piece::King
    };

    for (auto color : colors) {
        for (auto piece : pieces) {
            ColoredPiece combined = make_piece (color, piece);
            INFO( to_string(piece_type(combined)) );
            INFO( to_string(piece) );
            CHECK( piece_type(combined) == piece );
            CHECK( piece_color(combined) == color );
        }
    }
    CHECK( Piece_And_Color_None == make_piece (Color::None, Piece::None) );
    CHECK( piece_type (Piece_And_Color_None) == Piece::None );
    CHECK( piece_color (Piece_And_Color_None) == Color::None );
}

TEST_CASE( "Color invert" )
{
    CHECK( color_invert (Color::White) == Color::Black );
    CHECK( color_invert (Color::Black) == Color::White );
}
