#include "board.hpp"
#include "board_builder.hpp"
#include "fen_parser.hpp"
#include "game.hpp"
#include "history.hpp"
#include "logger.hpp"
#include "move.hpp"
#include "move_timer.hpp"
#include "search.hpp"
#include "tests.hpp"
#include "check.hpp"
#include "evaluate.hpp"

#include <iostream>

namespace wisdom::test
{
    using namespace wisdom;

    struct SearchHelper
    {
        History history {};
        std::unique_ptr<wisdom::Logger> logger;

        IterativeSearch build (const Board& board, int depth, int time = 30)
        {
            MoveTimer timer { time };
            if (!logger) {
                logger = makeNullLogger();
            }

            return { board, history, *logger, timer, depth };
        }
    };
}

using wisdom::test::SearchHelper;
using namespace wisdom;

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE( "Can find mate in 3" )
{
    BoardBuilder builder;

    builder.addPieces (Color::Black,
                       {
                           { "a8", Piece::Rook },
                           { "g8", Piece::Rook },
                           { "h8", Piece::King },
                           { "f7", Piece::Pawn },
                           { "h7", Piece::Pawn },
                       });
    builder.addPieces (Color::White,
                       {
                           { "f6", Piece::Rook },
                           { "e5", Piece::Bishop },
                           { "h2", Piece::Pawn },
                           { "h1", Piece::King },
                       });

    auto board = Board { builder };
    SearchHelper helper;
    IterativeSearch search = helper.build (board, 5);

    SearchResult result = search.iterativelyDeepen (Color::White);

    CHECK( result.score > Max_Non_Checkmate_Score );
    CHECK( result.move.has_value() );
}

//
// This position has multiple mating chances, as well as stalements, so this will test if the
// search can find the most efficient mate.
//
// Mating moves:
// ... Rd4+ 2. Ke5 f6#
// ... Bb7+ 2. Ke5 Re4#
//
TEST_CASE( "Can find mate in 2 1/2" )
{
    FenParser fen { "4n3/2k2p2/p5p1/2pK4/1r6/1n6/8/8 b - - 0 1" };
    auto game = fen.build();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.getBoard(), 5);

    SearchResult result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );
    REQUIRE( result.score > Max_Non_Checkmate_Score);
}

TEST_CASE( "scenario with heap overflow 1" )
{
    BoardBuilder builder;

    builder.addPieces (Color::Black,
                       { { "c8", Piece::Rook }, { "f8", Piece::Rook }, { "h8", Piece::King } });
    builder.addPieces (Color::Black,
                       {
                           { "c7", Piece::Pawn },
                           { "h7", Piece::Knight },
                       });
    builder.addPieces (Color::Black,
                       { { "a6", Piece::Pawn },
                         { "c6", Piece::Bishop },
                         { "b5", Piece::Pawn },
                         { "d5", Piece::Pawn } });

    builder.addPieces (Color::White,
                       { { "e5", Piece::Pawn },
                         { "a3", Piece::Knight },
                         { "c3", Piece::Pawn },
                         { "e3", Piece::Pawn },
                         { "a2", Piece::Pawn },
                         { "b2", Piece::Pawn },
                         { "h2", Piece::Pawn },
                         { "b1", Piece::King },
                         { "g1", Piece::Rook } });

    builder.setCurrentTurn (Color::Black);
    auto board = Board { builder };
    SearchHelper helper;
    IterativeSearch search = helper.build (board, 3, 300);

    SearchResult result = search.iterativelyDeepen (Color::Black);
    REQUIRE( result.move.has_value() );
}

TEST_CASE( "Promoting move is taken if possible" )
{
    BoardBuilder builder;

    builder.addPieces (Color::Black, { { "d7", Piece::King }, { "d2", Piece::Pawn } });
    builder.addPieces (Color::White, { { "a4", Piece::King }, { "h4", Piece::Pawn } });

    builder.setCurrentTurn (Color::Black);
    auto board = Board { builder };
    SearchHelper helper;

    auto search = helper.build (board, 1, 30);
    auto result = search.iterativelyDeepen (Color::Black);
    REQUIRE(asString (*result.move) == "d2 d1(Q)");
}

TEST_CASE( "Promoted pawn is promoted to highest value piece even when capturing" )
{
    FenParser parser { "rnb1kbnr/ppp1pppp/8/8/3B4/8/PPP1pPPP/RN2KB1R b KQkq - 0 1" };

    auto game = parser.build();

    SearchHelper helper;
    auto search = helper.build (game.getBoard(), 3);

    auto board_str = game.getBoard().asString();
    INFO( board_str );

    SearchResult result = search.iterativelyDeepen (Color::Black);
    REQUIRE( asString (*result.move) == "e2xf1(Q)" );
}

TEST_CASE( "Finding moves regression test" )
{
    FenParser parser { "r5rk/5p1p/5R2/4B3/8/8/7P/7K w - - 0 1" };

    auto game = parser.build();
    SearchHelper helper;
    History history;
    MoveTimer timer { 10 };
    IterativeSearch search = helper.build (game.getBoard(), 1, 10);

    SearchResult result = search.iterativelyDeepen (Color::White);
    REQUIRE( result.move.has_value() );
}

TEST_CASE( "Bishop is not sacrificed scenario 1" )
{
    FenParser fen { "r1bqk1nr/ppp2ppp/8/4p3/1bpP4/2P5/PP2NPPP/RNBQ1RK1 b kq - 0 1" };
    auto game = fen.build();

    SearchHelper helper;
    History history;
    auto search = helper.build (game.getBoard(), 3, 180);

    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    game.move (*result.move);

    // assert the bishop has moved:
    INFO ("Info:", asString (*result.move));
    REQUIRE( game.getBoard().pieceAt (coordParse ("b4"))
             != ColoredPiece::make (Color::Black, Piece::Bishop) );
}

TEST_CASE( "Bishop is not sacrificed scenario 2 (as white)" )
{
    FenParser fen { "2b2bnr/pr1ppkpp/p1p5/q3P3/2P2N2/BP3N2/P2P1PPP/R3K2R w KQ - 2 1" };
    auto game = fen.build();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.getBoard(), 3, 180);

    auto result = search.iterativelyDeepen (Color::White);

    REQUIRE (result.move.has_value());

    game.move (*result.move);

    // assert the bishop has moved:
    INFO( "Info:", asString (*result.move) );
    auto a3_piece = game.getBoard().pieceAt (coordParse ("a3"));
    bool bishop_sac = a3_piece != ColoredPiece::make (Color::White, Piece::Bishop);
    bool is_in_check = isKingThreatened (game.getBoard(), Color::Black,
                                         game.getBoard().getKingPosition (Color::Black));
    bool bishop_sac_or_is_in_check = bishop_sac || is_in_check;
    REQUIRE( bishop_sac_or_is_in_check );
}

TEST_CASE( "Advanced pawn should be captured" )
{
    FenParser fen { "rnb1k2r/ppp1qppp/4p3/3pP3/3P4/P1Q5/1PP2PPP/R3KBNR w KQkq d6 0 1" };
    auto game = fen.build();

    game.move (moveParse ("e5 d6 ep", Color::White));

    SearchHelper helper;
    auto search = helper.build (game.getBoard(), 3, 10);
    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    game.move (*result.move);
    // assert the pawn at d6 has been taken:
    INFO( "Chosen move:", asString (*result.move) );

    auto board = game.getBoard();
    auto target_piece = board.pieceAt (coordParse ("d6"));
    CHECK( target_piece != ColoredPiece::make (Color::White, Piece::Pawn));
    CHECK( pieceColor (target_piece) == Color::Black );
}

TEST_CASE( "Checkmate is preferred to stalemate" )
{
    FenParser fen { "6k1/1p3pp1/p1p4p/3r4/8/2K5/b2r4/8 b - - 0 1" };
    auto game = fen.build();

    SearchHelper helper;
    auto search = helper.build (game.getBoard(), 5, 10);

    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    game.move (*result.move);
    auto is_stalemate = isStalemated (game.getBoard(), Color::White);
    CHECK( !is_stalemate );
}

TEST_CASE( "Can avoid stalemate" )
{
    FenParser fen { "6k1/1pp2pp1/7p/Pb6/3r4/5K2/8/6q1 w - - 0 1" };
    auto game = fen.build();

    game.move (moveParse ("a5 a6", Color::White));

    SearchHelper helper;
    IterativeSearch search = helper.build (game.getBoard(), 5, 5);
    SearchResult result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move != std::nullopt );

    game.move (*result.move);

    auto is_stalemate = isStalemated (game.getBoard(), Color::White);
    CHECK( !is_stalemate );
}

TEST_CASE( "Doesn't sacrifice piece to undermine opponent's castle position" )
{
    FenParser fen { "r1bqkb1r/2pppppp/p4n2/np6/8/1B2PN2/PPPP1PPP/RNBQK2R w KQkq - 0 1 " };

    SUBCASE( "Depth 5" )
    {
        auto game = fen.build();

        SearchHelper helper;
        IterativeSearch search = helper.build (game.getBoard(), 5, 10);
        SearchResult result = search.iterativelyDeepen (Color::White);

        // Check the white bishop is not sacrificed:
        CHECK( *result.move != moveParse ("b3xf7") );
    }

    SUBCASE( "Depth 7" )
    {
        auto game = fen.build();

        SearchHelper helper;
        IterativeSearch search = helper.build (game.getBoard(), 7, 10);
        SearchResult result = search.iterativelyDeepen (Color::White);

        // Check the white bishop is not sacrificed:
        CHECK( *result.move != moveParse ("b3xf7") );
    }
}

