#include "catch.hpp"
#include "board_builder.hpp"
#include "parse_simple_move.hpp"

#include "board.h"
#include "search.h"
#include "move.h"
#include "timer.h"
#include "move_tree.h"

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE( "Can find mate in 3", "[search]" )
{
    board_builder builder;
    struct timer large_timer {};
    timer_init (&large_timer, 30);

    builder.add_pieces (COLOR_BLACK, {
            {"a8", PIECE_ROOK},
            {"g8", PIECE_ROOK},
            {"h8", PIECE_KING},
            {"f7", PIECE_PAWN},
            {"h7", PIECE_PAWN},
    });
    builder.add_pieces (COLOR_WHITE, {
            {"f6", PIECE_ROOK},
            {"e5", PIECE_BISHOP},
            {"h2", PIECE_PAWN},
            {"h1", PIECE_KING},
    });

    struct board *board = builder.build();

    move_t result;
    move_tree_head variation;
    int score = search (board, COLOR_WHITE, 4, 4, &result,
                        -INITIAL_ALPHA, INITIAL_ALPHA, 0,
                        &variation.tree, 0, &large_timer, nullptr);

    REQUIRE( !move_equals (result, move_null ()) );
    REQUIRE( variation.size() == 5 );

    my_move_list expected_moves { COLOR_WHITE, {"f6 a6", "f7 f6", "e5xf6", "g8 g7", "a6xa8" }};
    my_move_list computed_moves { variation.tree };

    REQUIRE( expected_moves == computed_moves );
    REQUIRE( score > INFINITE );
}

//
// This position has multiple mating chances, as well as stalements, so this will test if the
// search can find the most efficient mate.
//
// Mating moves:
// ... Rd4+ 2. Ke5 f6#
// ... Bb7+ 2. Ke5 Re4#
//
TEST_CASE( "Can find mate in 2 1/2", "[search]" )
{
    board_builder builder;
    struct timer large_timer {};
    timer_init (&large_timer, 30);

    builder.add_pieces (COLOR_BLACK, {
            {"e8", PIECE_KNIGHT},
            {"c7", PIECE_KING},
            {"f7", PIECE_PAWN},
            {"a6", PIECE_PAWN},
            {"g6", PIECE_PAWN},
            {"c5", PIECE_PAWN},
            {"b4", PIECE_ROOK},
            {"b3", PIECE_KNIGHT},
    });

    builder.add_piece ("d5", COLOR_WHITE, PIECE_KING);

    struct board *board = builder.build();

    move_t result;
    move_tree_head variation;
    int score = search (board, COLOR_BLACK, 5, 5, &result,
                        -INITIAL_ALPHA, INITIAL_ALPHA, 0,
                        &variation.tree, 0, &large_timer, nullptr);

    REQUIRE( !move_equals (result, move_null ()) );
    REQUIRE( variation.size() == 5 );

    my_move_list expected_moves { COLOR_BLACK, {"e8 f6", "d5 e5", "f6 d7", "e5 d5", "b4 d4" }};
    my_move_list computed_moves { variation.tree };

    REQUIRE( expected_moves == computed_moves );
    REQUIRE( score > INFINITE );
}

TEST_CASE( "scenario with heap overflow 1", "[search-test]" )
{
    board_builder builder;

    builder.add_pieces (COLOR_BLACK, {
            {"c8", PIECE_ROOK},
            {"f8", PIECE_ROOK},
            {"h8", PIECE_KING}
    });
    builder.add_pieces (COLOR_BLACK, {
            {"c7", PIECE_PAWN},
            {"h7", PIECE_KNIGHT},
    });
    builder.add_pieces (COLOR_BLACK, {
            {"a6", PIECE_PAWN},
            {"c6", PIECE_BISHOP},
            {"b5", PIECE_PAWN},
            {"d5", PIECE_PAWN}
    });

    builder.add_pieces (COLOR_WHITE, {
            {"e5", PIECE_PAWN},
            {"a3", PIECE_KNIGHT},
            {"c3", PIECE_PAWN},
            {"e3", PIECE_PAWN},
            {"a2", PIECE_PAWN},
            {"b2", PIECE_PAWN},
            {"h2", PIECE_PAWN},
            {"b1", PIECE_KING},
            {"g1", PIECE_ROOK}
    });

    struct board *board = builder.build();

    struct timer timer {};
    timer_init (&timer, 300);
    move_t best_move = iterate (board, COLOR_BLACK, nullptr, &timer, 7);
    REQUIRE( !move_equals(best_move, null_move) );
}