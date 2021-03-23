#include <catch/catch.hpp>

#include "piece.h"

TEST_CASE( "A piece can be converted", "[piece]" )
{
    auto color = GENERATE (Color::None, Color::White, Color::Black);
    auto piece = GENERATE (Piece::None, Piece::Pawn, Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen, Piece::King);

    ColoredPiece combined = make_piece (color, piece);
    CHECK(piece_type (combined) == piece );
    CHECK(piece_color (combined) == color );
}