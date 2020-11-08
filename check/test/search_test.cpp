#include "catch.hpp"
#include "board_builder.hpp"

extern "C"
{
#include "board.h"
#include "search.h"
#include "move.h"
#include "timer.h"
#include "move_tree.h"
}

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE( "Can find mate in 3", "[search]" )
{
    board_builder builder;
    struct timer large_timer {};
    timer_init (&large_timer, 30);

    builder.add_piece ("a8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece ("g8", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece ("h8", COLOR_BLACK, PIECE_KING);

    builder.add_piece ("f7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("h7", COLOR_BLACK, PIECE_PAWN);

    builder.add_piece ("f6", COLOR_WHITE, PIECE_ROOK);
    builder.add_piece ("e5", COLOR_WHITE, PIECE_BISHOP);
    builder.add_piece ("h2", COLOR_WHITE, PIECE_PAWN);
    builder.add_piece ("h1", COLOR_WHITE, PIECE_KING);

    struct board *board = builder.build();

    move_t result;
    move_tree_t *variation = nullptr;
    int score = search (board, COLOR_WHITE, 4, 0, &result,
                        -INFINITE, INFINITE, 0,
                        &variation, 0, &large_timer, nullptr);

    CHECK( !move_equals (result, move_null ()) );

    int equal_result = move_equals (result, move_parse ("f6 a6", COLOR_WHITE));
    CHECK( equal_result != 0 );
    CHECK( score == INFINITE );
}
