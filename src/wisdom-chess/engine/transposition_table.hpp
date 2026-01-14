#pragma once

#include "wisdom-chess/engine/global.hpp"
#include "wisdom-chess/engine/board_code.hpp"
#include "wisdom-chess/engine/move.hpp"

namespace wisdom
{
    struct TranspositionTableStats
    {
        size_t probes = 0;
        size_t hits = 0;
        size_t stored_entries = 0;
    };

    [[nodiscard]] inline auto
    computeHitRate (const TranspositionTableStats& start, const TranspositionTableStats& end)
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
    public:
        explicit TranspositionTable (size_t size_in_mb = 8);

        [[nodiscard]] auto
        probe (BoardHashCode hash, int depth, int alpha, int beta, int ply)
            -> optional<int>;

        [[nodiscard]] auto
        getBestMove (BoardHashCode hash)
            -> optional<Move>;

        void store (
            BoardHashCode hash,
            int score,
            int depth,
            BoundType bound_type,
            Move best_move,
            int ply
        );

        void clear();

        [[nodiscard]] auto
        getHitCount() const
            -> size_t
        {
            return my_hits;
        }

        [[nodiscard]] auto
        getProbeCount() const
            -> size_t
        {
            return my_probes;
        }

        [[nodiscard]] auto
        getStoredEntriesCount() const
            -> size_t
        {
            return my_stored_entries;
        }

        [[nodiscard]] auto
        getSize() const
            -> size_t
        {
            return my_entries.size();
        }

        [[nodiscard]] auto
        getStats() const
            -> TranspositionTableStats
        {
            return TranspositionTableStats { my_probes, my_hits, my_stored_entries };
        }

    private:
        [[nodiscard]] auto
        scoreToTT (int score, int ply) const
            -> int;

        [[nodiscard]] auto
        scoreFromTT (int score, int ply) const
            -> int;

        vector<TranspositionEntry> my_entries;
        size_t my_size_mask;
        size_t my_hits = 0;
        size_t my_probes = 0;
        size_t my_stored_entries = 0;
    };
}
