#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

namespace wisdom
{
    TranspositionTable::TranspositionTable (size_t size_in_mb)
    {
        constexpr size_t bytes_per_mb = 1024 * 1024;
        size_t entry_count = (size_in_mb * bytes_per_mb) / sizeof (TranspositionEntry);

        size_t power_of_2 = 1;
        while (power_of_2 < entry_count)
            power_of_2 <<= 1;
        power_of_2 >>= 1;

        my_entries.resize (power_of_2);
        my_size_mask = power_of_2 - 1;
    }

    auto
    TranspositionTable::scoreToTT (int score, int ply) const
        -> int
    {
        if (isCheckmatingOpponentScore (score))
            return score + ply;
        if (isCheckmatingOpponentScore (-score))
            return score - ply;
        return score;
    }

    auto
    TranspositionTable::scoreFromTT (int score, int ply) const
        -> int
    {
        if (isCheckmatingOpponentScore (score))
            return score - ply;
        if (isCheckmatingOpponentScore (-score))
            return score + ply;
        return score;
    }

    auto
    TranspositionTable::probe (
        BoardHashCode hash,
        int depth,
        int alpha,
        int beta,
        int ply
    )
        -> optional<int>
    {
        my_probes++;

        auto& entry = my_entries[hash & my_size_mask];

        if (entry.hash_code != hash)
            return nullopt;

        if (entry.depth < depth)
            return nullopt;

        int adjusted_score = scoreFromTT (entry.score, ply);

        switch (entry.bound_type)
        {
            case BoundType::Exact:
                my_hits++;
                return adjusted_score;

            case BoundType::LowerBound:
                if (adjusted_score >= beta)
                {
                    my_hits++;
                    return adjusted_score;
                }
                break;

            case BoundType::UpperBound:
                if (adjusted_score <= alpha)
                {
                    my_hits++;
                    return adjusted_score;
                }
                break;
        }

        return nullopt;
    }

    auto
    TranspositionTable::getBestMove (BoardHashCode hash)
        -> optional<Move>
    {
        auto& entry = my_entries[hash & my_size_mask];

        if (entry.hash_code != hash)
            return nullopt;

        if (entry.best_move.src == 0 && entry.best_move.dst == 0)
            return nullopt;

        return entry.best_move;
    }

    void
    TranspositionTable::store (
        BoardHashCode hash,
        int score,
        int depth,
        BoundType bound_type,
        Move best_move,
        int ply
    )
    {
        auto& entry = my_entries[hash & my_size_mask];

        if (entry.hash_code == hash && entry.depth > depth)
            return;

        entry.hash_code = hash;
        entry.score = scoreToTT (score, ply);
        entry.depth = narrow<int16_t> (depth);
        entry.bound_type = bound_type;
        entry.best_move = best_move;
    }

    void
    TranspositionTable::clear()
    {
        std::fill (my_entries.begin(), my_entries.end(), TranspositionEntry {});
        my_hits = 0;
        my_probes = 0;
    }
}
