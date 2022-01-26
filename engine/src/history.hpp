#ifndef WISDOM_HISTORY_HPP
#define WISDOM_HISTORY_HPP

#include "global.hpp"
#include "board_history.hpp"
#include "move_list.hpp"
#include "piece.hpp"
#include "board.hpp"

namespace wisdom
{
    class History
    {
    private:
        BoardHistory my_board_history;
        MoveList my_move_history;

    public:
        virtual ~History() = default;

        static bool is_fifty_move_repetition (const Board& board)
        {
            return board.get_half_move_clock () >= 100;
        }

        [[nodiscard]] bool is_third_repetition (const Board& board) const
        {
            auto count = my_board_history.position_count (board.get_code ());
            return (count == 3);
        }

        void add_position_and_move (Board& board, Move move)
        {
            this->my_board_history.add_board_code (board.get_code ());
            my_move_history.push_back (move);
        }

        void remove_position_and_last_move (Board& board)
        {
            this->my_board_history.remove_board_code (board.get_code ());
            my_move_history.pop_back ();
        }

        [[nodiscard]] auto get_board_history () const& -> const BoardHistory&
        {
            return my_board_history;
        }
        void get_board_history () const&& = delete;

        [[nodiscard]] auto get_move_history () const& -> const MoveList&
        {
            return my_move_history;
        }
        void get_move_history () const&& = delete;
    };
}

#endif //WISDOM_HISTORY_HPP
