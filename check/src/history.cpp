#include "history.hpp"
#include "board.h"

bool history::is_fifty_move_repetition (board &board, Color who) const
{
    if (my_board_history.total_full_moves () < 50)
        return false;
    // todo
    return false;
}

bool history::is_third_repetition (move_t move, Color who) const
{
    return false;
}

void history::add_position_and_move (board &board, move_t move)
{
    this->my_board_history.add_board_code (board.code);
    this->my_move_history.push_back (move);
}

void history::remove_position_and_last_move (board &board)
{
    this->my_board_history.remove_board_code (board.code);
    this->my_move_history.pop_back ();
}