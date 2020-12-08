
#ifndef WIZDUMB_HISTORY_HPP
#define WIZDUMB_HISTORY_HPP

#include "board_history.hpp"
#include "move_history.hpp"
#include "piece.h"
#include "board.h"

class history
{
private:
    class board_history my_board_history;
    struct move_history_t my_move_history;

public:
    bool is_fifty_move_repetition (board &board, Color who) const;
    bool is_third_repetition (move_t move, Color who) const;

    void add_position_and_move (board &board, move_t move)
    {
        this->my_board_history.add_board_code (board.code);
        this->my_move_history.push_back (move);
    }

    void remove_position_and_last_move (board &board)
    {
        this->my_board_history.remove_board_code (board.code);
        this->my_move_history.pop_back ();
    }

    board_history &get_board_history()
    {
        return my_board_history;
    }

    move_history_t &get_move_history()
    {
        return my_move_history;
    }
};

#endif //WIZDUMB_HISTORY_HPP
