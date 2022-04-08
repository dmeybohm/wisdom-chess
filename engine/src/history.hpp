#ifndef WISDOM_CHESS_HISTORY_HPP
#define WISDOM_CHESS_HISTORY_HPP

#include "global.hpp"
#include "board_history.hpp"
#include "move_list.hpp"
#include "piece.hpp"
#include "board.hpp"
#include "move.hpp"

namespace wisdom
{
    class History
    {
    private:
        BoardHistory my_board_history;
        MoveList my_move_history;

        // Board codes and undo positions sorted by move number:
        vector<BoardCode> my_board_codes;
        vector<UndoMove> my_undo_moves;

    public:
        History()
        {
            my_board_codes.reserve (100);
            my_undo_moves.reserve (100);
        }

        static bool is_fifty_move_repetition (const Board& board)
        {
            return board.get_half_move_clock () >= 100;
        }

        [[nodiscard]] bool is_third_repetition (const Board& board) const
        {
            auto& find_code = board.get_code ();
            return std::count_if (my_board_codes.begin (), my_board_codes.end (),
                    [find_code](const BoardCode& code){
                return (code == find_code);
            }) >= 3;
        }

        void add_position_and_move (const Board& board, Move move, const UndoMove& undo_state)
        {
            my_board_codes.push_back (board.get_code ());
            my_undo_moves.push_back (undo_state);
            my_move_history.push_back (move);
        }

        void remove_position_and_last_move (const Board& board)
        {
            my_board_codes.pop_back ();
            my_undo_moves.pop_back ();
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

#endif //WISDOM_CHESS_HISTORY_HPP
