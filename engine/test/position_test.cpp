#include <doctest/doctest.h>
#include "board_builder.hpp"

#include "board.hpp"
#include "position.hpp"
#include "move.hpp"

using namespace wisdom;

TEST_CASE( "Position is initialized correctly" )
{
    Board board;

    CHECK(board.getPosition ().individual_score (Color::White) < 0 );
    CHECK(board.getPosition ().individual_score (Color::Black) < 0 );
    CHECK(board.getPosition ().individual_score (Color::White) == board.getPosition ().individual_score (Color::Black) );
}

TEST_CASE( "Center pawn elevates position overall_score" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e4", Color::White, Piece::Pawn);
    builder.addPiece ("a6", Color::Black, Piece::Pawn);
    
    auto board = Board { builder };

    CHECK(board.getPosition ().overall_score (Color::White) > board.getPosition ().overall_score (Color::Black));
}

TEST_CASE( "Capture updates position overall_score correctly" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e4", Color::White, Piece::Knight);
    builder.addPiece ("d6", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    int initial_score_white = board.getPosition ().overall_score (Color::White);
    int initial_score_black = board.getPosition ().overall_score (Color::Black);

    Move e4xd6 = move_parse ("e4xd6", Color::White);

    board = board.withMove (Color::White, e4xd6);

    CHECK( initial_score_white != board.getPosition ().overall_score (Color::White) );
    CHECK( initial_score_black != board.getPosition ().overall_score (Color::Black) );
}

TEST_CASE( "En passant updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e5", Color::White, Piece::Pawn);
    builder.addPiece ("d5", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    int initial_score_white = board.getPosition ().overall_score (Color::White);
    int initial_score_black = board.getPosition ().overall_score (Color::Black);

    Move e5xd5 = move_parse ("e5d6 ep", Color::White);
    CHECK( e5xd5.is_en_passant() );

    board = board.withMove (Color::White, e5xd5);

    CHECK( initial_score_white != board.getPosition ().overall_score (Color::White) );
    CHECK( initial_score_black != board.getPosition ().overall_score (Color::Black) );
}

TEST_CASE( "Castling updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("h1", Color::White, Piece::Rook);
    builder.addPiece ("a1", Color::White, Piece::Rook);
    builder.addPiece ("d5", Color::Black, Piece::Pawn);

    auto board = Board { builder };
    int initial_score_white = board.getPosition ().overall_score (Color::White);
    int initial_score_black = board.getPosition ().overall_score (Color::Black);

    std::vector castling_moves { "o-o", "o-o-o" };
    for (auto castling_move_in : castling_moves)
    {
        Move castling_move = move_parse (castling_move_in, Color::White);
        CHECK( castling_move.is_castling ());

        Board after_castling = board.withMove (Color::White, castling_move);

        CHECK( initial_score_white != after_castling.getPosition ().overall_score (Color::White));
        CHECK( initial_score_black != after_castling.getPosition ().overall_score (Color::Black));
    }
}

TEST_CASE( "Promoting move updates position overall_score correctly")
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("h7", Color::White, Piece::Pawn);

    auto board = Board { builder };
    int initial_score_white = board.getPosition ().overall_score (Color::White);
    int initial_score_black = board.getPosition ().overall_score (Color::Black);

    std::vector promoting_moves { "h7h8 (Q)", "h7h8 (R)", "h7h8 (B)", "h7h8 (N)" };
    for (auto promoting_move_in : promoting_moves)
    {
        Move promoting_move = move_parse (promoting_move_in, Color::White);
        CHECK( promoting_move.is_promoting () );

        Board after_promotion = board.withMove (Color::White, promoting_move);

        CHECK( initial_score_white != after_promotion.getPosition ().overall_score (Color::White) );
        CHECK( initial_score_black != after_promotion.getPosition ().overall_score (Color::Black) );
    }
}

TEST_CASE("Double pawn moves are more appealing")
{
    Board board;

    auto e2e4 = move_parse ("e2e4");
    auto e7e5 = move_parse("e7e5");
    auto e7e6 = move_parse("e7e6");

    Board after_white = board.withMove (Color::White, e2e4);
    Board with_double = after_white.withMove (Color::Black, e7e5);
    auto black_big_score = with_double.getPosition ().individual_score (Color::Black);
    Board with_single = after_white.withMove (Color::Black, e7e6);
    auto black_small_score = with_single.getPosition ().individual_score (Color::Black);

    REQUIRE( black_big_score > black_small_score );
}