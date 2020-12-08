#include "catch.hpp"
#include "board_builder.hpp"
#include "parse_simple_move.hpp"

#include "board.h"
#include "search.h"
#include "move.h"
#include "move_timer.h"
#include "move_tree.h"
#include "output.hpp"
#include "history.hpp"

wisdom::null_output discard_output;

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE( "Can find mate in 3", "[search]" )
{
    board_builder builder;
    struct move_timer large_timer { 30 };

    builder.add_pieces (Color::Black, {
            {"a8", Piece::Rook},
            {"g8", Piece::Rook},
            {"h8", Piece::King},
            {"f7", Piece::Pawn},
            {"h7", Piece::Pawn},
    });
    builder.add_pieces (Color::White, {
            {"f6", Piece::Rook},
            {"e5", Piece::Bishop},
            {"h2", Piece::Pawn},
            {"h1", Piece::King},
    });

    struct board board = builder.build();
    std::unique_ptr<move_tree_t> variation;

    class history history;
    search_result_t result = search (board, Color::White, discard_output, history, large_timer, 4, 4,
                                     -INITIAL_ALPHA, INITIAL_ALPHA,
                                     variation);

    REQUIRE( result.move != null_move );
    REQUIRE( variation->size() == 5 );

    move_list_t expected_moves = { Color::White, {"f6 a6", "f7 f6", "e5xf6", "g8 g7", "a6xa8" }};
    move_list_t computed_moves = variation->to_list();

    REQUIRE( expected_moves == computed_moves );
    REQUIRE( result.score > INFINITE );
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
    move_timer large_timer { 10 };

    builder.add_pieces (Color::Black, {
            {"e8", Piece::Knight},
            {"c7", Piece::King},
            {"f7", Piece::Pawn},
            {"a6", Piece::Pawn},
            {"g6", Piece::Pawn},
            {"c5", Piece::Pawn},
            {"b4", Piece::Rook},
            {"b3", Piece::Knight},
    });

    builder.add_piece ("d5", Color::White, Piece::King);

    struct board board = builder.build();
    std::unique_ptr<move_tree_t> variation;
    class history history;
    search_result_t result = search (board, Color::Black, discard_output, history, large_timer,
                                     5, 5, -INITIAL_ALPHA, INITIAL_ALPHA,
                                     variation);

    REQUIRE( result.move != null_move );

    move_list_t expected_moves = { Color::Black, {"e8 f6", "d5 e5", "f6 d7", "e5 d5", "b4 d4" }};
    move_list_t computed_moves = variation->to_list();

    REQUIRE( variation->size() == 5 );
    REQUIRE( expected_moves == computed_moves );
    REQUIRE( result.score > INFINITE );
}

TEST_CASE( "scenario with heap overflow 1", "[search-test]" )
{
    board_builder builder;

    builder.add_pieces (Color::Black, {
            {"c8", Piece::Rook},
            {"f8", Piece::Rook},
            {"h8", Piece::King}
    });
    builder.add_pieces (Color::Black, {
            {"c7", Piece::Pawn},
            {"h7", Piece::Knight},
    });
    builder.add_pieces (Color::Black, {
            {"a6", Piece::Pawn},
            {"c6", Piece::Bishop},
            {"b5", Piece::Pawn},
            {"d5", Piece::Pawn}
    });

    builder.add_pieces (Color::White, {
            {"e5", Piece::Pawn},
            {"a3", Piece::Knight},
            {"c3", Piece::Pawn},
            {"e3", Piece::Pawn},
            {"a2", Piece::Pawn},
            {"b2", Piece::Pawn},
            {"h2", Piece::Pawn},
            {"b1", Piece::King},
            {"g1", Piece::Rook}
    });

    struct board board = builder.build();
    struct move_timer timer { 300 };
    class history history;
    move_t best_move = iterate (board, Color::Black, discard_output, history, timer, 3);
    REQUIRE( best_move != null_move );
}

TEST_CASE( "Promoting move is taken if possible", "[search-test]")
{
    board_builder builder;
    struct move_timer large_timer { 30 };

    builder.add_pieces (Color::Black, {
            {"d7", Piece::King},
            {"d2", Piece::Pawn}
    });
    builder.add_pieces (Color::White, {
            {"a4", Piece::King},
            {"h4", Piece::Pawn}
    });

    std::unique_ptr<move_tree_t> variation;
    class history history;
    struct board board = builder.build();
    search_result_t result = search (board, Color::Black, discard_output, history, large_timer,
                                     1, 1, -INITIAL_ALPHA, INITIAL_ALPHA,
                                     variation);

    REQUIRE( to_string(result.move) == "d2 d1(Q)" );
}