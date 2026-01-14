#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

TEST_CASE( "Transposition table" )
{
    SUBCASE( "stores and retrieves exact scores" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 100;
        int depth = 5;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::Exact, move, 0);

        auto result = tt.probe (hash, depth, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == score );
    }

    SUBCASE( "returns empty for different hash" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        BoardHashCode different_hash = 87654321ULL;
        int score = 100;
        int depth = 5;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::Exact, move, 0);

        auto result = tt.probe (different_hash, depth, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( !result.has_value() );
    }

    SUBCASE( "returns empty for insufficient depth" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 100;
        int stored_depth = 3;
        int query_depth = 5;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, stored_depth, BoundType::Exact, move, 0);

        auto result = tt.probe (hash, query_depth, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( !result.has_value() );
    }

    SUBCASE( "returns score when stored depth is greater" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 100;
        int stored_depth = 7;
        int query_depth = 5;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, stored_depth, BoundType::Exact, move, 0);

        auto result = tt.probe (hash, query_depth, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == score );
    }

    SUBCASE( "lower bound causes cutoff when score >= beta" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 500;
        int depth = 5;
        int alpha = 100;
        int beta = 400;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::LowerBound, move, 0);

        auto result = tt.probe (hash, depth, alpha, beta, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == score );
    }

    SUBCASE( "lower bound does not cause cutoff when score < beta" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 300;
        int depth = 5;
        int alpha = 100;
        int beta = 400;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::LowerBound, move, 0);

        auto result = tt.probe (hash, depth, alpha, beta, 0);
        CHECK( !result.has_value() );
    }

    SUBCASE( "upper bound causes cutoff when score <= alpha" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 50;
        int depth = 5;
        int alpha = 100;
        int beta = 400;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::UpperBound, move, 0);

        auto result = tt.probe (hash, depth, alpha, beta, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == score );
    }

    SUBCASE( "upper bound does not cause cutoff when score > alpha" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        int score = 200;
        int depth = 5;
        int alpha = 100;
        int beta = 400;
        Move move = Move::make (0, 0, 1, 1);

        tt.store (hash, score, depth, BoundType::UpperBound, move, 0);

        auto result = tt.probe (hash, depth, alpha, beta, 0);
        CHECK( !result.has_value() );
    }

    SUBCASE( "getBestMove returns stored move" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        Move move = Move::make (1, 2, 3, 4);

        tt.store (hash, 100, 5, BoundType::Exact, move, 0);

        auto result = tt.getBestMove (hash);
        REQUIRE( result.has_value() );
        CHECK( result->getSrc() == move.getSrc() );
        CHECK( result->getDst() == move.getDst() );
    }

    SUBCASE( "getBestMove returns empty for different hash" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        BoardHashCode different_hash = 87654321ULL;
        Move move = Move::make (1, 2, 3, 4);

        tt.store (hash, 100, 5, BoundType::Exact, move, 0);

        auto result = tt.getBestMove (different_hash);
        CHECK( !result.has_value() );
    }

    SUBCASE( "does not replace deeper entry with shallower one" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        Move move1 = Move::make (1, 1, 2, 2);
        Move move2 = Move::make (3, 3, 4, 4);

        tt.store (hash, 100, 7, BoundType::Exact, move1, 0);
        tt.store (hash, 200, 5, BoundType::Exact, move2, 0);

        auto result = tt.probe (hash, 5, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == 100 );

        auto move_result = tt.getBestMove (hash);
        REQUIRE( move_result.has_value() );
        CHECK( move_result->getSrc() == move1.getSrc() );
    }

    SUBCASE( "replaces entry with deeper or equal depth" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        Move move1 = Move::make (1, 1, 2, 2);
        Move move2 = Move::make (3, 3, 4, 4);

        tt.store (hash, 100, 5, BoundType::Exact, move1, 0);
        tt.store (hash, 200, 7, BoundType::Exact, move2, 0);

        auto result = tt.probe (hash, 5, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == 200 );
    }

    SUBCASE( "clear resets the table" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        tt.store (hash, 100, 5, BoundType::Exact, Move::make (0, 0, 1, 1), 0);

        tt.clear();

        auto result = tt.probe (hash, 5, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( !result.has_value() );
    }

    SUBCASE( "tracks hit and probe counts" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 12345678ULL;
        tt.store (hash, 100, 5, BoundType::Exact, Move::make (0, 0, 1, 1), 0);

        CHECK( tt.getProbeCount() == 0 );
        CHECK( tt.getHitCount() == 0 );

        (void)tt.probe (hash, 5, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( tt.getProbeCount() == 1 );
        CHECK( tt.getHitCount() == 1 );

        (void)tt.probe (hash, 10, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( tt.getProbeCount() == 2 );
        CHECK( tt.getHitCount() == 1 );
    }
}

TEST_CASE( "Transposition table with real board positions" )
{
    SUBCASE( "stores and retrieves using actual board hash" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);
        Board board = Board { BoardBuilder::fromDefaultPosition() };

        auto hash = board.getCode().getHashCode();
        Move move = Move::make (1, 4, 3, 4);

        tt.store (hash, 50, 4, BoundType::Exact, move, 0);

        auto result = tt.probe (hash, 4, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( result.has_value() );
        CHECK( *result == 50 );
    }

    SUBCASE( "different positions have different hashes" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);
        Board board1 = Board { BoardBuilder::fromDefaultPosition() };

        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("d4", Color::White, Piece::Pawn);
        Board board2 { builder };

        auto hash1 = board1.getCode().getHashCode();
        auto hash2 = board2.getCode().getHashCode();

        CHECK( hash1 != hash2 );

        tt.store (hash1, 100, 5, BoundType::Exact, Move::make (0, 0, 1, 1), 0);
        tt.store (hash2, 200, 5, BoundType::Exact, Move::make (2, 2, 3, 3), 0);

        auto result1 = tt.probe (hash1, 5, -Initial_Alpha, Initial_Alpha, 0);
        auto result2 = tt.probe (hash2, 5, -Initial_Alpha, Initial_Alpha, 0);

        REQUIRE( result1.has_value() );
        REQUIRE( result2.has_value() );
        CHECK( *result1 == 100 );
        CHECK( *result2 == 200 );
    }
}
