#include <iostream>

#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/game.hpp"
#include "wisdom-chess/engine/history.hpp"
#include "wisdom-chess/engine/logger.hpp"
#include "wisdom-chess/engine/move.hpp"
#include "wisdom-chess/engine/move_timer.hpp"
#include "wisdom-chess/engine/search.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include "wisdom-chess-tests.hpp"

namespace wisdom::test
{
    using namespace wisdom;

    struct SearchHelper
    {
        History history {};
        shared_ptr<Logger> logger = makeNullLogger();
        MoveTimer timer = MoveTimer { 30 };
        TranspositionTable transposition_table = TranspositionTable::fromMegabytes (TranspositionTable::Default_Size_In_Megabytes);

        auto build (const Board& board, int depth, int time = 30)
            -> IterativeSearch
        {
            timer.setSeconds (chrono::seconds { time });
            return IterativeSearch::create (board, history, logger, timer, depth, transposition_table);
        }
    };
}

using wisdom::test::SearchHelper;
using namespace wisdom;

// Mating moves: : 1.Ra6 f6 2.Bxf6 Rg7 3.Rxa8#
TEST_CASE( "Can find mate in 3" )
{
    BoardBuilder builder;

    builder.addPieces (
        Color::Black,
        {
            { "a8", Piece::Rook },
            { "g8", Piece::Rook },
            { "h8", Piece::King },
            { "f7", Piece::Pawn },
            { "h7", Piece::Pawn },
        }
    );
    builder.addPieces (
        Color::White,
        {
            { "f6", Piece::Rook },
            { "e5", Piece::Bishop },
            { "h2", Piece::Pawn },
            { "h1", Piece::King },
        }
    );

    auto board = Board { builder };
    SearchHelper helper;
    IterativeSearch search = helper.build (board, 6);

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
    IterativeSearch search = helper.build (game.getBoard(), 6);

    SearchResult result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );
    REQUIRE( result.score > Max_Non_Checkmate_Score );
}

TEST_CASE( "scenario with heap overflow 1" )
{
    BoardBuilder builder;

    builder.addPieces (
        Color::Black,
        {
            { "c8", Piece::Rook },
            { "f8", Piece::Rook },
            { "h8", Piece::King }
        }
    );
    builder.addPieces (
        Color::Black,
        {
            { "c7", Piece::Pawn },
            { "h7", Piece::Knight },
        }
    );
    builder.addPieces (
        Color::Black,
        {
            { "a6", Piece::Pawn },
            { "c6", Piece::Bishop },
            { "b5", Piece::Pawn },
            { "d5", Piece::Pawn }
        }
    );

    builder.addPieces (
        Color::White,
        {
            { "e5", Piece::Pawn },
            { "a3", Piece::Knight },
            { "c3", Piece::Pawn },
            { "e3", Piece::Pawn },
            { "a2", Piece::Pawn },
            { "b2", Piece::Pawn },
            { "h2", Piece::Pawn },
            { "b1", Piece::King },
            { "g1", Piece::Rook }
        }
    );

    builder.setCurrentTurn (Color::Black);
    auto board = Board { builder };
    SearchHelper helper;
    IterativeSearch search = helper.build (board, 4, 300);

    SearchResult result = search.iterativelyDeepen (Color::Black);
    REQUIRE( result.move.has_value() );
}

TEST_CASE( "Promoting move is taken if possible" )
{
    BoardBuilder builder;

    builder.addPieces (
        Color::Black,
        {
            { "d7", Piece::King },
            { "d2", Piece::Pawn }
        }
    );
    builder.addPieces (
        Color::White,
        {
            { "a4", Piece::King },
            { "h4", Piece::Pawn }
        }
    );

    builder.setCurrentTurn (Color::Black);
    auto board = Board { builder };
    SearchHelper helper;

    auto search = helper.build (board, 2, 30);
    auto result = search.iterativelyDeepen (Color::Black);
    REQUIRE( asString (*result.move) == "d2 d1(Q)" );
}

TEST_CASE( "Promoted pawn is promoted to highest value piece even when capturing" )
{
    FenParser parser { "rnb1kbnr/ppp1pppp/8/8/3B4/8/PPP1pPPP/RN2KB1R b KQkq - 0 1" };

    auto game = parser.build();

    SearchHelper helper;
    auto search = helper.build (game.getBoard(), 4);

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
    IterativeSearch search = helper.build (game.getBoard(), 2, 10);

    SearchResult result = search.iterativelyDeepen (Color::White);
    REQUIRE( result.move.has_value() );
}

TEST_CASE( "Bishop is not sacrificed scenario 1" )
{
    FenParser fen { "r1bqk1nr/ppp2ppp/8/4p3/1bpP4/2P5/PP2NPPP/RNBQ1RK1 b kq - 0 1" };
    auto game = fen.build();

    SearchHelper helper;
    History history;
    auto search = helper.build (game.getBoard(), 4, 180);

    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    game.move (*result.move);

    // assert the bishop has moved:
    INFO( "Info:", asString (*result.move) );
    REQUIRE( game.getBoard().pieceAt (coordParse ("b4"))
             != ColoredPiece::make (Color::Black, Piece::Bishop) );
}

TEST_CASE( "Bishop is not sacrificed scenario 2 (as white)" )
{
    FenParser fen { "2b2bnr/pr1ppkpp/p1p5/q3P3/2P2N2/BP3N2/P2P1PPP/R3K2R w KQ - 2 1" };
    auto game = fen.build();

    SearchHelper helper;
    IterativeSearch search = helper.build (game.getBoard(), 4, 180);

    auto result = search.iterativelyDeepen (Color::White);

    REQUIRE( result.move.has_value() );

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
    auto search = helper.build (game.getBoard(), 4, 10);
    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    game.move (*result.move);
    // assert the pawn at d6 has been taken:
    INFO( "Chosen move:", asString (*result.move) );

    auto board = game.getBoard();
    auto target_piece = board.pieceAt (coordParse ("d6"));
    CHECK( target_piece != ColoredPiece::make (Color::White, Piece::Pawn) );
    CHECK( pieceColor (target_piece) == Color::Black );
}

TEST_CASE( "Checkmate is preferred to stalemate" )
{
    FenParser fen { "6k1/1p3pp1/p1p4p/3r4/8/2K5/b2r4/8 b - - 0 1" };
    auto game = fen.build();

    SearchHelper helper;
    auto search = helper.build (game.getBoard(), 6, 10);

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
    IterativeSearch search = helper.build (game.getBoard(), 6, 5);
    SearchResult result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move != std::nullopt );

    game.move (*result.move);

    auto is_stalemate = isStalemated (game.getBoard(), Color::White);
    CHECK( !is_stalemate );
}

TEST_CASE( "Doesn't sacrifice piece to undermine opponent's castle position" )
{
    FenParser fen { "r1bqkb1r/2pppppp/p4n2/np6/8/1B2PN2/PPPP1PPP/RNBQK2R w KQkq - 0 1 " };

    SUBCASE( "Depth 6" )
    {
        auto game = fen.build();

        SearchHelper helper;
        IterativeSearch search = helper.build (game.getBoard(), 6, 10);
        SearchResult result = search.iterativelyDeepen (Color::White);

        // Check the white bishop is not sacrificed:
        CHECK( *result.move != moveParse ("b3xf7") );
    }

    SUBCASE( "Depth 8" )
    {
        auto game = fen.build();

        SearchHelper helper;
        IterativeSearch search = helper.build (game.getBoard(), 8, 10);
        SearchResult result = search.iterativelyDeepen (Color::White);

        // Check the white bishop is not sacrificed:
        CHECK( *result.move != moveParse ("b3xf7") );
    }
}

TEST_CASE( "Root TT hit should not bypass iterative deepening search" )
{
    Board board = Board { BoardBuilder::fromDefaultPosition() };
    TranspositionTable tt = TranspositionTable::fromMegabytes (1);

    auto hash = board.getCode().getHashCode();
    Move fake_move = Move::make (coordParse ("e2"), coordParse ("e4"));
    tt.store (hash, 100, 10, BoundType::Exact, fake_move, 0);

    History history;
    auto logger = makeNullLogger();
    MoveTimer timer { 30 };

    IterativeSearch search = IterativeSearch::create (board, history, logger, timer, 6, tt);

    auto stats_before = tt.getStats();
    SearchResult result = search.iterativelyDeepen (Color::White);
    auto stats_after = tt.getStats();

    REQUIRE( result.move.has_value() );

    auto probes = stats_after.probes - stats_before.probes;
    CHECK( probes > 10 );
}

TEST_CASE( "Engine should avoid moves that allow opponent to force a draw when ahead" )
{
    // Position: Black ahead (bishop+rook+pawn vs rook+pawn)
    // Black: Kg7, Bd6, Pf4, Rg4
    // White: Kb1, Ra6, Pg2
    //
    // If Black plays Bd6-b8 followed by Bb8-d6, White can play Ra2-a6
    // for 3rd repetition (draw). Black should avoid this since it's winning.
    //
    // The bug: When the transposition table has cached evaluations of
    // intermediate positions, the search may return a cached score that
    // doesn't account for the repetition history.

    BoardBuilder builder;
    builder.addPiece ("g7", Color::Black, Piece::King);
    builder.addPiece ("d6", Color::Black, Piece::Bishop);
    builder.addPiece ("f4", Color::Black, Piece::Pawn);
    builder.addPiece ("g4", Color::Black, Piece::Rook);
    builder.addPiece ("b1", Color::White, Piece::King);
    builder.addPiece ("a6", Color::White, Piece::Rook);
    builder.addPiece ("g2", Color::White, Piece::Pawn);
    builder.setCurrentTurn (Color::Black);

    auto initial_board = Board { builder };

    // Use a shared transposition table across all searches to simulate
    // how a real game would have cached positions from earlier searches.
    TranspositionTable tt = TranspositionTable::fromMegabytes (
        TranspositionTable::Default_Size_In_Megabytes);
    auto logger = makeNullLogger();
    MoveTimer timer { 3 };
    constexpr int intermediate_depth = 15;
    constexpr int final_depth = 17;

    // Build up history and run searches at each step to populate the TT
    auto board = initial_board;
    History history = History::fromInitialBoard (board);

    // 1st occurrence: Bd6, Ra6 (initial position) - already added by fromInitialBoard

    // Search from initial position (Black to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::Black);
    }

    // Black: Bd6-b8
    auto move1 = moveParse ("d6 b8");
    board = board.withMove (Color::Black, move1);
    history.addPosition (board, move1);

    // Search from this position (White to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::White);
    }

    // White: Ra6-a2
    auto move2 = moveParse ("a6 a2");
    board = board.withMove (Color::White, move2);
    history.addPosition (board, move2);

    // Search from this position (Black to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::Black);
    }

    // Black: Bb8-d6
    auto move3 = moveParse ("b8 d6");
    board = board.withMove (Color::Black, move3);
    history.addPosition (board, move3);

    // 2nd occurrence: Bd6, Ra6
    // Search from this position (White to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::White);
    }

    // White: Ra2-a6
    auto move4 = moveParse ("a2 a6");
    board = board.withMove (Color::White, move4);
    history.addPosition (board, move4);

    // Search from this position (Black to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::Black);
    }

    // Black: Bd6-b8
    auto move5 = moveParse ("d6 b8");
    board = board.withMove (Color::Black, move5);
    history.addPosition (board, move5);

    // Search from this position (White to move)
    {
        timer.setSeconds (chrono::seconds { 1 });
        auto search = IterativeSearch::create (board, history, logger, timer, intermediate_depth, tt);
        (void)search.iterativelyDeepen (Color::White);
    }

    // Current position: Bb8, Ra2 (Black to move)
    // White: Ra6-a2
    auto move6 = moveParse ("a6 a2");
    board = board.withMove (Color::White, move6);
    history.addPosition (board, move6);

    // Now if Black plays Bb8-d6, White plays Ra2-a6 = 3rd repetition = draw
    // The TT now has cached scores for these positions from earlier searches
    // that didn't have the full repetition history.

    timer.setSeconds (chrono::seconds { 1 });
    auto search = IterativeSearch::create (board, history, logger, timer, final_depth, tt);
    auto result = search.iterativelyDeepen (Color::Black);

    REQUIRE( result.move.has_value() );

    INFO( "Move chosen: ", asString (*result.move) );
    INFO( "Score: ", result.score );

    // Black should NOT choose Bb8-d6 because it allows White to draw
    // The material advantage (~600 centipawns) is worth more than a draw
    CHECK( *result.move != moveParse ("b8 d6") );

    // Alternatively, check that the score is significantly positive (not near 0/draw)
    CHECK( result.score > 100 );
}

