#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/coord.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "en passant" )
{
    SUBCASE( "En passant state starts out as not vulnerable" )
    {
        Board board;

        REQUIRE( board.getAnyEnPassantTarget() == nullopt );

        BoardBuilder builder;
        const auto& back_rank = BoardBuilder::Default_Piece_Row;
        builder.addRowOfSameColor ("a8", Color::Black, back_rank);
        builder.addRowOfSameColorAndPiece ("a7", Color::Black, Piece::Pawn);
        builder.addPiece ("e5", Color::White, Piece::Pawn);
        builder.addRowOfSameColor ("a1", Color::White, back_rank);

        auto builder_board = Board { builder };

        REQUIRE( builder_board.getAnyEnPassantTarget() == nullopt );
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

        REQUIRE( board.getAnyEnPassantTarget() == nullopt );

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

        auto target = board.getLegalEnPassantTarget();
        REQUIRE( target.has_value() );
        REQUIRE( target->vulnerable_color == Color::Black );

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
        REQUIRE( board.getAnyEnPassantTarget() == nullopt );

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

        auto target = board.getLegalEnPassantTarget();
        REQUIRE( target.has_value() );
        REQUIRE( target->vulnerable_color == Color::Black );

        board = board.withMove (Color::White, en_passant_move);

        ColoredPiece en_passant_pawn = board.pieceAt (2, 3);
        REQUIRE( pieceType (en_passant_pawn) == Piece::Pawn );
        REQUIRE( pieceColor (en_passant_pawn) == Color::White );

        ColoredPiece taken_pawn = board.pieceAt (3, 4);
        REQUIRE( pieceColor (taken_pawn) == Color::None );
        REQUIRE( pieceType (taken_pawn) == Piece::None );
    }
}

TEST_CASE( "Board code and equality leave out an unusable en passant target" )
{
    SUBCASE( "A board with no en passant target has the same code either way" )
    {
        Board board;

        REQUIRE( board.getAnyEnPassantTarget() == nullopt );
        CHECK( board.getBoardCode() == board.getUnnormalizedBoardCode() );
    }

    SUBCASE( "No enemy pawn is adjacent" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("a4", Color::White, Piece::Pawn);
        builder.setCurrentTurn (Color::Black);
        auto without_target = Board { builder };

        builder.setEnPassantTarget (Color::White, "a3");
        auto board = Board { builder };

        REQUIRE( board.getAnyEnPassantTarget().has_value() );
        CHECK( !board.getBoardCode().getAnyEnPassantTarget().has_value() );
        CHECK( board.getBoardCode() == without_target.getBoardCode() );
        CHECK( board.getUnnormalizedBoardCode() != without_target.getUnnormalizedBoardCode() );
        CHECK( board == without_target );
    }

    SUBCASE( "The adjacent enemy pawn is pinned" )
    {
        // Rank 4: white rook a4, white pawn d4 (just double-pushed), black
        // pawn e4, black king h4. Capturing en passant vacates both d4 and
        // e4, exposing the black king to the rook along the rank.
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("a4", Color::White, Piece::Rook);
        builder.addPiece ("d4", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.addPiece ("h4", Color::Black, Piece::King);
        builder.setCurrentTurn (Color::Black);
        auto without_target = Board { builder };

        builder.setEnPassantTarget (Color::White, "d3");
        auto board = Board { builder };

        REQUIRE( board.getAnyEnPassantTarget().has_value() );
        CHECK( !board.getBoardCode().getAnyEnPassantTarget().has_value() );
        CHECK( board.getBoardCode() == without_target.getBoardCode() );
        CHECK( board == without_target );
    }

    SUBCASE( "A legal en passant capture keeps the target" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("d4", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.setCurrentTurn (Color::Black);
        auto without_target = Board { builder };

        builder.setEnPassantTarget (Color::White, "d3");
        auto board = Board { builder };

        REQUIRE( board.getAnyEnPassantTarget().has_value() );
        CHECK( board.getBoardCode() == board.getUnnormalizedBoardCode() );
        CHECK( board.getBoardCode() != without_target.getBoardCode() );
        CHECK( board != without_target );
    }

    SUBCASE( "Double pushes made in either order reach the same position" )
    {
        Board start;

        auto queen_pawn_last = start
            .withMove (Color::White, moveParse ("e2 e4"))
            .withMove (Color::Black, moveParse ("a7 a6"))
            .withMove (Color::White, moveParse ("d2 d4"));
        auto king_pawn_last = start
            .withMove (Color::White, moveParse ("d2 d4"))
            .withMove (Color::Black, moveParse ("a7 a6"))
            .withMove (Color::White, moveParse ("e2 e4"));

        CHECK( queen_pawn_last.getUnnormalizedBoardCode()
               != king_pawn_last.getUnnormalizedBoardCode() );
        CHECK( queen_pawn_last.getBoardCode() == king_pawn_last.getBoardCode() );
        CHECK( queen_pawn_last == king_pawn_last );
    }

    SUBCASE( "The target stays on the board for FEN output" )
    {
        czstring fen_text = "rnbqkbnr/pppppppp/8/8/P7/8/1PPPPPPP/RNBQKBNR b KQkq a3 0 1";
        FenParser fen { fen_text };
        auto board = fen.buildBoard();

        CHECK( !board.getBoardCode().getAnyEnPassantTarget().has_value() );
        CHECK( board.getAnyEnPassantTarget().has_value() );
        CHECK( board.toFenString (Color::Black) == fen_text );
    }
}

TEST_CASE( "An en passant target's legality is decided when it is set" )
{
    SUBCASE( "A double push next to an enemy pawn leaves a legal target" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("d2", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        auto board = Board { builder }.withMove (Color::White, moveParse ("d2 d4"));

        auto target = board.getLegalEnPassantTarget();
        REQUIRE( target.has_value() );
        CHECK( target->coord == coordParse ("d3") );
        CHECK( target->vulnerable_color == Color::White );
    }

    SUBCASE( "A double push with no enemy pawn beside it leaves an illegal target" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("a2", Color::White, Piece::Pawn);
        auto board = Board { builder }.withMove (Color::White, moveParse ("a2 a4"));

        CHECK( board.getAnyEnPassantTarget().has_value() );
        CHECK( !board.getLegalEnPassantTarget().has_value() );
    }

    SUBCASE( "A double push beside a pinned enemy pawn leaves an illegal target" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("a4", Color::White, Piece::Rook);
        builder.addPiece ("d2", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.addPiece ("h4", Color::Black, Piece::King);
        auto board = Board { builder }.withMove (Color::White, moveParse ("d2 d4"));

        CHECK( board.getAnyEnPassantTarget().has_value() );
        CHECK( !board.getLegalEnPassantTarget().has_value() );
        auto potential_moves = generateAllPotentialMoves (board, Color::Black);
        CHECK( std::none_of (
            potential_moves.begin(), potential_moves.end(),
            [](Move move) { return move.isEnPassant(); }
        ) );
    }

    SUBCASE( "A board built with a target decides it" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("d4", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.setCurrentTurn (Color::Black);
        builder.setEnPassantTarget (Color::White, "d3");
        auto board = Board { builder };

        CHECK( board.getLegalEnPassantTarget().has_value() );
    }

    SUBCASE( "Changing the side to move decides it again" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("d4", Color::White, Piece::Pawn);
        builder.addPiece ("e4", Color::Black, Piece::Pawn);
        builder.setCurrentTurn (Color::Black);
        builder.setEnPassantTarget (Color::White, "d3");
        auto board = Board { builder };

        auto white_to_move = board.withCurrentTurn (Color::White);
        CHECK( white_to_move.getAnyEnPassantTarget().has_value() );
        CHECK( !white_to_move.getLegalEnPassantTarget().has_value() );

        auto black_to_move = white_to_move.withCurrentTurn (Color::Black);
        CHECK( black_to_move.getLegalEnPassantTarget().has_value() );
        CHECK( black_to_move == board );
    }
}
