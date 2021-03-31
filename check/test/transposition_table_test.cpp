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

    table.add( { board, 10, 3, {} }, Color::White );
    auto result = table.lookup (hash_code, Color::White);
    REQUIRE(result.has_value() );
    CHECK(table.size() == 1);
    CHECK(result->score == 10 );
}

TEST_CASE("Adding a value already added to a transposition table")
{
    Board board;
    TranspositionTable table;

    auto hash_code = board.code.hash_code ();

    table.add( { board, 10, 3 , {} }, Color::White );
    auto result1 = table.lookup (hash_code, Color::White);

    table.add( { board, 10, 3, {} }, Color::White );
    auto result2 = table.lookup (hash_code, Color::White);
    REQUIRE( result1.has_value() );
    CHECK(table.size() == 1);
    CHECK(result1->score == 10 );
}

TEST_CASE("Initializing transposition table")
{
    std::vector<RelativeTransposition> positions;
    TranspositionTable table;
    int num_iterations = 10000;

    for (int i = 0; i < num_iterations; i++)
    {
        RelativeTransposition position { static_cast<std::size_t>(i), BoardCode{}, i * num_iterations, 3, {} };
        positions.push_back (position);
    }

    for (auto position : positions)
    {
        table.add (position, Color::White);
    }

    for (std::size_t i = 0; i < num_iterations; i++)
    {
        auto result = table.lookup (i, Color::White);
        CHECK( result.has_value() );
        CHECK( result->score == i * num_iterations );
    }

    CHECK(table.size() == num_iterations);
}