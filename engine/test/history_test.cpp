#include <doctest/doctest.h>

#include "move_list.hpp"
#include "history.hpp"
#include "board.hpp"
#include "move.hpp"
#include "board_builder.hpp"

using namespace wisdom;

TEST_CASE( "Third repetition is detected" )
{
    History history;
    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    auto board = builder.build();

    Move black_move = move_parse ("e8 d8");
    Move black_return_move = move_parse ("d8 e8");

    Move white_move = move_parse ("e1 d1");
    Move white_return_move = move_parse ("d1 e1");

    // Record initial position. we don't care about move here.
    Move initial_move = move_parse ("d8 e8");
    history.add_position_and_move (*board, initial_move);

    board->make_move (Color::White, white_move);
    history.add_position_and_move (*board, white_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::Black, black_move);
    history.add_position_and_move (*board, black_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::White, white_return_move);
    history.add_position_and_move (*board, white_return_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::Black, black_return_move);
    history.add_position_and_move (*board, black_return_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::White, white_move);
    history.add_position_and_move (*board, white_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::Black, black_move);
    history.add_position_and_move (*board, black_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::White, white_return_move);
    history.add_position_and_move (*board, white_return_move);
    REQUIRE( history.is_third_repetition (*board) == false );

    board->make_move (Color::Black, black_return_move);
    history.add_position_and_move (*board, black_return_move);
    REQUIRE( history.is_third_repetition (*board) == true );
}

TEST_CASE( "Fifty move repetition is detected" )
{
    History history;
    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    auto board = builder.build();

    Move black_move = move_parse ("e8 d8");
    Move black_return_move = move_parse ("d8 e8");

    Move white_move = move_parse ("e1 d1");
    Move white_return_move = move_parse ("d1 e1");

    for (auto i = 0; i < 24; i++)
    {
        board->make_move (Color::White, white_move);
        REQUIRE( History::is_fifty_move_repetition (*board) == false );
        board->make_move (Color::Black, black_move);
        REQUIRE( History::is_fifty_move_repetition (*board) == false );
        board->make_move (Color::White, white_return_move);
        REQUIRE( History::is_fifty_move_repetition (*board) == false );
        board->make_move (Color::Black, black_return_move);
        REQUIRE( History::is_fifty_move_repetition (*board) == false );
    }

    board->make_move (Color::White, white_move);
    REQUIRE( History::is_fifty_move_repetition (*board) == false );
    board->make_move (Color::Black, black_move);
    REQUIRE( History::is_fifty_move_repetition (*board) == false );
    board->make_move (Color::White, white_return_move);
    REQUIRE( History::is_fifty_move_repetition (*board) == false );

    board->make_move (Color::Black, black_return_move);
    REQUIRE( History::is_fifty_move_repetition (*board) == true );
}
