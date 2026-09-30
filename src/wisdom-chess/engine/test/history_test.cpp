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

        // Record initial position.
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
        // The white pawn makes the double push capturable en passant, so
        // the first position carries a target the later ones do not.
        builder.addPiece ("d5", Color::White, Piece::Pawn);
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
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        // This is the initial draw position, because both castle states are reset here:
        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == false );

        for (int i = 0; i < 2; i++)
        {
            board = board.withMove (Color::White, white_return_move);
            history.addTentativePosition (board);
            REQUIRE( history.isProbablyThirdRepetition (board) == false );

            board = board.withMove (Color::Black, black_return_move);
            history.addTentativePosition (board);
            REQUIRE( history.isProbablyThirdRepetition (board) == false );

            board = board.withMove (Color::White, white_move);
            history.addTentativePosition (board);
            REQUIRE( history.isProbablyThirdRepetition (board) == false );

            if (i == 1)
                break;

            board = board.withMove (Color::Black, black_move);
            history.addTentativePosition (board);
            REQUIRE( history.isProbablyThirdRepetition (board) == false );
        }

        REQUIRE( history.isProbablyThirdRepetition (board) == false );
        board = board.withMove (Color::Black, black_move);
        history.addTentativePosition (board);
        REQUIRE( history.isProbablyThirdRepetition (board) == true );
    }

}

TEST_CASE( "Repetition counts a position despite an unusable en passant target" )
{
    SUBCASE( "no adjacent enemy pawn" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("a2", Color::White, Piece::Pawn);

        auto board = Board { builder };
        auto history = History::fromInitialBoard (board);

        Move double_push = moveParse ("a2 a4");
        Move black_out = moveParse ("e8 d8");
        Move black_back = moveParse ("d8 e8");
        Move white_out = moveParse ("e1 d1");
        Move white_back = moveParse ("d1 e1");

        board = board.withMove (Color::White, double_push);
        history.addPosition (board, double_push);
        REQUIRE( board.getAnyEnPassantTarget().has_value() );

        auto shuffle_back_to_start = [&]
        {
            board = board.withMove (Color::Black, black_out);
            history.addPosition (board, black_out);

            board = board.withMove (Color::White, white_out);
            history.addPosition (board, white_out);

            board = board.withMove (Color::Black, black_back);
            history.addPosition (board, black_back);

            board = board.withMove (Color::White, white_back);
            history.addPosition (board, white_back);
        };

        shuffle_back_to_start();
        REQUIRE( !history.isThirdRepetition (board) );

        shuffle_back_to_start();
        REQUIRE( history.isThirdRepetition (board) );

        shuffle_back_to_start();
        shuffle_back_to_start();
        REQUIRE( history.isFifthRepetition (board) );
    }

    SUBCASE( "adjacent enemy pawn pinned against its king" )
    {
        // Rank 4: white rook a4, white pawn d4 (just double-pushed), black
        // pawn e4, black king h4. Capturing en passant vacates both d4 and
        // e4, exposing the black king to the rook along the rank, so black
        // has no legal en passant capture despite the adjacent pawn.
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("a4", Color::White, Piece::Rook);
        builder.addPiece ("d2", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.addPiece ("h4", Color::Black, Piece::King);

        auto board = Board { builder };
        auto history = History::fromInitialBoard (board);

        Move double_push = moveParse ("d2 d4");
        Move black_out = moveParse ("h4 h5");
        Move black_back = moveParse ("h5 h4");
        Move white_out = moveParse ("e1 f1");
        Move white_back = moveParse ("f1 e1");

        board = board.withMove (Color::White, double_push);
        history.addPosition (board, double_push);
        REQUIRE( board.getAnyEnPassantTarget().has_value() );

        auto shuffle_back_to_start = [&]
        {
            board = board.withMove (Color::Black, black_out);
            history.addPosition (board, black_out);

            board = board.withMove (Color::White, white_out);
            history.addPosition (board, white_out);

            board = board.withMove (Color::Black, black_back);
            history.addPosition (board, black_back);

            board = board.withMove (Color::White, white_back);
            history.addPosition (board, white_back);
        };

        shuffle_back_to_start();
        REQUIRE( !history.isThirdRepetition (board) );

        shuffle_back_to_start();
        REQUIRE( history.isThirdRepetition (board) );

        shuffle_back_to_start();
        shuffle_back_to_start();
        REQUIRE( history.isFifthRepetition (board) );
    }
}

TEST_CASE( "A legal en passant capture keeps a position distinct until the right expires" )
{
    BoardBuilder builder;
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("d2", Color::White, Piece::Pawn);
    builder.addPiece ("e4", Color::Black, Piece::Pawn);

    auto board = Board { builder };
    auto history = History::fromInitialBoard (board);

    Move double_push = moveParse ("d2 d4");
    Move black_out = moveParse ("e8 d8");
    Move black_back = moveParse ("d8 e8");
    Move white_out = moveParse ("e1 d1");
    Move white_back = moveParse ("d1 e1");

    board = board.withMove (Color::White, double_push);
    history.addPosition (board, double_push);
    REQUIRE( board.getAnyEnPassantTarget().has_value() );

    auto shuffle_back_to_start = [&]
    {
        board = board.withMove (Color::Black, black_out);
        history.addPosition (board, black_out);

        board = board.withMove (Color::White, white_out);
        history.addPosition (board, white_out);

        board = board.withMove (Color::Black, black_back);
        history.addPosition (board, black_back);

        board = board.withMove (Color::White, white_back);
        history.addPosition (board, white_back);
    };

    // Black declines the capture, so the en passant right expires and the
    // position keeps recurring without a target. The very first occurrence
    // carried a genuine, usable target and must stay distinct from those
    // later ones instead of being folded into the same repetition count.
    shuffle_back_to_start();
    REQUIRE( !history.isThirdRepetition (board) );

    shuffle_back_to_start();
    REQUIRE( !history.isThirdRepetition (board) );

    shuffle_back_to_start();
    REQUIRE( history.isThirdRepetition (board) );
}

TEST_CASE( "An unusable en passant target is ignored however the position enters the history" )
{
    BoardBuilder builder;
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("a2", Color::White, Piece::Pawn);

    auto board = Board { builder };
    board = board.withMove (Color::White, moveParse ("a2 a4"));
    REQUIRE( board.getAnyEnPassantTarget().has_value() );

    Move black_out = moveParse ("e8 d8");
    Move black_back = moveParse ("d8 e8");
    Move white_out = moveParse ("e1 d1");
    Move white_back = moveParse ("d1 e1");

    SUBCASE( "as the initial board" )
    {
        auto history = History::fromInitialBoard (board);

        auto shuffle_back_to_start = [&]
        {
            board = board.withMove (Color::Black, black_out);
            history.addPosition (board, black_out);

            board = board.withMove (Color::White, white_out);
            history.addPosition (board, white_out);

            board = board.withMove (Color::Black, black_back);
            history.addPosition (board, black_back);

            board = board.withMove (Color::White, white_back);
            history.addPosition (board, white_back);
        };

        shuffle_back_to_start();
        REQUIRE( !history.isThirdRepetition (board) );

        shuffle_back_to_start();
        REQUIRE( history.isThirdRepetition (board) );
    }

    SUBCASE( "as a replacement for the last position" )
    {
        auto history = History::fromInitialBoard (Board { builder });
        history.replaceLastPosition (board);

        auto shuffle_back_to_start = [&]
        {
            board = board.withMove (Color::Black, black_out);
            history.addPosition (board, black_out);

            board = board.withMove (Color::White, white_out);
            history.addPosition (board, white_out);

            board = board.withMove (Color::Black, black_back);
            history.addPosition (board, black_back);

            board = board.withMove (Color::White, white_back);
            history.addPosition (board, white_back);
        };

        shuffle_back_to_start();
        REQUIRE( !history.isThirdRepetition (board) );

        shuffle_back_to_start();
        REQUIRE( history.isThirdRepetition (board) );
    }

    SUBCASE( "as a tentative position" )
    {
        History history;
        history.addTentativePosition (board);

        auto shuffle_back_to_start = [&]
        {
            board = board.withMove (Color::Black, black_out);
            history.addTentativePosition (board);

            board = board.withMove (Color::White, white_out);
            history.addTentativePosition (board);

            board = board.withMove (Color::Black, black_back);
            history.addTentativePosition (board);

            board = board.withMove (Color::White, white_back);
            history.addTentativePosition (board);
        };

        shuffle_back_to_start();
        REQUIRE( !history.isProbablyThirdRepetition (board) );

        shuffle_back_to_start();
        REQUIRE( history.isProbablyThirdRepetition (board) );
    }
}

TEST_CASE( "Repetition check tolerates a half move clock longer than the history" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.setHalfMovesClock (Max_Half_Move_Clock);

    auto board = Board { builder };
    auto history = History::fromInitialBoard (board);

    CHECK( history.isProbablyNthRepetition (board, 1) );
    CHECK( !history.isProbablyNthRepetition (board, 2) );
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

    auto make_useless_moves = [&board, first, second, third, fourth] (int count)
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

TEST_CASE( "Positions cannot be committed while tentative positions are pending" )
{
    Board board = Board { BoardBuilder::fromDefaultPosition() };
    History history = History::fromInitialBoard (board);
    Move move = moveParse ("e2 e4", Color::White);

    history.addTentativePosition (board);

    CHECK_THROWS_AS( history.addPosition (board, move), PreconditionError );

    history.removeLastTentativePosition();

    CHECK_NOTHROW( history.addPosition (board, move) );
}
