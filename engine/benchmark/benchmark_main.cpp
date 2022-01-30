#include <benchmark/benchmark.h>
#include <iostream>

#include "game.hpp"
#include "logger.hpp"

static void benchmark_search (benchmark::State& state)
{
    using namespace wisdom;
    std::vector<Move> human_moves {
        move_parse("e2 e4", Color::White),
        move_parse("g1 f3", Color::White),
        move_parse("b1 c3", Color::White)
    };

    for (auto _ : state)
    {
        state.PauseTiming();
        auto &output = make_null_logger();

        Game game { Color::White, Color::Black };

        for (auto human_move : human_moves)
        {
            game.move (human_move);

            state.ResumeTiming();

            auto optional_move = game.find_best_move (output);
            state.PauseTiming ();
            game.move (*optional_move);
        }
    }
}

BENCHMARK(benchmark_search);
BENCHMARK_MAIN();