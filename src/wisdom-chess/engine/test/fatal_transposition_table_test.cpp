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
        "transposition-table-one-entry",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: entry_count >= 2"
    )
    {
        [[maybe_unused]] auto table = TranspositionTable::fromEntries (1);
    }

    FATAL_CASE(
        "transposition-table-entries-not-a-power-of-two",
        "Precondition failed at .*transposition_table\\.cpp:[0-9]+: std::has_single_bit \\(entry_count\\)"
    )
    {
        [[maybe_unused]] auto table = TranspositionTable::fromEntries (6);
    }
}
