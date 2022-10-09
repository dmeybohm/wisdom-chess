#include <doctest/doctest.h>
#include "game.hpp"
#include "board.hpp"
#include "fen_parser.hpp"

using namespace wisdom;

TEST_CASE( "FEN notation for the starting position" )
{
    FenParser parser { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

    Game game = parser.build ();
    Board default_board;

    CHECK( game.get_board () == default_board );

    auto default_black_state = default_board.get_castle_state (Color::Black);
    auto default_white_state = default_board.get_castle_state (Color::White);
    auto fen_white_state = game.get_board ().get_castle_state (Color::White);
    auto fen_black_state = game.get_board ().get_castle_state (Color::Black);

    CHECK( default_white_state == fen_white_state );
    CHECK( default_black_state == fen_black_state );
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

    auto expected = Board { builder };
    CHECK( game.get_board () == expected );

    CastlingState white_state = game.get_board ().get_castle_state (Color::White);
    CastlingState black_state = game.get_board ().get_castle_state (Color::Black);
    CastlingState exp_white_state = expected.get_castle_state (Color::White);
    CastlingState exp_black_state = expected.get_castle_state (Color::Black);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling" )
{
    FenParser parser_full { "4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1" };

    Game game = parser_full.build();

    REQUIRE(game.get_board ().get_castle_state (Color::White) == Castle_None);
    REQUIRE(game.get_board ().get_castle_state (Color::Black) == Castle_None);

    FenParser parser_no_black_king { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black_king.build();

    REQUIRE(game.get_board ().get_castle_state (Color::White) == Castle_None);
    REQUIRE(game.get_board ().get_castle_state (Color::Black) == Castle_Kingside);

    FenParser parser_no_black { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black.build();

    REQUIRE(game.get_board ().get_castle_state (Color::White) == Castle_None);
    REQUIRE(game.get_board ().get_castle_state (Color::Black) == Castle_Kingside);
    FenParser parser_nothing { "4r2/8/8/8/8/8/k7/4K2R w - - 0 1" };

    game = parser_nothing.build();

    REQUIRE(game.get_board ().get_castle_state (Color::White) == (Castle_Kingside | Castle_Queenside));
    REQUIRE(game.get_board ().get_castle_state (Color::Black) == (Castle_Kingside | Castle_Queenside));
}

TEST_CASE( "FEN notation for en passant" )
{
    FenParser parser_with_black_target { "4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1"};

    Game game = parser_with_black_target.build();
    auto &board = game.get_board ();

    REQUIRE( !board.is_en_passant_vulnerable (Color::White) );
    REQUIRE( board.is_en_passant_vulnerable (Color::Black) );

    auto black_target = board.get_en_passant_target (Color::Black);
    REQUIRE( black_target == coord_parse ("e6") );
}

TEST_CASE( "Parsing half and full moves")
{
    SUBCASE( "With castling and en passant square" )
    {
        FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w Kk e6 10 5" };

        Game game = parser.build ();
        const auto& board = game.get_board ();
        CHECK (board.get_half_move_clock () == 10);
        CHECK (board.get_full_move_clock () == 5);
    }

    SUBCASE( "Without castling or en passant square" )
    {
        FenParser parser { "4r2/8/8/8/8/8/k7/4K2R w - - 10 5" };

        Game game = parser.build ();
        const auto& board = game.get_board ();
        CHECK (board.get_half_move_clock () == 10);
        CHECK (board.get_full_move_clock () == 5);
    }
}