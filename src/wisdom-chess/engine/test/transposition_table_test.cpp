#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess-tests.hpp"

#include <bitset>
#include <unordered_set>

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

TEST_CASE( "Hash collision analysis" )
{
    SUBCASE( "Metadata bits produce distinct hashes" )
    {
        BoardBuilder builder;
        builder.addPiece ("e1", Color::White, Piece::King);
        builder.addPiece ("a1", Color::White, Piece::Rook);
        builder.addPiece ("h1", Color::White, Piece::Rook);
        builder.addPiece ("e8", Color::Black, Piece::King);
        builder.addPiece ("a8", Color::Black, Piece::Rook);
        builder.addPiece ("h8", Color::Black, Piece::Rook);
        builder.addPiece ("e4", Color::White, Piece::Pawn);
        builder.addPiece ("d4", Color::Black, Piece::Pawn);

        std::unordered_set<BoardHashCode> hashes;

        auto addHash = [&hashes] (const BoardCode& code) {
            auto hash = code.getHashCode();
            auto [it, inserted] = hashes.insert (hash);
            CHECK( inserted );
        };

        BoardCode code = BoardCode::fromBoardBuilder (builder);
        addHash (code);

        code.setCurrentTurn (Color::Black);
        addHash (code);
        code.setCurrentTurn (Color::White);

        code.setCastleState (Color::White, CastlingEligibility::Neither_Side);
        addHash (code);

        code.setCastleState (Color::White, CastlingRights::Kingside);
        addHash (code);

        code.setCastleState (Color::White, CastlingRights::Queenside);
        addHash (code);

        code.setCastleState (Color::White, CastlingEligibility::Either_Side);
        code.setCastleState (Color::Black, CastlingEligibility::Neither_Side);
        addHash (code);

        code.setCastleState (Color::Black, CastlingEligibility::Either_Side);

        for (int col = 0; col < 8; ++col)
        {
            code.clearEnPassantTarget();
            code.setEnPassantTarget (Color::White, makeCoord (White_En_Passant_Row, col));
            addHash (code);
        }

        for (int col = 0; col < 8; ++col)
        {
            code.clearEnPassantTarget();
            code.setEnPassantTarget (Color::Black, makeCoord (Black_En_Passant_Row, col));
            addHash (code);
        }
    }

    SUBCASE( "Zobrist table has unique values" )
    {
        std::unordered_set<uint64_t> seen_values;

        for (const auto& hash_value : Hash_Code_Table)
        {
            if (hash_value == 0)
                continue;

            auto [it, inserted] = seen_values.insert (hash_value);
            CHECK_MESSAGE ( inserted, "Duplicate value found in Hash_Code_Table" );
        }
    }

    SUBCASE( "Zobrist table has good bit distribution" )
    {
        std::array<int, 48> bit_counts {};

        for (const auto& hash_value : Hash_Code_Table)
        {
            if (hash_value == 0)
                continue;

            for (int bit = 0; bit < 48; ++bit)
            {
                if ((hash_value >> bit) & 1)
                    bit_counts[bit]++;
            }
        }

        int non_zero_entries = 0;
        for (const auto& hash_value : Hash_Code_Table)
        {
            if (hash_value != 0)
                non_zero_entries++;
        }

        for (int bit = 0; bit < 48; ++bit)
        {
            double ratio = static_cast<double> (bit_counts[bit]) / non_zero_entries;
            CHECK_MESSAGE ( ratio > 0.3, "Bit " << bit << " is set too rarely: " << ratio );
            CHECK_MESSAGE ( ratio < 0.7, "Bit " << bit << " is set too often: " << ratio );
        }
    }

    SUBCASE( "Different piece placements produce unique hashes" )
    {
        std::unordered_set<BoardHashCode> hashes;

        for (int row = 1; row < Num_Rows - 1; ++row)
        {
            for (int col = 0; col < Num_Columns; ++col)
            {
                BoardBuilder builder;
                builder.addPiece ("e1", Color::White, Piece::King);
                builder.addPiece ("e8", Color::Black, Piece::King);
                builder.addPiece (row, col, Color::White, Piece::Knight);

                Board board { builder };
                auto hash = board.getCode().getHashCode();
                auto [it, inserted] = hashes.insert (hash);
                CHECK( inserted );
            }
        }
    }
}
