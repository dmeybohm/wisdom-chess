#include "perft.hpp"
#include "fen_parser.hpp"
#include "board.hpp"
#include "str.hpp"
#include "move_list.hpp"

#include <iostream>
#include <argh.h>

using wisdom::Board;
using wisdom::MoveList;
using wisdom::Color;
using wisdom::perft::Stats;

auto apply_list (Board &board, Color color, const MoveList &list) -> Color
{
    for (auto &move : list)
    {
        board.make_move (color, move);
        color = wisdom::color_invert (color);
    }
    return color;
}

void print_perf (Board &board, Color player, int depth)
{
    Stats stats;
    Stats cummulative;

    auto moves = wisdom::generate_moves (board, player);

    for (const auto &move : moves)
    {

    }

    stats.search_moves (board, player, 0, depth);
}

int main (int argc, char *argv[])
{
    if (argc != 4 && argc != 5)
    {
        std::cerr << "Need more args" << "\n";
        return EXIT_FAILURE;
    }

    auto depth = wisdom::to_int (argv[1]);
    wisdom::FenParser fen { argv[2] };
    auto board = fen.build_board () ;
    auto current_player = fen.get_active_player ();

    if (argc == 5)
    {
        auto moves = wisdom::perft::to_move_list (board, current_player, argv[3]);
        current_player = apply_list (board, current_player, moves);
    }

    print_perf (board, current_player, depth);
    return EXIT_SUCCESS;
}
