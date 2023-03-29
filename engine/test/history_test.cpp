#include <doctest/doctest.h>
#include <iostream>

#include "move_list.hpp"
#include "history.hpp"
#include "board.hpp"
#include "move.hpp"
#include "board_builder.hpp"

using namespace wisdom;

TEST_CASE( "Third repetition is detected" )
{
    SUBCASE( "in the regular case" )
    {
        History history;
        BoardBuilder builder;

        builder.add_piece ("e8", Color::Black, Piece::King);
        builder.add_piece ("e1", Color::White, Piece::King);

        auto board = Board { builder };

        Move black_move = move_parse ("e8 d8");
        Move black_return_move = move_parse ("d8 e8");

        Move white_move = move_parse ("e1 d1");
        Move white_return_move = move_parse ("d1 e1");

        // Record initial position. we don't care about move here.
        Move initial_move = move_parse ("d8 e8");
        history.add_position_and_move (&board, initial_move);

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_return_move);
        history.add_position_and_move (&board, white_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_return_move);
        history.add_position_and_move (&board, black_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_return_move);
        history.add_position_and_move (&board, white_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_return_move);
        history.add_position_and_move (&board, black_return_move);

        REQUIRE( history.is_third_repetition (board) == true );
    }

    SUBCASE( "ignored for one move when en passant state is different" )
    {
        History history;
        BoardBuilder builder;

        builder.add_piece ("e8", Color::Black, Piece::King);
        builder.add_piece ("e7", Color::Black, Piece::Pawn);
        builder.add_piece ("e1", Color::White, Piece::King);
        builder.set_current_turn (Color::Black);

        auto board = Board { builder };

        Move black_move = move_parse ("e8 d8");
        Move black_return_move = move_parse ("d8 e8");

        Move white_move = move_parse ("e1 d1");
        Move white_return_move = move_parse ("d1 e1");

        // Record initial position. we don't care about move here.
        Move initial_move = move_parse ("e7 e5");
        board = board.with_move (Color::Black, initial_move);
        history.add_position_and_move (&board, initial_move);

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_return_move);
        history.add_position_and_move (&board, white_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_return_move);
        history.add_position_and_move (&board, black_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_return_move);
        history.add_position_and_move (&board, white_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, black_return_move);
        history.add_position_and_move (&board, black_return_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == true );
    }

    SUBCASE( "From the initial position, black gets a draw due to castle state." )
    {
        History history;
        Board board;

        Move initial_white_pawn_move = move_parse ("e2 e4");
        Move initial_black_pawn_move = move_parse ("e7 e5");

        board = board.with_move (Color::White, initial_white_pawn_move);
        history.add_position_and_move (&board, initial_white_pawn_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, initial_black_pawn_move);
        history.add_position_and_move (&board, initial_black_pawn_move);
        REQUIRE( history.is_third_repetition (board) == false );

        Move white_move = move_parse ("e1 e2");
        Move white_return_move = move_parse ("e2 e1");

        Move black_move = move_parse ("e8 e7");
        Move black_return_move = move_parse ("e7 e8");

        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE (history.is_third_repetition (board) == false);

        // This is the initial draw position, because both castle states are reset here:
        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE (history.is_third_repetition (board) == false);

        for (int i = 0; i < 2; i++)
        {
            board = board.with_move (Color::White, white_return_move);
            history.add_position_and_move (&board, white_return_move);
            REQUIRE (history.is_third_repetition (board) == false);

            board = board.with_move (Color::Black, black_return_move);
            history.add_position_and_move (&board, black_return_move);
            REQUIRE (history.is_third_repetition (board) == false);

            board = board.with_move (Color::White, white_move);
            history.add_position_and_move (&board, white_move);
            REQUIRE (history.is_third_repetition (board) == false);

            if (i == 1)
                break;

            board = board.with_move (Color::Black, black_move);
            history.add_position_and_move (&board, black_move);
            REQUIRE (history.is_third_repetition (board) == false);
        }

        REQUIRE( history.is_third_repetition (board) == false );
        board = board.with_move (Color::Black, black_move);
        history.add_position_and_move (&board, black_move);
        REQUIRE( history.is_third_repetition (board) == true );
    }

    SUBCASE( "From the initial position, white gets a draw due to the bishop." )
    {
        History history;
        Board board;

        Move initial_white_pawn_move = move_parse ("e2 e4");
        Move initial_black_pawn_move = move_parse ("e7 e5");

        board = board.with_move (Color::White, initial_white_pawn_move);
        history.add_position_and_move (&board, initial_white_pawn_move);
        REQUIRE( history.is_third_repetition (board) == false );

        board = board.with_move (Color::Black, initial_black_pawn_move);
        history.add_position_and_move (&board, initial_black_pawn_move);
        REQUIRE( history.is_third_repetition (board) == false );

        Move white_move = move_parse ("f1 e2");
        Move white_return_move = move_parse ("e2 f1");

        Move black_move = move_parse ("f8 e7");
        Move black_return_move = move_parse ("e7 f8");

        for (int i = 0; i < 2; i++)
        {
            board = board.with_move (Color::White, white_move);
            history.add_position_and_move (&board, white_move);
            REQUIRE (history.is_third_repetition (board) == false);

            board = board.with_move (Color::Black, black_move);
            history.add_position_and_move (&board, black_move);
            REQUIRE (history.is_third_repetition (board) == false);

            board = board.with_move (Color::White, white_return_move);
            history.add_position_and_move (&board, white_return_move);
            REQUIRE (history.is_third_repetition (board) == false);

            board = board.with_move (Color::Black, black_return_move);
            history.add_position_and_move (&board, black_return_move);
            REQUIRE (history.is_third_repetition (board) == false);
        }

        REQUIRE( history.is_third_repetition (board) == false );
        board = board.with_move (Color::White, white_move);
        history.add_position_and_move (&board, white_move);
        REQUIRE( history.is_third_repetition (board) == true );
    }
}

TEST_CASE( "Many moves without progress are detected" )
{
    History history;
    BoardBuilder builder;

    builder.add_piece ("e8", Color::Black, Piece::King);
    builder.add_piece ("e1", Color::White, Piece::King);

    auto board = Board { builder };

    Move first = move_parse ("e1 d1");
    Move second = move_parse ("e8 d8");
    Move third = move_parse ("d1 e1");
    Move fourth = move_parse ("d8 e8");

    auto make_useless_moves = [&board, first, second, third, fourth](int count)
    {
        for (int i = 0; i < count; i++)
        {
            board = board.with_move (Color::White, first);
            board = board.with_move (Color::Black, second);
            board = board.with_move (Color::White, third);
            board = board.with_move (Color::Black, fourth);
        }
    };

    SUBCASE( "Fifty moves without progress is detected" )
    {
        make_useless_moves (24);
        REQUIRE( History::has_been_fifty_moves_without_progress (board) == false );

        board = board.with_move (Color::White, first);
        REQUIRE( History::has_been_fifty_moves_without_progress (board) == false );

        board = board.with_move (Color::Black, second);
        REQUIRE( History::has_been_fifty_moves_without_progress (board) == false );

        board = board.with_move (Color::White, third);
        REQUIRE( History::has_been_fifty_moves_without_progress (board) == false );

        board = board.with_move (Color::Black, fourth);
        REQUIRE( History::has_been_fifty_moves_without_progress (board) == true );
    }

    SUBCASE( "Seventy-five moves without progress are detected" )
    {
        make_useless_moves (37);
        REQUIRE( History::has_been_seventy_five_moves_without_progress (board) == false );

        board = board.with_move (Color::White, first);
        REQUIRE( History::has_been_seventy_five_moves_without_progress (board) == false );

        board = board.with_move (Color::Black, second);
        REQUIRE( History::has_been_seventy_five_moves_without_progress (board) == true );
    }
}
