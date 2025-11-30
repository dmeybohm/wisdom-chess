#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/position.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/board_builder.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Position is initialized correctly" )
{
    Board board;

    CHECK( board.getPosition().individualScore (Color::White) < 0 );
    CHECK( board.getPosition().individualScore (Color::Black) < 0 );
    CHECK( board.getPosition().individualScore (Color::White) == board.getPosition().individualScore (Color::Black) );
}

TEST_CASE( "Center pawn elevates position overallScore" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e4", Color::White, Piece::Pawn);
    builder.addPiece ("a6", Color::Black, Piece::Pawn);
    
    auto board = Board { builder };

	auto white_score = board.getPosition().overallScore (Color::White);
	auto black_score = board.getPosition().overallScore (Color::Black);
    CHECK( white_score > black_score );
}

TEST_CASE( "Capture updates position overallScore correctly" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e4", Color::White, Piece::Knight);
    builder.addPiece ("d6", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    int initial_score_white = board.getPosition().overallScore (Color::White);
    int initial_score_black = board.getPosition().overallScore (Color::Black);

    Move e4xd6 = moveParse ("e4xd6", Color::White);

    board = board.withMove (Color::White, e4xd6);

    CHECK( initial_score_white != board.getPosition().overallScore (Color::White) );
    CHECK( initial_score_black != board.getPosition().overallScore (Color::Black) );
}

TEST_CASE( "En passant updates position overallScore correctly" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("e5", Color::White, Piece::Pawn);
    builder.addPiece ("d5", Color::Black, Piece::Pawn);

    auto board = Board { builder };

    int initial_score_white = board.getPosition().overallScore (Color::White);
    int initial_score_black = board.getPosition().overallScore (Color::Black);

    Move e5xd5 = moveParse ("e5d6 ep", Color::White);
    CHECK( e5xd5.isEnPassant() );

    board = board.withMove (Color::White, e5xd5);

    CHECK( initial_score_white != board.getPosition().overallScore (Color::White) );
    CHECK( initial_score_black != board.getPosition().overallScore (Color::Black) );
}

TEST_CASE( "Castling updates position overallScore correctly" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("h1", Color::White, Piece::Rook);
    builder.addPiece ("a1", Color::White, Piece::Rook);
    builder.addPiece ("d5", Color::Black, Piece::Pawn);

    auto board = Board { builder };
    int initial_score_white = board.getPosition().overallScore (Color::White);
    int initial_score_black = board.getPosition().overallScore (Color::Black);

    std::vector castling_moves { "o-o", "o-o-o" };
    for (auto castling_move_in : castling_moves)
    {
        Move castling_move = moveParse (castling_move_in, Color::White);
        CHECK( castling_move.isCastling() );

        Board after_castling = board.withMove (Color::White, castling_move);

        CHECK( initial_score_white != after_castling.getPosition().overallScore (Color::White) );
        CHECK( initial_score_black != after_castling.getPosition().overallScore (Color::Black) );
    }
}

TEST_CASE( "Promoting move updates position overallScore correctly" )
{
    BoardBuilder builder;

    builder.addPiece ("e1", Color::White, Piece::King);
    builder.addPiece ("e8", Color::Black, Piece::King);

    builder.addPiece ("h7", Color::White, Piece::Pawn);

    auto board = Board { builder };
    int initial_score_white = board.getPosition().overallScore (Color::White);
    int initial_score_black = board.getPosition().overallScore (Color::Black);

    std::vector promoting_moves { "h7h8 (Q)", "h7h8 (R)", "h7h8 (B)", "h7h8 (N)" };
    for (auto promoting_move_in : promoting_moves)
    {
        Move promoting_move = moveParse (promoting_move_in, Color::White);
        CHECK( promoting_move.isPromoting() );

        Board after_promotion = board.withMove (Color::White, promoting_move);

        CHECK( initial_score_white != after_promotion.getPosition().overallScore (Color::White) );
        CHECK( initial_score_black != after_promotion.getPosition().overallScore (Color::Black) );
    }
}

TEST_CASE( "Double pawn moves are more appealing" )
{
    Board board;

    auto e2e4 = moveParse ("e2e4");
    auto e7e5 = moveParse ("e7e5");
    auto e7e6 = moveParse ("e7e6");

    Board after_white = board.withMove (Color::White, e2e4);
    Board with_double = after_white.withMove (Color::Black, e7e5);
    auto black_big_score = with_double.getPosition().individualScore (Color::Black);
    Board with_single = after_white.withMove (Color::Black, e7e6);
    auto black_small_score = with_single.getPosition().individualScore (Color::Black);

    REQUIRE( black_big_score > black_small_score );
}
