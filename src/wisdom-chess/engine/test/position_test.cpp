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

TEST_CASE( "Position scores are the same for both colors on mirrored ranks" )
{
    BoardBuilder builder;

    builder.addPiece ("g1", Color::White, Piece::King);
    builder.addPiece ("g8", Color::Black, Piece::King);

    builder.addPiece ("f3", Color::White, Piece::Pawn);
    builder.addPiece ("f6", Color::Black, Piece::Pawn);

    builder.addPiece ("g5", Color::White, Piece::Bishop);
    builder.addPiece ("g4", Color::Black, Piece::Bishop);

    auto board = Board { builder };

    CHECK( board.getPosition().individualScore (Color::White)
           == board.getPosition().individualScore (Color::Black) );
    CHECK( board.getPosition().overallScore (Color::White) == 0 );
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

// The piece-square tables are private to position.cpp, so read each cell
// back through a board holding the kings and one extra piece.
static auto
squareScore (Piece piece, int row, int col)
    -> int
{
    BoardBuilder builder;
    builder.addPiece ("a1", Color::White, Piece::King);
    builder.addPiece ("h8", Color::Black, Piece::King);
    auto kings_only_board = Board { builder };
    auto kings_only = kings_only_board.getPosition().individualScore (Color::White);

    builder.addPiece (row, col, Color::White, piece);
    auto board = Board { builder };
    return board.getPosition().individualScore (Color::White) - kings_only;
}

TEST_CASE( "Piece-square tables are symmetric between the two wings" )
{
    // The queen's table is deliberately asymmetric. Pawns never stand on
    // the first or last rank, and a1/h8 hold the kings.
    for (auto piece : { Piece::Pawn, Piece::Knight, Piece::Bishop, Piece::Rook })
    {
        int first_row = piece == Piece::Pawn ? 1 : 0;
        int last_row = piece == Piece::Pawn ? 6 : 7;

        for (int row = first_row; row <= last_row; row++)
        {
            for (int col = 0; col < Num_Columns / 2; col++)
            {
                int mirror_col = Last_Column - col;
                if ((row == 7 && col == 0) || (row == 0 && mirror_col == 7))
                    continue;

                INFO( "piece ", static_cast<int> (piece), " row ", row, " col ", col );
                CHECK( squareScore (piece, row, col) == squareScore (piece, row, mirror_col) );
            }
        }
    }
}

TEST_CASE( "The king's table is symmetric between the two wings" )
{
    for (int row = 0; row < Num_Rows; row++)
    {
        for (int col = 0; col < Num_Columns / 2; col++)
        {
            int mirror_col = Last_Column - col;

            auto scoreWithKingAt = [row] (int king_col)
            {
                BoardBuilder builder;
                builder.addPiece (row, king_col, Color::White, Piece::King);
                builder.addPiece (row == 0 ? 7 : 0, 7, Color::Black, Piece::King);
                auto board = Board { builder };
                return board.getPosition().individualScore (Color::White);
            };

            INFO( "row ", row, " col ", col );
            CHECK( scoreWithKingAt (col) == scoreWithKingAt (mirror_col) );
        }
    }
}
