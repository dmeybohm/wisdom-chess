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

