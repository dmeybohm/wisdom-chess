#include "catch.hpp"
#include "game.h"
#include "board.h"
#include "fen.hpp"

TEST_CASE( "FEN notation for the starting position", "[fen-test]" )
{
    fen parser { "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" };

    struct game *game = parser.build();
    struct board *default_board = board_new ();

    CHECK( board_equals (*game->board, *default_board) );

    int castled_result = memcmp (game->board->castled, default_board->castled, sizeof (*default_board->castled));
    CHECK( castled_result == 0 );
}

TEST_CASE( "FEN notation for non-starting position", "[fen-test]" )
{
    fen parser { "4r2/8/8/8/8/8/k7/4K2R w K - 0 1" };

    struct game *game = parser.build();

    board_builder builder;

    builder.add_piece ("e8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece ("a2", COLOR_BLACK, PIECE_KING);
    builder.add_piece ("e1", COLOR_WHITE, PIECE_KING);
    builder.add_piece ("h1", COLOR_WHITE, PIECE_ROOK);

    struct board *expected = builder.build();
    CHECK( board_equals (*game->board, *expected) );

    castle_state_t white_state = board_get_castle_state (game->board, COLOR_WHITE);
    castle_state_t black_state = board_get_castle_state (game->board, COLOR_BLACK);
    castle_state_t exp_white_state = board_get_castle_state (expected, COLOR_WHITE);
    castle_state_t exp_black_state = board_get_castle_state (expected, COLOR_BLACK);

    CHECK( white_state == exp_white_state );
    CHECK( black_state == exp_black_state );
}

TEST_CASE( "FEN notation for castling", "[fen-test]" )
{
    fen parser_full { "4r2/8/8/8/8/8/k7/4K2R w KQkq - 0 1" };

    struct game *game = parser_full.build();

    REQUIRE( game->board->castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game->board->castled[COLOR_INDEX_BLACK] == CASTLE_NONE );

    fen parser_no_black_king { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black_king.build();

    REQUIRE( game->board->castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game->board->castled[COLOR_INDEX_BLACK] == CASTLE_KINGSIDE );

    fen parser_no_black { "4r2/8/8/8/8/8/k7/4K2R w KQq - 0 1" };

    game = parser_no_black.build();

    REQUIRE( game->board->castled[COLOR_INDEX_WHITE] == CASTLE_NONE );
    REQUIRE( game->board->castled[COLOR_INDEX_BLACK] == CASTLE_KINGSIDE );

    fen parser_nothing { "4r2/8/8/8/8/8/k7/4K2R w - - 0 1" };

    game = parser_nothing.build();

    REQUIRE( game->board->castled[COLOR_INDEX_WHITE] == (CASTLE_KINGSIDE | CASTLE_QUEENSIDE) );
    REQUIRE( game->board->castled[COLOR_INDEX_BLACK] == (CASTLE_KINGSIDE | CASTLE_QUEENSIDE) );
}

TEST_CASE( "FEN notation for en passant", "[fen-test]" )
{
    fen parser_with_black_target {"4r2/8/8/8/8/8/k7/4K2R w KQkq e6 0 1"};

    struct game *game = parser_with_black_target.build();

    REQUIRE( !is_en_passant_vulnerable (game->board, COLOR_WHITE) );
    REQUIRE( is_en_passant_vulnerable (game->board, COLOR_BLACK) );
    REQUIRE( game->board->en_passant_target[COLOR_INDEX_BLACK] == coord_parse("e6") );
}