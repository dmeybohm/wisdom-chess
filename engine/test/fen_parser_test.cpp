#include <doctest/doctest.h>
#include "game.hpp"
#include "board.hpp"
#include "fen_parser.hpp"

#include <cstring>

using namespace wisdom;

TEST_CASE( "FEN notation for the starting position" )
{
    FenParser parser { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

    Game game = parser.build();
    Board default_board;

    CHECK( board_equals (game.get_board (), default_board) );

    int castled_result = std::memcmp (game.get_board ().castled,
                                      default_board.castled, sizeof (default_board.castled));
    CHECK( castled_result == 0 );
}

TEST_CASE( "FEN notation for non-starting position" )
{
    FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w K - 0 1" };

    Game game  = parser.build();

    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::Rook);
    builder.add_piece ("a2", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);
    builder.add_piece ("h1", Color::White, Piece::Rook);

    auto expected = builder.build();
    CHECK( board_equals (game.get_board (), *expected) );

    CastlingState white_state = board_get_castle_state (game.get_board (), Color::White);
    CastlingState black_state = board_get_castle_state (game.get_board (), Color::Black);
    CastlingState exp_white_state = board_get_castle_state (*expected, Color::White);
    CastlingState exp_black_state = board_get_castle_state (*expected, Color::Black);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling" )
{
    FenParser parser_full { "4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1" };

    Game game = parser_full.build();

    REQUIRE(game.get_board ().castled[Color_Index_White] == Castle_None);
    REQUIRE(game.get_board ().castled[Color_Index_Black] == Castle_None);

    FenParser parser_no_black_king { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black_king.build();

    REQUIRE(game.get_board ().castled[Color_Index_White] == Castle_None);
    REQUIRE(game.get_board ().castled[Color_Index_Black] == Castle_Kingside);

    FenParser parser_no_black { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black.build();

    REQUIRE(game.get_board ().castled[Color_Index_White] == Castle_None);
    REQUIRE(game.get_board ().castled[Color_Index_Black] == Castle_Kingside);

    FenParser parser_nothing { "4r2/8/8/8/8/8/k7/4K2R w - - 0 1" };

    game = parser_nothing.build();

    REQUIRE(game.get_board ().castled[Color_Index_White] == (Castle_Kingside | Castle_Queenside));
    REQUIRE(game.get_board ().castled[Color_Index_Black] == (Castle_Kingside | Castle_Queenside));
}

TEST_CASE( "FEN notation for en passant" )
{
    FenParser parser_with_black_target { "4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1"};

    Game game = parser_with_black_target.build();

    REQUIRE( !is_en_passant_vulnerable (game.get_board (), Color::White) );
    REQUIRE( is_en_passant_vulnerable (game.get_board (), Color::Black) );
    REQUIRE( game.get_board ().en_passant_target[Color_Index_Black] == coord_parse ("e6") );
}