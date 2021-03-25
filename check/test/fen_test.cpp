#include "doctest/doctest.h"
#include "game.h"
#include "board.h"
#include "fen.hpp"

#include <cstring>

using namespace wisdom;

TEST_CASE( "FEN notation for the starting position" )
{
    Fen parser {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

    Game game = parser.build();
    Board default_board;

    CHECK( board_equals (game.board, default_board) );

    int castled_result = std::memcmp (game.board.castled, default_board.castled, sizeof (default_board.castled));
    CHECK( castled_result == 0 );
}

TEST_CASE( "FEN notation for non-starting position" )
{
    Fen parser {"4r2/8/8/8/8/8/k7/4K2R w K - 0 1" };

    Game game  = parser.build();

    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::Rook);
    builder.add_piece ("a2", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);
    builder.add_piece ("h1", Color::White, Piece::Rook);

    Board expected = builder.build();
    CHECK( board_equals (game.board, expected) );

    CastlingState white_state = board_get_castle_state (game.board, Color::White);
    CastlingState black_state = board_get_castle_state (game.board, Color::Black);
    CastlingState exp_white_state = board_get_castle_state (expected, Color::White);
    CastlingState exp_black_state = board_get_castle_state (expected, Color::Black);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling" )
{
    Fen parser_full {"4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1" };

    Game game = parser_full.build();

    REQUIRE( game.board.castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game.board.castled[COLOR_INDEX_BLACK] == CASTLE_NONE );

    Fen parser_no_black_king {"4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black_king.build();

    REQUIRE( game.board.castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game.board.castled[COLOR_INDEX_BLACK] == CASTLE_KINGSIDE );

    Fen parser_no_black {"4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black.build();

    REQUIRE( game.board.castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game.board.castled[COLOR_INDEX_BLACK] == CASTLE_KINGSIDE );

    Fen parser_nothing {"4r2/8/8/8/8/8/k7/4K2R w - - 0 1" };

    game = parser_nothing.build();

    REQUIRE( game.board.castled[COLOR_INDEX_WHITE] == (CASTLE_KINGSIDE | CASTLE_QUEENSIDE) );
    REQUIRE( game.board.castled[COLOR_INDEX_BLACK] == (CASTLE_KINGSIDE | CASTLE_QUEENSIDE) );
}

TEST_CASE( "FEN notation for en passant" )
{
    Fen parser_with_black_target {"4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1"};

    Game game = parser_with_black_target.build();

    REQUIRE( !is_en_passant_vulnerable (game.board, Color::White) );
    REQUIRE( is_en_passant_vulnerable (game.board, Color::Black) );
    REQUIRE( game.board.en_passant_target[COLOR_INDEX_BLACK] == coord_parse("e6") );
}