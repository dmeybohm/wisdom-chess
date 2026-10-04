#include <bit>

#include "wisdom-chess/engine/transposition_table.hpp"
#include "wisdom-chess/engine/evaluate.hpp"

namespace wisdom
{
    TranspositionTable::TranspositionTable()
        : TranspositionTable { Default_Size_In_Megabytes }
    {
    }

    TranspositionTable::TranspositionTable (int size_in_mb)
    {
        // A zero or negative size would round down to an empty table with an
        // all-ones mask, and every probe would then index out of bounds.
        EXPECTS( size_in_mb >= 1 );

        constexpr size_t Bytes_Per_Megabyte = 1024 * 1024;
        size_t entry_count = (to_unsigned<size_t> (size_in_mb) * Bytes_Per_Megabyte)
            / sizeof (TranspositionEntry);

        size_t power_of_2 = std::bit_floor (entry_count);

        ENSURES( power_of_2 >= TranspositionBucket::Size );

        allocate (power_of_2);
    }

    auto
    TranspositionTable::fromMegabytes (int size)
        -> TranspositionTable
    {
        return TranspositionTable { size };
    }

    TranspositionTable::TranspositionTable (FromEntriesTag, size_t entry_count)
    {
        EXPECTS( entry_count >= TranspositionBucket::Size );

        // The index mask below would otherwise leave buckets unreachable.
        EXPECTS( std::has_single_bit (entry_count) );
        allocate (entry_count);
    }

    auto
    TranspositionTable::fromEntries (size_t entry_count)
        -> TranspositionTable
    {
        return TranspositionTable { FromEntriesTag {}, entry_count };
    }

    void
    TranspositionTable::allocate (size_t entry_count)
    {
        auto bucket_count = entry_count / TranspositionBucket::Size;
        my_buckets.resize (bucket_count);
        my_bucket_mask = bucket_count - 1;
    }

    auto
    TranspositionTable::findBucket (BoardHashCode hash) noexcept
        -> TranspositionBucket&
    {
        return my_buckets[foldHashTo32Bits (hash) & my_bucket_mask];
    }

    auto
    TranspositionTable::findEntry (BoardHashCode hash) noexcept
        -> nullable<TranspositionEntry>
    {
        for (auto& entry : findBucket (hash).entries)
        {
            if (entry.hash_code == hash && entry.bound_type != BoundType::Empty)
            {
                entry.generation = my_generation;
                return &entry;
            }
        }
        return nullptr;
    }

    auto
    TranspositionTable::getAge (const TranspositionEntry& entry) const noexcept
        -> int
    {
        // In eight bits, so that the generation can wrap.
        return truncate<uint8_t> (unsigned { my_generation } - unsigned { entry.generation });
    }

    auto
    TranspositionTable::chooseEntryToReplace (TranspositionBucket& bucket) const noexcept
        -> TranspositionEntry&
    {
        for (auto& entry : bucket.entries)
        {
            if (entry.bound_type == BoundType::Empty)
                return entry;
        }

        auto worth = [this] (const TranspositionEntry& entry) {
            return entry.depth_and_score.getDepth() - Age_Weight * getAge (entry);
        };

        auto* least = &bucket.entries[0];
        for (auto& entry : bucket.entries)
        {
            if (worth (entry) < worth (*least))
                least = &entry;
        }
        return *least;
    }

    auto
    TranspositionTable::scoreToTT (int score, int ply) const noexcept
        -> int
    {
        if (isCheckmatingOpponentScore (score))
            return score + ply;
        if (isCheckmatingOpponentScore (-score))
            return score - ply;
        return score;
    }

    auto
    TranspositionTable::scoreFromTT (int score, int ply) const noexcept
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
    ) noexcept
        -> optional<int>
    {
        my_probes++;

        auto found = findEntry (hash);
        if (!found)
            return nullopt;

        auto& entry = *found.value();

        if (entry.depth_and_score.getDepth() < depth)
            return nullopt;

        int adjusted_score = scoreFromTT (entry.depth_and_score.getScore(), ply);

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

            case BoundType::Empty:
                break;
        }

        return nullopt;
    }

    auto
    TranspositionTable::getBestMove (BoardHashCode hash) noexcept
        -> optional<Move>
    {
        auto found = findEntry (hash);
        if (!found)
            return nullopt;

        auto best_move = found.value()->best_move;
        if (best_move.isNullMove())
            return nullopt;

        return best_move;
    }

    void
    TranspositionTable::store (
        BoardHashCode hash,
        int score,
        int depth,
        BoundType bound_type,
        Move best_move,
        int ply
    ) noexcept
    {
        EXPECTS( bound_type != BoundType::Empty );

        auto found = findEntry (hash);
        if (found && found.value()->depth_and_score.getDepth() > depth)
            return;

        auto& entry = found ? *found.value() : chooseEntryToReplace (findBucket (hash));

        if (entry.bound_type == BoundType::Empty)
            my_stored_entries++;

        entry.hash_code = hash;
        entry.depth_and_score = DepthAndScoreBits::make (depth, scoreToTT (score, ply));
        entry.bound_type = bound_type;
        entry.best_move = best_move;
        entry.generation = my_generation;
    }

    void
    TranspositionTable::startSearch() noexcept
    {
        my_generation++;
    }

    void
    TranspositionTable::clear() noexcept
    {
        std::fill (my_buckets.begin(), my_buckets.end(), TranspositionBucket {});
        my_generation = 0;
        my_hits = 0;
        my_probes = 0;
        my_stored_entries = 0;
    }
}
