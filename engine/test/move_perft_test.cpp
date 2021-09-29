#include "tests.hpp"
#include "piece.hpp"
#include "generate.hpp"
#include "move_list.hpp"
#include "board.hpp"
#include "check.hpp"

namespace wisdom::tests
{
    struct MoveCounter
    {
        int nodes = 0;
        int captures = 0;
        int en_passants = 0;
    };

    struct CounterExpection
    {
        int depth;
        MoveCounter expectation;
    };

    struct Stats
    {
        MoveCounter counters;

        void search_moves (Board &board, Color side, int depth, int max_depth)
        {
            if (depth >= max_depth)
                return;

            const auto moves = generate_moves (board, side);

            for (auto move : moves)
            {
                UndoMove undo_state = board.make_move (side, move);


                if (!was_legal_move (board, side, move))
                {
                    board.take_back (side, move, undo_state);
                    continue;
                }

                counters.nodes++;
                if (is_any_capturing_move (move))
                    counters.captures++;

                if (is_en_passant_move(move))
                    counters.en_passants++;

                search_moves (board, color_invert (side), depth + 1, max_depth);

                board.take_back (side, move, undo_state);
            }
        }
    };


}

using wisdom::Board;
using wisdom::Color;
using wisdom::tests::MoveCounter;
using wisdom::tests::CounterExpection;
using wisdom::tests::Stats;

TEST_CASE("Perft algorithm move test")
{
    Board board;
    std::vector<CounterExpection> expectations = {
            { 1, { 20, 0, 0 } },
            { 2, { 400, 0, 0 } },
            { 3, { 8902, 34, 0 } },
            { 4, { 197281, 1576, 0 } },
            { 5, { 4865609, 82719, 258 } },
    };

    auto color = Color::White;
    MoveCounter sums {};

    for (const auto [depth, expectation] : expectations)
    {
        Stats stats;

        sums.nodes += expectation.nodes;
        sums.captures += expectation.captures;
        sums.en_passants += expectation.en_passants;

        stats.search_moves (board, color, 0, depth);
        color = color_invert (color);

        CHECK( stats.counters.nodes == sums.nodes );
        CHECK( stats.counters.captures == sums.captures );
        CHECK( stats.counters.en_passants == sums.en_passants );
    }
}