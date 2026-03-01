#include <iostream>

#include "wisdom-chess/engine/piece.hpp"
#include "wisdom-chess/engine/generate.hpp"
#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/fen_parser.hpp"

#include "wisdom-chess-tests.hpp"
#include "wisdom-chess-perft.hpp"

using wisdom::Board;
using wisdom::Color;
using wisdom::perft::MoveCounter;
using wisdom::perft::CounterExpectation;
using wisdom::perft::Stats;
using wisdom::FenParser;
using std::vector;
using wisdom::perft::PerftResults;
using wisdom::MoveGenerator;

//
// These loaded from https://www.chessprogramming.org/Perft_Results
//

namespace
{
    void doCheck (
        Board &board,
        const vector<CounterExpectation> &expectations,
        Color color
    )
    {
        for (const auto [depth, expectation] : expectations)
        {
            Stats stats;
            stats.searchMoves (board, color, 0, depth);

            CHECK( stats.counters.nodes == expectation.nodes );
            CHECK( stats.counters.captures == expectation.captures );
            CHECK( stats.counters.en_passants == expectation.en_passants );
        }
    }
}

TEST_CASE( "Perft: Initial position" )
{
    Board board;
    vector<CounterExpectation> expectations = {
        { 1, { 20, 0, 0 } },
        { 2, { 400, 0, 0 } },
        { 3, { 8'902, 34, 0 } },
        { 4, { 197'281, 1'576, 0 } },
        { 5, { 4'865'609, 82'719, 258 } },
    };

    doCheck (board, expectations, Color::White);
}

TEST_CASE( "Perft: Consistency at depth 2" )
{
    Board perft_board;
    Board test_board;
    vector<CounterExpectation> expectations = {
        { 1, { 20, 0, 0 } },
        { 2, { 400, 0, 0 } }
    };

    auto perft_results = wisdom::perft::perftResults (perft_board, Color::White, 2);

    doCheck (test_board, expectations, Color::White);
    auto test_sum = perft_results.total_nodes;

    CHECK( test_sum == 400 );
}

TEST_CASE( "Perft: Consistency at depth 3" )
{
    Board perft_board;
    Board test_board;
    vector<CounterExpectation> expectations = {
        { 1, { 20, 0, 0 } },
        { 2, { 400, 0, 0 } },
        { 3, { 8'902, 34, 0 } }
    };

    auto perft_results = wisdom::perft::perftResults (perft_board, Color::White, 3);
    auto sum = perft_results.total_nodes;

    doCheck (test_board, expectations, Color::White);
    CHECK( sum == 8902 );
}

TEST_CASE( "Perft: Position 2 (kiwipete)" )
{
    FenParser parser { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" };
    auto test_board = parser.buildBoard();
    auto perft_board = test_board;
    std::string check = test_board.toFenString (Color::White);
    auto sub = check.substr (0, check.size() - 4);
    CHECK( sub == "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"  );

    vector<CounterExpectation> expectations = {
        { 1, { 48, 8, 0 } },
        { 2, { 2'039, 351, 1 } },
        { 3, { 97'862, 17'102, 45 } },
        { 4, { 4'085'603, 757'163, 1'929 } },
        { 5, { 193'690'690, 35'043'416, 73'365 } },
    };

    auto perft_results = wisdom::perft::perftResults (perft_board, Color::White, 2);
    auto sum = perft_results.total_nodes;

    CHECK( sum == 2039 );
    doCheck (test_board, expectations, Color::White);
}
