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
            ColoredPiece combined = ColoredPiece::make (color, piece);
            INFO(asString (pieceType (combined)) );
            INFO(asString (piece) );
            CHECK( pieceType(combined) == piece );
            CHECK( pieceColor(combined) == color );
        }
    }
    CHECK( Piece_And_Color_None == ColoredPiece::make (Color::None, Piece::None) );
    CHECK( pieceType (Piece_And_Color_None) == Piece::None );
    CHECK( pieceColor (Piece_And_Color_None) == Color::None );
}

TEST_CASE( "Color invert" )
{
    CHECK( colorInvert (Color::White) == Color::Black );
    CHECK( colorInvert (Color::Black) == Color::White );
}
