#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

#include "wisdom-chess/engine/board_builder.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/transposition_table.hpp"

using wisdom::Board;
using wisdom::BoardBuilder;
using wisdom::BoardHashCode;
using wisdom::Color;
using wisdom::colorInvert;
using wisdom::foldHashTo32Bits;
using wisdom::generateAllPotentialMoves;
using wisdom::isLegalPositionAfterMove;
using wisdom::Num_Piece_Types;
using wisdom::Num_Players;
using wisdom::Num_Squares;
using wisdom::zobristPieceIndex;

namespace
{
    constexpr std::size_t Zobrist_Table_Size = (Num_Players + 1) * Num_Piece_Types * Num_Squares;
    constexpr int Total_Metadata_Bits = 16;

    using ZobristTable = std::array<std::uint64_t, Zobrist_Table_Size>;

    class RuntimePcg32
    {
    public:
        explicit RuntimePcg32 (std::uint64_t seed)
            : my_state { 0 }
            , my_inc { seed | 1ULL }
        {
            next();
            my_state += seed;
            next();
        }

        auto next() -> std::uint32_t
        {
            std::uint64_t old_state = my_state;
            my_state = old_state * 6364136223846793005ULL + my_inc;
            std::uint32_t xorshifted = static_cast<std::uint32_t> (
                ((old_state >> 18u) ^ old_state) >> 27u
            );
            std::uint32_t rot = static_cast<std::uint32_t> (old_state >> 59u);
            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }

        auto next48() -> std::uint64_t
        {
            return ((next() & 0xffff0000ULL) << 16ULL) | next();
        }

    private:
        std::uint64_t my_state;
        std::uint64_t my_inc;
    };

    auto generateZobristTable (std::uint64_t seed) -> ZobristTable
    {
        ZobristTable table {};
        RuntimePcg32 rng { seed };

        for (std::size_t i = 0; i < Zobrist_Table_Size; i++)
            table[i] = rng.next48();

        return table;
    }

    auto computeHashWithTable (const Board& board, const ZobristTable& table) -> BoardHashCode
    {
        BoardHashCode hash = 0;

        for (auto coord : Board::allCoords())
        {
            auto piece = board.pieceAt (coord);
            if (piece == wisdom::Piece_And_Color_None)
                continue;

            auto piece_index = zobristPieceIndex (piece.color(), piece.type());
            auto table_index = static_cast<std::size_t> (piece_index * Num_Squares + coord.index());
            hash ^= table[table_index] << Total_Metadata_Bits;
        }

        hash |= board.getCode().getMetadataBits();
        return hash;
    }

    void collectPositions (
        const Board& board,
        Color side,
        int depth,
        int max_depth,
        std::vector<Board>& positions
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

            positions.push_back (new_board);

            collectPositions (
                new_board, colorInvert (side), depth + 1, max_depth, positions
            );
        }
    }

    auto collectAllPositions() -> std::vector<Board>
    {
        std::vector<Board> positions;
        positions.reserve (5'500'000);

        Board board { BoardBuilder::fromDefaultPosition() };
        positions.push_back (board);

        std::cout << "Collecting positions from depth-5 perft..." << std::endl;
        collectPositions (board, Color::White, 0, 5, positions);
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
        stats.avg_bucket = static_cast<double> (hashes.size()) / static_cast<double> (table_size);
        stats.max_avg_ratio = static_cast<double> (stats.max_bucket) / stats.avg_bucket;

        double sum_sq_diff = 0.0;
        for (auto count : buckets)
        {
            double diff = static_cast<double> (count) - stats.avg_bucket;
            sum_sq_diff += diff * diff;
        }
        stats.std_dev = std::sqrt (sum_sq_diff / static_cast<double> (table_size));

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
        result.avg_std_dev = total_std_dev / static_cast<double> (Table_Sizes.size());

        return result;
    }

    void printStats (const DistributionStats& stats)
    {
        int log2_size = static_cast<int> (std::log2 (static_cast<double> (stats.table_size)));
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
    constexpr std::uint64_t Current_Seed = 0x123456789abcdefaULL;
    constexpr std::uint64_t Num_Seeds_To_Test = 1000;

    auto positions = collectAllPositions();
    std::cout << "\nPositions collected: " << positions.size() << "\n" << std::endl;

    std::cout << "Current seed (0x" << std::hex << Current_Seed << std::dec << "):" << std::endl;
    auto current_result = evaluateSeed (Current_Seed, positions);
    for (const auto& stats : current_result.stats_per_size)
        printStats (stats);
    std::cout << "Worst ratio: " << current_result.worst_ratio << "\n" << std::endl;

    std::cout << "Testing " << Num_Seeds_To_Test << " seeds..." << std::endl;

    std::vector<SeedResult> results;
    results.reserve (Num_Seeds_To_Test);

    for (std::uint64_t seed = 1; seed <= Num_Seeds_To_Test; seed++)
    {
        auto result = evaluateSeed (seed, positions);
        results.push_back (result);

        if (seed % 100 == 0)
            std::cout << "  Progress: " << seed << "/" << Num_Seeds_To_Test << std::endl;
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
