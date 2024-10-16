#include <iostream>

#include "wisdom-chess/engine/fen_parser.hpp"
#include "wisdom-chess/engine/board.hpp"
#include "wisdom-chess/engine/str.hpp"
#include "wisdom-chess/engine/move_list.hpp"
#include "wisdom-chess/engine/evaluate.hpp"
#include "wisdom-chess/engine/move_list.hpp"

#include "wisdom-chess-perft.hpp"

using wisdom::Board;
using wisdom::MoveList;
using wisdom::Color;
using wisdom::perft::Stats;
using wisdom::perft::PerftResults;

int main (int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        std::cerr << "Need two or three args" << "\n";
        return EXIT_FAILURE;
    }

    auto depth = wisdom::toInt (argv[1]);
    if (!depth.has_value())
    {
        std::cerr << "Invalid depth: " << argv[1] << "\n";
        return EXIT_FAILURE;
    }

    wisdom::FenParser fen { argv[2] };
    auto board = fen.buildBoard() ;
    auto current_player = fen.getActivePlayer();

    if (argc == 4)
    {
        auto moves = wisdom::perft::toMoveList (board, current_player, argv[3]);
        current_player = wisdom::perft::applyList (board, current_player, moves);
    }

    PerftResults results = wisdom::perft::perftResults (board, current_player, *depth);
    std::cout << wisdom::perft::asString (results);

    return EXIT_SUCCESS;
}
