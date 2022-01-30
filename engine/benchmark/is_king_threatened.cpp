#include <benchmark/benchmark.h>
#include <iostream>

#include "board.hpp"
#include "check.hpp"

static void benchmark_is_king_threatened_initial_position (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened (board, Color::White, Last_Row, King_Column));
    }

}

static void benchmark_is_black_king_threatened (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    board.randomize_positions ();
    auto king_pos = board.get_king_position (Color::Black);
    auto king_row = Row (king_pos);
    auto king_col = Column (king_pos);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened (board, Color::White, king_row, king_col));
    }
}

static void benchmark_is_white_king_threatened (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    board.randomize_positions ();
    auto king_pos = board.get_king_position (Color::Black);
    auto king_row = Row (king_pos);
    auto king_col = Column (king_pos);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened (board, Color::Black, king_row, king_col));
    }
}

BENCHMARK(benchmark_is_king_threatened_initial_position);
BENCHMARK(benchmark_is_black_king_threatened);
BENCHMARK(benchmark_is_white_king_threatened);
