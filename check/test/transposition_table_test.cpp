#include "doctest/doctest.h"
#include "board.hpp"
#include "board_code.hpp"
#include "transposition_table.hpp"

using namespace wisdom;

TEST_CASE("Initializing transposition table")
{
    Board board;
    TranspositionTable table;

    auto hash_code = board.code.hash_code ();

    table.add( { board, 10 } );
    auto result = table.lookup (hash_code);
    REQUIRE(result.has_value() );
    CHECK(table.size() == 1);
    CHECK(result->score == 10 );
}

TEST_CASE("Initializing transposition table")
{
    std::vector<Transposition> positions;
    TranspositionTable table;
    int num_iterations = 10000;

    for (int i = 0; i < num_iterations; i++)
    {
        Transposition position { static_cast<std::size_t>(i), i * num_iterations };
        positions.push_back (position);
    }

    for (auto position : positions)
    {
        table.add (position);
    }

    for (std::size_t i = 0; i < num_iterations; i++)
    {
        auto result = table.lookup (i);
        CHECK( result.has_value() );
        CHECK( result->score == i * num_iterations );
    }

    CHECK(table.size() == num_iterations);
}