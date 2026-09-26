#include <algorithm>
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
// The counters are in the order: nodes, captures, en passants, castles,
// promotions, checks, checkmates.
//

namespace
{
    void doCheck (
        Board &board,
        const vector<CounterExpectation> &expectations,
        Color color
    )
    {
        for (const auto& [depth, expectation, count_checks] : expectations)
        {
            Stats stats;
            stats.count_checks = count_checks;
            stats.searchMoves (board, color, 0, depth);

            INFO( "depth ", depth );
            CHECK( stats.counters.nodes == expectation.nodes );
            CHECK( stats.counters.captures == expectation.captures );
            CHECK( stats.counters.en_passants == expectation.en_passants );
            CHECK( stats.counters.castles == expectation.castles );
            CHECK( stats.counters.promotions == expectation.promotions );

            if (count_checks)
            {
                CHECK( stats.counters.checks == expectation.checks );
                CHECK( stats.counters.checkmates == expectation.checkmates );
            }
        }
    }

    struct NodeExpectation
    {
        int depth;
        int64_t nodes;
    };

    // For the positions whose published results have only node counts.
    void doCheckNodes (
        const Board& board,
        const vector<NodeExpectation>& expectations,
        Color color
    )
    {
        for (const auto [depth, nodes] : expectations)
        {
            Stats stats;
            stats.searchMoves (board, color, 0, depth);

            INFO( "depth ", depth );
            CHECK( stats.counters.nodes == nodes );
        }
    }
}

TEST_CASE( "Perft: Initial position" )
{
    Board board;
    vector<CounterExpectation> expectations = {
        { 1, { 20, 0, 0, 0, 0, 0, 0 } },
        { 2, { 400, 0, 0, 0, 0, 0, 0 } },
        { 3, { 8'902, 34, 0, 0, 0, 12, 0 } },
        { 4, { 197'281, 1'576, 0, 0, 0, 469, 8 } },
        { 5, { 4'865'609, 82'719, 258, 0, 0, 27'351, 347 } },
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
        { 3, { 8'902, 34, 0, 0, 0, 12, 0 } }
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
        { 1, { 48, 8, 0, 2, 0, 0, 0 } },
        { 2, { 2'039, 351, 1, 91, 0, 3, 0 } },
        { 3, { 97'862, 17'102, 45, 3'162, 0, 993, 1 } },
        { 4, { 4'085'603, 757'163, 1'929, 128'013, 15'172, 25'523, 43 } },
        // Checks are left out at this depth: counting them nearly doubles the
        // time of what is already the longest test.
        { 5, { 193'690'690, 35'043'416, 73'365, 4'993'637, 8'392 }, false },
    };

    auto perft_results = wisdom::perft::perftResults (perft_board, Color::White, 2);
    auto sum = perft_results.total_nodes;

    CHECK( sum == 2039 );
    doCheck (test_board, expectations, Color::White);
}

TEST_CASE( "Perft: Position 3" )
{
    FenParser parser { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" };
    auto board = parser.buildBoard();

    vector<CounterExpectation> expectations = {
        { 1, { 14, 1, 0, 0, 0, 2, 0 } },
        { 2, { 191, 14, 0, 0, 0, 10, 0 } },
        { 3, { 2'812, 209, 2, 0, 0, 267, 0 } },
        { 4, { 43'238, 3'348, 123, 0, 0, 1'680, 17 } },
        { 5, { 674'624, 52'051, 1'165, 0, 0, 52'950, 0 } },
        { 6, { 11'030'083, 940'350, 33'325, 0, 7'552, 452'473, 2'733 } },
    };

    doCheck (board, expectations, Color::White);
}

TEST_CASE( "Perft: Position 4" )
{
    FenParser parser { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1" };
    auto board = parser.buildBoard();

    vector<CounterExpectation> expectations = {
        { 1, { 6, 0, 0, 0, 0, 0, 0 } },
        { 2, { 264, 87, 0, 6, 48, 10, 0 } },
        { 3, { 9'467, 1'021, 4, 0, 120, 38, 22 } },
        { 4, { 422'333, 131'393, 0, 7'795, 60'032, 15'492, 5 } },
        { 5, { 15'833'292, 2'046'173, 6'512, 0, 329'464, 200'568, 50'562 } },
    };

    doCheck (board, expectations, Color::White);
}

TEST_CASE( "Perft: Position 4 mirrored" )
{
    FenParser parser { "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1" };
    auto board = parser.buildBoard();

    vector<CounterExpectation> expectations = {
        { 1, { 6, 0, 0, 0, 0, 0, 0 } },
        { 2, { 264, 87, 0, 6, 48, 10, 0 } },
        { 3, { 9'467, 1'021, 4, 0, 120, 38, 22 } },
        { 4, { 422'333, 131'393, 0, 7'795, 60'032, 15'492, 5 } },
    };

    doCheck (board, expectations, Color::Black);
}

TEST_CASE( "Perft: Position 5" )
{
    FenParser parser { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8" };
    auto board = parser.buildBoard();

    vector<NodeExpectation> expectations = {
        { 1, 44 },
        { 2, 1'486 },
        { 3, 62'379 },
        { 4, 2'103'487 },
    };

    doCheckNodes (board, expectations, Color::White);
}

TEST_CASE( "Perft: Position 6" )
{
    FenParser parser {
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    };
    auto board = parser.buildBoard();

    vector<NodeExpectation> expectations = {
        { 1, 46 },
        { 2, 2'079 },
        { 3, 89'890 },
        { 4, 3'894'594 },
    };

    doCheckNodes (board, expectations, Color::White);
}

namespace
{
    struct CaptureCheck
    {
        int64_t positions = 0;
        int64_t mismatches = 0;
        int64_t en_passants = 0;
        int64_t promotions = 0;
        std::string first_mismatch;
    };

    auto
    sortedByValue (const wisdom::MoveList& list)
        -> vector<wisdom::Move>
    {
        vector<wisdom::Move> result { list.begin(), list.end() };
        std::sort (
            result.begin(),
            result.end(),
            [] (wisdom::Move a, wisdom::Move b) { return a.toInt() < b.toInt(); }
        );
        return result;
    }

    // Compare generateCaptures() with the full move list at every node of the
    // perft tree, down to the given depth.
    void checkCapturesAtEveryNode ( // NOLINT(misc-no-recursion)
        const Board& board,
        Color side,
        int depth,
        CaptureCheck& check
    )
    {
        auto all_moves = wisdom::generateAllPotentialMoves (board, side);
        auto captures = wisdom::generateCaptures (board, side);

        wisdom::MoveList expected;
        for (auto move : all_moves)
        {
            if (move.isPromoting()
                    ? move.getPromotedPiece() == wisdom::Piece::Queen
                    : move.isAnyCapturing())
            {
                expected.append (move);
            }
        }

        check.positions++;
        if (sortedByValue (captures) != sortedByValue (expected))
        {
            if (check.mismatches == 0)
                check.first_mismatch = board.toFenString (side);
            check.mismatches++;
        }

        for (auto move : captures)
        {
            if (move.isEnPassant())
                check.en_passants++;
            if (move.isPromoting())
                check.promotions++;
        }

        if (depth == 0)
            return;

        for (auto move : all_moves)
        {
            Board child = board.withMove (side, move);

            if (wisdom::isLegalPositionAfterMove (child, side, move))
                checkCapturesAtEveryNode (child, wisdom::colorInvert (side), depth - 1, check);
        }
    }
}

TEST_CASE( "Perft: generateCaptures agrees with the full move list at every node" )
{
    struct Position
    {
        const char* fen;
        int depth;
    };

    const Position positions[] = {
        { "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3 },
        { "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 4 },
        { "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3 },
        { "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3 },
        { "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3 },
    };

    CaptureCheck total;

    for (const auto& position : positions)
    {
        FenParser parser { position.fen };
        auto board = parser.buildBoard();

        CaptureCheck check;
        checkCapturesAtEveryNode (board, board.getCurrentTurn(), position.depth, check);

        INFO( position.fen );
        INFO( "first mismatch: ", check.first_mismatch );
        CHECK( check.mismatches == 0 );

        total.positions += check.positions;
        total.en_passants += check.en_passants;
        total.promotions += check.promotions;
    }

    INFO( "positions checked: ", total.positions );
    CHECK( total.en_passants > 0 );
    CHECK( total.promotions > 0 );
}
