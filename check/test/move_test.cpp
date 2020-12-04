#include "catch.hpp"

#include "board.h"
#include "move.h"
#include "generate.h"

TEST_CASE( "Moving and undoing a move works", "[move]" )
{
    struct board board;

    move_t e2e4 = move_parse ("e2e4", COLOR_WHITE);
    move_t d6d8 = move_parse ("d7d5", COLOR_BLACK);
    move_t b1c3 = move_parse ("b1c3", COLOR_WHITE);
    move_t d5xe4 = move_parse ("d5xe4", COLOR_BLACK);
    move_t c3xe4 = move_parse ("c3xe4", COLOR_WHITE);

    undo_move_t undo_state;

    undo_state = do_move (board, COLOR_WHITE, e2e4);
    undo_move (board, COLOR_WHITE, e2e4, undo_state);
    do_move (board, COLOR_WHITE, e2e4);

    undo_state = do_move (board, COLOR_BLACK, d6d8);
    undo_move (board, COLOR_BLACK, d6d8, undo_state);
    do_move (board, COLOR_BLACK, d6d8);

    undo_state = do_move (board, COLOR_WHITE, b1c3);
    undo_move (board, COLOR_WHITE, b1c3, undo_state);
    do_move (board, COLOR_WHITE, b1c3);

    undo_state = do_move (board, COLOR_BLACK, d5xe4);
    undo_move (board, COLOR_BLACK, d5xe4, undo_state);
    do_move (board, COLOR_BLACK, d5xe4);

    undo_state = do_move (board, COLOR_WHITE, c3xe4);
    undo_move (board, COLOR_WHITE, c3xe4, undo_state);
    do_move (board, COLOR_WHITE, c3xe4);
}


