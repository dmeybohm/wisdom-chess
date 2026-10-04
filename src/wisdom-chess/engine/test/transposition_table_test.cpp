#include <bitset>
#include <unordered_set>

#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/global.hpp"

#include "wisdom-chess-tests.hpp"

using namespace wisdom;

// A table has exactly one owner: it can be moved, but never copied, so that a
// silent duplicate cannot be shared between two searching threads.
static_assert (!std::is_copy_constructible_v<TranspositionTable>);
static_assert (!std::is_copy_assignable_v<TranspositionTable>);
static_assert (std::is_nothrow_move_constructible_v<TranspositionTable>);
static_assert (std::is_nothrow_move_assignable_v<TranspositionTable>);

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

        CHECK( tt.getStats().probes == 0 );
        CHECK( tt.getStats().hits == 0 );

        (void)tt.probe (hash, 5, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( tt.getStats().probes == 1 );
        CHECK( tt.getStats().hits == 1 );

        (void)tt.probe (hash, 10, -Initial_Alpha, Initial_Alpha, 0);
        CHECK( tt.getStats().probes == 2 );
        CHECK( tt.getStats().hits == 1 );
    }

    SUBCASE( "counts an entry with a zero hash once" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);

        BoardHashCode hash = 0;
        tt.store (hash, 100, 5, BoundType::Exact, Move::make (0, 0, 1, 1), 0);
        tt.store (hash, 100, 6, BoundType::Exact, Move::make (0, 0, 1, 1), 0);

        CHECK( tt.getStats().stored_entries == 1 );

        tt.clear();
        CHECK( tt.getStats().stored_entries == 0 );

        tt.store (hash, 100, 5, BoundType::Exact, Move::make (0, 0, 1, 1), 0);
        CHECK( tt.getStats().stored_entries == 1 );
    }
}

TEST_CASE( "Depth and score bits" )
{
    SUBCASE( "default is depth zero and score zero" )
    {
        DepthAndScoreBits bits {};
        CHECK( bits.getDepth() == 0 );
        CHECK( bits.getScore() == 0 );
    }

    SUBCASE( "round trips the limits of each field" )
    {
        for (int depth : { 0, 1, Max_Search_Depth, DepthAndScoreBits::Max_Depth })
        {
            for (int score : {
                     0, 1, -1,
                     Checkmate_Score, -Checkmate_Score,
                     DepthAndScoreBits::Max_Score, DepthAndScoreBits::Min_Score
                 })
            {
                CAPTURE( depth );
                CAPTURE( score );
                auto bits = DepthAndScoreBits::make (depth, score);
                CHECK( bits.getDepth() == depth );
                CHECK( bits.getScore() == score );
            }
        }
    }

    SUBCASE( "keeps mate scores at a distance from the root" )
    {
        TranspositionTable tt = TranspositionTable::fromEntries (4);
        BoardHashCode hash = 12345678ULL;
        int mate_score = checkmateScoreInMoves (5);

        tt.store (hash, mate_score, 3, BoundType::Exact, Move::make (0, 0, 1, 1), 2);
        auto from_same_ply = tt.probe (hash, 3, -Initial_Alpha, Initial_Alpha, 2);
        auto from_deeper_ply = tt.probe (hash, 3, -Initial_Alpha, Initial_Alpha, 4);

        REQUIRE( from_same_ply.has_value() );
        REQUIRE( from_deeper_ply.has_value() );
        CHECK( *from_same_ply == mate_score );
        CHECK( *from_deeper_ply == mate_score - 2 );

        tt.store (hash, -mate_score, 4, BoundType::Exact, Move::make (0, 0, 1, 1), 2);
        auto losing = tt.probe (hash, 4, -Initial_Alpha, Initial_Alpha, 4);

        REQUIRE( losing.has_value() );
        CHECK( *losing == -mate_score + 2 );
    }
}

TEST_CASE( "Transposition table buckets" )
{
    // One bucket, so every position shares it.
    TranspositionTable tt = TranspositionTable::fromEntries (4);
    auto move = Move::make (0, 0, 1, 1);

    auto store = [&] (BoardHashCode hash, int depth, int score = 0) {
        tt.store (hash, score, depth, BoundType::Exact, move, 0);
    };
    auto holds = [&] (BoardHashCode hash) {
        return tt.getBestMove (hash).has_value();
    };

    SUBCASE( "holds four positions" )
    {
        for (BoardHashCode hash = 1; hash <= 4; hash++)
            store (hash, narrow<int> (hash), narrow<int> (hash) * 10);

        for (BoardHashCode hash = 1; hash <= 4; hash++)
        {
            CAPTURE( hash );
            auto score = tt.probe (hash, 1, -Initial_Alpha, Initial_Alpha, 0);
            REQUIRE( score.has_value() );
            CHECK( *score == narrow<int> (hash) * 10 );
        }
        CHECK( tt.getStats().stored_entries == 4 );
    }

    SUBCASE( "a fifth position replaces the shallowest" )
    {
        store (1, 3);
        store (2, 1);
        store (3, 4);
        store (4, 2);
        store (5, 5);

        CHECK( !holds (2) );
        CHECK( holds (1) );
        CHECK( holds (3) );
        CHECK( holds (4) );
        CHECK( holds (5) );
        CHECK( tt.getStats().stored_entries == 4 );
    }

    SUBCASE( "an older entry is replaced before a newer one of the same depth" )
    {
        store (1, 3);
        store (2, 3);
        store (3, 3);
        store (4, 3);
        tt.startSearch();
        store (1, 3);
        store (2, 3);
        store (3, 3);
        store (5, 3);

        CHECK( !holds (4) );
        CHECK( holds (1) );
        CHECK( holds (2) );
        CHECK( holds (3) );
        CHECK( holds (5) );
    }

    SUBCASE( "an older entry not deep enough is replaced before shallower ones" )
    {
        store (1, 1);
        store (2, 1);
        store (3, 1);
        store (4, 5);
        tt.startSearch();
        store (1, 1);
        store (2, 1);
        store (3, 1);
        store (5, 1);

        CHECK( !holds (4) );
        CHECK( holds (1) );
        CHECK( holds (2) );
        CHECK( holds (3) );
        CHECK( holds (5) );
    }

    SUBCASE( "an older entry deep enough outweighs its age" )
    {
        store (1, 12);
        tt.startSearch();
        store (2, 1);
        store (3, 1);
        store (4, 1);
        store (5, 1);

        CHECK( holds (1) );
        CHECK( !holds (2) );
        CHECK( holds (3) );
        CHECK( holds (4) );
        CHECK( holds (5) );
    }

    SUBCASE( "a stored position is updated in place unless it is deeper" )
    {
        store (1, 3, 10);
        store (2, 1);
        store (3, 1);
        store (4, 1);

        store (1, 5, 20);
        auto deeper = tt.probe (1, 1, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( deeper.has_value() );
        CHECK( *deeper == 20 );

        store (1, 2, 30);
        auto kept = tt.probe (1, 1, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( kept.has_value() );
        CHECK( *kept == 20 );

        CHECK( holds (2) );
        CHECK( holds (3) );
        CHECK( holds (4) );
        CHECK( tt.getStats().stored_entries == 4 );
    }

    SUBCASE( "finding a position makes its entry current" )
    {
        store (1, 3);
        store (2, 3);
        store (3, 3);
        store (4, 3);
        tt.startSearch();

        (void)tt.probe (1, 3, -Initial_Alpha, Initial_Alpha, 0);
        store (5, 3);

        CHECK( holds (1) );
        CHECK( !holds (2) );
    }

    SUBCASE( "a zero hash does not match an empty entry" )
    {
        CHECK( !tt.probe (0, 0, -Initial_Alpha, Initial_Alpha, 0).has_value() );
        CHECK( !holds (0) );

        store (0, 1, 40);
        auto score = tt.probe (0, 1, -Initial_Alpha, Initial_Alpha, 0);
        REQUIRE( score.has_value() );
        CHECK( *score == 40 );
    }

    SUBCASE( "clear empties the bucket" )
    {
        for (BoardHashCode hash = 1; hash <= 4; hash++)
            store (hash, 1);
        tt.startSearch();
        tt.clear();

        for (BoardHashCode hash = 1; hash <= 4; hash++)
            CHECK( !holds (hash) );
        CHECK( tt.getStats().stored_entries == 0 );
    }
}

TEST_CASE( "Transposition table sizing" )
{
    SUBCASE( "Smallest allowed size is a non-empty power of two" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);
        auto size = tt.getSize();

        CHECK( size >= 2 );
        CHECK( (size & (size - 1)) == 0 );
        CHECK( size * sizeof (TranspositionEntry) <= 1024 * 1024 );
    }

    SUBCASE( "Default size matches the named constant" )
    {
        TranspositionTable by_default {};
        TranspositionTable by_constant
            = TranspositionTable::fromMegabytes (TranspositionTable::Default_Size_In_Megabytes);

        CHECK( by_default.getSize() == by_constant.getSize() );
        CHECK( by_default.getSize() >= 2 );
    }

    SUBCASE( "A size that is a whole number of entries uses all of it" )
    {
        auto entries_per_megabyte = 1024 * 1024 / sizeof (TranspositionEntry);
        REQUIRE( std::has_single_bit (entries_per_megabyte) );

        CHECK( TranspositionTable::fromMegabytes (1).getSize() == entries_per_megabyte );
        CHECK( TranspositionTable::fromMegabytes (16).getSize() == 16 * entries_per_megabyte );
    }

    SUBCASE( "An entry count that is a power of two is kept" )
    {
        CHECK( TranspositionTable::fromEntries (4).getSize() == 4 );
    }
}

TEST_CASE( "Transposition table with real board positions" )
{
    SUBCASE( "stores and retrieves using actual board hash" )
    {
        TranspositionTable tt = TranspositionTable::fromMegabytes (1);
        Board board = Board { BoardBuilder::fromDefaultPosition() };

        auto hash = board.getBoardCode().getHashCode();
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

        auto hash1 = board1.getBoardCode().getHashCode();
        auto hash2 = board2.getBoardCode().getHashCode();

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

        auto add_hash = [&hashes] (const BoardCode& code) {
            auto hash = code.getHashCode();
            auto [it, inserted] = hashes.insert (hash);
            CHECK( inserted );
        };

        BoardCode code = BoardCode::fromBoardBuilder (builder);
        add_hash (code);

        code.setCurrentTurn (Color::Black);
        add_hash (code);
        code.setCurrentTurn (Color::White);

        code.setCastleState (Color::White, CastlingEligibility::Neither_Side);
        add_hash (code);

        code.setCastleState (Color::White, CastlingRights::Kingside);
        add_hash (code);

        code.setCastleState (Color::White, CastlingRights::Queenside);
        add_hash (code);

        code.setCastleState (Color::White, CastlingEligibility::Both_Sides);
        code.setCastleState (Color::Black, CastlingEligibility::Neither_Side);
        add_hash (code);

        code.setCastleState (Color::Black, CastlingEligibility::Both_Sides);

        for (int col = 0; col < 8; ++col)
        {
            code.clearEnPassantTarget();
            code.setEnPassantTarget (
                Color::White,
                makeCoord (White_En_Passant_Row, col),
                EnPassantTargetState::Legal
            );
            add_hash (code);
        }

        for (int col = 0; col < 8; ++col)
        {
            code.clearEnPassantTarget();
            code.setEnPassantTarget (
                Color::Black,
                makeCoord (Black_En_Passant_Row, col),
                EnPassantTargetState::Legal
            );
            add_hash (code);
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
            CHECK_MESSAGE( inserted, "Duplicate value found in Hash_Code_Table" );
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
            double ratio = to_double (bit_counts[bit]) / non_zero_entries;
            CHECK_MESSAGE( ratio > 0.3, "Bit " << bit << " is set too rarely: " << ratio );
            CHECK_MESSAGE( ratio < 0.7, "Bit " << bit << " is set too often: " << ratio );
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
                auto hash = board.getBoardCode().getHashCode();
                auto [it, inserted] = hashes.insert (hash);
                CHECK( inserted );
            }
        }
    }
}
