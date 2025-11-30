#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "en passant" )
{
    SUBCASE( "En passant state starts out as not vulnerable" )
    {
        Board board;

        REQUIRE( !board.isEnPassantVulnerable (Color::White) );
        REQUIRE( !board.isEnPassantVulnerable (Color::Black) );

        REQUIRE( board.getEnPassantTarget() == nullopt );

        BoardBuilder builder;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;
        builder.addRowOfSameColor ("a8", Color::Black, back_rank);
        builder.addRowOfSameColorAndPiece ("a7", Color::Black, Piece::Pawn);
        builder.addPiece ("e5", Color::White, Piece::Pawn);
        builder.addRowOfSameColor ("a1", Color::White, back_rank);

        auto builder_board = Board { builder };

        REQUIRE( !builder_board.isEnPassantVulnerable (Color::White) );
        REQUIRE( !builder_board.isEnPassantVulnerable (Color::Black) );
    }

    SUBCASE( "En passant moves work on the right" )
    {
        BoardBuilder builder;

        const auto& back_rank = BoardBuilder::Default_Piece_Row;

        builder.addRowOfSameColor ("a8", Color::Black, back_rank);
        builder.addRowOfSameColorAndPiece ("a7", Color::Black, Piece::Pawn);

        builder.addPiece ("e5", Color::White, Piece::Pawn);
        builder.addRowOfSameColor ("a1", Color::White, back_rank);

        builder.setCurrentTurn (Color::Black);

        auto board = Board { builder };

        REQUIRE( !board.isEnPassantVulnerable (Color::Black) );
        REQUIRE( !board.isEnPassantVulnerable (Color::White) );

        Move pawn_move = moveParse ("f7f5");
        board = board.withMove (Color::Black, pawn_move);

        MoveList move_list = generateAllPotentialMoves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin(), move_list.end(), std::mem_fn (&Move::isEnPassant));

        REQUIRE( maybe_en_passant_move != move_list.end() );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE( en_passant_move.isEnPassant() );

        // Check position:
        REQUIRE( coordRow (en_passant_move.getSrc()) == 3 );
        REQUIRE( coordColumn (en_passant_move.getSrc()) == 4 );
        REQUIRE( coordRow (en_passant_move.getDst()) == 2 );
        REQUIRE( coordColumn (en_passant_move.getDst()) == 5 );

        REQUIRE( board.isEnPassantVulnerable (Color::Black) );
        REQUIRE( !board.isEnPassantVulnerable (Color::White) );

        board = board.withMove (Color::White, en_passant_move);

        ColoredPiece en_passant_pawn = board.pieceAt (2, 5);
        REQUIRE( pieceType (en_passant_pawn) == Piece::Pawn );
        REQUIRE( pieceColor (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.pieceAt (3, 4);
        REQUIRE( pieceColor (taken_pawn) == Color::None );
        REQUIRE( pieceType (taken_pawn) == Piece::None );
    }

    SUBCASE( "En passant moves work on the left" )
    {
        BoardBuilder builder;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;

        builder.addRowOfSameColor ("a8", Color::Black, back_rank);
        builder.addRowOfSameColorAndPiece ("a7", Color::Black, Piece::Pawn);
        builder.addPiece ("e5", Color::White, Piece::Pawn);
        builder.addRowOfSameColor ("a1", Color::White, back_rank);
        builder.setCurrentTurn (Color::Black);

        auto board = Board { builder };
        Move pawn_move = moveParse ("d7d5");
        REQUIRE( !board.isEnPassantVulnerable (Color::Black) );
        REQUIRE( !board.isEnPassantVulnerable (Color::White) );

        board = board.withMove (Color::Black, pawn_move);

        MoveList move_list = generateAllPotentialMoves (board, Color::White);
        auto maybe_en_passant_move = std::find_if (
                move_list.begin(), move_list.end(), std::mem_fn (&Move::isEnPassant));

        REQUIRE( maybe_en_passant_move != move_list.end() );
        auto en_passant_move = *maybe_en_passant_move;

        // Check move types:
        REQUIRE( en_passant_move.isEnPassant() );

        // Check position:
        REQUIRE( coordRow (en_passant_move.getSrc()) == 3 );
        REQUIRE( coordColumn (en_passant_move.getSrc()) == 4 );
        REQUIRE( coordRow (en_passant_move.getDst()) == 2 );
        REQUIRE( coordColumn (en_passant_move.getDst()) == 3 );

        REQUIRE( board.isEnPassantVulnerable (Color::Black) );
        REQUIRE( !board.isEnPassantVulnerable (Color::White) );

        board = board.withMove (Color::White, en_passant_move);

        ColoredPiece en_passant_pawn = board.pieceAt (2, 3);
        REQUIRE( pieceType (en_passant_pawn) == Piece::Pawn );
        REQUIRE( pieceColor (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.pieceAt (3, 4);
        REQUIRE( pieceColor (taken_pawn) == Color::None );
        REQUIRE( pieceType (taken_pawn) == Piece::None );
    }
}
