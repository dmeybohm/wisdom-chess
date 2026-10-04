#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/material.hpp"
#include "wisdom-chess/engine/position.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "findFirstCoordWithPiece()" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("a2", Color::White, Piece::Pawn);
    builder.addPiece ("a3", Color::White, Piece::Pawn);
    builder.addPiece ("a7", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    SUBCASE( "Returns the first position if there are multiple positions with the same combo" )
    {
        auto black_pawn_pos = board.findFirstCoordWithPiece (Piece::Pawn);
        auto expected_black_pawn_pos = toCoord ("a7");
        CHECK( *black_pawn_pos == expected_black_pawn_pos );
    }

    SUBCASE( "Returns the first position if there is only one position with the combo" )
    {
        auto black_king_pos = board.findFirstCoordWithPiece (Piece::King);
        REQUIRE( black_king_pos.has_value() );
        auto after_black_king = nextCoord (*black_king_pos);
        REQUIRE( after_black_king.has_value() );

        auto white_king_pos = board.findFirstCoordWithPiece (Piece::King, *after_black_king);
        REQUIRE( white_king_pos.has_value() );
        auto black_pawn_pos = board.findFirstCoordWithPiece (Piece::Pawn);

        auto expected_white_king_pos = toCoord ("e1");
        auto expected_black_king_pos= toCoord ("e8");
        auto expected_black_pawn_pos = toCoord ("a7");

        CHECK( *white_king_pos == expected_white_king_pos );
        CHECK( *black_king_pos == expected_black_king_pos );
        CHECK( *black_pawn_pos == expected_black_pawn_pos );
    }

    SUBCASE( "Returns nullopt if no piece is found" )
    {
        auto white_queen_pos = board.findFirstCoordWithPiece (Piece::Queen);
        CHECK( !white_queen_pos.has_value() );
    }
}

namespace
{
    auto
    hasKingAtKingPosition (const Board& board, Color who)
        -> bool
    {
        return board.pieceAt (board.getKingPosition (who)) == ColoredPiece::make (who, Piece::King);
    }

    // Counts the boards reached by legal moves whose incrementally updated
    // state differs from one recomputed from the squares.
    auto
    countInconsistentBoards (const Board& board, int depth)
        -> int
    {
        if (depth == 0)
            return 0;

        int count = 0;
        auto who = board.getCurrentTurn();
        for (auto move : generateLegalMoves (board, who))
        {
            auto next = board.withMove (who, move);
            bool consistent = next.getUnnormalizedBoardCode() == BoardCode::fromBoard (next)
                && next.getMaterial() == Material { next }
                && next.getPosition() == Position { next }
                && hasKingAtKingPosition (next, Color::White)
                && hasKingAtKingPosition (next, Color::Black);
            if (!consistent)
                count++;

            count += countInconsistentBoards (next, depth - 1);
        }
        return count;
    }
}

TEST_CASE( "Incrementally updated board state matches a recompute" )
{
    // Between them these reach castling, en passant, captures and promotions.
    for (czstring fen : {
             "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
             "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
             "n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - - 0 1",
             "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
         })
    {
        CAPTURE( fen );
        FenParser parser { fen };
        auto board = parser.buildBoard();

        CHECK( countInconsistentBoards (board, 3) == 0 );
    }
}
