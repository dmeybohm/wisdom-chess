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
    CHECK( castled_result == 0);
}
