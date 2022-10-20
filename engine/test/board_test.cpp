#include "tests.hpp"
#include "board.hpp"
#include "piece.hpp"
#include "board_builder.hpp"

using namespace wisdom;

TEST_CASE( "Pawn direction is negative for white and positive for black" )
{
    CHECK( pawn_direction (Color::White) == -1 );
    CHECK( pawn_direction (Color::Black) == +1 );
}

TEST_CASE( "find_first_coord_with_piece()" )
{
    BoardBuilder builder;

    builder.add_piece ("e1", Color::White, Piece::King);
    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("a2", Color::White, Piece::Pawn);
    builder.add_piece ("a3", Color::White, Piece::Pawn);
    builder.add_piece ("a7", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    SUBCASE( "Returns the first position if there is only one position with the combo" )
    {
        auto white_king = ColoredPiece::make (Color::White, Piece::King);
        auto black_king = ColoredPiece::make (Color::Black, Piece::King);
        auto black_pawn = ColoredPiece::make (Color::Black, Piece::Pawn);
        auto white_king_pos = board.find_first_coord_with_piece (white_king);
        auto black_king_pos = board.find_first_coord_with_piece (black_king);
        auto black_pawn_pos = board.find_first_coord_with_piece (black_pawn);

        auto expected_white_king_pos = coord_parse ("e1");
        auto expected_black_king_pos= coord_parse("e8");
        auto expected_black_pawn_pos = coord_parse ("a7");

        CHECK( *white_king_pos == expected_white_king_pos );
        CHECK( *black_king_pos == expected_black_king_pos );
        CHECK( *black_pawn_pos == expected_black_pawn_pos );
    }

    SUBCASE( "Returns the first position if there are multiple positions with the same combo" )
    {
        auto white_pawn = ColoredPiece::make (Color::White, Piece::Pawn);
        auto white_pawn_pos = board.find_first_coord_with_piece (white_pawn);
        auto expected_white_pawn_pos = coord_parse ("a3");
        CHECK( *white_pawn_pos == expected_white_pawn_pos );
    }

    SUBCASE( "Returns nullopt if no piece is found" )
    {
        auto white_queen = ColoredPiece::make (Color::White, Piece::Queen);
        auto white_queen_pos = board.find_first_coord_with_piece (white_queen);
        CHECK( !white_queen_pos.has_value () );
    }
}

TEST_CASE( "coord_color()" )
{
    auto top_left = make_coord (First_Row, First_Column);
    auto bottom_right = make_coord (Last_Row, Last_Column);
    auto top_right = make_coord (First_Row, Last_Column);
    auto bottom_left = make_coord (Last_Row, First_Column);

    CHECK( coord_color (top_left) == Color::White );
    CHECK( coord_color (bottom_right) == Color::White );
    CHECK( coord_color (top_right) == Color::Black );
    CHECK( coord_color (bottom_left) == Color::Black );

    auto d5 = coord_parse ("d5");
    auto e5 = coord_parse ("e5");
    auto d4 = coord_parse ("d4");
    auto e4 = coord_parse ("e4");

    CHECK( coord_color (d5) == Color::White );
    CHECK( coord_color (e4) == Color::White );
    CHECK( coord_color (e5) == Color::Black );
    CHECK( coord_color (d4) == Color::Black );
}
