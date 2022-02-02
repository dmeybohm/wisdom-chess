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
BENCHMARK(benchmark_is_king_threatened_initial_position);

static void benchmark_is_king_threatened_initial_position_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_inline (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_initial_position_inline);

static void benchmark_is_king_threatened_row (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_row (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_row);

static void benchmark_is_king_threatened_column (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_column (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_column);

static void benchmark_is_king_threatened_diagonal (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_diagonal (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_diagonal);

static void benchmark_is_king_threatened_diagonal_dumb (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_diagonal_dumb (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_diagonal_dumb);

static void benchmark_is_king_threatened_knight (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_knight (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_knight);

static void benchmark_is_king_threatened_knight_direct (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_knight_direct (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_knight_direct);

static void benchmark_is_king_threatened_pawn (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_pawn (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_pawn);

static void benchmark_is_king_threatened_pawn_dumb (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_pawn_dumb (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_pawn_dumb);

static void benchmark_is_king_threatened_pawn_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_pawn_inline (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_pawn_inline);

static void benchmark_is_king_threatened_pawn_c (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_pawn_c (board, 1, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_pawn_c);

static void benchmark_is_king_threatened_king (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_king (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_king);

static void benchmark_is_king_threatened_king_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_king_inline (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(benchmark_is_king_threatened_king_inline);

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
BENCHMARK(benchmark_is_black_king_threatened);

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
BENCHMARK(benchmark_is_white_king_threatened);
