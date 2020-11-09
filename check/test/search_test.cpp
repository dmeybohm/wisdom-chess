#include "catch.hpp"
#include "board_builder.hpp"
#include "parse_simple_move.hpp"

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
    move_tree_destroy (variation);

    CHECK( !move_equals (result, move_null ()) );

    int equal_result = move_equals (result, parse_simple_move("f6 a6"));
    CHECK( equal_result != 0 );
    CHECK( score == INFINITE );
}

//
// This position has multiple mating chances, as well as stalements, so this will test if the
// search can find the most efficient mate.
//
// Mating moves:
// ... Rd4+ 2. Ke5 f6#
// ... Bb7+ 2. Ke5 Re4#
//
TEST_CASE( "Can find mate in 1 1/2", "[search]" )
{
    board_builder builder;
    struct timer large_timer {};
    timer_init (&large_timer, 30);

    builder.add_piece ("e8", COLOR_BLACK, PIECE_KNIGHT);
    builder.add_piece ("c7", COLOR_BLACK, PIECE_KING);
    builder.add_piece ("f7", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("a6", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("g6", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("c5", COLOR_BLACK, PIECE_PAWN);
    builder.add_piece ("b4", COLOR_BLACK, PIECE_ROOK);
    builder.add_piece ("b3", COLOR_BLACK, PIECE_KNIGHT);

    builder.add_piece ("d5", COLOR_WHITE, PIECE_KING);

    struct board *board = builder.build();

    move_t result;
    move_tree_t *variation = nullptr;
    int score = search (board, COLOR_BLACK, 9, 0, &result,
                        -INFINITE, INFINITE, 0,
                        &variation, 0, &large_timer, nullptr);

    print_reverse_recur (variation);
    move_tree_destroy (variation);
    CHECK( !move_equals (result, move_null ()) );

    // todo update for removal of bishop
    CHECK( (move_equals (result, parse_simple_move("b4 d4")) != 0 ||
        move_equals (result, parse_simple_move("c8 b7"))
   ));
    CHECK( score == INFINITE );
}
