#include <iostream>

#include "wisdom-chess/engine/piece.hpp"

#include "wisdom-chess-tests.hpp"

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

    for (auto color : colors)
    {
        for (auto piece : pieces)
        {
            ColoredPiece combined = ColoredPiece::make (color, piece);
            INFO( asString (pieceType (combined)) );
            INFO( asString (piece) );
            CHECK( pieceType (combined) == piece );
            CHECK( pieceColor (combined) == color );
        }
    }
    CHECK( !optional<ColoredPiece> {}.has_value() );
    CHECK( !optional<Piece> {}.has_value() );
}

TEST_CASE( "Color invert" )
{
    CHECK( colorInvert (Color::White) == Color::Black );
    CHECK( colorInvert (Color::Black) == Color::White );
}

TEST_CASE( "colorBits() places each color above the piece type" )
{
    CHECK( colorBits (Color::White) == 0b01'000 );
    CHECK( colorBits (Color::Black) == 0b10'000 );

    CHECK( (colorBits (Color::White) & colorBits (Color::Black)) == 0 );
    CHECK( (colorBits (Color::White) & Piece_Type_Mask) == 0 );
    CHECK( (colorBits (Color::Black) & Piece_Type_Mask) == 0 );
}

namespace
{
    template <typename Target, typename Source>
    concept ConvertsToInt = requires (const Source& value) { to_int<Target> (value); };
}

TEST_CASE( "to_int gives a colored piece's packed value" )
{
    CHECK( to_int (ColoredPiece::make (Color::White, Piece::Pawn)) == 0b01'001 );
    CHECK( to_int (ColoredPiece::make (Color::Black, Piece::King)) == 0b10'110 );

    CHECK( to_int<int8_t> (ColoredPiece::make (Color::Black, Piece::King)) == 0b10'110 );

    CHECK( ConvertsToInt<int64_t, ColoredPiece> );
    CHECK_FALSE( ConvertsToInt<unsigned, ColoredPiece> );
    CHECK_FALSE( std::is_convertible_v<ColoredPiece, int> );
    CHECK_FALSE( std::is_constructible_v<bool, ColoredPiece> );
}
