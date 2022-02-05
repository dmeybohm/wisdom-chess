#include <benchmark/benchmark.h>
#include <iostream>

#include "board.hpp"
#include "check.hpp"
#include "threats.hpp"

static void bm_is_king_threatened (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(bm_is_king_threatened);

static void bm_is_king_threatened_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(is_king_threatened_inline (board, Color::White, Last_Row, King_Column));
    }
}
BENCHMARK(bm_is_king_threatened_inline);

static void bm_threats_row (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.row ());
    }
}
BENCHMARK(bm_threats_row);

static void bm_threats_column (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.column ());
    }
}
BENCHMARK(bm_threats_column);

static void bm_threats_diagonal (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.diagonal ());
    }
}
BENCHMARK(bm_threats_diagonal);

static void bm_threats_diagonal_dumb (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.diagonal_dumb ());
    }
}
BENCHMARK(bm_threats_diagonal_dumb);

static void bm_threats_knight (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.knight ());
    }
}
BENCHMARK(bm_threats_knight);

static void bm_threats_knight_direct (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.knight_direct ());
    }
}
BENCHMARK(bm_threats_knight_direct);

static void bm_threats_pawn (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.pawn ());
    }
}
BENCHMARK(bm_threats_pawn);

static void bm_threats_pawn_dumb (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.pawn_dumb ());
    }
}
BENCHMARK(bm_threats_pawn_dumb);

static void bm_threats_pawn_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.pawn_inline ());
    }
}
BENCHMARK(bm_threats_pawn_inline);

static void bm_threats_pawn_c (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.pawn_c (0));
    }
}
BENCHMARK(bm_threats_pawn_c);

static void bm_threats_king (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.king ());
    }
}
BENCHMARK(bm_threats_king);

static void bm_threats_king_inline (benchmark::State& state)
{
    using namespace wisdom;

    Board board;
    InlineThreats threats { board, Color::White,
                            make_coord (Last_Row, King_Column) };


    for (auto _ : state)
    {
        benchmark::DoNotOptimize(threats.king_inline ());
    }
}
BENCHMARK(bm_threats_king_inline);

static void bm_threats_is_black_king_threatened (benchmark::State& state)
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
BENCHMARK(bm_threats_is_black_king_threatened);

static void bm_threats_is_white_king_threatened (benchmark::State& state)
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
BENCHMARK(bm_threats_is_white_king_threatened);
