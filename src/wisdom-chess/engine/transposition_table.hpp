#pragma once

#include <bit>

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/move.hpp"

namespace wisdom
{
    [[nodiscard]] constexpr auto
    foldHashTo32Bits (BoardHashCode hash) noexcept
        -> uint32_t
    {
        return static_cast<uint32_t> ((hash >> 32) ^ hash);
    }

    struct TranspositionTableStats
    {
        size_t probes = 0;
        size_t hits = 0;
        size_t stored_entries = 0;
    };

    [[nodiscard]] inline auto
    computeHitRate (const TranspositionTableStats& start, const TranspositionTableStats& end) noexcept
        -> double
    {
        auto delta_probes = end.probes - start.probes;
        auto delta_hits = end.hits - start.hits;
        return delta_probes > 0 ? (100.0 * static_cast<double> (delta_hits) / static_cast<double> (delta_probes)) : 0.0;
    }

    enum class BoundType : uint8_t
    {
        Empty,
        Exact,
        LowerBound,
        UpperBound
    };

    // A depth and a score packed into 32 bits: the depth in the low bits,
    // unsigned, and the score in the rest, signed.
    class DepthAndScoreBits
    {
    public:
        static constexpr int Depth_Bits = 7;
        static constexpr int Score_Bits = 32 - Depth_Bits;
        static constexpr int Max_Depth = (1 << Depth_Bits) - 1;
        static constexpr int Max_Score = (1 << (Score_Bits - 1)) - 1;
        static constexpr int Min_Score = -Max_Score - 1;

        static_assert (Max_Search_Depth <= Max_Depth);
        static_assert (Checkmate_Score <= Max_Score);

        constexpr DepthAndScoreBits() noexcept = default;

        [[nodiscard]] static constexpr auto
        make (int depth, int score) noexcept
            -> DepthAndScoreBits
        {
            EXPECTS( depth >= 0 && depth <= Max_Depth );
            EXPECTS( score >= Min_Score && score <= Max_Score );

            return DepthAndScoreBits {
                (std::bit_cast<uint32_t> (score) << Depth_Bits) | to_unsigned<uint32_t> (depth)
            };
        }

        [[nodiscard]] constexpr auto
        getDepth() const noexcept
            -> int
        {
            return narrow<int> (my_bits & Max_Depth);
        }

        [[nodiscard]] constexpr auto
        getScore() const noexcept
            -> int
        {
            return std::bit_cast<int32_t> (my_bits) >> Depth_Bits;
        }

    private:
        constexpr explicit DepthAndScoreBits (uint32_t bits) noexcept
            : my_bits { bits }
        {
        }

        uint32_t my_bits = 0;
    };

    struct TranspositionEntry
    {
        BoardHashCode hash_code = 0;
        DepthAndScoreBits depth_and_score {};
        Move best_move {};
        BoundType bound_type = BoundType::Empty;
    };
    static_assert (sizeof (TranspositionEntry) == 16);

    class TranspositionTable
    {
        explicit TranspositionTable (int size_in_megabytes);

        struct FromEntriesTag {};
        TranspositionTable (FromEntriesTag, size_t entry_count);

    public:
        static constexpr int Default_Size_In_Megabytes = 16;

        explicit TranspositionTable();

        // A table has exactly one owner. Copying is deleted so that a table
        // cannot be silently duplicated or shared between threads.
        TranspositionTable (const TranspositionTable&) = delete;
        auto operator= (const TranspositionTable&) -> TranspositionTable& = delete;

        TranspositionTable (TranspositionTable&&) noexcept = default;
        auto operator= (TranspositionTable&&) noexcept -> TranspositionTable& = default;

        ~TranspositionTable() = default;

        [[nodiscard]] static auto
        fromMegabytes (int size)
            -> TranspositionTable;

        // The entry count must be a power of two, and at least two.
        [[nodiscard]] static auto
        fromEntries (size_t entry_count)
            -> TranspositionTable;

        [[nodiscard]] auto
        probe (BoardHashCode hash, int depth, int alpha, int beta, int ply) noexcept
            -> optional<int>;

        [[nodiscard]] auto
        getBestMove (BoardHashCode hash) noexcept
            -> optional<Move>;

        void store (
            BoardHashCode hash,
            int score,
            int depth,
            BoundType bound_type,
            Move best_move,
            int ply
        ) noexcept;

        void clear() noexcept;

        [[nodiscard]] auto
        getSize() const noexcept
            -> size_t
        {
            return my_entries.size();
        }

        [[nodiscard]] auto
        getStats() const noexcept
            -> TranspositionTableStats
        {
            return TranspositionTableStats { my_probes, my_hits, my_stored_entries };
        }

    private:
        [[nodiscard]] auto
        scoreToTT (int score, int ply) const noexcept
            -> int;

        [[nodiscard]] auto
        scoreFromTT (int score, int ply) const noexcept
            -> int;

        vector<TranspositionEntry> my_entries;
        size_t my_size_mask;
        size_t my_hits = 0;
        size_t my_probes = 0;
        size_t my_stored_entries = 0;
    };
}
