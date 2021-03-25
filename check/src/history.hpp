#ifndef WIZDUMB_HISTORY_HPP
#define WIZDUMB_HISTORY_HPP

#include "board_history.hpp"
#include "move_history.hpp"
#include "piece.h"
#include "board.h"

namespace wisdom
{
    class History
    {
    private:
        BoardHistory my_board_history;
        MoveHistory my_move_history;

    public:
        static bool is_fifty_move_repetition (Board &board)
        {
            return board.half_move_clock >= 100;
        }

        bool is_third_repetition (Board &board) const
        {
            auto count = my_board_history.position_count (board.code);
            return (count == 3);
        }

        void add_position_and_move (Board &board, Move move)
        {
            this->my_board_history.add_board_code (board.code);
            my_move_history.push_back (move);
        }

        void remove_position_and_last_move (Board &board)
        {
            this->my_board_history.remove_board_code (board.code);
            my_move_history.pop_back ();
        }

        BoardHistory &get_board_history ()
        {
            return my_board_history;
        }

        MoveHistory &get_move_history ()
        {
            return my_move_history;
        }
    };
}
#endif //WIZDUMB_HISTORY_HPP
