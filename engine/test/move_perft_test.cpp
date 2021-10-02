#include "tests.hpp"
#include "piece.hpp"
#include "generate.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "check.hpp"
#include "fen_parser.hpp"
#include "perft.hpp"

using wisdom::Board;
using wisdom::Color;
using wisdom::perft::MoveCounter;
using wisdom::perft::CounterExpectation;
using wisdom::perft::Stats;
using wisdom::FenParser;
using std::vector;

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
        MoveCounter sums {};
        for (const auto [depth, expectation] : expectations)
        {
            Stats stats;

            sums += expectation;

            stats.search_moves (board, color, 0, depth);
            color = color_invert (color);

            CHECK( stats.counters.nodes == sums.nodes );
            CHECK( stats.counters.captures == sums.captures );
            CHECK( stats.counters.en_passants == sums.en_passants );
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

    SUBCASE( "Position 2")
    {
        FenParser parser { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -" };
        auto board = parser.build_board ();

        vector<CounterExpectation> expectations = {
            { 1, { 48, 8, 0 } },
            { 2, { 2039, 351, 1 } },
            { 3, { 97862, 17102, 45 } },
//            { 4, { 197281, 1576, 0 } },
//            { 5, { 4865609, 82719, 258 } },
        };

        do_check (board, expectations, Color::White);
    }
}
