#include <algorithm>
#include <cmath>
#include <iostream>
#include <unordered_map>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

#include "wisdom-chess-tests.hpp"

using std::string;
using wisdom::Board;
using wisdom::BoardBuilder;
using wisdom::BoardHashCode;
using wisdom::Color;
using wisdom::FenParser;
using wisdom::colorInvert;
using wisdom::generateAllPotentialMoves;
using wisdom::isLegalPositionAfterMove;

namespace
{
    auto normalizePositionFen (const string& fen) -> string
    {
        std::size_t last_space = fen.rfind (' ');
        if (last_space == string::npos)
            return fen;
        std::size_t second_last_space = fen.rfind (' ', last_space - 1);
        if (second_last_space == string::npos)
            return fen;
        return fen.substr (0, second_last_space);
    }

    struct CollisionStats
    {
        int64_t positions_visited = 0;
        int64_t collisions_found = 0;
        std::vector<std::pair<string, string>> collision_examples;
    };

    void searchForCollisions (
        const Board& board,
        Color side,
        int depth,
        int max_depth,
        std::unordered_map<BoardHashCode, string>& seen_positions,
        CollisionStats& stats
    )
    {
        if (depth >= max_depth)
            return;

        const auto moves = generateAllPotentialMoves (board, side);

        for (auto move : moves)
        {
            Board new_board = board.withMove (side, move);

            if (!isLegalPositionAfterMove (new_board, side, move))
                continue;

            auto hash = new_board.getCode().getHashCode();
            auto full_fen = new_board.toFenString (colorInvert (side));
            auto position_fen = normalizePositionFen (full_fen);

            stats.positions_visited++;

            auto it = seen_positions.find (hash);
            if (it != seen_positions.end())
            {
                if (it->second != position_fen)
                {
                    stats.collisions_found++;
                    if (stats.collision_examples.size() < 5)
                        stats.collision_examples.emplace_back (it->second, position_fen);
                }
            }
            else
            {
                seen_positions[hash] = position_fen;
            }

            searchForCollisions (
                new_board, colorInvert (side), depth + 1, max_depth, seen_positions, stats
            );
        }
    }

    auto runCollisionTest (const Board& board, Color active_player, int depth) -> CollisionStats
    {
        std::unordered_map<BoardHashCode, string> seen_positions;
        CollisionStats stats;

        auto initial_hash = board.getCode().getHashCode();
        auto initial_fen = normalizePositionFen (board.toFenString (active_player));
        seen_positions[initial_hash] = initial_fen;
        stats.positions_visited = 1;

        searchForCollisions (board, active_player, 0, depth, seen_positions, stats);

        return stats;
    }
}

TEST_CASE( "Large-scale hash collision detection" )
{
    SUBCASE( "Perft depth 5 from starting position" )
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        auto stats = runCollisionTest (board, Color::White, 5);

        MESSAGE ( "Positions visited: " << stats.positions_visited );
        MESSAGE ( "Collisions found: " << stats.collisions_found );

        if (stats.collisions_found > 0)
        {
            MESSAGE ( "Example collisions:" );
            for (const auto& [fen1, fen2] : stats.collision_examples)
            {
                MESSAGE ( "  FEN1: " << fen1 );
                MESSAGE ( "  FEN2: " << fen2 );
            }
        }

        CHECK( stats.positions_visited > 4000000 );
        CHECK( stats.collisions_found == 0 );
    }

    SUBCASE( "Multiple starting positions for diversity" )
    {
        std::vector<std::pair<string, int>> test_positions = {
            { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4 },
            { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 5 },
            { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4 },
            { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1", 4 },
        };

        int64_t total_positions = 0;
        int64_t total_collisions = 0;

        for (const auto& [fen, depth] : test_positions)
        {
            FenParser parser { fen };
            Board board = parser.buildBoard();
            Color active = parser.getActivePlayer();

            auto stats = runCollisionTest (board, active, depth);
            total_positions += stats.positions_visited;
            total_collisions += stats.collisions_found;

            MESSAGE ( "Position: " << fen.substr (0, 40) << "..." );
            MESSAGE ( "  Visited: " << stats.positions_visited << ", Collisions: " << stats.collisions_found );

            if (stats.collisions_found > 0)
            {
                MESSAGE ( "  Example collisions:" );
                for (const auto& [fen1, fen2] : stats.collision_examples)
                {
                    MESSAGE ( "    FEN1: " << fen1 );
                    MESSAGE ( "    FEN2: " << fen2 );
                }
            }
        }

        MESSAGE ( "Total positions: " << total_positions );
        MESSAGE ( "Total collisions: " << total_collisions );

        CHECK( total_positions > 1000000 );
        CHECK( total_collisions == 0 );
    }
}

namespace
{
    struct IndexDistributionStats
    {
        int64_t total_hashes = 0;
        size_t num_buckets = 0;
        size_t min_bucket_count = 0;
        size_t max_bucket_count = 0;
        double avg_bucket_count = 0.0;
        double std_deviation = 0.0;
    };

    void collectHashesForDistribution (
        const Board& board,
        Color side,
        int depth,
        int max_depth,
        std::vector<BoardHashCode>& hashes
    )
    {
        if (depth >= max_depth)
            return;

        const auto moves = generateAllPotentialMoves (board, side);

        for (auto move : moves)
        {
            Board new_board = board.withMove (side, move);

            if (!isLegalPositionAfterMove (new_board, side, move))
                continue;

            hashes.push_back (new_board.getCode().getHashCode());

            collectHashesForDistribution (
                new_board, colorInvert (side), depth + 1, max_depth, hashes
            );
        }
    }

    auto analyzeIndexDistribution (
        const std::vector<BoardHashCode>& hashes,
        size_t table_size
    )
        -> IndexDistributionStats
    {
        size_t size_mask = table_size - 1;
        std::vector<size_t> bucket_counts (table_size, 0);

        for (auto hash : hashes)
        {
            auto index = wisdom::foldHashTo32Bits (hash) & size_mask;
            bucket_counts[index]++;
        }

        IndexDistributionStats stats;
        stats.total_hashes = wisdom::narrow_cast<int64_t> (hashes.size());
        stats.num_buckets = table_size;

        auto [min_it, max_it] = std::minmax_element (
            bucket_counts.begin(), bucket_counts.end()
        );
        stats.min_bucket_count = *min_it;
        stats.max_bucket_count = *max_it;
        stats.avg_bucket_count = static_cast<double> (hashes.size())
            / static_cast<double> (table_size);

        double sum_squared_diff = 0.0;
        for (auto count : bucket_counts)
        {
            double diff = static_cast<double> (count) - stats.avg_bucket_count;
            sum_squared_diff += diff * diff;
        }
        stats.std_deviation = std::sqrt (sum_squared_diff / static_cast<double> (table_size));

        return stats;
    }
}

TEST_CASE( "Transposition table index distribution" )
{
    SUBCASE( "Index hash produces uniform distribution" )
    {
        Board board { BoardBuilder::fromDefaultPosition() };
        std::vector<BoardHashCode> hashes;
        hashes.reserve (5000000);

        hashes.push_back (board.getCode().getHashCode());
        collectHashesForDistribution (board, Color::White, 0, 5, hashes);

        MESSAGE ( "Collected " << hashes.size() << " hashes" );

        constexpr size_t table_size = 524288;
        auto stats = analyzeIndexDistribution (hashes, table_size);

        MESSAGE ( "Index distribution statistics:" );
        MESSAGE ( "  Total hashes: " << stats.total_hashes );
        MESSAGE ( "  Table size: " << stats.num_buckets );
        MESSAGE ( "  Min bucket count: " << stats.min_bucket_count );
        MESSAGE ( "  Max bucket count: " << stats.max_bucket_count );
        MESSAGE ( "  Avg bucket count: " << stats.avg_bucket_count );
        MESSAGE ( "  Std deviation: " << stats.std_deviation );

        double max_to_avg_ratio = static_cast<double> (stats.max_bucket_count)
            / stats.avg_bucket_count;
        MESSAGE ( "  Max/Avg ratio: " << max_to_avg_ratio );

        CHECK( stats.total_hashes > 4000000 );
        CHECK( max_to_avg_ratio < 10.0 );
        CHECK( stats.std_deviation < stats.avg_bucket_count );
    }
}
