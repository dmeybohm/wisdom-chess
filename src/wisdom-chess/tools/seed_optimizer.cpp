#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <compare>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <vector>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/random.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

using wisdom::Board;
using wisdom::BoardBuilder;
using wisdom::BoardHashCode;
using wisdom::Color;
using wisdom::colorInvert;
using wisdom::CompileTimeRandom;
using wisdom::foldHashTo32Bits;
using wisdom::generateAllPotentialMoves;
using wisdom::isLegalPositionAfterMove;
using wisdom::nonnull;
using wisdom::Num_Squares;
using wisdom::randomInitialState;
using wisdom::randomSeed;
using wisdom::to_double;
using wisdom::to_int;
using wisdom::to_unsigned;
using wisdom::Zobrist_Table_Size;
using wisdom::zobristKey;
using wisdom::zobristPieceIndex;

namespace
{

    using ZobristTable = std::array<std::uint64_t, Zobrist_Table_Size>;

    auto generateZobristTable (std::uint64_t seed) -> ZobristTable
    {
        ZobristTable table {};
        CompileTimeRandom rng { CompileTimeRandom::RandomState { randomInitialState(), seed } };

        for (std::size_t i = 0; i < Zobrist_Table_Size; i++)
            table[i] = zobristKey (&rng);

        return table;
    }

    auto computeHashWithTable (const Board& board, const ZobristTable& table) -> BoardHashCode
    {
        BoardHashCode hash = 0;

        for (auto coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (!piece)
                continue;

            auto piece_index = zobristPieceIndex (piece->color(), piece->type());
            auto table_index = to_unsigned<std::size_t> (piece_index * Num_Squares + coord.index());
            hash ^= table[table_index];
        }

        hash |= board.getUnnormalizedBoardCode().getMetadataBits();
        return hash;
    }

    // What a board's hash covers: the piece on each square, and the
    // metadata. Boards with the same key always hash alike, so each counts
    // once in the distribution.
    struct PositionKey
    {
        std::array<std::int8_t, Num_Squares> squares {};
        std::uint16_t metadata = 0;

        auto operator<=> (const PositionKey&) const = default;
    };

    auto positionKey (const Board& board) -> PositionKey
    {
        PositionKey key;

        for (auto coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (piece)
                key.squares[coord.index<std::size_t>()] = to_int<std::int8_t> (*piece);
        }

        key.metadata = board.getUnnormalizedBoardCode().getMetadataBits();
        return key;
    }

    // Keeps each distinct position once. A duplicate is still searched: it
    // may be reached here in fewer moves than before, with more depth left.
    void collectPositions (
        const Board& board,
        Color side,
        int depth,
        int max_depth,
        nonnull<std::vector<Board>> positions,
        nonnull<std::set<PositionKey>> seen
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

            if (seen->insert (positionKey (new_board)).second)
                positions->push_back (new_board);

            collectPositions (
                new_board, colorInvert (side), depth + 1, max_depth, positions, seen
            );
        }
    }

    auto collectAllPositions() -> std::vector<Board>
    {
        std::vector<Board> positions;
        std::set<PositionKey> seen;

        Board board { BoardBuilder::fromDefaultPosition() };
        positions.push_back (board);
        seen.insert (positionKey (board));

        std::cout << "Collecting distinct positions from depth-5 perft..." << std::endl;
        collectPositions (board, Color::White, 0, 5, &positions, &seen);
        std::cout << "Collected " << positions.size() << " positions" << std::endl;

        return positions;
    }

    struct DistributionStats
    {
        std::size_t table_size = 0;
        std::size_t min_bucket = 0;
        std::size_t max_bucket = 0;
        double avg_bucket = 0.0;
        double std_dev = 0.0;
        double max_avg_ratio = 0.0;
    };

    auto analyzeDistribution (const std::vector<BoardHashCode>& hashes, std::size_t table_size)
        -> DistributionStats
    {
        std::size_t size_mask = table_size - 1;
        std::vector<std::size_t> buckets (table_size, 0);

        for (auto hash : hashes)
        {
            auto index = foldHashTo32Bits (hash) & size_mask;
            buckets[index]++;
        }

        DistributionStats stats;
        stats.table_size = table_size;

        auto [min_it, max_it] = std::minmax_element (buckets.begin(), buckets.end());
        stats.min_bucket = *min_it;
        stats.max_bucket = *max_it;
        stats.avg_bucket = to_double (hashes.size()) / to_double (table_size);
        stats.max_avg_ratio = to_double (stats.max_bucket) / stats.avg_bucket;

        double sum_sq_diff = 0.0;
        for (auto count : buckets)
        {
            double diff = to_double (count) - stats.avg_bucket;
            sum_sq_diff += diff * diff;
        }
        stats.std_dev = std::sqrt (sum_sq_diff / to_double (table_size));

        return stats;
    }

    struct SeedResult
    {
        std::uint64_t seed = 0;
        double worst_ratio = 0.0;
        double avg_std_dev = 0.0;
        std::array<DistributionStats, 4> stats_per_size {};
    };

    constexpr std::array<std::size_t, 4> Table_Sizes = {
        131072,    // 2^17
        524288,    // 2^19
        1048576,   // 2^20
        2097152,   // 2^21
    };

    auto evaluateSeed (
        std::uint64_t seed,
        const std::vector<Board>& positions
    ) -> SeedResult
    {
        auto table = generateZobristTable (seed);

        std::vector<BoardHashCode> hashes;
        hashes.reserve (positions.size());
        for (const auto& board : positions)
            hashes.push_back (computeHashWithTable (board, table));

        SeedResult result;
        result.seed = seed;
        result.worst_ratio = 0.0;

        double total_std_dev = 0.0;
        for (std::size_t i = 0; i < Table_Sizes.size(); i++)
        {
            result.stats_per_size[i] = analyzeDistribution (hashes, Table_Sizes[i]);
            result.worst_ratio = std::max (result.worst_ratio, result.stats_per_size[i].max_avg_ratio);
            total_std_dev += result.stats_per_size[i].std_dev;
        }
        result.avg_std_dev = total_std_dev / to_double (Table_Sizes.size());

        return result;
    }

    void printStats (const DistributionStats& stats)
    {
        auto log2_size = std::bit_width (stats.table_size) - 1;
        std::cout << "  2^" << log2_size << ": max/avg=" << std::fixed << std::setprecision (2)
                  << stats.max_avg_ratio << ", std=" << stats.std_dev
                  << " (max=" << stats.max_bucket << ", avg=" << stats.avg_bucket << ")"
                  << std::endl;
    }

    void printSeedResult (const SeedResult& result, bool verbose = false)
    {
        std::cout << "Seed 0x" << std::hex << result.seed << std::dec
                  << ": worst=" << std::fixed << std::setprecision (2) << result.worst_ratio
                  << ", avg_std=" << result.avg_std_dev << std::endl;
        if (verbose)
        {
            for (const auto& stats : result.stats_per_size)
                printStats (stats);
        }
    }
}

auto main() -> int
{
    const std::uint64_t Current_Seed = randomSeed();
    constexpr std::uint64_t Num_Seeds_To_Test = 1000;

    auto positions = collectAllPositions();
    std::cout << "\nPositions collected: " << positions.size() << "\n" << std::endl;

    std::cout << "Current seed (0x" << std::hex << Current_Seed << std::dec << "):" << std::endl;
    auto current_result = evaluateSeed (Current_Seed, positions);
    for (const auto& stats : current_result.stats_per_size)
        printStats (stats);
    std::cout << "Worst ratio: " << current_result.worst_ratio << "\n" << std::endl;

    std::cout << "Testing " << Num_Seeds_To_Test << " random seeds..." << std::endl;

    std::random_device rd;
    std::mt19937_64 rng { rd() };

    std::vector<SeedResult> results;
    results.reserve (Num_Seeds_To_Test);

    for (std::uint64_t i = 0; i < Num_Seeds_To_Test; i++)
    {
        auto seed = rng();
        auto result = evaluateSeed (seed, positions);
        results.push_back (result);

        if ((i + 1) % 100 == 0)
            std::cout << "  Progress: " << (i + 1) << "/" << Num_Seeds_To_Test << std::endl;
    }

    std::sort (results.begin(), results.end(), [] (const auto& a, const auto& b) {
        if (a.worst_ratio != b.worst_ratio)
            return a.worst_ratio < b.worst_ratio;
        return a.avg_std_dev < b.avg_std_dev;
    });

    std::cout << "\nTop 10 seeds by worst max/avg ratio:" << std::endl;
    for (std::size_t i = 0; i < std::min<std::size_t> (10, results.size()); i++)
    {
        const auto& result = results[i];
        double improvement = 100.0 * (current_result.worst_ratio - result.worst_ratio)
            / current_result.worst_ratio;
        std::cout << "  " << (i + 1) << ". ";
        printSeedResult (result);
        std::cout << "     Improvement: " << std::fixed << std::setprecision (1)
                  << improvement << "%" << std::endl;
        for (const auto& stats : result.stats_per_size)
            printStats (stats);
        std::cout << std::endl;
    }

    std::cout << "\nWorst 3 seeds:" << std::endl;
    for (std::size_t i = 0; i < std::min<std::size_t> (3, results.size()); i++)
    {
        const auto& result = results[results.size() - 1 - i];
        std::cout << "  " << (i + 1) << ". ";
        printSeedResult (result);
    }

    return 0;
}
