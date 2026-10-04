#include "wisdom-chess/engine/transposition_table.hpp"

#include "fatal_test.hpp"

using namespace wisdom;

namespace
{
    FATAL_CASE(
        "transposition-table-no-megabytes",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: size_in_mb >= 1"
    )
    {
        int volatile size = 0;
        [[maybe_unused]] auto table = TranspositionTable::fromMegabytes (size);
    }

    FATAL_CASE(
        "transposition-table-less-than-a-bucket",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: entry_count >= TranspositionBucket::Size"
    )
    {
        [[maybe_unused]] auto table = TranspositionTable::fromEntries (2);
    }

    FATAL_CASE(
        "transposition-table-entries-not-a-power-of-two",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: std::has_single_bit \\(entry_count\\)"
    )
    {
        [[maybe_unused]] auto table = TranspositionTable::fromEntries (6);
    }

    FATAL_CASE(
        "transposition-table-store-empty-bound",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: bound_type != BoundType::Empty"
    )
    {
        auto table = TranspositionTable::fromEntries (4);
        table.store (1, 0, 1, BoundType::Empty, Move::make (0, 0, 1, 1), 0);
    }

    FATAL_CASE(
        "depth-and-score-bits-depth-too-large",
        "Precondition failed at .*transposition_table\\.hpp:[0-9]+: depth >= 0 && depth <= Max_Depth"
    )
    {
        int volatile depth = DepthAndScoreBits::Max_Depth + 1;
        [[maybe_unused]] auto bits = DepthAndScoreBits::make (depth, 0);
    }

    FATAL_CASE(
        "depth-and-score-bits-score-too-large",
        "Precondition failed at .*transposition_table\\.hpp:[0-9]+: score >= Min_Score && score <= Max_Score"
    )
    {
        int volatile score = DepthAndScoreBits::Max_Score + 1;
        [[maybe_unused]] auto bits = DepthAndScoreBits::make (1, score);
    }
}
