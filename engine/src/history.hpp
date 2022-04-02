#ifndef WISDOM_HISTORY_HPP
#define WISDOM_HISTORY_HPP

#include "global.hpp"
#include "board_history.hpp"
#include "move_list.hpp"
#include "piece.hpp"
#include "board.hpp"

namespace wisdom
{
    struct HistoricalBoard
    {
        int count;
        ColoredPiece squares[Num_Rows][Num_Columns];
    };

    class History
    {
    private:
        BoardHistory my_board_history;
        MoveList my_move_history;
        array<HistoricalBoard, 500> my_historical_boards;

    public:
        static bool is_fifty_move_repetition (const Board& board)
        {
            return board.get_half_move_clock () >= 100;
        }

        [[nodiscard]] int find_board_pos (const Board& board) const
        {
            auto size = my_move_history.size ();
            for (int i = 0; i < size; i++)
            {
                const auto& historical_board = my_historical_boards[i];

                if (!memcmp (historical_board.squares, board.squares_ptr (), sizeof (historical_board.squares)))
                    return i;
            }
            return -1;
        }

        [[nodiscard]] bool is_third_repetition (const Board& board) const
        {
            // todo: implement this efficiently.
            return false;

            auto pos = find_board_pos (board);
            if (pos >= 0)
                return my_historical_boards[pos].count >= 3;
            return false;
//            auto count = my_board_history.position_count (board.get_code ());
//            return (count >= 3);
        }


        void add_position_and_move (const Board& board, Move move)
        {
            my_move_history.push_back (move);
            // TODO handle when there's an overlap
            return;

            auto index = my_move_history.size ();
            auto pos = find_board_pos (board);
            if (pos >= 0)
            {
                my_historical_boards[pos].count += 1;
            }
            else
            {
                board.copy_squares (my_historical_boards[index].squares);
                my_historical_boards[index].count = 1;
            }
//            this->my_board_history.add_board_code (board.get_code ());
        }

        void remove_position_and_last_move (const Board& board)
        {
//            this->my_board_history.remove_board_code (board.get_code ());
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
