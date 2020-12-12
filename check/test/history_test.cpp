#include "lib/catch/catch.hpp"

#include "move_list.hpp"
#include "history.hpp"
#include "board.h"
#include "move.h"
#include "board_builder.hpp"

TEST_CASE( "Third repetition is detected", "[history]" )
{
    class history history;
    board_builder builder;

    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    struct board board = builder.build();

    move_t black_move = parse_move ("e8 d8");
    move_t black_return_move = parse_move ("d8 e8");

    move_t white_move = parse_move ("e1 d1");
    move_t white_return_move = parse_move ("d1 e1");

    // Record initial position. we don't care about move here.
    history.add_position_and_move (board, null_move);

    do_move (board, Color::White, white_move);
    history.add_position_and_move (board, white_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::Black, black_move);
    history.add_position_and_move (board, black_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::White, white_return_move);
    history.add_position_and_move (board, white_return_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::Black, black_return_move);
    history.add_position_and_move (board, black_return_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::White, white_move);
    history.add_position_and_move (board, white_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::Black, black_move);
    history.add_position_and_move (board, black_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::White, white_return_move);
    history.add_position_and_move (board, white_return_move);
    REQUIRE( history.is_third_repetition (board) == false );

    do_move (board, Color::Black, black_return_move);
    history.add_position_and_move (board, black_return_move);
    REQUIRE( history.is_third_repetition (board) == true );
}

TEST_CASE( "Fifty move repetition is detected", "[history]" )
{
    class history history;
    board_builder builder;

    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    struct board board = builder.build();

    move_t black_move = parse_move ("e8 d8");
    move_t black_return_move = parse_move ("d8 e8");

    move_t white_move = parse_move ("e1 d1");
    move_t white_return_move = parse_move ("d1 e1");

    for (auto i = 0; i < 24; i++)
    {
        do_move (board, Color::White, white_move);
        REQUIRE( history::is_fifty_move_repetition(board) == false );
        do_move (board, Color::Black, black_move);
        REQUIRE( history::is_fifty_move_repetition(board) == false );
        do_move (board, Color::White, white_return_move);
        REQUIRE( history::is_fifty_move_repetition(board) == false );
        do_move (board, Color::Black, black_return_move);
        REQUIRE( history::is_fifty_move_repetition(board) == false );
    }

    do_move (board, Color::White, white_move);
    REQUIRE( history::is_fifty_move_repetition(board) == false );
    do_move (board, Color::Black, black_move);
    REQUIRE( history::is_fifty_move_repetition(board) == false );
    do_move (board, Color::White, white_return_move);
    REQUIRE( history::is_fifty_move_repetition(board) == false );

    do_move (board, Color::Black, black_return_move);
    REQUIRE( history::is_fifty_move_repetition(board) == true );

}
