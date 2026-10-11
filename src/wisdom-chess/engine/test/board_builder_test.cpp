#include <random>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "board_builder" )
{
    SUBCASE( "Specifying coordinates in algebraic notation" )
    {
        CHECK( coordRow (toCoord ("a8")) == 0 );
        CHECK( coordRow (toCoord ("a1")) == 7 );
        CHECK( coordColumn (toCoord ("a8")) == 0 );
        CHECK( coordColumn (toCoord ("a1")) == 0 );
        CHECK( coordRow (toCoord ("h1")) == 7 );
        CHECK( coordRow (toCoord ("h8")) == 0 );
        CHECK( coordColumn (toCoord ("h1")) == 7 );
        CHECK( coordColumn (toCoord ("h8")) == 7 );
    }

    SUBCASE( "Initializing the board builder" )
    {
        BoardBuilder builder;

        builder.addPiece ("a7", Color::White, Piece::Pawn);
        builder.addPiece ("g8", Color::White, Piece::King);
        builder.addPiece ("a1", Color::Black, Piece::King);

        auto board = Board { builder };

        CHECK( board.pieceAt (1, 0) == ColoredPiece::make (Color::White, Piece::Pawn) );
        CHECK( board.pieceAt (0, 6) == ColoredPiece::make (Color::White, Piece::King) );
        CHECK( board.pieceAt (7, 0) == ColoredPiece::make (Color::Black, Piece::King) );
        CHECK( board.pieceAt (4, 5) == nullopt );
    }
}

TEST_CASE( "Board builder accepts move clocks at their limits" )
{
    BoardBuilder builder;

    builder.setHalfMovesClock (0);
    builder.setFullMoves (1);
    CHECK( builder.getHalfMoveClock() == 0 );
    CHECK( builder.getFullMoveClock() == 1 );

    builder.setHalfMovesClock (Max_Half_Move_Clock);
    builder.setFullMoves (Max_Full_Move_Number);
    CHECK( builder.getHalfMoveClock() == Max_Half_Move_Clock );
    CHECK( builder.getFullMoveClock() == Max_Full_Move_Number );
}
