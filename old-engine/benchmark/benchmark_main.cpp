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
    extern int is_king_threatened_row (struct board*, int color, unsigned char, unsigned char);
    extern int is_king_threatened_column (struct board*, int color, unsigned char, unsigned char);
    extern int is_king_threatened_diagonal (struct board*, int color, unsigned char, unsigned char);
    extern int is_king_threatened_knight (struct board*, int color, unsigned char, unsigned char);
    extern int is_king_threatened_pawn (struct board*, int color, unsigned char, unsigned char);
    extern int is_king_threatened_king (struct board*, int color, unsigned char, unsigned char);

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

static void benchmark_row (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_row (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_row);

static void benchmark_diagonal (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_diagonal (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_diagonal);

static void benchmark_column (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_column (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_column);

static void benchmark_knight (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_knight (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_knight);

static void benchmark_pawn (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_pawn (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_pawn);

static void benchmark_king (benchmark::State& state)
{
    struct board *board = board_new ();

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_king (board, 0, 7, 4));
    }
}
BENCHMARK(benchmark_king);

BENCHMARK_MAIN();
