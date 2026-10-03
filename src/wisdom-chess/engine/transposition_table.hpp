#pragma once

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
        Exact,
        LowerBound,
        UpperBound
    };

    struct TranspositionEntry
    {
        BoardHashCode hash_code = 0;
        Move best_move {};
        int score = 0;
        int16_t depth = 0;
        BoundType bound_type = BoundType::Exact;
    };

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
