#include "tests.hpp"
#include "piece.hpp"
#include "generate.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "check.hpp"
#include "fen_parser.hpp"
#include "perft.hpp"

#include <iostream>

using wisdom::Board;
using wisdom::Color;
using wisdom::perft::MoveCounter;
using wisdom::perft::CounterExpectation;
using wisdom::perft::Stats;
using wisdom::FenParser;
using std::vector;
using wisdom::perft::PerftResults;

//
// These loaded from https://www.chessprogramming.org/Perft_Results
//

TEST_CASE("Perft cases loaded from https://www.chessprogramming.org/Perft_Results")
{
    auto do_check = [](
        Board &board,
        const vector<CounterExpectation> &expectations,
        Color color
    ) {
        Board copy_board = board;

        for (const auto [depth, expectation] : expectations)
        {
            Stats stats;

            stats.search_moves (copy_board, color, 0, depth);
            color = color_invert (color);

            CHECK( stats.counters.nodes == expectation.nodes );
            CHECK( stats.counters.captures == expectation.captures );
            CHECK( stats.counters.en_passants == expectation.en_passants );
        }
    };

    SUBCASE( "Initial position")
    {
        Board board;
        vector<CounterExpectation> expectations = {
            { 1, { 20, 0, 0 } },
            { 2, { 400, 0, 0 } },
            { 3, { 8902, 34, 0 } },
            { 4, { 197281, 1576, 0 } },
            { 5, { 4865609, 82719, 258 } },
        };

        do_check (board, expectations, Color::White);
    }

    SUBCASE( "Consistency between this tests and PerftResults at depth 2" )
    {
        Board perft_board;
        Board test_board;
        vector<CounterExpectation> expectations = {
            { 1, { 20, 0, 0 } },
            { 2, { 400, 0, 0 } }
        };

        auto perft_results = wisdom::perft::perft_results (perft_board, Color::White, 2);

        do_check (test_board, expectations, Color::White);
        auto test_sum = perft_results.total_nodes;

        CHECK( test_sum == 400 );
    }

    SUBCASE( "Consistency between this tests and PerftResults at depth 3" )
    {
        Board perft_board;
        Board test_board;
        vector<CounterExpectation> expectations = {
            { 1, { 20, 0, 0 } },
            { 2, { 400, 0, 0 } },
            { 3, { 8902, 34, 0 } }
        };

        auto perft_results = wisdom::perft::perft_results (perft_board, Color::White, 3);
        auto sum = perft_results.total_nodes;

        do_check (test_board, expectations, Color::White);
        CHECK( sum == 8902 );
    }

    SUBCASE( "Position 2")
    {
        FenParser parser { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" };
        auto test_board = parser.build_board ();
        auto perft_board = test_board;
        std::string check = test_board.to_fen_string (Color::White);
        auto sub = check.substr (0, check.size() - 4);
        CHECK( sub == "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"  );

        vector<CounterExpectation> expectations = {
            { 1, { 48, 8, 0 } },
            { 2, { 2039, 351, 1 } },
//            { 3, { 97862, 17102, 45 } },
//            { 4, { 197281, 1576, 0 } },
//            { 5, { 4865609, 82719, 258 } },
        };

        auto perft_results = wisdom::perft::perft_results (perft_board, Color::White, 2);
        auto sum = perft_results.total_nodes;

        CHECK( sum == 2039 );
        do_check (test_board, expectations, Color::White);
    }
}

