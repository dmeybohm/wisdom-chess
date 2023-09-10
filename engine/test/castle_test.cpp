#include <doctest/doctest.h>
#include "board_builder.hpp"
#include "fen_parser.hpp"
#include "board.hpp"

using namespace wisdom;

TEST_CASE("Initializing castling state")
{
    SUBCASE( "From default position" )
    {
        Board board;
        for (auto color : { Color::White, Color::Black} )
        {
            auto both = board.ableToCastle (color,
                                            CastlingEligible::KingsideIneligible
                                                | CastlingEligible::QueensideIneligible);
            auto king = board.ableToCastle (color, CastlingEligible::KingsideIneligible);
            auto queen = board.ableToCastle (color, CastlingEligible::QueensideIneligible);
            CHECK( both );
            CHECK( king );
            CHECK( queen );
        }
    }

    SUBCASE( "With only kings" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        auto board = Board { builder };

        for (auto color : { Color::White, Color::Black} )
        {
            auto both = board.ableToCastle (color,
                                            CastlingEligible::KingsideIneligible
                                                | CastlingEligible::QueensideIneligible);
            auto king = board.ableToCastle (color, CastlingEligible::KingsideIneligible);
            auto queen = board.ableToCastle (color, CastlingEligible::QueensideIneligible);
            CHECK( !both );
            CHECK( !king );
            CHECK( !queen );
        }
    }
}

TEST_CASE( "Castling state is modified and restored for rooks" )
{
    BoardBuilder::PieceRow back_rank = {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::Queen, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    BoardBuilder builder;
    builder.addRowOfSameColor (0, Color::Black, back_rank);
    builder.addRowOfSameColor (7, Color::White, back_rank);
    builder.setCurrentTurn (Color::Black);

    Board board { builder };
    Move mv = Move::make (0, 0, 0, 1);

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::Black, mv);

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible );
}

TEST_CASE( "Castling state is modified and restored for kings" )
{
    BoardBuilder::PieceRow back_rank = {
        Piece::Rook,   Piece::None, Piece::Bishop, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    BoardBuilder builder;
    builder.addRowOfSameColor (0, Color::Black, back_rank);
    builder.addRowOfSameColor (7, Color::White, back_rank);
    builder.setCurrentTurn (Color::Black);
    Board board { builder };
    Move mv = Move::make (0, 4, 0, 3);

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::Black, mv);;

    CHECK (!board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible));
    CHECK (!board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible));
    CHECK (!board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)));
    CHECK (board.getCastlingEligibility (Color::Black)
           == (CastlingEligible::QueensideIneligible | CastlingEligible::KingsideIneligible));
}

TEST_CASE( "Castling state is modified and restored for castling queenside" )
{
    array<Piece, Num_Columns> back_rank = {
        Piece::Rook,   Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::Bishop, Piece::None, Piece::Rook
    };

    BoardBuilder builder;
    builder.addRowOfSameColor (0, Color::Black, back_rank);
    builder.addRowOfSameColor (7, Color::White, back_rank);
    builder.setCurrentTurn (Color::Black);
    Board board { builder };
    Move mv = Move::makeCastling (0, 4, 0, 2);

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::QueensideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::Black, mv);;

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::BothSidesIneligible) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::BothSidesIneligible );

    // Check rook and king position updated:
    CHECK( Row (board.getKingPosition (Color::Black)) == 0 );
    CHECK( Column (board.getKingPosition (Color::Black)) == 2 );
    CHECK( pieceType (board.pieceAt (0, 2)) == Piece::King );
    CHECK( pieceColor (board.pieceAt (0, 2)) == Color::Black );
    CHECK( pieceType (board.pieceAt (0, 3)) == Piece::Rook );
    CHECK( pieceColor (board.pieceAt (0, 3)) == Color::Black );
}

TEST_CASE("Castling state is modified and restored for castling kingside")
{
    array<Piece, Num_Columns> back_rank = {
        Piece::Rook,  Piece::None, Piece::None, Piece::None, Piece::King,
        Piece::None, Piece::None, Piece::Rook
    };

    BoardBuilder builder;
    builder.addRowOfSameColor (0, Color::Black, back_rank);
    builder.addRowOfSameColor (7, Color::White, back_rank);
    Board board { builder };
    Move mv = Move::makeCastling (7, 4, 7, 6);

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::White, mv);

    CHECK( !board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( !board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK( !board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::BothSidesIneligible );

    // Check rook and king position updated:
    CHECK( Row (board.getKingPosition (Color::White)) == 7 );
    CHECK( Column (board.getKingPosition (Color::White)) == 6 );
    CHECK( pieceType (board.pieceAt (7, 6)) == Piece::King );
    CHECK( pieceColor (board.pieceAt (7, 6)) == Color::White );
    CHECK( pieceType (board.pieceAt (7, 5)) == Piece::Rook );
    CHECK( pieceColor (board.pieceAt (7, 5)) == Color::White );
}

TEST_CASE( "Opponent's castling state is modified when his rook is taken" )
{
    BoardBuilder builder;

    builder.addPiece ("a8", Color::Black, Piece::Rook);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("h8", Color::Black, Piece::Rook);
    builder.addPiece ("a1", Color::White, Piece::Rook);
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("h1", Color::White, Piece::Rook);

    // add bishop to capture rook:
    builder.addPiece ("b7", Color::White, Piece::Bishop);

    auto board = Board { builder };
    
    Move mv = Move::makeNormalCapturing (1, 1, 0, 0);

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::White, mv);

    CHECK (board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible));
    CHECK (board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible));
    CHECK (board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)));
    CHECK (board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible);

    CHECK (!board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible));
    CHECK (board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible));
    CHECK (board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)));
    CHECK (board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible);
}

TEST_CASE("Castling state is updated when rook captures a piece")
{
    BoardBuilder builder;

    builder.addPiece ("a8", Color::Black, Piece::Rook);
    builder.addPiece ("e8", Color::Black, Piece::King);
    builder.addPiece ("h8", Color::Black, Piece::Rook);
    builder.addPiece ("a1", Color::White, Piece::Rook);
    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("h1", Color::White, Piece::Rook);

    // add bishop for rook to capture:
    builder.addPiece ("a7", Color::White, Piece::Bishop);
    builder.setCurrentTurn (Color::Black);

    auto board = Board { builder };

    Move mv = Move::makeNormalCapturing (0, 0, 1, 0);

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::Black, mv);;

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible );
}

TEST_CASE("Opponent's castling state is modified when his rook is taken (failure scenario)")
{
    BoardBuilder builder;

    builder.addRowOfSameColor (0, Color::Black,
                               { Piece::Rook, Piece::None, Piece::Queen, Piece::None, Piece::King,
                                 Piece::Bishop, Piece::Knight, Piece::Rook });
    builder.addRowOfSameColor (1, Color::Black,
                               { Piece::Pawn, Piece::None, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                                 Piece::Pawn, Piece::Pawn, Piece::Pawn });
    builder.addRowOfSameColor (6, Color::White,
                               { Piece::Pawn, Piece::Pawn, Piece::Pawn, Piece::None, Piece::None,
                                 Piece::Pawn, Piece::Pawn, Piece::Pawn });
    builder.addRowOfSameColor (7, Color::White,
                               { Piece::Rook, Piece::Knight, Piece::Bishop, Piece::None,
                                 Piece::King, Piece::None, Piece::Knight, Piece::Rook });

    builder.addPiece ("a6", Color::Black, Piece::Pawn);
    builder.addPiece ("e5", Color::Black, Piece::Bishop);
    builder.addPiece ("d3", Color::White, Piece::Pawn);
    // add the queen ready for rook to capture:
    builder.addPiece ("b8", Color::White, Piece::Queen);
    builder.setCurrentTurn (Color::Black);

    auto board = Board { builder };
    Move mv = Move::makeNormalCapturing (0, 0, 0, 1);

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::EitherSideEligible );

    board = board.withMove (Color::Black, mv);;

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible );
}

TEST_CASE("Castling state is modified when rook takes a piece on same column (scenario 2)")
{
    BoardBuilder builder;

    builder.addRowOfSameColor (0, Color::Black,
                               { Piece::None, Piece::None, Piece::Bishop, Piece::Queen, Piece::King,
                                 Piece::Bishop, Piece::Knight, Piece::Rook });
    builder.addRowOfSameColor (1, Color::Black,
                               { Piece::Pawn, Piece::None, Piece::Pawn, Piece::Pawn, Piece::Pawn,
                                 Piece::Pawn, Piece::Pawn, Piece::Pawn });
    builder.addRowOfSameColor (6, Color::White,
                               { Piece::None, Piece::None, Piece::Pawn, Piece::Pawn, Piece::None,
                                 Piece::Pawn, Piece::Pawn, Piece::Pawn });
    builder.addRowOfSameColor (7, Color::White,
                               { Piece::Rook, Piece::Knight, Piece::Bishop, Piece::Queen,
                                 Piece::King, Piece::None, Piece::Knight, Piece::Rook });

    builder.addPiece ("e6", Color::White, Piece::Pawn);

    // Rook white will capture:
    builder.addPiece ("a2", Color::Black, Piece::Rook);

    auto board = Board { builder };

    Move mv = Move::makeNormalCapturing (7, 0, 6, 0);

    CHECK( board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::EitherSideEligible );

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible );

    board = board.withMove (Color::White, mv);

    CHECK( !board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::White,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::White) == CastlingEligible::QueensideIneligible );

    CHECK( !board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible) );
    CHECK( board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible) );
    CHECK(board.ableToCastle (
        Color::Black,
        (CastlingEligible::KingsideIneligible | CastlingEligible::KingsideIneligible)) );
    CHECK( board.getCastlingEligibility (Color::Black) == CastlingEligible::QueensideIneligible );
}

TEST_CASE( "Test can castle" )
{
    MoveList moves { Color::White, {"e2 e4",
            "d7 d5","e4 e5",
            "c8 d7","d2 d4","e7 e6","c2 c3","d7 c6","b1 a3","f8xa3","b2xa3","c6 a4",
            "d1 d2","g8 e7","a1 b1","e7 c6","b1xb7","b8 d7","g1 f3","a8 b8","d2 b2",
            "b8 a8","f1 e2","f7 f6","e5xf6","d7xf6"}
    };

    Board board;
    Color color = Color::White;
    REQUIRE(board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible));

    int i = 0;
    for (auto move : moves)
    {
        INFO("Move : ");
        CAPTURE(i);
        i++;
        board = board.withMove (color, move);
        CHECK( board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible) );
        color = colorInvert (color);
    }
    auto castling = moveParse ("o-o", Color::White);
    (void)board.withMove (Color::White, castling);
}

TEST_CASE( "Kingside castle state after moving queenside rook" )
{
    FenParser parser { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" };
    Board board = parser.buildBoard();

    MoveList move_list { Color::White, { "e5 c6", "a8 d8", "c6xd8" } };
    Color color = Color::White;
    for (auto move : move_list)
    {
        board = board.withMove (color, move);
        color = colorInvert (color);
    }
    bool castle_king_side = board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible);
    bool castle_queen_side = board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible);
    CHECK( !castle_queen_side );
    CHECK( castle_king_side );
}

TEST_CASE( "Test ableToCastle" )
{
    SUBCASE( "Initial state" )
    {
        Board board;

        auto white_castle = board.ableToCastle (Color::White, CastlingEligible::EitherSideEligible);
        auto black_castle = board.ableToCastle (Color::Black, CastlingEligible::EitherSideEligible);
        CHECK( white_castle );
        CHECK( black_castle );
    }

    SUBCASE( "When eligible is set explicitly" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        builder.setCastling (Color::White, CastlingEligible::EitherSideEligible);
        builder.setCastling (Color::Black, CastlingEligible::EitherSideEligible);
        auto board = Board { builder };

        auto white_castle = board.ableToCastle (Color::White, CastlingEligible::EitherSideEligible);
        auto black_castle = board.ableToCastle (Color::Black, CastlingEligible::EitherSideEligible);

        CHECK( white_castle );
        CHECK( black_castle );
    }

    SUBCASE( "When ineligible is set explicitly" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        builder.setCastling (Color::White, CastlingEligible::BothSidesIneligible);
        builder.setCastling (Color::Black, CastlingEligible::BothSidesIneligible);
        auto board = Board { builder };

        auto white_castle = board.ableToCastle (Color::White, CastlingEligible::EitherSideEligible);
        auto black_castle = board.ableToCastle (Color::Black, CastlingEligible::EitherSideEligible);
        CHECK( !white_castle );
        CHECK( !black_castle );
    }

    SUBCASE( "When queenside is set ineligible" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        builder.setCastling (Color::White, CastlingEligible::QueensideIneligible);
        builder.setCastling (Color::Black, CastlingEligible::QueensideIneligible);
        auto board = Board { builder };

        auto white_castle = board.ableToCastle (Color::White, CastlingEligible::EitherSideEligible);
        auto black_castle = board.ableToCastle (Color::Black, CastlingEligible::EitherSideEligible);
        CHECK( white_castle );
        CHECK( black_castle );

        white_castle = board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible);
        black_castle = board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible);
        CHECK( white_castle );
        CHECK( black_castle );

        white_castle = board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible);
        black_castle = board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible);
        CHECK( !white_castle );
        CHECK( !black_castle );
    }

    SUBCASE( "When kingside is set ineligible" )
    {
        BoardBuilder builder = BoardBuilder::fromDefaultPosition();
        builder.setCastling (Color::White, CastlingEligible::KingsideIneligible);
        builder.setCastling (Color::Black, CastlingEligible::KingsideIneligible);
        auto board = Board { builder };

        auto white_castle = board.ableToCastle (Color::White, CastlingEligible::EitherSideEligible);
        auto black_castle = board.ableToCastle (Color::Black, CastlingEligible::EitherSideEligible);
        CHECK( white_castle );
        CHECK( black_castle );

        white_castle = board.ableToCastle (Color::White, CastlingEligible::QueensideIneligible);
        black_castle = board.ableToCastle (Color::Black, CastlingEligible::QueensideIneligible);
        CHECK( white_castle );
        CHECK( black_castle );

        white_castle = board.ableToCastle (Color::White, CastlingEligible::KingsideIneligible);
        black_castle = board.ableToCastle (Color::Black, CastlingEligible::KingsideIneligible);
        CHECK( !white_castle );
        CHECK( !black_castle );
    }
}