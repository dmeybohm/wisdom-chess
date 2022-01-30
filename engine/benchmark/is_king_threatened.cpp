#include <benchmark/benchmark.h>
#include <iostream>

#include "board.hpp"
#include "check.hpp"

static void benchmark_is_king_threatened (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        state.PauseTiming ();
        board.randomize_positions();
        state.ResumeTiming ();
        benchmark::DoNotOptimize(is_king_threatened (board, Color::White, 7, 4));
    }
}

BENCHMARK(benchmark_is_king_threatened);
