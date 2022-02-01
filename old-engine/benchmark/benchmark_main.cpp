#include <benchmark/benchmark.h>
#include <iostream>

extern "C"
{

    enum color
    {
        COLOR_WHITE = 0,
        COLOR_BLACK = 1,
        COLOR_LAST  = 2,
        COLOR_NONE  = 3,
    };
    struct board;

    enum color;

    extern int is_king_threatened (struct board*, int color, unsigned char, unsigned char);
    extern struct board* board_new ();
}

static void benchmark_search (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened (board, 0, 7, 4));
    }
}

BENCHMARK(benchmark_search);
BENCHMARK_MAIN();
