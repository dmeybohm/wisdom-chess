#include <iostream>

#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/board_builder.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Third repetition is detected" )
{
    SUBCASE( "in the regular case" )
    {
        History history;
        BoardBuilder builder;

        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e1", Color::White, Piece::King);

        auto board = Board { builder };

        Move black_move = moveParse ("e8 d8");
        Move black_return_move = moveParse ("d8 e8");

        Move white_move = moveParse ("e1 d1");
        Move white_return_move = moveParse ("d1 e1");

        // Record initial position. we don't care about move here.
        Move initial_move = moveParse ("d8 e8");
        history.addTentativePosition (board);

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_return_move);
        history.addTentativePosition (board);

        REQUIRE( history.isProbablyThirdRepetition (board) == true );
    }

    SUBCASE( "ignored for one move when en passant state is different" )
    {
        History history;
        BoardBuilder builder;

        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("e7", Color::Black, Piece::Pawn);
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.setCurrentTurn (Color::Black);

        auto board = Board { builder };

        Move black_move = moveParse ("e8 d8");
        Move black_return_move = moveParse ("d8 e8");

        Move white_move = moveParse ("e1 d1");
        Move white_return_move = moveParse ("d1 e1");

        // Record initial position. we don't care about move here.
        Move initial_move = moveParse ("e7 e5");
        board = board.withMove (Color::Black, initial_move);
        history.addTentativePosition (board);

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, black_return_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == true );
    }

    SUBCASE( "From the initial position, black gets a draw due to castle state." )
    {
        History history;
        Board board;

        Move initial_white_pawn_move = moveParse ("e2 e4");
        Move initial_black_pawn_move = moveParse ("e7 e5");

        board = board.withMove (Color::White, initial_white_pawn_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, initial_black_pawn_move);
        history.addTentativePosition (board);
        Move white_move = moveParse ("e1 e2");
        Move white_return_move = moveParse ("e2 e1");

        Move black_move = moveParse ("e8 e7");
        Move black_return_move = moveParse ("e7 e8");

        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE (history.isProbablyThirdRepetition (board) == false);

        // This is the initial draw position, because both castle states are reset here:
        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE (history.isProbablyThirdRepetition (board) == false);

        for (int i = 0; i < 2; i++)
        {
            board = board.withMove (Color::White, white_return_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            board = board.withMove (Color::Black, black_return_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            board = board.withMove (Color::White, white_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            if (i == 1)
                break;

            board = board.withMove (Color::Black, black_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);
        }

        REQUIRE( history.isProbablyThirdRepetition (board) == false );
        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == true );
    }

    SUBCASE( "From the initial position, white gets a draw due to the bishop." )
    {
        History history;
        Board board;

        Move initial_white_pawn_move = moveParse ("e2 e4");
        Move initial_black_pawn_move = moveParse ("e7 e5");

        board = board.withMove (Color::White, initial_white_pawn_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        board = board.withMove (Color::Black, initial_black_pawn_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        Move white_move = moveParse ("f1 e2");
        Move white_return_move = moveParse ("e2 f1");

        Move black_move = moveParse ("f8 e7");
        Move black_return_move = moveParse ("e7 f8");

        for (int i = 0; i < 2; i++)
        {
            board = board.withMove (Color::White, white_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            board = board.withMove (Color::Black, black_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            board = board.withMove (Color::White, white_return_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);

            board = board.withMove (Color::Black, black_return_move);
            history.addTentativePosition (board);
            REQUIRE (history.isProbablyThirdRepetition (board) == false);
        }

        REQUIRE( history.isProbablyThirdRepetition (board) == false );
        board = board.withMove (Color::White, white_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == true );
    }
}

TEST_CASE( "Many moves without progress are detected" )
{
    History history;
    BoardBuilder builder;

    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("e1", Color::White, Piece::King);

    auto board = Board { builder };

    Move first = moveParse ("e1 d1");
    Move second = moveParse ("e8 d8");
    Move third = moveParse ("d1 e1");
    Move fourth = moveParse ("d8 e8");

    auto make_useless_moves = [&board, first, second, third, fourth](int count)
    {
        for (int i = 0; i < count; i++)
        {
            board = board.withMove (Color::White, first);
            board = board.withMove (Color::Black, second);
            board = board.withMove (Color::White, third);
            board = board.withMove (Color::Black, fourth);
        }
    };

    SUBCASE( "Fifty moves without progress is detected" )
    {
        make_useless_moves (24);
        REQUIRE( History::hasBeenFiftyMovesWithoutProgress (board) == false );

        board = board.withMove (Color::White, first);
        REQUIRE( History::hasBeenFiftyMovesWithoutProgress (board) == false );

        board = board.withMove (Color::Black, second);
        REQUIRE( History::hasBeenFiftyMovesWithoutProgress (board) == false );

        board = board.withMove (Color::White, third);
        REQUIRE( History::hasBeenFiftyMovesWithoutProgress (board) == false );

        board = board.withMove (Color::Black, fourth);
        REQUIRE( History::hasBeenFiftyMovesWithoutProgress (board) == true );
    }

    SUBCASE( "Seventy-five moves without progress are detected" )
    {
        make_useless_moves (37);
        REQUIRE( History::hasBeenSeventyFiveMovesWithoutProgress (board) == false );

        board = board.withMove (Color::White, first);
        REQUIRE( History::hasBeenSeventyFiveMovesWithoutProgress (board) == false );

        board = board.withMove (Color::Black, second);
        REQUIRE( History::hasBeenSeventyFiveMovesWithoutProgress (board) == true );
    }
}
