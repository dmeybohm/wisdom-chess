#include "doctest/doctest.h"

#include "piece.h"

#include <vector>

using std::vector;

TEST_CASE( "A piece can be converted" )
{
   vector<Color> colors {
        Color::None, Color::White, Color::Black
    };
    vector<Piece> pieces {
        Piece::None, Piece::Pawn, Piece::Bishop, Piece::Knight, Piece::Rook, Piece::Queen, Piece::King
    };

    for (auto color : colors) {
        for (auto piece : pieces) {
            ColoredPiece combined = make_piece (color, piece);
            CHECK( piece_type(combined) == piece );
            CHECK( piece_color(combined) == color );
        }
    }
}