#include "perft.hpp"
#include "fen_parser.hpp"
#include "board.hpp"
#include "str.hpp"
#include "move_list.hpp"
#include "check.hpp"
#include "move_list.hpp"

#include <iostream>

using wisdom::Board;
using wisdom::MoveList;
using wisdom::Color;
using wisdom::perft::Stats;
using wisdom::perft::PerftResults;

int main (int argc, char *argv[])
{
    wisdom::MoveGenerator move_generator;

    if (argc != 3 && argc != 4)
    {
        std::cerr << "Need two or three args" << "\n";
        return EXIT_FAILURE;
    }

    auto depth = wisdom::to_int (argv[1]);
    wisdom::FenParser fen { argv[2] };
    auto board = fen.buildBoard() ;
    auto current_player = fen.getActivePlayer();

    if (argc == 4)
    {
        auto moves = wisdom::perft::to_move_list (board, current_player, argv[3]);
        current_player = wisdom::perft::apply_list (board, current_player, moves);
    }

    PerftResults results = wisdom::perft::perft_results (board, current_player, depth, move_generator);
    std::cout << wisdom::perft::to_string (results);

    return EXIT_SUCCESS;
}
